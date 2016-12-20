//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Sebastien Grange
    \version   3.2.0 $Rev: 2166 $
*/

//==============================================================================


#include <algorithm>
#include <iostream>
using namespace std;

#include "v_repExtCHAI3D.h"
#include "luaFunctionData.h"
#include "v_repLib.h"

#include "chai3d.h"
using namespace chai3d;

#ifdef _WIN32
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#define PLUGIN_VERSION 1



/** \addtogroup V-REP
 *  @{
 */



//---------------------------------------------------------------------------
//
//  globals
//
//---------------------------------------------------------------------------



// forward declarations
class  Object;
struct Device;

///  \brief Haptic thread control flag.
bool SimulationRunning;

///  \brief Haptic thread handle.
cThread *HapticThread;

///  \brief Haptic thread stop synchronization lock.
cMutex *SimulationLock;

///  \brief Haptic \ref scene "scene" synchronization lock.
cMutex *SceneLock;

///  \brief Haptic \ref Object "objects" synchronization lock.
cMutex *ObjectLock;

///  \brief Haptic \ref Object "objects" list.
vector<Object*> Objects;

///  \brief CHAI3D haptic device handler.
cHapticDeviceHandler *DeviceHandler;

///  \brief Haptic \ref scene "scene" container.
cWorld *World;

///  \brief CHAI3D haptic device list.
map<int,cGenericHapticDevicePtr> Devices;

///  \brief Haptic \ref Device "device" list.
map<int,Device*> Cursors;

cPrecisionClock Clock;
double BiggestRadius   =  0.0;
double LowestStiffness = -1.0;



//---------------------------------------------------------------------------
//
//  objects
//
//---------------------------------------------------------------------------



///  \brief Types of haptic objects.
///
///  Each \ref Object in the \ref scene "scene" must be assigned one of the following types.

enum ObjectType
{
  ///  \brief Arbitrary 3D mesh object.
  SHAPE,

  ///  \brief Constraint that holds the haptic device at a given point.
  ///  The point is defined by \ref State::Pos.
  CONSTRAINT_POINT,

  ///  \brief Constraint that holds the haptic device on a given segment.
  ///  The segment is defined by a point (\ref State::Pos) and a vector (\ref State::Dir).
  CONSTRAINT_SEGMENT,

  ///  \brief Constraint that holds the haptic device on a given plane.
  ///  The plane is defined by a point (\ref State::Pos) and a normal direction (\ref State::Dir).
  CONSTRAINT_PLANE
};



///  \brief Data structure that contains an object state.

struct State
{
  ///  \brief Constructor.
  State()
  {
    Pos.set(0.0, 0.0, 0.0);
    Rot.identity();
    Dir.set(0.0, 0.0, 0.0);
    Kp = Kv = Fmax = 0.0;
  }

  ///  \brief \ref SHAPE and constraints position.
  cVector3d Pos;

  ///  \brief \ref SHAPE orientation matrix.
  cMatrix3d Rot;

  ///  \brief Constraint direction.
  ///  \note
  ///  For \ref CONSTRAINT_POINT, this member is ignored.<br>
  ///  For \ref CONSTRAINT_SEGMENT, this member describes the segment length and direction from the \ref Pos member.<br>
  ///  For \ref CONSTRAINT_PLANE, this member describes the normal direction to the plane.
  cVector3d Dir;

  ///  \brief Constraints stiffness factor, expressed as a factor (0.0 < Kp < 1.0) of the maximum allowed stiffness (which depends on the type of haptic devices connected to the scene).
  double Kp;

  ///  \brief Constraints viscosity factor, expressed as a factor (0.0 < Kv < 1.0) of the maximum allowed damping (which depends on the type of haptic devices connected to the scene).
  double Kv;

  ///  \brief Constraints maximum force generated by the \ref Kp member, expressed in N.
  double Fmax;
};



///  \brief Data structure that contains a haptic device handle and related state information.

struct Device
{
  ///  \brief Constructor.
  Device(cWorld *world)
  {
    Tool = new cToolCursor(world);
    Pos.set(0.0, 0.0, 0.0);
    Vel.set(0.0, 0.0, 0.0);
    Force.set(0.0, 0.0, 0.0);
    MaxDamping = 0.0;
    MaxStiffness = 0.0;
  }

  ///  \brief Destructor.
  virtual ~Device()
  {
    delete Tool;
  }

  ///  \brief Haptic device handle.
  cToolCursor *Tool;

  ///  \brief Haptic device position.
  cVector3d Pos;

  ///  \brief Haptic device velocity.
  cVector3d Vel;

  ///  \brief Additional force to apply to haptic device.
  cVector3d Force;

  ///  \brief Maximum stiffness allowed for this device.
  double MaxStiffness;

  ///  \brief Maximum damping allowed for this device.
  double MaxDamping;
};



///  \brief Data structure that contains an object of type \ref ObjectType used to generate haptic feedback.

class Object
{
private:

  ///  \brief Initial object timestamp.
  double t0;

  ///  \brief Initial object state.
  State  S0;

  ///  \brief Target state timestamp.
  double tT;

  ///  \brief Target object state.
  State  ST;


  ///  \brief Set new target state.
  ///  This method is shared by \ref SetTarget(float pos[3], float rot[3], float stiffnessFactor)
  ///  and \ref SetTarget(float pos[3], float dir[3], float Kp, float Kv, float Fmax).

  void SetTarget()
  {
    // update current position and timestep
    double t = Clock.getCPUTimeSeconds();
    double timestep = t - t0;
    Update(t);

    // set starting point as current position
    S0 = S;
    t0 = t;

    // store new target
    tT = t + timestep;
  }


public:

  ///  \brief Object type.
  ObjectType Type;

  ///  \brief Internal handle to a \ref Device assigned to this object.
  ///  \note This member can be negative if the object is not assigned to any particular device (e.g. \ref SHAPE).
  ///        All \ref constraints "constraint" objects are assigned to a device.
  int Device;

  ///  \brief Object mesh that this object renders haptically in the scene.
  cMesh *Mesh;

  ///  \brief Current object state.
  State S;


  ///  \brief \ref SHAPE constructor.
  ///
  ///  This member creates a new \ref SHAPE and adds it to the \ref scene "scene".
  ///
  ///  \param pos               \ref SHAPE position in the scene, expressed in m.
  ///  \param rot               \ref SHAPE orientation in the scene, expressed as ZYX extrinsic Euler angles.
  ///  \param vertices          list of vertices that define the \ref Mesh member.
  ///  \param indices           list of vertex indices that define the \ref Mesh triangles.
  ///  \param stiffnessFactor   object stiffness factor (must be between 0.0 and 1.0).

