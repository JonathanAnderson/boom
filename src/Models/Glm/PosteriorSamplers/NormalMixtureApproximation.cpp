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

#include <Models/Glm/PosteriorSamplers/NormalMixtureApproximation.hpp>

#include <sstream>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cpputil/lse.hpp>
#include <cpputil/math_utils.hpp>
#include <cpputil/report_error.hpp>
#include <cpputil/index_table.hpp>
#include <cpputil/apply_permutation.hpp>
#include <distributions.hpp>
#include <numopt/Brent.hpp>
#include <numopt/Integral.hpp>
#include <numopt/Powell.hpp>
#include <iomanip>

namespace {
  using namespace BOOM;
  Vector inverse_logit(const Vector &logit_w) {
    //  logit_w = log(w / w[0]);
    //  exp(logit_w) = w/w0;
    BOOM::Vector ans = concat(1, exp(logit_w));
    ans = ans / sum(ans);
    return ans;
  }

  inline Vector vector_logit(const Vector &w) {
    Vector tail(ConstVectorView(w, 1));
    return log(tail / w[0]);
  }

  inline double square(double x) { return x * x; }
} // namespace

namespace BOOM {

  ApproximationDistance::ApproximationDistance(
      ScalarTarget logf,
      const NormalMixtureApproximation & approximation,
      double lower_limit,
      double upper_limit,
      double guess_at_mode)
      : logf_(logf),
        approx_(approximation),
        lower_limit_(lower_limit),
        upper_limit_(upper_limit),
        guess_at_mode_(guess_at_mode)
  {}

  double ApproximationDistance::operator()(const Vector &theta)const{
    approx_.set(theta);
    return current_distance();
  }

  double ApproximationDistance::current_distance()const{
    ScalarTarget ig = boost::bind(
        &ApproximationDistance::integrand, this, _1);
    Integral integral1(ig, lower_limit_, guess_at_mode_, 1000);
    integral1.throw_on_error(false);
    Integral integral2(ig, guess_at_mode_, upper_limit_, 1000);
    integral2.throw_on_error(false);
    return integral1.integrate() + integral2.integrate();
  }

  double ApproximationDistance::logf(double x)const{return logf_(x);}
  double ApproximationDistance::approximation(double x)const{return approx_.logp(x);}
  double ApproximationDistance::lower_limit()const{return lower_limit_;}
  double ApproximationDistance::upper_limit()const{return upper_limit_;}

  KullbackLeiblerDivergence::KullbackLeiblerDivergence(
      ScalarTarget logf,
      const NormalMixtureApproximation &approx,
      double lower_limit,
      double upper_limit,
      double guess_at_mode)
      : ApproximationDistance(logf, approx, lower_limit, upper_limit, guess_at_mode) {}

  double KullbackLeiblerDivergence::integrand(double x)const{
    double logfx = logf(x);
    double fx = exp(logfx);
    double ans = fx * (logfx - approximation(x));
    return ans;
  }

  AbsNormDistance::AbsNormDistance(
      ScalarTarget logf,
      const NormalMixtureApproximation &approx,
      double lower_limit,
      double upper_limit,
      double guess_at_mode)
      : ApproximationDistance(logf, approx, lower_limit, upper_limit, guess_at_mode) {}

  double AbsNormDistance::integrand(double x)const{
    return fabs(exp(logf(x)) - exp(approximation(x)));
  }
  //======================================================================

  NormalMixtureApproximation::NormalMixtureApproximation(int n)
      : mu_(n),
        sigma_(n),
        weights_(n),
        log_weights_(n),
        wsp_(n),
        force_zero_mu_(false),
        kullback_leibler_(negative_infinity()),
        number_of_function_evaluations_(-1)
  {
    check_sizes();
  }

  NormalMixtureApproximation::NormalMixtureApproximation(
      const Vector &mu,
      const Vector &sigma,
      const Vector &weights)
      : mu_(mu),
        sigma_(sigma),
        weights_(weights),
        wsp_(mu.size()),
        force_zero_mu_(false),
        kullback_leibler_(negative_infinity()),
        number_of_function_evaluations_(-1)
  {
    order_by_mu();
    log_weights_ = log(weights_);
    check_sizes();
  }

