/*
  Copyright (C) 2005-2013 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

// This file was modified from the public domain cephes math library
// taken from netlib.

#include "cephes_impl.hpp"

namespace BOOM {
  namespace Cephes {

  /*							spence.c
   *
   *	Dilogarithm
   *
   *
   *
   * SYNOPSIS:
   *
   * double x, y, spence();
   *
   * y = spence( x );
   *
   *
   *
   * DESCRIPTION:
   *
   * Computes the integral
   *
   *                    x
   *                    -
   *                   | | log t
   * spence(x)  =  -   |   ----- dt
   *                 | |   t - 1
   *                  -
   *                  1
   *
   * for x >= 0.  A rational approximation gives the integral in
   * the interval (0.5, 1.5).  Transformation formulas for 1/x
   * and 1-x are employed outside the basic expansion range.
   *
   *
   *
   * ACCURACY:
   *
   *                      Relative error:
   * arithmetic   domain     # trials      peak         rms
   *    IEEE      0,4         30000       3.9e-15     5.4e-16
   *    DEC       0,4          3000       2.5e-16     4.5e-17
   *
   *
   */

  /*							spence.c */


  /*
    Cephes Math Library Release 2.8:  June, 2000
    Copyright 1985, 1987, 1989, 2000 by Stephen L. Moshier
  */

  static double A[8] = {
    4.65128586073990045278E-5,
    7.31589045238094711071E-3,
    1.33847639578309018650E-1,
    8.79691311754530315341E-1,
    2.71149851196553469920E0,
    4.25697156008121755724E0,
    3.29771340985225106936E0,
    1.00000000000000000126E0,
  };
  static double B[8] = {
    6.90990488912553276999E-4,
    2.54043763932544379113E-2,
    2.82974860602568089943E-1,
    1.41172597751831069617E0,
    3.63800533345137075418E0,
    5.03278880143316990390E0,
    3.54771340985225096217E0,
    9.99999999999999998740E-1,
  };

  double spence(double x) {
    double w, y, z;
    int flag;

    if( x < 0.0 )
    {
      report_error("Domain error in BOOM::Cephes::spence:  x < 0.");
      return(0.0);
    }

    if( x == 1.0 )
      return( 0.0 );

    if( x == 0.0 )
      return( PI*PI/6.0 );

    flag = 0;

    if( x > 2.0 )
    {
      x = 1.0/x;
      flag |= 2;
    }

    if( x > 1.5 )
    {
      w = (1.0/x) - 1.0;
      flag |= 2;
    }

    else if( x < 0.5 )
    {
      w = -x;
      flag |= 1;
    }

    else
      w = x - 1.0;


    y = -w * polevl( w, A, 7) / polevl( w, B, 7 );

    if( flag & 1 )
      y = (PI * PI)/6.0  - log(x) * log(1.0-x) - y;

    if( flag & 2 )
    {
      z = log(x);
      y = -0.5 * z * z  -  y;
    }

    return( y );
  }
  }  // namespace Cephes
}  // namespace BOOM