  Object(float pos[3], float rot[3], vector<float>vertices, vector<int> indices, float stiffnessFactor)
  {
    Type   = SHAPE;
    Device = -1;
    Mesh   = new cMesh();

    for (int i=0; i<int(indices.size())/3; i++)
    {
      // build mesh
      int ind[3] = { indices[3*i+0], indices[3*i+1], indices[3*i+2] };
      cVector3d v1 = cVector3d(vertices[3*ind[0]+0], vertices[3*ind[0]+1], vertices[3*ind[0]+2]);
      cVector3d v2 = cVector3d(vertices[3*ind[1]+0], vertices[3*ind[1]+1], vertices[3*ind[1]+2]);
      cVector3d v3 = cVector3d(vertices[3*ind[2]+0], vertices[3*ind[2]+1], vertices[3*ind[2]+2]);
      Mesh->newTriangle(v1, v2, v3);
    }

    // set default material properties
    Mesh->m_material->setDynamicFriction(0.0);
    Mesh->m_material->setStaticFriction(0.0);
    Mesh->m_material->setHapticTriangleSides(true, false);

    // compute mesh properties
    Mesh->computeAllNormals();
    Mesh->computeBoundaryBox(true);
    Mesh->createAABBCollisionDetector(BiggestRadius);

    // set origin parameters
    t0 = Clock.getCPUTimeSeconds();
    S0.Kp = stiffnessFactor;
    S0.Pos.set(pos[0], pos[1], pos[2]);
    S0.Rot.setExtrinsicEulerRotationRad(rot[0], rot[1], rot[2], C_EULER_ORDER_ZYX);

    // set target at origin for now
    tT = t0;
    ST = S0;

    // update current position
    S = S0;
  }


  ///  \brief \ref constraints "Constraints" constructor.
  ///
  ///  This member creates a new \ref constraints "constraint" and adds it to the \ref scene "scene".
  ///
  ///  \param constraintType    type of constraint to create.
  ///  \param device            device handle (returned by \ref hapticConnect()) to assign this constraint to.
  ///  \param pos               constraint position in the scene, expressed in m.
  ///  \param dir               constraint direction, as defined in \ref ObjectType.
  ///  \param Kp                constraint stiffness factor (between 0.0 and 1.0).
  ///  \param Kv                constraint viscosity factor (between 0.0 and 1.0).
  ///  \param Fmax              constraint max force for Kp, expressed in N.

  Object(ObjectType constraintType, int device, float pos[3], float dir[3], float Kp, float Kv, float Fmax)
  {
    Type   = constraintType;
    Device = device;
    Mesh   = NULL;

    // set origin parameters
    t0 = Clock.getCPUTimeSeconds();
    S0.Kp   = Kp;
    S0.Kv   = Kv;
    S0.Fmax = Fmax;
    S0.Pos.set(pos[0], pos[1], pos[2]);
    if (dir)
    {
      S0.Dir.set(dir[0], dir[1], dir[2]);
    }
    else
    {
      S0.Dir.set(1.0, 1.0, 1.0);
    }

    // set target at origin for now
    tT = t0;
    ST = S0;

    // update current position
    S = S0;
  }


  ///  \brief Destructor.

  virtual ~Object()
  {
    if (Mesh)
    {
      delete Mesh;
    }
  }


  ///  \brief Set new \ref SHAPE target state.
  ///
  ///  \param pos               new target \ref SHAPE target position, expressed in m.
  ///  \param rot               new target \ref SHAPE orientation in the scene, expressed as ZYX extrinsic Euler angles.
  ///  \param stiffnessFactor   new target object stiffness factor (between 0.0 and 1.0).

  void SetTarget(float pos[3], float rot[3], float stiffnessFactor)
  {
    // update current position and timestep
    SetTarget();

    // store new target
    ST.Pos.set(pos[0], pos[1], pos[2]);
    ST.Rot.setExtrinsicEulerRotationRad(rot[0], rot[1], rot[2], C_EULER_ORDER_ZYX);
    ST.Kp = stiffnessFactor;
  }


  ///  \brief Set new \ref constraints "constraint" target state.
  ///
  ///  \param pos   new constraint position in the scene, expressed in m.
  ///  \param dir   new constraint direction, as defined in \ref ObjectType.
  ///  \param Kp    new constraint stiffness factor (between 0.0 and 1.0).
  ///  \param Kv    new constraint viscosity factor (between 0.0 and 1.0).
  ///  \param Fmax  new constraint max force for Kp, expressed in N.

  void SetTarget(float pos[3], float dir[3], float Kp, float Kv, float Fmax)
  {
    // update current position and timestep
    SetTarget();

    // store new target
    ST.Pos.set(pos[0], pos[1], pos[2]);
    ST.Dir.set(dir[0], dir[1], dir[2]);
    ST.Kp   = Kp;
    ST.Kv   = Kv;
    ST.Fmax = Fmax;
  }


  ///  \brief Interpolate all object parameters to a given timestamp.
  ///
  ///  The state at the timestamp is linearly interpolated between the initial state (\ref Object::S0) and
  ///  the current target state (\ref Object::ST), and is available after computation in \ref Object::S.
  ///
  ///  \param t   timestamp, expressed in s.

  void Update(double t)
  {
    // safeguards
    if (tT <= t0 || t  <  t0)
    {
      S = S0;
    }
    else if (t  >  tT)
    {
      S = ST;
    }

    // interpolation case
    else
    {
      double ratio = (t-t0) / (tT-t0);

      // interpolate parameters
      S.Kp   = S0.Kp   + ratio * (ST.Kp   - S0.Kp);
      S.Kv   = S0.Kv   + ratio * (ST.Kv   - S0.Kv);
      S.Fmax = S0.Fmax + ratio * (ST.Fmax - S0.Fmax);

      // interpolate position
      S.Pos = S0.Pos + ratio * (ST.Pos - S0.Pos);

      // interpolate orientation based on object type
      switch (Type)
      {
      case SHAPE:
      {
        cVector3d axis;
        double angle;
        cMatrix3d s0t;
        S0.Rot.transr(s0t);
        (s0t * ST.Rot).toAxisAngle (axis, angle);
        cMatrix3d rot;
        rot.setAxisAngleRotationRad(axis, angle*ratio);
        S0.Rot.mulr(rot, S.Rot);
      }
      break;
      default:
        S.Dir = S0.Dir + ratio * (ST.Dir - S0.Dir);
        break;
      }
    }

    // apply to mesh if any
    if (Mesh)
    {
      Mesh->setLocalPos(S.Pos);
      Mesh->setLocalRot(S.Rot);
      Mesh->m_material->setStiffness(S.Kp * LowestStiffness);
    }
  }


