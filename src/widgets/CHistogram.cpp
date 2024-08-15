//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2019, CHAI3D
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
    \author    Francois Conti
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "widgets/CHistogram.h"
//------------------------------------------------------------------------------
#include "graphics/CPrimitives.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    Constructor of cHistogram.
*/
//==============================================================================
cHistogram::cHistogram()
{
    // clear signals
    clearDataValues();

    // set default radius values
    m_panelRadiusTopLeft = 10;
    m_panelRadiusTopRight = 10;
    m_panelRadiusBottomLeft = 10;
    m_panelRadiusBottomRight = 10;

    // set default color signals
    m_color.setBlueCornflower();

    // default line width
    m_lineWidth = 1.0;

    // default panel background color settings
    m_panelColorTopLeft.setGrayLevel(0.3f);
    m_panelColorTopRight.setGrayLevel(0.3f);
    m_panelColorBottomLeft.setGrayLevel(0.2f);
    m_panelColorBottomRight.setGrayLevel(0.2f);

    // initialize values
    m_mode = C_HISTOGRAM_MODE_BAR;
    m_sampleSize = 256;
    m_histogramWidth = 0.0;
    m_histogramHeight = 0.0;
    m_histogramPosition.set(0.0, 0.0, 0.0);

    // set a default size
    setSize(600, 200);
}


//==============================================================================
/*!
    This method sets a data value of the histogram.

    \param  a_index      Index of data value.
    \param  a_dataValue  Data value.
*/
//==============================================================================
void cHistogram::setDataValue(const unsigned int a_index, const double a_dataValue)
{
    if (a_index < C_HISTOGRAM_MAX_SAMPLES)
    {
        m_dataValues[a_index] = a_dataValue;
        double value = cClamp(a_dataValue, m_minValue, m_maxValue);
        m_dataPoints[a_index] = (int)((double)(m_histogramHeight) * (value - m_minValue) / (m_maxValue - m_minValue));
    }
}


//==============================================================================
/*!
    This method returns a data value from the histogram at a desired index location.

    \param  a_index      Index of data value.

    \return Data value
*/
//==============================================================================
double cHistogram::getDataValue(const unsigned int a_index)
{
    if (a_index < C_HISTOGRAM_MAX_SAMPLES)
    {
        return (m_dataValues[a_index]);
    }
    else
    {
        return (0);
    }
}


//==============================================================================
/*!
    This method resets all histogram values to the minimum range value.
*/
//==============================================================================
void cHistogram::clearDataValues()
{
    for (unsigned int i=0; i < C_HISTOGRAM_MAX_SAMPLES; i++)
    {
        setDataValue(i, m_minValue);
    }
}


//==============================================================================
/*!
    This method sets the range of values that can be displayed by the histogram.

    \param  a_minValue  Minimum value.
    \param  a_maxValue  Maximum value.
*/
//==============================================================================
void cHistogram::setRange(const double a_minValue, 
                          const double a_maxValue)
{
    // sanity check
    if (a_minValue == a_maxValue)
    {
        return;
    }

    // store values
    m_minValue = cMin(a_minValue, a_maxValue);
    m_maxValue = cMax(a_minValue, a_maxValue);

    // update data points by rewritting all data values
    for (unsigned int i = 0; i < C_HISTOGRAM_MAX_SAMPLES; i++)
    {
        setDataValue(i, m_dataValues[i]);
    }
}


//==============================================================================
/*!
    This method sets the number of samples to be disdplayed by the histogram.

    \param  a_sampleSize  Sample size.

    \return __true__ if operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cHistogram::setSampleSize(const unsigned int a_sampleSize)
{
    if (a_sampleSize > C_HISTOGRAM_MAX_SAMPLES)
    {
        return (false);
    }
    else
    {
        m_sampleSize = a_sampleSize;
        return (true);
    }
}


//==============================================================================
/*!
    This method sets the size of the scope by defining its width and height.

    \param  a_width   Width of scope.
    \param  a_height  Height of scope.
*/
//==============================================================================
void cHistogram::setSize(const double& a_width, const double& a_height)
{
    // set width
    m_width = a_width;
    m_height = a_height;

    // compute max radius
    double radius = cMax(m_panelRadiusTopLeft, m_panelRadiusBottomLeft) +
                    cMax(m_panelRadiusTopRight, m_panelRadiusBottomRight);

    // adjust margins if needed
    m_marginTop = cMax(m_marginTop, radius);
    m_marginBottom = cMax(m_marginBottom, radius);
    m_marginLeft = cMax(m_marginLeft, radius);
    m_marginRight = cMax(m_marginRight, radius);

    // update model of panel
    updatePanelMesh();

    // set dimension of scope.
    m_histogramWidth = cMax(0.0, m_width - m_marginLeft - m_marginRight);
    m_histogramHeight = cMax(0.0, m_height - m_marginTop - m_marginBottom);

    // set position of scope within panel
    m_histogramPosition.set(m_marginLeft, m_marginBottom, 0.0);

    // update data points by rewritting all data values
    for (unsigned int i = 0; i < C_HISTOGRAM_MAX_SAMPLES; i++)
    {
        setDataValue(i, m_dataValues[i]);
    }
}


