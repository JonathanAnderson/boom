/*
  Copyright (C) 2005-2009 Steven L. Scott

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

#ifndef BOOM_CUMULATIVE_LOGIT_SAMPLER_HPP_
#define BOOM_CUMULATIVE_LOGIT_SAMPLER_HPP_

#include <Models/Glm/CumulativeLogitModel.hpp>
#include <Models/Glm/WeightedRegressionModel.hpp>
#include <Models/Glm/PosteriorSamplers/draw_logit_lambda.hpp>
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/MvnBase.hpp>

namespace BOOM{

  class CumulativeLogitSampler
      : public PosteriorSampler
  {
   public:
    CumulativeLogitSampler(CumulativeLogitModel *m,
                            Ptr<MvnBase> beta_prior);
    void impute_latent_data();
    void draw_beta();
    void draw_delta();
    virtual void draw();
    virtual double logpri()const;
   private:
    CumulativeLogitModel *m_;
    Ptr<MvnBase> beta_prior_;
    WeightedRegSuf suf_;
    Spd ivar_;
    Vec mu_;
    Vec beta_;
    Vec delta_;
    // assume a flat prior on delta
  };


}
#endif// BOOM_CUMULATIVE_LOGIT_SAMPLER_HPP_