  ///  \brief Compute additional force contribution for a given \ref Device position and velocity.
  ///
  ///  This additional force provides the constraint defined by the relevant \ref ObjectType. The
  ///  force is computed using the \ref Device handle and is added to the \ref Device::Force member.

  void ComputeForce()
  {
    cVector3d point;
    cVector3d force(0.0, 0.0, 0.0);

    // safeguard
    if (Device < 0 || Type == SHAPE || S.Fmax == 0.0)
    {
      return;
    }

    // compute interaction point based on constraint type
    switch (Type)
    {
    case CONSTRAINT_PLANE:
      point = cProjectPointOnPlane(Cursors[Device]->Pos, S.Pos, S.Dir);
      break;
    case CONSTRAINT_SEGMENT:
      point = cProjectPointOnSegment(Cursors[Device]->Pos, S.Pos, S.Pos+S.Dir);
      break;
    case CONSTRAINT_POINT:
    default:
      point = S.Pos;
      break;
    }

    // compute constraint force at interaction point
    force -= S.Kp * Cursors[Device]->MaxStiffness * (Cursors[Device]->Pos - point);

    // limit to force max
    double norm = force.length();
    if (norm > S.Fmax) force *= S.Fmax / norm;

    // add viscosity (only along constraints)
    force -= cProject(S.Kv * Cursors[Device]->MaxDamping * Cursors[Device]->Vel, force);

    // add force to device
    Cursors[Device]->Force.add(force);
  }

};



//---------------------------------------------------------------------------
//
//  object management
//
//---------------------------------------------------------------------------



///  \brief Add generic object to the \ref scene "scene".
///
///  \param object  Haptic \ref Object to add to the \ref scene "scene".
///
///  \return
///  A unique identifier for the object if successful,
///  -1 otherwise.

int addObject(Object *object)
{
  if (!object)
  {
    return -1;
  }

  // we cannot add/remove objects while the simulation needs them
  SceneLock->acquire();

  // add to world if necessary
  if (object->Mesh)
  {
    World->addChild(object->Mesh);
  }

  // identify free slot in object list
  int size  = (int)Objects.size();
  int index = -1;
  for (int i=0; i<size; i++)
  {
    if (Objects[i] == NULL)
    {
      Objects[i] = object;
      index = i;
      break;
    }
  }

  // otherwise, push at the back of the list
  if (index < 0)
  {
    Objects.push_back(object);
    index = (int)Objects.size()-1;
  }

  // simulation may proceed
  SceneLock->release();

  return index;
}



///  \brief Update the parameters of a \ref SHAPE haptic \ref Object "object".
///
///  \param objectID          object identifier (returned by \ref addObject).
///  \param pos               new target \ref SHAPE target position, expressed in m.
///  \param rot               new target \ref SHAPE orientation in the scene, expressed as ZYX extrinsic Euler angles.
///  \param stiffnessFactor   new target object stiffness factor (between 0.0 and 1.0).
///
///  \return
///  \b true on success, \b false otherwise.

bool updateShape(int objectID, float pos[3], float rot[3], float stiffnessFactor)
{
  // safeguards
  if (objectID < 0 || objectID >= (int)Objects.size())        return false;
  if (!Objects[objectID] || Objects[objectID]->Type != SHAPE) return false;

  // safely update shape target position between simulation steps
  SceneLock->acquire();
  Objects[objectID]->SetTarget(pos, rot, stiffnessFactor);
  SceneLock->release();

  return true;
}



///  \brief Update the parameters of a \ref constraints "constraint" haptic \ref Object "object".
///
///  \param objectID  object identifier (returned by \ref addObject).
///  \param pos       new constraint position in the scene, expressed in m.
///  \param dir       new constraint direction, as defined in \ref ObjectType.
///  \param Kp        new constraint stiffness factor (between 0.0 and 1.0).
///  \param Kv        new constraint viscosity factor (between 0.0 and 1.0).
///  \param Fmax      new constraint max force for Kp, expressed in N.
///
///  \return
///  \b true on success, \b false otherwise.

bool updateConstraint(int objectID, float pos[3], float dir[3], float Kp, float Kv, float Fmax)
{
  // safeguards
  if (objectID < 0 || objectID >= (int)Objects.size())        return false;
  if (!Objects[objectID] || Objects[objectID]->Type == SHAPE) return false;

  // safely update constraint target position between simulation steps
  SceneLock->acquire();
  Objects[objectID]->SetTarget(pos, dir, Kp, Kv, Fmax);
  SceneLock->release();

  return true;
}



///  \brief Safely remove an \ref Object "object" from the \ref Objects "object list".
///
///  \param objectID    object identifier (returned by \ref addObject).
///
///  \return
///  \b true on success, \b false otherwise.

bool removeObject(int objectID)
{
  // safeguards
  if (objectID < 0 || objectID >= (int)Objects.size()) return false;
  if (!Objects[objectID])                              return false;

  // remove object between simulation steps
  SceneLock->acquire();

  if (Objects[objectID]->Mesh)
  {
    World->removeChild (Objects[objectID]->Mesh);
  }
  delete Objects[objectID];
  Objects[objectID] = NULL;

  SceneLock->release();

  return true;
}



///  \brief Safely remove an \ref Object "object" from the \ref Objects "object list".
///
///  \param object  pointer to \ref Object "object" to remove.
///
///  \return
///  \b true on success, \b false otherwise.

bool removeObject(Object *object)
{
  int size = (int)Objects.size();

  // identify the index of the object to remove
  for (int i=0; i<size; i++)
  {
    if (object == Objects[i])
    {
      // remove object
      return removeObject(i);
    }
  }

  return false;
}



//---------------------------------------------------------------------------
//
//  haptic loop
//
//---------------------------------------------------------------------------



///  \brief Main haptic loop.
///
///  The haptic loop runs in the background (with high priority) and renders all
///  \ref Objects haptically to all \ref Devices "devices" in the \ref scene "scene". Use
///  \ref hapticStart() to start the haptic loop, and \ref hapticReset() to stop it.
///
///  \note
///  The hapticLoop is started and stopped automatically by \ref hapticConnect() and
///  \ref hapticDisconnect(), according to the number of devices connected.

