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

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <Models/PosteriorSamplers/PoissonGammaPosteriorSampler.hpp>

namespace BOOM {

  PoissonGammaPosteriorSampler::PoissonGammaPosteriorSampler(
      PoissonGammaModel *model,
      Ptr<DoubleModel> mean_prior_distribution,
      Ptr<DoubleModel> sample_size_prior)
      : model_(model),
        prior_mean_prior_distribution_(mean_prior_distribution),
        prior_sample_size_prior_distribution_(sample_size_prior),
        prior_mean_sampler_(
            boost::bind(&PoissonGammaPosteriorSampler::logp,
                        this,
                        _1,
                        model_->prior_sample_size()),
            false),
        prior_sample_size_sampler_(
            boost::bind(&PoissonGammaPosteriorSampler::logp,
                        this,
                        model_->prior_mean(),
                        _1),
            false)
  {
    prior_mean_sampler_.set_lower_limit(0.0);
    prior_sample_size_sampler_.set_lower_limit(0.0);
  }

  void PoissonGammaPosteriorSampler::draw(){
    double prior_sample_size =
        prior_sample_size_sampler_.draw(model_->prior_sample_size());
    model_->set_b(prior_sample_size);
    double prior_mean = prior_mean_sampler_.draw(model_->prior_mean());
    model_->set_prior_mean_and_sample_size(prior_mean, prior_sample_size);
  }

  double PoissonGammaPosteriorSampler::logpri()const{
    return prior_mean_prior_distribution_->logp(model_->prior_mean())
        + prior_sample_size_prior_distribution_->logp(
            model_->prior_sample_size());
  }

  double PoissonGammaPosteriorSampler::logp(
      double prior_mean, double prior_sample_size)const{
    double ans = prior_mean_prior_distribution_->logp(prior_mean);
    ans += prior_sample_size_prior_distribution_->logp(prior_sample_size);
    double b = prior_sample_size;
    double a = prior_mean * b;
    ans += model_->loglike(a, b);
    return ans;
  }

} // namespace BOOM
