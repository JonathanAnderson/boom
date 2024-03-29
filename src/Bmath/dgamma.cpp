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
 *  AUTHOR
 *    Catherine Loader, catherine@research.bell-labs.com.
 *    October 23, 2000.
 *
 *  Merge in to R:
 *	Copyright (C) 2000 The R Core Development Team
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
 *
 *
 * DESCRIPTION
 *
 *   Computes the density of the gamma distribution,
 *
 *                   1/s (x/s)^{a-1} exp(-x/s)
 *        p(x;a,s) = -----------------------
 *                            (a-1)!
 *
 *   where `s' is the scale (= 1/lambda in other parametrizations)
 *     and `a' is the shape parameter ( = alpha in other contexts).
 *
 * The old (R 1.1.1) version of the code is available via `#define D_non_pois'
 */

#include "nmath.hpp"
#include "dpq.hpp"
namespace Rmath{

double dgamma(double x, double shape, double scale, int give_log)
{
#ifndef D_non_pois
    double pr;
#endif
#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(shape) || ISNAN(scale))
        return x + shape + scale;
#endif
    if (shape <= 0 || scale <= 0) ML_ERR_return_NAN;
    if (x < 0)
	return R_D__0;
    if (x == 0) {
      //	if (shape < 1) ML_ERR_return_NAN;
      if(shape < 1) return BOOM::infinity();
      if(shape > 1) return R_D__0;
      /* else */
      return give_log ? -log(scale) : 1 / scale;
    }

#ifdef D_non_pois

    x /= scale;
    return give_log ?
	   ((shape - 1) * log(x) - lgammafn(shape) - x) - log(scale) :
	exp((shape - 1) * log(x) - lgammafn(shape) - x) / scale;

#else /* new dpois() based code */

    if (shape < 1) {
	pr = dpois_raw(shape, x/scale, give_log);
	return give_log ?  pr + log(shape/x) : pr*shape/x;
    }
    /* else  shape >= 1 */
    pr = dpois_raw(shape-1, x/scale, give_log);
    return give_log ? pr - log(scale) : pr/scale;
#endif
}
}