void hapticLoop()
{
  // announce that we are running
  SimulationRunning = true;
  SimulationLock->acquire();

  // loop
  while(SimulationRunning)
  {
    // update each cursor
    for (const auto& cursor : Cursors)
    {
      // get device data
      cursor.second->Tool->updateFromDevice();

      // update cursor position and velocity
      cursor.second->Pos = cursor.second->Tool->getDeviceGlobalPos();
      cursor.second->Vel = cursor.second->Tool->getDeviceGlobalLinVel();

      // reset constraints force for that cursor
      cursor.second->Force.set(0.0, 0.0, 0.0);
    }

    // prevent modifications to the haptic scene during updates
    SceneLock->acquire();

    // update scene and constraint forces
    double t = Clock.getCPUTimeSeconds();
    for (const auto& object : Objects)
    {
      if (object)
      {
        // update all objects positions and parameter values (interpolation)
        object->Update(t);

        // compute additional constraint forces
        object->ComputeForce();
      }
    }

    // update world
    World->computeGlobalPositions(true);

    // compute interaction forces
    for (const auto& cursor : Cursors) cursor.second->Tool->computeInteractionForces();

    // we can now safely modify the haptic scene without affecting the interaction
    SceneLock->release();

    // add constraint forces and apply to each device
    for (const auto& cursor : Cursors)
    {
      cursor.second->Tool->addDeviceGlobalForce(cursor.second->Force);
      cursor.second->Tool->applyToDevice();
    }

    // there is no point going on if no devices are connected
    if (Cursors.size() == 0) SimulationRunning = false;
  }

  // let hapticStop() know that we are done
  SimulationLock->release();
}



//---------------------------------------------------------------------------
//
//  haptic loop management
//
//---------------------------------------------------------------------------



///  \brief Start the haptic loop.

void hapticStart ()
{
  // start haptic thread
  if (!SimulationRunning)
  {
    HapticThread->start(hapticLoop, CTHREAD_PRIORITY_HAPTICS);
  }
}



///  \brief Stop the haptic loop.

void hapticStop ()
{
  if (SimulationRunning)
  {
    SimulationRunning = false;

    // wait for the thread to exit
    SimulationLock->acquire();
    SimulationLock->release();
  }
}



///  \\brief Connect to a haptic device and add it to the \ref scene "scene".
///
///  If successful, the device is added to the \ref Devices "device list" and
///  will be controlled by the \ref hapticLoop().
///
///  \param deviceIndex       index of the device to open in the list of devices detected by the operating system.
///                           This is guaranteed to be the same between executions as long as no hardware changes occur.
///  \param toolRadius        radius of the haptic device proxy (tool) in the simulated \ref scene "scene", expressed in m.
///  \param workspaceRadius   radius of the simulated workspace that the physical device workspace must be scaled to, expressed in m.
///
///  \return
///  \b true on success, \b false otherwise.

bool hapticConnect(int deviceIndex, float toolRadius, float workspaceRadius)
{
  SceneLock->acquire();

  // update list of available devices
  DeviceHandler->update();

  // create haptic device
  cGenericHapticDevicePtr device = cGenericHapticDevice::create();

  // get access to the first available haptic device found
  if (!DeviceHandler->getDevice(device, deviceIndex))
  {
    SceneLock->release();
    return false;
  }

  // create a tool (cursor)
  Device *cursor = new Device(World);

  // connect the haptic device to the virtual tool
  cursor->Tool->setHapticDevice(device);

  // insert cursor into the world
  World->addChild(cursor->Tool);

  // define a radius for the virtual tool (sphere)
  cursor->Tool->setRadius(toolRadius);

  // scale the haptic device workspace as desired
  cursor->Tool->setWorkspaceRadius(workspaceRadius);

  // objects in the scene are going to rotate of translate
  cursor->Tool->enableDynamicObjects(true);

  // start the haptic tool
  device->calibrate();
  cursor->Tool->setUseForceRise(true);
  cursor->Tool->setRiseTime(1.0);
  cursor->Tool->start();

  // store biggest tool radius
  if (toolRadius > BiggestRadius)
  {
    BiggestRadius = toolRadius;

    // update collision detection
    for (const auto& object : Objects)
    {
      if (object && object->Mesh)
      {
        object->Mesh->createAABBCollisionDetector(BiggestRadius);
      }
    }
  }

  // add cursor to cursor list
  Devices[deviceIndex] = device;
  Cursors[deviceIndex] = cursor;

  // retrieve max stiffness and damping
  cHapticDeviceInfo info = device->getSpecifications();
  cursor->MaxStiffness = info.m_maxLinearStiffness;
  cursor->MaxDamping   = info.m_maxLinearDamping;

  // determine world-wide max stiffness allowed
  double maxStiffness = cursor->MaxStiffness / cursor->Tool->getWorkspaceScaleFactor();
  if (maxStiffness < LowestStiffness || LowestStiffness < 0.0)
  {
    LowestStiffness = maxStiffness;

    // update all object stiffnesses
    for (const auto& object : Objects)
    {
      if (object)
      {
        object->Mesh->m_material->setStiffness(object->S.Kp * LowestStiffness);
      }
    }
  }

  SceneLock->release();

  // start haptic loop if necessary
  if (!SimulationRunning) hapticStart();

  return true;
}



///  \brief Disconnect from haptic device.
///
///  This will remove all objects associated with this particular device,
///  and will remove the device from the \ref scene "scene".
///
///  \param index   index of the device to disconnect. This should be the same as the \c index given to \ref hapticConnect().
///
///  \return
///  \b true on success, \b false otherwise.

bool hapticDisconnect(int index)
{
  // make sure the device exists
  if (Cursors.find(index) == Cursors.end())
  {
    return false;
  }

  // we cannot remove a device while the haptic thread is running
  if (SimulationRunning) hapticStop();

  // delete constraint objects associated with device
  for (const auto& object : Objects)
  {
    if (object && object->Device == index)
    {
      removeObject(object);
    }
  }

  // stop the haptic tool
  Cursors[index]->Tool->stop();

  // remove tool (cursor) from the world
  World->removeChild(Cursors[index]->Tool);

  // delete device and cursor
  delete Cursors[index];
  Devices[index].reset();
  Cursors.erase(index);
  Devices.erase(index);

  return true;
}



///  \brief Clean up the \ref scene "scene".
///
///  This function disconnects all devices and removes all objects from the \ref scene "scene".

void hapticReset ()
{
  // disconnect from all devices
  for (const auto& cursor : Cursors)
  {
    hapticDisconnect(cursor.first);
  }

  // delete all objects
  for (const auto& object : Objects)
  {
    removeObject(object);
  }
  Objects.clear();
}



//---------------------------------------------------------------------------
//
//  LUA interface
//
//---------------------------------------------------------------------------



