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
#ifndef BOOM_POISSON_GAMMA_METHOD_HPP
#define BOOM_POISSON_GAMMA_METHOD_HPP

#include "PosteriorSampler.hpp"

namespace BOOM{
  class PoissonModel;
  class GammaModel;

  class PoissonGammaSampler : public PosteriorSampler{
  public:
    PoissonGammaSampler(Ptr<PoissonModel> p, Ptr<GammaModel> g);
    virtual void draw();
    virtual double logpri()const;
    double alpha()const;
    double beta()const;
    void find_posterior_mode();
  private:
    Ptr<GammaModel>   gam;
    Ptr<PoissonModel> pois;
  };
}
#endif // BOOM_POISSON_GAMMA_METHOD_HPP
