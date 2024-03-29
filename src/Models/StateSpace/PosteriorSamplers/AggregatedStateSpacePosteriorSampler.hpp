/*
  Copyright (C) 2005-2011 Steven L. Scott

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
#ifndef BOOM_AGGREGATED_STATE_SPACE_POSTERIOR_SAMPLER_HPP_
#define BOOM_AGGREGATED_STATE_SPACE_POSTERIOR_SAMPLER_HPP_

#include <Models/StateSpace/AggregatedStateSpaceRegression.hpp>
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>

namespace BOOM{
  class AggregatedStateSpacePosteriorSampler : public PosteriorSampler {
   public:
    AggregatedStateSpacePosteriorSampler(AggregatedStateSpaceRegression *model);
    virtual void draw();
    virtual double logpri()const;
   private:
    AggregatedStateSpaceRegression *m_;
  };
}
#endif // BOOM_AGGREGATED_STATE_SPACE_POSTERIOR_SAMPLER_HPP_
