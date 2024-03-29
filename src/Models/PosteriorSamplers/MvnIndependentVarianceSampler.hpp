/*
  Copyright (C) 2005-2012 Steven L. Scott

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

#ifndef BOOM_MVN_INDEPENDENT_VARIANCE_SAMPLER_HPP_
#define BOOM_MVN_INDEPENDENT_VARIANCE_SAMPLER_HPP_

#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/GammaModel.hpp>
#include <Models/MvnModel.hpp>
#include <vector>
#include <cpputil/math_utils.hpp>

namespace BOOM {

  class MvnIndependentVarianceSampler
      : public PosteriorSampler {
   public:
    MvnIndependentVarianceSampler(
        MvnModel *model,
        const std::vector<Ptr<GammaModelBase> > &siginv_priors,
        const Vec & upper_sigma_truncation_point);
    void draw();
    double logpri()const;

   private:
    MvnModel *model_;
    std::vector<Ptr<GammaModelBase> > priors_;
    std::vector<double> upper_sigma_truncation_point_;
  };

}  // namespace BOOM

#endif //  BOOM_MVN_INDEPENDENT_VARIANCE_SAMPLER_HPP_