//==============================================================================
/*!
    This method renders the scope using OpenGL.

    \param  a_options  Rendering options.
*/
//==============================================================================
void cHistogram::render(cRenderOptions& a_options)
{
#ifdef C_USE_OPENGL

    // render background panel
    if (m_showPanel)
    {
        cMesh::render(a_options);
    }

    /////////////////////////////////////////////////////////////////////////
    // Render parts that are always opaque
    /////////////////////////////////////////////////////////////////////////
    if (SECTION_RENDER_OPAQUE_PARTS_ONLY(a_options))
    {
        // position scope within panel
        glPushMatrix();
        glTranslated(m_histogramPosition(0), m_histogramPosition(1), 0.0);

        // disable lighting
        glDisable(GL_LIGHTING);

        // render histogram
        if (m_sampleSize > 0)
        {
            if (m_mode == C_HISTOGRAM_MODE_LINE)
            {
                // set line width
                glLineWidth((GLfloat)m_lineWidth);

                // set color
                m_color.render();

                // compute offset
                double dx = m_histogramWidth / (double)(m_sampleSize - 1);

                if (m_sampleSize == 1)
                {
                    glBegin(GL_LINES);
                    glVertex3d(0.0, m_dataPoints[0], 0.0);
                    glVertex3d(m_histogramWidth, m_dataPoints[0], 0.0);
                    glEnd();
                }
                else
                {
                    for (unsigned int i = 0; i < (m_sampleSize - 1); i++)
                    {
                        glBegin(GL_LINES);
                        glVertex3d(i * dx, m_dataPoints[i], 0.0);
                        glVertex3d((i+1) * dx, m_dataPoints[i+1], 0.0);
                        glEnd();
                    }
                }

                // set line width
                glLineWidth(1.0);

            }

            else if (m_mode == C_HISTOGRAM_MODE_LINE_FILL)
            {

            }

            else if (m_mode == C_HISTOGRAM_MODE_BAR)
            {
                // set color
                m_color.render();

                // compute offset
                double space = 2.0;
                double dx = (m_histogramWidth / (double)(m_sampleSize));
                double w = dx - space;

                if (w > 0.0)
                {
                    if (m_sampleSize == 1)
                    {
                        glBegin(GL_TRIANGLES);
                            glVertex3d(0.0, m_dataPoints[0], 0.0);
                            glVertex3d(0.0, 0.0, 0.0);
                            glVertex3d(m_histogramWidth, 0.0, 0.0);

                            glVertex3d(0.0, m_dataPoints[0], 0.0);
                            glVertex3d(m_histogramWidth, 0.0, 0.0);
                            glVertex3d(m_histogramWidth, m_dataPoints[0], 0.0);
                        glEnd();
                    }
                    else
                    {
                        for (unsigned int i = 0; i < (m_sampleSize - 1); i++)
                        {
                            double dx0 = (double)(i)* dx;
                            double dx1 = (double)(i)* dx+w;

                            glBegin(GL_TRIANGLES);
                            glVertex3d(dx0, m_dataPoints[i], 0.0);
                            glVertex3d(dx0, 0.0, 0.0);
                            glVertex3d(dx1, 0.0, 0.0);

                            glVertex3d(dx0, m_dataPoints[i], 0.0);
                            glVertex3d(dx1, 0.0, 0.0);
                            glVertex3d(dx1, m_dataPoints[i], 0.0);
                            glEnd();
                        }
                    }
                }
            }
        }

        // restore OpenGL settings
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

#endif
}


//==============================================================================
/*!
    This method creates a copy of itself.

    \param  a_duplicateMaterialData   If __true__, material (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateTextureData    If __true__, texture data (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateMeshData       If __true__, mesh data (if available) is duplicated, otherwise it is shared.
    \param  a_buildCollisionDetector  If __true__, collision detector (if available) is duplicated, otherwise it is shared.

    \return Pointer to new object.
*/
//==============================================================================
cHistogram* cHistogram::copy(const bool a_duplicateMaterialData,
    const bool a_duplicateTextureData, 
    const bool a_duplicateMeshData,
    const bool a_buildCollisionDetector)
{
    // create new instance
    cHistogram* obj = new cHistogram();

    // copy properties of cGenericObject
    copyScopeProperties(obj, 
        a_duplicateMaterialData, 
        a_duplicateTextureData,
        a_duplicateMeshData,
        a_buildCollisionDetector);

    // return
    return (obj);
}


//==============================================================================
/*!
    This method copies all properties of this object to another.

    \param  a_obj                     Destination object where properties are copied to.
    \param  a_duplicateMaterialData   If __true__, material (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateTextureData    If __true__, texture data (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateMeshData       If __true__, mesh data (if available) is duplicated, otherwise it is shared.
    \param  a_buildCollisionDetector  If __true__, collision detector (if available) is duplicated, otherwise it is shared.
*/
//==============================================================================
void cHistogram::copyScopeProperties(cHistogram* a_obj,
    const bool a_duplicateMaterialData,
    const bool a_duplicateTextureData, 
    const bool a_duplicateMeshData,
    const bool a_buildCollisionDetector)
{
    // copy properties of cPanel
    copyPanelProperties(a_obj, 
        a_duplicateMaterialData, 
        a_duplicateTextureData,
        a_duplicateMeshData,
        a_buildCollisionDetector);

    // copy properties of cHistogram
    a_obj->m_minValue = m_minValue;
    a_obj->m_maxValue = m_maxValue;
    a_obj->m_lineWidth = m_lineWidth;
    a_obj->m_color = m_color;
    a_obj->setSize(m_width, m_height);
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
