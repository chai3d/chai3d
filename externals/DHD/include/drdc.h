/****************************************************************************
 *
 *  DRD - Robotic SDK ver 3.17.5
 *  Copyright (C) 2001-2023 Force Dimension
 *  All Rights Reserved.
 *
 *  contact: support@forcedimension.com
 *
 ****************************************************************************/


/* C header */

#ifndef __DRDC_H__
#define __DRDC_H__

#include "dhdc.h"


#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 *  SDK
 ****************************************************************************/

  int    __SDK drdOpen                                        ();
  int    __SDK drdOpenID                                      (char ID);
  int    __SDK drdSetDevice                                   (char ID);
  int    __SDK drdGetDeviceID                                 ();
  int    __SDK drdClose                                       (char ID = -1);
  bool   __SDK drdIsSupported                                 (char ID = -1);
  bool   __SDK drdIsRunning                                   (char ID = -1);
  bool   __SDK drdIsMoving                                    (char ID = -1);
  bool   __SDK drdIsFiltering                                 (char ID = -1);
  double __SDK drdGetTime                                     ();
  void   __SDK drdSleep                                       (double sec);
  void   __SDK drdWaitForTick                                 (char ID = -1);
  bool   __SDK drdIsInitialized                               (char ID = -1);
  int    __SDK drdAutoInit                                    (char ID = -1);
  int    __SDK drdCheckInit                                   (char ID = -1);
  int    __SDK drdPrecisionInit                               (char ID = -1);
  int    __SDK drdGetPositionAndOrientation                   (double *px, double *py, double *pz, double *oa, double *ob, double *og, double *pg, double matrix[3][3], char ID = -1);
  int    __SDK drdGetVelocity                                 (double *vx, double *vy, double *vz, double *wx, double *wy, double *wz, double *vg, char ID = -1);
  double __SDK drdGetCtrlFreq                                 (char ID = -1);
  int    __SDK drdStart                                       (char ID = -1);
  int    __SDK drdRegulatePos                                 (bool on, char ID = -1);
  int    __SDK drdRegulateRot                                 (bool on, char ID = -1);
  int    __SDK drdRegulateGrip                                (bool on, char ID = -1);
  int    __SDK drdSetForceAndTorqueAndGripperForce            (double fx, double fy, double fz, double tx, double ty, double tz, double fg, char ID = -1);
  int    __SDK drdSetForceAndWristJointTorquesAndGripperForce (double fx, double fy, double fz, double t0, double t1, double t2, double fg, char ID = -1);
  int    __SDK drdEnableFilter                                (bool on, char ID = -1);
  int    __SDK drdMoveToPos                                   (double px, double py, double pz, bool block = true, char ID = -1);
  int    __SDK drdMoveToRot                                   (double oa, double ob, double og, bool block = true, char ID = -1);
  int    __SDK drdMoveToGrip                                  (double pg, bool block = true, char ID = -1);
  int    __SDK drdMoveTo                                      (double p[DHD_MAX_DOF], bool block = true, char ID = -1);
  int    __SDK drdMoveToEnc                                   (int enc0, int enc1, int enc2, bool block = true, char ID = -1);
  int    __SDK drdMoveToAllEnc                                (int enc[DHD_MAX_DOF], bool block = true, char ID = -1);
  int    __SDK drdTrackPos                                    (double px, double py, double pz, char ID = -1);
  int    __SDK drdTrackRot                                    (double oa, double ob, double og, char ID = -1);
  int    __SDK drdTrackGrip                                   (double pg, char ID = -1);
  int    __SDK drdTrack                                       (double p[DHD_MAX_DOF], char ID = -1);
  int    __SDK drdTrackEnc                                    (int enc0, int enc1, int enc2, char ID = -1);
  int    __SDK drdTrackAllEnc                                 (int enc[DHD_MAX_DOF], char ID = -1);
  int    __SDK drdHold                                        (char ID = -1);
  int    __SDK drdLock                                        (unsigned char mask, bool init, char ID = -1);
  int    __SDK drdStop                                        (bool frc = false, char ID = -1);
  int    __SDK drdGetPriorities                               (int *prio, int *ctrlprio, char ID = -1);
  int    __SDK drdSetPriorities                               (int prio, int ctrlprio, char ID = -1);
  int    __SDK drdSetEncPGain                                 (double gain, char ID = -1);
  double __SDK drdGetEncPGain                                 (char ID = -1);
  int    __SDK drdSetEncIGain                                 (double gain, char ID = -1);
  double __SDK drdGetEncIGain                                 (char ID = -1);
  int    __SDK drdSetEncDGain                                 (double gain, char ID = -1);
  double __SDK drdGetEncDGain                                 (char ID = -1);
  int    __SDK drdSetMotRatioMax                              (double scale, char ID = -1);
  double __SDK drdGetMotRatioMax                              (char ID = -1);
  int    __SDK drdSetEncMoveParam                             (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetEncTrackParam                            (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetPosMoveParam                             (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetPosTrackParam                            (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetRotMoveParam                             (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetRotTrackParam                            (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetGripMoveParam                            (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdSetGripTrackParam                           (double  amax, double  vmax, double  jerk, char ID = -1);
  int    __SDK drdGetEncMoveParam                             (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetEncTrackParam                            (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetPosMoveParam                             (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetPosTrackParam                            (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetRotMoveParam                             (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetRotTrackParam                            (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetGripMoveParam                            (double *amax, double *vmax, double *jerk, char ID = -1);
  int    __SDK drdGetGripTrackParam                           (double *amax, double *vmax, double *jerk, char ID = -1);


#ifdef __cplusplus
}
#endif


#endif