  NormalMixtureApproximation::NormalMixtureApproximation(
      ScalarTarget logf,
      const Vector &initial_mu,
      const Vector &initial_sigma,
      const Vector &initial_weights,
      double precision,
      int max_evals,
      double initial_stepsize,
      bool force_zero_mu)
      : mu_(initial_mu),
        sigma_(initial_sigma),
        weights_(initial_weights),
        log_weights_(weights_),
        wsp_(mu_.size()),
        force_zero_mu_(force_zero_mu)
  {
    check_sizes();
    BrentMaximizer brent(logf);
    brent.maximize(0.0);
    double guess_at_mode = brent.maximizing_x();
    double mode_value = brent.maximum_value();
    double lower_limit = guess_at_mode - 1;
    double flo = logf(lower_limit);
    while(mode_value - flo < 30) {
      flo = logf(--lower_limit);
    }

    double upper_limit = guess_at_mode + 1;
    double fhi = logf(upper_limit);
    while(mode_value - fhi < 30) {
      fhi = logf(++upper_limit);
    }

    Vector log_sigma = log(initial_sigma);
    Vector logit_w = vector_logit(initial_weights);

    Vector theta;
    if (force_zero_mu_) {
      mu_ = 0;
      theta = log_sigma;
      theta.concat(logit_w);
    } else {
      theta = initial_mu;
      theta.concat(log_sigma);
      theta.concat(logit_w);
    }

    KullbackLeiblerDivergence kl(logf, *this, lower_limit, upper_limit, guess_at_mode);
    PowellMinimizer powell(kl);
    powell.set_evaluation_limit(max_evals);  //
    powell.set_precision(precision);
    //    powell.set_precision(1e-8);
    powell.set_initial_stepsize(initial_stepsize);
    powell.minimize(theta);
    theta = powell.minimizing_value();
    kullback_leibler_ = powell.minimum();
    number_of_function_evaluations_ = powell.number_of_function_evaluations();

    int number_of_components = initial_mu.size();
    if (force_zero_mu_) {
      ConstVectorView final_log_sigma(theta, 0, number_of_components);
      ConstVectorView final_logit_w(
          theta, number_of_components, number_of_components - 1);
      mu_ = 0;
      set(mu_, exp(final_log_sigma), inverse_logit(final_logit_w));
    } else {
      ConstVectorView final_mu(theta, 0, number_of_components);
      ConstVectorView final_log_sigma(
          theta, number_of_components, number_of_components);
      ConstVectorView final_logit_w(
          theta, 2*number_of_components, number_of_components - 1);
      set(final_mu, exp(final_log_sigma), inverse_logit(final_logit_w));
    }
  }

  void NormalMixtureApproximation::set(const Vector &theta) {
    if (force_zero_mu_) {
      int dimension = (theta.size() + 1) / 2;
      sigma_ = exp(ConstVectorView(theta, 0, dimension));
      weights_ = inverse_logit(
          ConstVectorView(theta, dimension, dimension - 1));
      mu_.resize(dimension);
      mu_ = 0;
      wsp_.resize(dimension);
      order_by_sigma();
    } else {
      int dimension = (theta.size() + 1) / 3;
      mu_ = ConstVectorView(theta, 0, dimension);
      sigma_ = exp(ConstVectorView(theta, dimension, dimension));
      weights_ = inverse_logit(ConstVectorView(theta, 2*dimension, dimension - 1));
      wsp_.resize(mu_.size());
      order_by_mu();
    }
    log_weights_ = log(weights_);
    check_sizes();
  }

  void NormalMixtureApproximation::set(const Vector &mu,
                                       const Vector &sigma,
                                       const Vector &weights) {
    if (mu.size() != sigma.size() ||
       mu.size() != weights.size()) {
      report_error("mu, sigma, and weights must all be the same size in "
                   "NormalMixtureApproximation::set().");
    }
    mu_ = mu;
    sigma_ = sigma;
    weights_ = weights;
    wsp_.resize(mu_.size());
    if (force_zero_mu_) {
      mu_ = 0;
      order_by_sigma();
    } else {
      order_by_mu();
    }
    log_weights_ = log(weights_);
  }

