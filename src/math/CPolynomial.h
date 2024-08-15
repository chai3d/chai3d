//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2024, CHAI3D
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
#ifndef CPolynomialH
#define CPolynomialH
//------------------------------------------------------------------------------
#include "math/CMaths.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CPolynomial.h
    \ingroup    math

    \brief
    Implements polynomial equations.
*/
//==============================================================================

//==============================================================================
/*!
    \struct     cPolynomial
    \ingroup    math

    \brief
    This class implements a polynomial function of linear, quadratic or
    cubic type.

    \details
    This class implements a polynomial function of linear, quadratic or
    cubic type. The 
*/
//==============================================================================
    class cPolynomial
    {
        //--------------------------------------------------------------------------
        // CONSTRUCTOR & DESTRUCTOR:
        //--------------------------------------------------------------------------

    public:

        //--------------------------------------------------------------------------
        /*!
            Constructor of cPolynomial.
        */
        //--------------------------------------------------------------------------
        cPolynomial() {}


        //--------------------------------------------------------------------------
        /*!
            \brief
            Constructor of cVector3d.

            \details
            This constructor initializes a vector by passing three doubles by
            argument.

            \param  a_x  X value.
            \param  a_y  Y value.
            \param  a_z  Z value.
        */
        //--------------------------------------------------------------------------
        cVector3d(const double a_x, const double a_y, const double a_z)
        {
            (*this)(0) = a_x;
            (*this)(1) = a_y;
            (*this)(2) = a_z;
        }



    private:

        //! Polynomial coefficient __a__.
        double m_a;

        //! Polynomial coefficient __b__.
        double m_b;

        //! Polynomial coefficient __c__.
        double m_c;

        //! Polynomial coefficient __d__.
        double m_d;


//------------------------------------------------------------------------------
}       // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif  // CPolynomialH
//------------------------------------------------------------------------------


