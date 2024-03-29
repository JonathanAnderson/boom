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

#include <Models/PosteriorSamplers/GammaPosteriorSampler.hpp>
#include <cpputil/math_utils.hpp>

namespace BOOM{

  // A private namespace for defining functors used just for implementation.
  namespace {
    // A functor for evaluating the log posterior of the mean
    // parameter in the mean/alpha parameterization.
    class GammaMeanAlphaLogPosterior {
     public:
      GammaMeanAlphaLogPosterior(
          const GammaModel *model,
          const DoubleModel *mean_prior)
          : model_(model),
            mean_prior_(mean_prior)
      {}

      double operator()(double mean) const {
        if (mean <= 0) {
          return negative_infinity();
        }
        double a = model_->alpha();
        double b = a/mean;
        double ans = mean_prior_->logp(mean);
        ans += model_->loglikelihood(a, b);
        return ans;
      }

     private:
      const GammaModel *model_;
      const DoubleModel *mean_prior_;
    };

    // A functor for evaluating the log posterior of the mean
    // parameter in the mean/beta parameterization.
    class GammaMeanBetaLogPosterior {
     public:
      GammaMeanBetaLogPosterior(
          const GammaModel *model,
          const DoubleModel *mean_prior)
          : model_(model),
            mean_prior_(mean_prior) {}

      double operator()(double mean)const {
        if (mean <= 0.0) {
          return negative_infinity();
        }
        double ans = mean_prior_->logp(mean);
        double b = model_->beta();
        double a = mean * b;
        ans += model_->loglikelihood(a, b);
        return ans;
      }

     private:
      const GammaModel *model_;
      const DoubleModel *mean_prior_;
    };

    // A functor for evaluating the log posterior of the alpha
    // parameter in the mean/alpha parameterization.
    class GammaAlphaLogPosterior {
     public:
      GammaAlphaLogPosterior(const GammaModel *model,
                             const DoubleModel *alpha_prior)
          : model_(model),
            alpha_prior_(alpha_prior)
      {}

      double operator()(double alpha) const {
        if (alpha <= 0) {
          return negative_infinity();
        }
        double mean = model_->alpha() / model_->beta();
        double beta = alpha / mean;
        double ans = alpha_prior_->logp(alpha);
        ans += model_->loglikelihood(alpha, beta);
        return ans;
      }

     private:
      const GammaModel *model_;
      const DoubleModel *alpha_prior_;
    };

    // A functor for evaluating the log posterior of the beta
    // parameter in the mean/beta parameterization.
    class GammaBetaLogPosterior {
     public:
      GammaBetaLogPosterior(
          const GammaModel *model,
          const DoubleModel *beta_prior)
          : model_(model), beta_prior_(beta_prior) {}

      double operator()(double beta)const {
        if (beta <= 0.0) {
          return negative_infinity();
        }
        double ans = beta_prior_->logp(beta);
        double mean = model_->alpha() / model_->beta();
        double a = mean * beta;
        ans += model_->loglikelihood(a, beta);
        return ans;
      }

     private:
      const GammaModel *model_;
      const DoubleModel *beta_prior_;
    };

  }  // namespace

  //======================================================================
  GammaPosteriorSampler::GammaPosteriorSampler(
      GammaModel *model,
      Ptr<DoubleModel> mean_prior,
      Ptr<DoubleModel> alpha_prior)
      : model_(model),
        mean_prior_(mean_prior),
        alpha_prior_(alpha_prior),
        mean_sampler_(GammaMeanAlphaLogPosterior(
            model_, mean_prior_.get()), true),
        alpha_sampler_(GammaAlphaLogPosterior(
            model_, alpha_prior_.get()), true)
  {
    mean_sampler_.set_lower_limit(0);
    alpha_sampler_.set_lower_limit(0);
  }

  void GammaPosteriorSampler::draw(){
    double alpha = alpha_sampler_.draw(model_->alpha());
    double mean = model_->mean();
    double beta = alpha / mean;
    model_->set_params(alpha, beta);

    mean = mean_sampler_.draw(mean);
    beta = alpha / mean;
    model_->set_params(alpha, beta);
  }

  double GammaPosteriorSampler::logpri()const{
    double a = model_->alpha();
    double mean = a / model_->beta();
    return mean_prior_->logp(mean) + alpha_prior_->logp(a);
  }

  //======================================================================

  GammaPosteriorSamplerBeta::GammaPosteriorSamplerBeta(
      GammaModel *model,
      Ptr<DoubleModel> mean_prior,
      Ptr<DoubleModel> beta_prior)
      : model_(model),
        mean_prior_(mean_prior),
        beta_prior_(beta_prior),
        mean_sampler_(GammaMeanBetaLogPosterior(
            model, mean_prior.get())),
        beta_sampler_(GammaBetaLogPosterior(
            model, beta_prior.get()))
  {}

  void GammaPosteriorSamplerBeta::draw(){
    double beta = beta_sampler_.draw(model_->beta());
    double mean = model_->mean();
    double alpha = beta * mean;
    model_->set_params(alpha, beta);

    mean = mean_sampler_.draw(mean);
    alpha = beta * mean;
    model_->set_params(alpha, beta);
  }

  double GammaPosteriorSamplerBeta::logpri()const{
    double beta = model_->beta();
    double mean = model_->alpha() / beta;
    if (mean <= 0 || beta <= 0) {
      return negative_infinity();
    }
    return mean_prior_->logp(mean) + beta_prior_->logp(beta);
  }

}  // namespace BOOM