  void NormalMixtureApproximation::set_order(
      const std::vector<int> &permutation) {
    permute_inplace(permutation, mu_);
    permute_inplace(permutation, sigma_);
    permute_inplace(permutation, weights_);
    permute_inplace(permutation, log_weights_);
  }

  void NormalMixtureApproximation::order_by_mu(){
    set_order(index_table(mu_));
  }

  void NormalMixtureApproximation::order_by_sigma() {
    set_order(index_table(sigma_));
  }

  double NormalMixtureApproximation::logp(double y)const{
    wsp_ = log_weights_;
    for (int s = 0; s < mu_.size(); ++s) {
      wsp_[s] += dnorm(y, mu_[s], sigma_[s], true);
    }
    return lse(wsp_);
  }

  void NormalMixtureApproximation::unmix(
      RNG &rng, double u, double *mu, double *sigsq) const {
    wsp_ = log_weights_;
    for (int s = 0; s < dim(); ++s) {
      wsp_[s] += dnorm(u, mu_[s], sigma_[s], true);
    }
    wsp_.normalize_logprob();
    int mixture_indicator = rmulti_mt(rng, wsp_);
    *mu = mu_[mixture_indicator];
    *sigsq = square(sigma_[mixture_indicator]);
  }

  double NormalMixtureApproximation::kullback_leibler()const{
    return kullback_leibler_;}

  double NormalMixtureApproximation::kullback_leibler(ScalarTarget target){
    check_sizes();
    BrentMaximizer brent(target);
    brent.maximize(0.0);
    double guess_at_mode = brent.maximizing_x();
    double mode_value = brent.maximum_value();
    double lower_limit = guess_at_mode - 1;
    double flo = logf(lower_limit);
    while(mode_value - flo < 30) {
      flo = target(--lower_limit);
    }

    double upper_limit = guess_at_mode + 1;
    double fhi = logf(upper_limit);
    while(mode_value - fhi < 30) {
      fhi = target(++upper_limit);
    }

    KullbackLeiblerDivergence kl(target, *this, lower_limit, upper_limit, guess_at_mode);
    kullback_leibler_ = kl.current_distance();
    return kullback_leibler_;
  }

  int NormalMixtureApproximation::number_of_function_evaluations()const{
    return number_of_function_evaluations_;}

  void NormalMixtureApproximation::check_sizes(){
    int n = mu_.size();
    if(sigma_.size() != n ||
       weights_.size() != n ||
       log_weights_.size() != n ||
       wsp_.size() != n) {
      ostringstream err;
      err << "Error in NormalMixtureApproximation:  vectors have different sizes." << endl
          << "mu_           : " << mu_.size() << endl
          << "sigma_        : " << sigma_.size() << endl
          << "weights_      : " << sigma_.size() << endl
          << "log_weights_  : " << sigma_.size() << endl
          << "wsp_          : " << sigma_.size() << endl;
      report_error(err.str());
    }
  }

  ostream & NormalMixtureApproximation::print(ostream &out) const {

    out << "mu:      " << std::setprecision(15) << mu_ << endl
        << "sigma:   " << sigma_ << endl
        << "weights: " << weights_ << endl
        << "kl:      " << kullback_leibler_ << endl
        << "evals:   " << number_of_function_evaluations_ << endl;
    return out;
  }

  //======================================================================
  NormalMixtureApproximationTable::NormalMixtureApproximationTable() {}

  NormalMixtureApproximationTable::NormalMixtureApproximationTable(
      const NormalMixtureApproximationTable &rhs)
      : index_(rhs.index_),
        approximations_(rhs.approximations_)
  {}

  NormalMixtureApproximationTable & NormalMixtureApproximationTable::operator=(
      const NormalMixtureApproximationTable &rhs) {
    if (&rhs == this) {
      return *this;
    }
    index_ = rhs.index_;
    approximations_ = rhs.approximations_;
    return *this;
  }