LIBRARY vrepLib; // the V-REP library that we will dynamically load and bind

#define CONCAT(x,y,z)     x y z
#define strConCat(x,y,z)	CONCAT(x,y,z)



// definitions for LUA_START_COMMAND
#define LUA_START_COMMAND "simExtCHAI3D_start"
const int inArgs_START[] = {
    3,
    sim_lua_arg_int,   0,
    sim_lua_arg_float, 0,
    sim_lua_arg_float, 0,
};

///  \brief Connect to a haptic device.
///
///  This LUA callback is a wrapper for \ref hapticConnect().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref hapticConnect(int deviceIndex, float toolRadius, float workspaceRadius).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    number simExtCHAI3D_start(number deviceIndex, number toolRadius, number workspaceRadius)
///  \endcode
///
///  \return
///  1 on success, -1 otherwise.

void LUA_START_CALLBACK(SLuaCallBack* p)
{
  int commandResult = -1;

  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_START, inArgs_START[0], LUA_START_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();
    int   deviceIndex     = inData->at(0).intData[0];
    float toolRadius      = inData->at(1).floatData[0];
    float workspaceRadius = inData->at(2).floatData[0];

    if (hapticConnect(deviceIndex, toolRadius, workspaceRadius)) commandResult = 1;
    else simSetLastError(LUA_START_COMMAND, "Initialization failed.");
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem(commandResult));
  data.writeDataToLua(p);
}



// definitions for LUA_RESET_COMMAND
#define LUA_RESET_COMMAND "simExtCHAI3D_reset"
const int inArgs_RESET[] = {
    0
};

///  \brief Disconnect from a haptic device and reset the \ref scene "scene".
///
///  This LUA callback is a wrapper for \ref hapticReset().
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_reset()
///  \endcode

void LUA_RESET_CALLBACK(SLuaCallBack* p)
{
  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_RESET, inArgs_RESET[0], LUA_RESET_COMMAND))
  {
    hapticReset();
  }

  // populate reply
  p->outputArgCount = 0;
  data.writeDataToLua(p);
}



// definitions for LUA_ADD_SHAPE_COMMAND
#define LUA_ADD_SHAPE_COMMAND "simExtCHAI3D_addShape"
const int inArgs_ADD_SHAPE[] = {
    5,
    sim_lua_arg_float|sim_lua_arg_table, 9,
    sim_lua_arg_int|sim_lua_arg_table,   3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0
};

///  \brief Add a \ref SHAPE object to the \ref scene "scene".
///
///  This LUA callback is a wrapper for \ref addObject() and \ref Object::Object().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref Object::Object(float pos[3], float rot[3], vector<float>vertices, vector<int> indices, float stiffnessFactor).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_addShape(table_3 pos, table_3 rot, table vertices, table indices, number stiffnessFactor)
///  \endcode

void LUA_ADD_SHAPE_CALLBACK(SLuaCallBack* p)
{
  int objectID = -1;

  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_ADD_SHAPE, inArgs_ADD_SHAPE[0], LUA_ADD_SHAPE_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    vector<float> vertices(inData->at(0).floatData);
    vector<int>   indices (inData->at(1).intData);

    float pos[3];
    pos[0] = inData->at(2).floatData[0];
    pos[1] = inData->at(2).floatData[1];
    pos[2] = inData->at(2).floatData[2];

    float rot[3];
    rot[2] = inData->at(3).floatData[0];
    rot[1] = inData->at(3).floatData[1];
    rot[0] = inData->at(3).floatData[2];

    float stiffnessFactor = inData->at(4).floatData[0];

    // bound stiffness factor to a valid range
    if (stiffnessFactor > 1.0) stiffnessFactor = 1.0;
    if (stiffnessFactor < 0.0) stiffnessFactor = 0.0;

    objectID = addObject(new Object(pos, rot, vertices, indices, stiffnessFactor));
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem(objectID));
  data.writeDataToLua(p);
}



// definitions for LUA_ADD_CONSTRAINT_POINT_COMMAND
#define LUA_ADD_CONSTRAINT_POINT_COMMAND "simExtCHAI3D_addConstraintPoint"
const int inArgs_ADD_CONSTRAINT_POINT[] = {
    5,
    sim_lua_arg_int,                     0,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0
};

///  \brief Add a \ref CONSTRAINT_POINT object to the \ref scene "scene".
///
///  This LUA callback is a wrapper for \ref addObject() and \ref Object::Object().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref Object::Object(ObjectType constraintType, int device, float pos[3], float dir[3], float Kp, float Kv, float Fmax).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_addConstraintPoint(number deviceIndex, table_3 pos, number Kp, number Kv, number Fmax)
///  \endcode

void LUA_ADD_CONSTRAINT_POINT_CALLBACK(SLuaCallBack* p)
{
  int objectID = -1;

  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_ADD_CONSTRAINT_POINT, inArgs_ADD_CONSTRAINT_POINT[0], LUA_ADD_CONSTRAINT_POINT_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    float pos[3];
    pos[0] = inData->at(1).floatData[0];
    pos[1] = inData->at(1).floatData[1];
    pos[2] = inData->at(1).floatData[2];

    float dir[3];
    dir[0] = 0.0;
    dir[1] = 0.0;
    dir[2] = 1.0;

    float Kp   = inData->at(2).floatData[0];
    float Kv   = inData->at(3).floatData[0];
    float Fmax = inData->at(4).floatData[0];

    // bound stiffness factor to a valid range
    if (Kp > 1.0) Kp = 1.0;
    if (Kp < 0.0) Kp = 0.0;

    // bound damping factor to a valid range
    if (Kv > 1.0) Kv = 1.0;
    if (Kv < 0.0) Kv = 0.0;

    objectID = addObject(new Object(CONSTRAINT_POINT, deviceIndex, pos, dir, Kp, Kv, Fmax));
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem(objectID));
  data.writeDataToLua(p);
}



// definitions for LUA_ADD_CONSTRAINT_SEGMENT_COMMAND
#define LUA_ADD_CONSTRAINT_SEGMENT_COMMAND "simExtCHAI3D_addConstraintSegment"
const int inArgs_ADD_CONSTRAINT_SEGMENT[] = {
    6,
    sim_lua_arg_int,                     0,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0
};

///  \brief Add a \ref CONSTRAINT_SEGMENT object to the \ref scene "scene".
///
///  This LUA callback is a wrapper for \ref addObject() and \ref Object::Object().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref Object::Object(ObjectType constraintType, int device, float pos[3], float dir[3], float Kp, float Kv, float Fmax).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_addConstraintSegment(table_3 posA, table_3 posB, number Kp, number Kv, number Fmax)
///  \endcode

