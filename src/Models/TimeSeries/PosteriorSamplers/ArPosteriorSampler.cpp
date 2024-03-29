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

#include <Models/TimeSeries/PosteriorSamplers/ArPosteriorSampler.hpp>
#include <distributions.hpp>
#include <distributions/trun_gamma.hpp>
#include <cpputil/math_utils.hpp>

namespace BOOM{

  ArPosteriorSampler::ArPosteriorSampler(
      ArModel *model, Ptr<GammaModelBase> siginv_prior)
        : model_(model),
          siginv_prior_(siginv_prior),
          max_number_of_regression_proposals_(3),
          upper_sigma_truncation_point_(infinity())
    {}

  void ArPosteriorSampler::draw(){
    draw_phi();
    draw_sigma();
  }

  double ArPosteriorSampler::logpri()const{
    bool ok = model_->check_stationary(model_->phi());
    if(!ok) return negative_infinity();
    return siginv_prior_->logp(1.0 / model_->sigsq());
  }

  // Draws sigma given phi and observed data.
  void ArPosteriorSampler::draw_sigma(){
    // ss = y - xb  y - xb
    //     = yty - 2 bt xty + bt xtx b
    const Vec &phi(model_->phi());
    const Vec &xty(model_->suf()->xty());
    const Spd &xtx(model_->suf()->xtx());
    double ss = 2 * siginv_prior_->beta();
    ss += xtx.Mdist(phi) - 2 * phi.dot(xty) + model_->suf()->yty();

    double df = 2 * siginv_prior_->alpha();
    df += model_->suf()->n();

    double siginv;
    if (upper_sigma_truncation_point_ == BOOM::infinity()) {
      siginv = rgamma_mt(rng(), df/2, ss/2);
    } else {
      siginv = rtrun_gamma_mt(rng(), df/2, ss/2,
                           1.0/pow(upper_sigma_truncation_point_, 2));
    }
    model_->set_sigsq(1.0 / siginv);
  }

  void ArPosteriorSampler::draw_phi(){
    const Spd &xtx(model_->suf()->xtx());
    const Vec &xty(model_->suf()->xty());
    Vec phi_hat = xtx.solve(xty);
    bool ok = false;
    int attempts = 0;
    while (!ok && ++attempts <= max_number_of_regression_proposals_) {
      Vec phi = rmvn_ivar(phi_hat, xtx / model_->sigsq());
      ok = ArModel::check_stationary(phi);
      if(ok) model_->set_phi(phi);
    }
    if(!ok){
      draw_phi_univariate();
    }
  }

  void ArPosteriorSampler::draw_phi_univariate() {
    int p = model_->phi().size();
    Vec phi = model_->phi();
    if (!model_->check_stationary(phi)) {
      report_error("ArPosteriorSampler::draw_phi_univariate was called with an "
                   "illegal initial value of phi.  That should never happen.");
    }
    const Spd &xtx(model_->suf()->xtx());
    const Vec &xty(model_->suf()->xty());

    for (int i = 0; i < p; ++i) {
      double initial_phi = phi[i];

      double lo = -1;
      double hi = 1;

      // y - xb  y - xb
      //   bt xtx b  - 2 bt xty + yty

      //  bt xtx b = sum_i sum_j beta[i] beta[j] xtx[i, j]
      //           = beta [i]^2 xtx[i,i] + 2 * sum_{j != i} beta[i] xtx[i, j] * beta[j]
      //             - 2 * beta[i] * xty[i];

      // mean is (xty[i] - sum_{j != i}  )
      double ivar = xtx(i, i);
      double mu = (xty[i] - (phi.dot(xtx.col(i)) - phi[i] * xtx(i, i))) / ivar;
      bool ok = false;
      while (!ok) {
        double candidate = rtrun_norm_2_mt(rng(), mu, sqrt(1.0/ivar), lo, hi);
        phi[i] = candidate;
        if (ArModel::check_stationary(phi)) {
          ok = true;
        } else {
          if (candidate > initial_phi) hi = candidate;
          else lo = candidate;
        }
      }
    }
    model_->set_phi(phi);
  }

  void ArPosteriorSampler::set_max_number_of_regression_proposals(
      int number_of_proposals) {
    // Note that negative values are okay here.  If
    // number_of_proposals is negative, it is a signal that the user
    // always wants to draw phi one coefficient at a time.
    max_number_of_regression_proposals_ = number_of_proposals;
  }

  void ArPosteriorSampler::set_sigma_upper_limit(double max_sigma){
    if(max_sigma <= 0) {
      ostringstream err;
      err << "ArPosteriorSampler::set_sigma_upper_limit expects "
          << "a positive argument.  It was given "
          << max_sigma << "." << endl;
      report_error(err.str());
    }
    upper_sigma_truncation_point_ = max_sigma;
  }

}