  void NormalMixtureApproximationTable::add(
      int index, const NormalMixtureApproximation &approximation){
    if (index_.empty() || index > index_.back()) {
      index_.push_back(index);
      approximations_.push_back(approximation);
    } else {
      std::vector<int>::iterator lower_bound = std::lower_bound(
          index_.begin(), index_.end(), index);
      int position = lower_bound - index_.begin();
      index_.insert(lower_bound, index);

      approximations_.insert(approximations_.begin() + position,
                             approximation);
    }
  }

  int NormalMixtureApproximationTable::smallest_index()const{
    return index_[0];
  }

  int NormalMixtureApproximationTable::largest_index()const{
    return index_.back();
  }

  inline double seconds(clock_t start) {
    clock_t finish = clock();
    double dt = finish - start;
    return dt / CLOCKS_PER_SEC;
  }

  NormalMixtureApproximation &
  NormalMixtureApproximationTable::approximate(int nu){
    std::vector<int>::const_iterator lower_bound = std::lower_bound(
        index_.begin(), index_.end(), nu);
    // *lower_bound is the first index element greater than or equal
    // *to nu.
    int position = lower_bound - index_.begin();
    if (*lower_bound == nu) return approximations_[position];

    clock_t start = clock();
    NegLogGamma target(nu);

    int nu0 = index_[position - 1];
    const NormalMixtureApproximation & approximation_0(
        approximations_[position-1]);
    int nu1 = index_[position];
    const NormalMixtureApproximation &approximation_1(
        approximations_[position]);

    double weight = (nu - nu0) / (1.0 * (nu1 - nu0));
    double precision = 1e-6;
    int max_evals = 20000;
    double stepsize = .5 / sqrt(nu);

    // std::string sep(80, '-');
    // cerr << sep << endl
    //      << "building table element for nu = " << nu << endl
    //      << "previous table element is for nu = " << nu0 << endl
    //      << approximation_0 << endl
    //      << "next table element is for nu = " << nu1 << endl
    //      << approximation_1 << endl
    //      << "weight on nu1 is " << weight << endl
    //     ;

    if(approximation_0.dim() == approximation_1.dim()) {
      Vector mu = linear_combination(
          1-weight, approximation_0.mu(),
          weight, approximation_1.mu());
      Vector sigma = linear_combination(
          1-weight, approximation_0.sigma(),
          weight, approximation_1.sigma());
      Vector weights = linear_combination(
          1-weight, approximation_0.weights(),
          weight, approximation_1.weights());
      NormalMixtureApproximation approximation(mu, sigma, weights);
      double kl = approximation.kullback_leibler(target);
      if(kl < 1e-5) {
        add(nu, approximation);
        // cerr << "found approximation by linear interpolation: " << endl
        //      << approximation << endl
        //      << "approximation took " << seconds(start) << " seconds" << endl;
        return this->approximate(nu);
      } else {
        // Use direct approximation because linear interpolation is
        // too imprecise.
        int number_of_components = approximation_0.dim();
        mu = -log(nu);
        sigma = sqrt(1.0/nu);
        weights = 1.0/number_of_components;
        NormalMixtureApproximation better_approximation(
            target, mu, sigma, weights,
            precision, max_evals, stepsize);
        // cerr << "linear interpolation failed.  kl = " << kl
        //      << ".   Found approximation "
        //      << "by direct optimization." << endl
        //      << better_approximation << endl
        //      << "approximation took " << seconds(start) << " seconds." << endl;
        add(nu, better_approximation);
        return this->approximate(nu);
      }
    } else {
      // Could not do linear interpolation because nu fell between two
      // approximations of different dimension.

      int number_of_components = approximation_0.dim();
      Vector mu(number_of_components, -log(nu));
      Vector sigma(number_of_components, 1.0/sqrt(nu));
      Vector weights(number_of_components, 1.0/number_of_components);
      NormalMixtureApproximation approximation(
          target, mu, sigma, weights, precision, max_evals, stepsize);
      add(nu, approximation);
        // cerr << "Linear interpolation was not possible because of differences "
        //      << "in dimension.  Found approximation "
        //      << "by direct optimization." << endl
        //      << approximation << endl
        //      << "approximation took " << seconds(start) << " seconds." << endl;

      return this->approximate(nu);
    }
  }

}