void LUA_ADD_CONSTRAINT_SEGMENT_CALLBACK(SLuaCallBack* p)
{
  int objectID = -1;

  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_ADD_CONSTRAINT_SEGMENT, inArgs_ADD_CONSTRAINT_SEGMENT[0], LUA_ADD_CONSTRAINT_SEGMENT_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    float pos[3];
    pos[0] = inData->at(1).floatData[0];
    pos[1] = inData->at(1).floatData[1];
    pos[2] = inData->at(1).floatData[2];

    float dir[3];
    dir[0] = inData->at(2).floatData[0];
    dir[1] = inData->at(2).floatData[1];
    dir[2] = inData->at(2).floatData[2];

    float Kp   = inData->at(3).floatData[0];
    float Kv   = inData->at(4).floatData[0];
    float Fmax = inData->at(5).floatData[0];

    // bound stiffness factor to a valid range
    if (Kp > 1.0) Kp = 1.0;
    if (Kp < 0.0) Kp = 0.0;

    // bound damping factor to a valid range
    if (Kv > 1.0) Kv = 1.0;
    if (Kv < 0.0) Kv = 0.0;

    objectID = addObject(new Object(CONSTRAINT_SEGMENT, deviceIndex, pos, dir, Kp, Kv, Fmax));
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem(objectID));
  data.writeDataToLua(p);
}



// definitions for LUA_ADD_CONSTRAINT_PLANE_COMMAND
#define LUA_ADD_CONSTRAINT_PLANE_COMMAND "simExtCHAI3D_addConstraintPlane"
const int inArgs_ADD_CONSTRAINT_PLANE[] = {
    6,
    sim_lua_arg_int,                     0,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0
};

///  \brief Add a \ref CONSTRAINT_PLANE object to the \ref scene "scene".
///
///  This LUA callback is a wrapper for \ref addObject() and \ref Object::Object().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref Object::Object(ObjectType constraintType, int device, float pos[3], float dir[3], float Kp, float Kv, float Fmax).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_addConstraintPlane(table_3 pos, table_3 normal, number Kp, number Kv, number Fmax)
///  \endcode

void LUA_ADD_CONSTRAINT_PLANE_CALLBACK(SLuaCallBack* p)
{
  int objectID = -1;

  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_ADD_CONSTRAINT_PLANE, inArgs_ADD_CONSTRAINT_PLANE[0], LUA_ADD_CONSTRAINT_PLANE_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    float pos[3];
    pos[0] = inData->at(1).floatData[0];
    pos[1] = inData->at(1).floatData[1];
    pos[2] = inData->at(1).floatData[2];

    float dir[3];
    dir[0] = inData->at(2).floatData[0];
    dir[1] = inData->at(2).floatData[1];
    dir[2] = inData->at(2).floatData[2];

    float Kp   = inData->at(3).floatData[0];
    float Kv   = inData->at(4).floatData[0];
    float Fmax = inData->at(5).floatData[0];

    // bound stiffness factor to a valid range
    if (Kp > 1.0) Kp = 1.0;
    if (Kp < 0.0) Kp = 0.0;

    // bound damping factor to a valid range
    if (Kv > 1.0) Kv = 1.0;
    if (Kv < 0.0) Kv = 0.0;

    objectID = addObject(new Object(CONSTRAINT_PLANE, deviceIndex, pos, dir, Kp, Kv, Fmax));
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem(objectID));
  data.writeDataToLua(p);
}



// definitions for LUA_UPDATE_SHAPE_COMMAND
#define LUA_UPDATE_SHAPE_COMMAND "simExtCHAI3D_updateShape"
const int inArgs_UPDATE_SHAPE[] = {
    4,
    sim_lua_arg_int,                     0,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0
};

///  \brief Update the parameters of a \ref SHAPE haptic \ref Object "object".
///
///  This LUA callback is a wrapper for \ref updateShape().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref updateShape(int objectID, float pos[3], float rot[3], float stiffnessFactor).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_updateShape(number objectID, table_3 pos, table_3 rot, number stiffnessFactor)
///  \endcode

void LUA_UPDATE_SHAPE_CALLBACK(SLuaCallBack* p)
{
  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_UPDATE_SHAPE, inArgs_UPDATE_SHAPE[0], LUA_UPDATE_SHAPE_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int objectID = inData->at(0).intData[0];

    float pos[3];
    pos[0] = inData->at(1).floatData[0];
    pos[1] = inData->at(1).floatData[1];
    pos[2] = inData->at(1).floatData[2];

    float rot[3];
    rot[2] = inData->at(2).floatData[0];
    rot[1] = inData->at(2).floatData[1];
    rot[0] = inData->at(2).floatData[2];

    float stiffnessFactor = inData->at(3).floatData[0];

    // bound stiffness factor to a valid range
    if (stiffnessFactor > 1.0) stiffnessFactor = 1.0;
    if (stiffnessFactor < 0.0) stiffnessFactor = 0.0;

    if (!updateShape(objectID, pos, rot, stiffnessFactor))
    {
      simSetLastError(LUA_UPDATE_SHAPE_COMMAND, "Invalid shape ID.");
    }
  }

  // populate reply
  p->outputArgCount = 0;
  data.writeDataToLua(p);
}



// definitions for LUA_UPDATE_CONSTRAINT_COMMAND
#define LUA_UPDATE_CONSTRAINT_COMMAND "simExtCHAI3D_updateConstraint"
const int inArgs_UPDATE_CONSTRAINT[] = {
    6,
    sim_lua_arg_int,                     0,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float|sim_lua_arg_table, 3,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0,
    sim_lua_arg_float,                   0
};

///  \brief Update the parameters of a \ref constraints "constraint" haptic \ref Object "object".
///
///  This LUA callback is a wrapper for \ref updateShape().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref updateConstraint(int objectID, float pos[3], float dir[3], float Kp, float Kv, float Fmax).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_updateConstraint(number objectID, table_3 pos, table_3 dir, number Kp, number Kv, number Fmax)
///  \endcode

