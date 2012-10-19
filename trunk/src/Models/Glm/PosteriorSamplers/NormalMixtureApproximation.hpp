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

#ifndef BOOM_NORMAL_MIXTURE_APPROXIMATION_HPP_
#define BOOM_NORMAL_MIXTURE_APPROXIMATION_HPP_

#include <numopt.hpp>
#include <LinAlg/Vector.hpp>
#include <distributions/rng.hpp>

namespace BOOM {

  class NormalMixtureApproximation {
   public:
    NormalMixtureApproximation(int n);
    NormalMixtureApproximation(const Vector &mu,
                               const Vector &sigma,
                               const Vector &weights);
    NormalMixtureApproximation(
        ScalarTarget target,
        const Vector &initial_mu,
        const Vector &initial_sigma,
        const Vector &initial_weights,
        double precision = 1e-6,   // the precision of the final KL divergence
        int max_evals = 20000,     // max number of times to evaluate logf
        double initial_stepsize = 10.0);

    void set(const Vector &mu, const Vector &sigma, const Vector &weights);

    // If the dimension of the approximation is k, then the first k
    // elements of theta are the k mu's, then the k values of
    // log(sigma), then the k-1 values of log(weights / weights[0]).
    void set(const Vector &theta);

    int dim()const{return mu_.size();}
    const Vector &mu()const{return mu_;}
    const Vector &sigma()const{return sigma_;}
    const Vector &weights()const{return weights_;}
    const Vector &log_weights()const{return log_weights_;}

    // Return the log of the approximating normal mixture density at x.
    double logp(double x)const;

    void unmix(RNG &rng, double u, double *mu, double *sigsq)const;

    double kullback_leibler()const;
    // Records the answer in state.
    double kullback_leibler(ScalarTarget target);

    int number_of_function_evaluations()const;

    ostream & print(ostream &out)const;
   private:
    // Ensures that mu, sigma, and weights are all the correct size.
    void check_sizes();
    void order_by_mu();
    Vector mu_;
    Vector sigma_;
    Vector weights_;
    Vector log_weights_;
    mutable Vector wsp_;

    // The kl distance between the approximation and the target.  This
    // is set automatically by the constructor that takes the target.
    // It is set to -infinity by the other constructors and there is
    // no way to modify it.
    double kullback_leibler_;
    int number_of_function_evaluations_;
  };

  inline ostream & operator<<(ostream &out,
                              const NormalMixtureApproximation &approximation){
    return approximation.print(out);}

  //======================================================================
  class ApproximationDistance {
   public:
    // A base class for a distance metric for measuring the closeness
    // between the NormalMixtureApproximation and the target function.
    ApproximationDistance(ScalarTarget logf,
                          const NormalMixtureApproximation &approximation,
                          double lower_limit,
                          double upper_limit,
                          double guess_at_mode);

    virtual ~ApproximationDistance() {}

    // The distance metric is a function of a set of parameters
    // contained in the vector theta.  A default method is provided,
    // but it can be over-ridden.
    virtual double operator()(const Vector &theta)const;

    double current_distance()const;

    // The integrand
    virtual double integrand(double x) const = 0;

    double logf(double x) const;
    double approximation(double x)const;
    double lower_limit() const;
    double upper_limit() const;

   private:
    ScalarTarget logf_;
    mutable NormalMixtureApproximation approx_;
    double lower_limit_;
    double upper_limit_;
    double guess_at_mode_;
  };

  class KullbackLeiblerDivergence : public ApproximationDistance {
   public:
    KullbackLeiblerDivergence(
        ScalarTarget logf, const NormalMixtureApproximation &approx,
        double lower_limit, double upper_limit, double guess_at_mode);
    virtual double integrand(double x)const;
  };

  class AbsNormDistance : public ApproximationDistance {
   public:
    AbsNormDistance(
        ScalarTarget logf, const NormalMixtureApproximation &approx,
        double lower_limit, double upper_limit, double guess_at_mode);
    virtual double integrand(double x)const;
  };

  //======================================================================

  class NormalMixtureApproximationTable {
   public:
    NormalMixtureApproximationTable();
    ~NormalMixtureApproximationTable(){}
    NormalMixtureApproximationTable(const NormalMixtureApproximationTable &rhs);
    NormalMixtureApproximationTable & operator=(
        const NormalMixtureApproximationTable &rhs);

    // Add an entry into the table.
    void add(int index, const NormalMixtureApproximation &spec);

    // Return the smallest and lagest indices contained in the table.
    int smallest_index()const;
    int largest_index()const;

    // If a numerical approximation exists at index_value, then return
    // it.  Otherwise return an interpolation between the two nearest
    // indices.
    NormalMixtureApproximation & approximate(int index_value);

   private:
    std::vector<int> index_;

    // These should all be of the same dimension.
    std::vector<NormalMixtureApproximation> approximations_;
  };

  // The density for -1 times the log of a gamma(nu, 1) random
  // variable.
  class NegLogGamma {
   public:
    NegLogGamma(double nu) : nu_(nu) {}
    double operator()(double y)const{
      return -nu_*y - exp(-y) - std::lgamma(nu_); }
   private:
    double nu_;
  };

}  // namespace BOOM
#endif //  BOOM_NORMAL_MIXTURE_APPROXIMATION_HPP_
