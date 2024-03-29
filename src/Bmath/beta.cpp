/*
  Copyright (C) 2005 Steven L. Scott

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

/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000-2001 The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  SYNOPSIS
 *
 *    #include <Bmath.hpp>
 *    double beta(double a, double b);
 *
 *  DESCRIPTION
 *
 *    This function returns the value of the beta function
 *    evaluated with arguments a and b.
 *
 *  NOTES
 *
 *    This routine is a translation into C of a Fortran subroutine
 *    by W. Fullerton of Los Alamos Scientific Laboratory.
 *    Some modifications have been made so that the routines
 *    conform to the IEEE 754 standard.
 */

#include "nmath.hpp"
namespace Rmath{
  double beta(double a, double b)
    {
      double val;

// #ifdef NOMORE_FOR_THREADS
//       static double xmin, xmax = 0;/*-> typically = 171.61447887 for IEEE */
//       static double lnsml = 0;/*-> typically = -708.3964185 */

//       if (xmax == 0) {
// 	gammalims(&xmin, &xmax);
// 	lnsml = log(d1mach(1));
//       }
// #else
      /* For IEEE double precision numeric_limits<double>::epsilon() = 2^-52 = 2.220446049250313e-16 :
       *   xmin, xmax : see ./gammalims.c
       *   lnsml = log(numeric_limits<double>::min()) = log(2 ^ -1022) = -1022 * log(2)
       */
# define xmin  -170.5674972726612
# define xmax   171.61447887182298
# define lnsml -708.39641853226412
      //#endif


#ifdef IEEE_754
      /* NaNs propagated correctly */
      if(ISNAN(a) || ISNAN(b)) return a + b;
#endif

      if (a < 0 || b < 0)
	ML_ERR_return_NAN
	  else if (a == 0 || b == 0) {
	    return BOOM::infinity();
	  }
      else if (!R_FINITE(a) || !R_FINITE(b)) {
	return 0;
      }

      if (a + b < xmax) /* ~= 171.61 for IEEE */
	return gammafn(a) * gammafn(b) / gammafn(a+b);

      val = lbeta(a, b);
      if (val < lnsml) {
	/* a and/or b so big that beta underflows */
	ML_ERROR(ME_UNDERFLOW);
	return ML_UNDERFLOW;
      }
      return exp(val);
    }
}