void LUA_UPDATE_CONSTRAINT_CALLBACK(SLuaCallBack* p)
{
  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_UPDATE_CONSTRAINT, inArgs_UPDATE_CONSTRAINT[0], LUA_UPDATE_CONSTRAINT_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int objectID = inData->at(0).intData[0];

    float pos[3];
    pos[0] = inData->at(1).floatData[0];
    pos[1] = inData->at(1).floatData[1];
    pos[2] = inData->at(1).floatData[2];

    float dir[3];
    dir[0] = inData->at(2).floatData[0];
    dir[1] = inData->at(2).floatData[1];
    dir[2] = inData->at(2).floatData[2];

    float Kp   = inData->at(3).floatData[0];
    float Kv   = inData->at(4).floatData[0];
    float Fmax = inData->at(5).floatData[0];

    // bound stiffness factor to a valid range
    if (Kp > 1.0) Kp = 1.0;
    if (Kp < 0.0) Kp = 0.0;

    // bound damping factor to a valid range
    if (Kv > 1.0) Kv = 1.0;
    if (Kv < 0.0) Kv = 0.0;

    if (!updateConstraint(objectID, pos, dir, Kp, Kv, Fmax))
    {
      simSetLastError(LUA_UPDATE_CONSTRAINT_COMMAND, "Invalid shape ID.");
    }
  }

  // populate reply
  p->outputArgCount = 0;
  data.writeDataToLua(p);
}



// definitions for LUA_REMOVE_OBJECT_COMMAND
#define LUA_REMOVE_OBJECT_COMMAND "simExtCHAI3D_removeObject"
const int inArgs_REMOVE_OBJECT[] = {
    1,
    sim_lua_arg_int, 0
};

///  \brief Safely remove an \ref Object "object" from the \ref Objects "object list".
///
///  This LUA callback is a wrapper for \ref removeObject().<br>
///  The arguments parsed from the \c p buffer are passed on to \ref removeObject(int objectID).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    simExtCHAI3D_removeObject(number objectID)
///  \endcode

