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
#ifndef CHistogramH
#define CHistogramH
//------------------------------------------------------------------------------
#include "widgets/CPanel.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
const unsigned int C_HISTOGRAM_MAX_SAMPLES = 6000;
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CHistogram.h

    \brief
    Implements a 2D histogram widget
*/
//==============================================================================

enum cHistogramMode
{
    C_HISTOGRAM_MODE_LINE,
    C_HISTOGRAM_MODE_LINE_FILL,
    C_HISTOGRAM_MODE_BAR
};

//==============================================================================
/*!
    \class      cHistogram
    \ingroup    widgets

    \brief
    This class implements a 2D histogram to display an array of data value.

    \details
    This class implements a 2D histogram to display an array of data value.
*/
//==============================================================================
class cHistogram : public cPanel
{    
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cHistogram.
    cHistogram();

    //! Destructor of cHistogram.
    virtual ~cHistogram() {};


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method returns the name of the object class.
    virtual std::string getClassName() { return ("Histogram"); }

    //! This method creates a copy of itself.
    virtual cHistogram* copy(const bool a_duplicateMaterialData = false,
        const bool a_duplicateTextureData = false, 
        const bool a_duplicateMeshData = false,
        const bool a_buildCollisionDetector = false);

    //! This method sets the size of this scope.
    virtual void setSize(const double& a_width, const double& a_height);

    //! This method sets the number of samples to be disdplayed by the histogram.
    virtual bool setSampleSize(const unsigned int a_sampleSize);

    //! This method sets the number of samples to be disdplayed by the histogram.
    virtual unsigned int getSampleSize() { return (m_sampleSize); }

    //! This method sets the line width of the histogram in wire mode.
    inline void setLineWidth(const double a_lineWidth) { m_lineWidth = fabs(a_lineWidth); }

    //! This method returns the line width of the line
    inline double getLineWidth() const { return (m_lineWidth); }

    //! This method sets a data value in the histogram at a desired index location.
    void setDataValue(const unsigned int a_index, const double a_dataValue);

    //! This method returns a data value from the histogram at a desired index location.
    double getDataValue(const unsigned int a_index);

    //! This method clears all data values to minimum range value.
    void clearDataValues();

    //! This method sets the range of input values which can be displayed on the scope.
    virtual void setRange(const double a_minValue, 
                          const double a_maxValue); 

    //! This method returns the minimum value from the range.
    inline double getRangeMin() const { return (m_minValue); }

    //! This method returns the maximum value from the range.
    inline double getRangeMax() const { return (m_maxValue); }


    //--------------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //--------------------------------------------------------------------------

public:

    //! Color settings for histogram.
    cColorf m_color;


    //--------------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //--------------------------------------------------------------------------

protected:

    //! Rendering mode of histogram
    cHistogramMode m_mode;

    //! Sample size to be displayed in histogram
    unsigned int m_sampleSize;

    //! Range - minimum value.
    double m_minValue;

    //! Range - maximum value.
    double m_maxValue;

    //! Data values.
    double m_dataValues[C_HISTOGRAM_MAX_SAMPLES];

    //! Data values.
    int m_dataPoints[C_HISTOGRAM_MAX_SAMPLES];

    //! Width used to render lines.
    double m_lineWidth;

    //! Internal width of histogram display.
    double m_histogramWidth;

    //! Internal height of histogram display.
    double m_histogramHeight;

    //! Position of scope in reference to Panel.
    cVector3d m_histogramPosition;


    //--------------------------------------------------------------------------
    // PROTECTED METHODS:
    //--------------------------------------------------------------------------

protected:

    //! This method renders the object graphically using OpenGL.
    virtual void render(cRenderOptions& a_options);

    //! This method copies all properties of this object to another.
    void copyScopeProperties(cHistogram* a_obj,
        const bool a_duplicateMaterialData,
        const bool a_duplicateTextureData, 
        const bool a_duplicateMeshData,
        const bool a_buildCollisionDetector);
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