void LUA_REMOVE_OBJECT_CALLBACK(SLuaCallBack* p)
{
  CLuaFunctionData data;

  // validate argument count and types
  if (data.readDataFromLua(p, inArgs_REMOVE_OBJECT, inArgs_REMOVE_OBJECT[0], LUA_REMOVE_OBJECT_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int objectID = inData->at(0).intData[0];

    if (!removeObject(objectID))
    {
      simSetLastError(LUA_REMOVE_OBJECT_COMMAND, "Invalid shape ID.");
    }
  }

  // populate reply
  p->outputArgCount = 0;
  data.writeDataToLua(p);
}



// definitions for LUA_READ_POSITION_COMMAND
#define LUA_READ_POSITION_COMMAND "simExtCHAI3D_readPosition"
const int inArgs_READ_POSITION[] = {
    1,
    sim_lua_arg_int, 0
};

///  \brief Retrieve the current position of a haptic device (identified by its \c deviceIndex) in the \ref scene "scene".
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    table_3 simExtCHAI3D_readPosition(number deviceIndex)
///  \endcode
///
///  \return
///  An array containing the XYZ position Cartesian coordinates of the device end-effector in the simulated \ref scene "scene".

void LUA_READ_POSITION_CALLBACK(SLuaCallBack* p)
{
  std::vector<float> retPos(3, 0.0f);

  // validate argument count and types
  CLuaFunctionData data;
  if (data.readDataFromLua(p, inArgs_READ_POSITION, inArgs_READ_POSITION[0], LUA_READ_POSITION_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    if (Cursors.find(deviceIndex) != Cursors.end())
    {
        cVector3d pos = Cursors[deviceIndex]->Tool->getDeviceGlobalPos();
        retPos[0] = (float)pos.x();
        retPos[1] = (float)pos.y();
        retPos[2] = (float)pos.z();
    }
  }

  // populate reply
  p->outputArgCount = 0;
  if (retPos.size() >= 3)
  {
    data.pushOutData(CLuaFunctionDataItem(retPos));
  }
  data.writeDataToLua(p);
}



// definitions for LUA_READ_FORCE_COMMAND
#define LUA_READ_FORCE_COMMAND "simExtCHAI3D_readForce"
const int inArgs_READ_FORCE[] = {
    1,
    sim_lua_arg_int, 0
};

///  \brief Retrieve the force currently displayed by a haptic device (identified by its \c deviceIndex).
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    table_3 simExtCHAI3D_readForce(number deviceIndex)
///  \endcode
///
///  \return
///  An array containing the XYZ Cartesian force component at the device end-effector.

void LUA_READ_FORCE_CALLBACK(SLuaCallBack* p)
{
  std::vector<float> retForce(3, 0.0f);

  // validate argument count and types
  CLuaFunctionData data;
  if (data.readDataFromLua(p, inArgs_READ_FORCE, inArgs_READ_FORCE[0], LUA_READ_FORCE_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    if (Cursors.find(deviceIndex) != Cursors.end())
    {
        cVector3d force = Cursors[deviceIndex]->Tool->getDeviceGlobalForce();
        retForce[0] = (float)force.x();
        retForce[1] = (float)force.y();
        retForce[2] = (float)force.z();
    }
  }

  // populate reply
  p->outputArgCount = 0;
  if (retForce.size() >= 3)
  {
    data.pushOutData(CLuaFunctionDataItem(retForce));
  }
  data.writeDataToLua(p);
}



// definitions for LUA_READ_BUTTONS_COMMAND
#define LUA_READ_BUTTONS_COMMAND "simExtCHAI3D_readButtons"
const int inArgs_READ_BUTTONS[] = {
    1,
    sim_lua_arg_int, 0
};

///  \brief Retrieve a haptic device (identified by its \c deviceIndex) end-effector buttons status.
///
///  The buttons are returned in the form of an array of bit. A bit state of \c 1 indicates that the button
///  at the given bit array index is pressed, \c 0 means it is not.
///
///  \param p   SLuaCallBack I/O buffer.
///
///  The corresponding LUA function is:
///  \code
///    table_3 simExtCHAI3D_readButtons(number deviceIndex)
///  \endcode
///
///  \return
///  An array of bits containing the buttons state for the given device.

void LUA_READ_BUTTONS_CALLBACK(SLuaCallBack* p)
{
  unsigned int buttons = 0;

  // validate argument count and types
  CLuaFunctionData data;
  if (data.readDataFromLua(p, inArgs_READ_BUTTONS, inArgs_READ_BUTTONS[0], LUA_READ_BUTTONS_COMMAND))
  {
    vector<CLuaFunctionDataItem>* inData = data.getInDataPtr();

    int deviceIndex = inData->at(0).intData[0];

    if (Cursors.find(deviceIndex) != Cursors.end())
    {
        buttons = Cursors[deviceIndex]->Tool->getUserSwitches();
    }
  }

  // populate reply
  p->outputArgCount = 0;
  data.pushOutData(CLuaFunctionDataItem((int)buttons));
  data.writeDataToLua(p);
}



///  \brief V-REP shared library initialization.

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt)
{
  // setup plumbing
  HapticThread   = new cThread;
  SimulationLock = new cMutex;
  SceneLock      = new cMutex;
  ObjectLock     = new cMutex;
  DeviceHandler  = new cHapticDeviceHandler;
  World          = new cWorld;

  char curDirAndFile[1024];
#ifdef _WIN32
  GetModuleFileName(NULL,curDirAndFile,1023);
  PathRemoveFileSpec(curDirAndFile);
#elif defined (__linux) || defined (__APPLE__)
  if (getcwd(curDirAndFile, sizeof(curDirAndFile)) == NULL) strcpy(curDirAndFile, "");
#endif
  std::string currentDirAndPath(curDirAndFile);
  std::string temp(currentDirAndPath);
#ifdef _WIN32
  temp+="\\v_rep.dll";
#elif defined (__linux)
  temp+="/libv_rep.so";
#elif defined (__APPLE__)
  temp+="/libv_rep.dylib";
#endif
  vrepLib=loadVrepLibrary(temp.c_str());
  if (vrepLib==NULL)
  {
    std::cout << "Error, could not find or correctly load the V-REP library. Cannot start 'CHAI3D' plugin.\n";
    return(0);
  }
  if (getVrepProcAddresses(vrepLib)==0)
  {
    std::cout << "Error, could not find all required functions in the V-REP library. Cannot start 'CHAI3D' plugin.\n";
    unloadVrepLibrary(vrepLib);
    return(0);
  }

  int vrepVer;
  simGetIntegerParameter(sim_intparam_program_version,&vrepVer);
  if (vrepVer<30103)
  {
    std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start 'CHAI3D' plugin.\n";
    unloadVrepLibrary(vrepLib);
    return(0);
  }

  // register LUA commands

  std::vector<int> inArgs;

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_START,inArgs);
  simRegisterCustomLuaFunction(LUA_START_COMMAND,strConCat("number result=",LUA_START_COMMAND,"(number deviceIndex,number toolRadius,number workspaceRadius)"),&inArgs[0],LUA_START_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_RESET,inArgs);
  simRegisterCustomLuaFunction(LUA_RESET_COMMAND,strConCat("",LUA_RESET_COMMAND,"()"),&inArgs[0],LUA_RESET_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_ADD_SHAPE,inArgs);
  simRegisterCustomLuaFunction(LUA_ADD_SHAPE_COMMAND,strConCat("number objectID=",LUA_ADD_SHAPE_COMMAND,"(table vertices,table indices,table_3 position,table_3 orientation,number stiffnessFactor)"),&inArgs[0],LUA_ADD_SHAPE_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_ADD_CONSTRAINT_POINT,inArgs);
  simRegisterCustomLuaFunction(LUA_ADD_CONSTRAINT_POINT_COMMAND,strConCat("number objectID=",LUA_ADD_CONSTRAINT_POINT_COMMAND,"(number deviceIndex,table_3 position,number Kp,number Kv,number Fmax)"),&inArgs[0],LUA_ADD_CONSTRAINT_POINT_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_ADD_CONSTRAINT_SEGMENT,inArgs);
  simRegisterCustomLuaFunction(LUA_ADD_CONSTRAINT_SEGMENT_COMMAND,strConCat("number objectID=",LUA_ADD_CONSTRAINT_SEGMENT_COMMAND,"(number deviceIndex,table_3 point,table_3 segment,number Kp,number Kv,number Fmax)"),&inArgs[0],LUA_ADD_CONSTRAINT_SEGMENT_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_ADD_CONSTRAINT_PLANE,inArgs);
  simRegisterCustomLuaFunction(LUA_ADD_CONSTRAINT_PLANE_COMMAND,strConCat("number objectID=",LUA_ADD_CONSTRAINT_PLANE_COMMAND,"(number deviceIndex,table_3 position,table_3 normal,number Kp,number Kv,number Fmax)"),&inArgs[0],LUA_ADD_CONSTRAINT_PLANE_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_UPDATE_SHAPE,inArgs);
  simRegisterCustomLuaFunction(LUA_UPDATE_SHAPE_COMMAND,strConCat("",LUA_UPDATE_SHAPE_COMMAND,"(number objectID,table_3 position,table_3 orientation,number stiffnessFactor)"),&inArgs[0],LUA_UPDATE_SHAPE_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_UPDATE_CONSTRAINT,inArgs);
  simRegisterCustomLuaFunction(LUA_UPDATE_CONSTRAINT_COMMAND,strConCat("",LUA_UPDATE_CONSTRAINT_COMMAND,"(number objectID,table_3 positionA,table_3 positionB,number Kp,number Kv,number Fmax)"),&inArgs[0],LUA_UPDATE_CONSTRAINT_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_REMOVE_OBJECT,inArgs);
  simRegisterCustomLuaFunction(LUA_REMOVE_OBJECT_COMMAND,strConCat("",LUA_REMOVE_OBJECT_COMMAND,"(number objectID)"),&inArgs[0],LUA_REMOVE_OBJECT_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_READ_POSITION,inArgs);
  simRegisterCustomLuaFunction(LUA_READ_POSITION_COMMAND,strConCat("table_3 position=",LUA_READ_POSITION_COMMAND,"(number deviceIndex)"),&inArgs[0],LUA_READ_POSITION_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_READ_FORCE,inArgs);
  simRegisterCustomLuaFunction(LUA_READ_FORCE_COMMAND,strConCat("table_3 force=",LUA_READ_FORCE_COMMAND,"(number deviceIndex)"),&inArgs[0],LUA_READ_FORCE_CALLBACK);

  CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_READ_BUTTONS,inArgs);
  simRegisterCustomLuaFunction(LUA_READ_BUTTONS_COMMAND,strConCat("number buttons=",LUA_READ_BUTTONS_COMMAND,"(number deviceIndex)"),&inArgs[0],LUA_READ_BUTTONS_CALLBACK);

  return PLUGIN_VERSION;
}



///  \brief V-REP shared library disconnect.

VREP_DLLEXPORT void v_repEnd()
{
  // stop haptic thread and cleanup
  hapticReset();

  unloadVrepLibrary(vrepLib);
}



///  \brief V-REP shared library message processing callback.

VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{
  int   errorModeSaved;
  void *retVal = NULL;

  simGetIntegerParameter(sim_intparam_error_report_mode, &errorModeSaved);
  simSetIntegerParameter(sim_intparam_error_report_mode, sim_api_errormessage_ignore);
  simSetIntegerParameter(sim_intparam_error_report_mode, errorModeSaved);

  return retVal;
}



/** @}*/
