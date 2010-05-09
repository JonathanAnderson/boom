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

#ifndef POISSON_MODEL_H
#define POISSON_MODEL_H

#include "ModelTypes.hpp"
#include "EmMixtureComponent.hpp"
#include "Sufstat.hpp"
#include <Models/Policies/ParamPolicy_1.hpp>
#include <Models/Policies/SufstatDataPolicy.hpp>
#include <Models/Policies/ConjugatePriorPolicy.hpp>

//----------------------------------------------------------------------//
namespace BOOM{
  class GammaModel;
  class PoissonGammaSampler;

  class PoissonSuf : public SufstatDetails<IntData>{
  public:
    // constructor
    PoissonSuf();
    PoissonSuf(const PoissonSuf &rhs);
    PoissonSuf *clone() const;

    void clear();
    double sum()const;
    double n()const;
    double lognc()const;

    void Update(const IntData & dat);
    void add_mixture_data(double y, double prob);
    void combine(Ptr<PoissonSuf>);
    void combine(const PoissonSuf &);
    PoissonSuf * abstract_combine(Sufstat *s){
      return abstract_combine_impl(this,s); }

    virtual Vec vectorize(bool minimal=true)const;
    virtual Vec::const_iterator unvectorize(Vec::const_iterator &v,
					    bool minimal=true);
    virtual Vec::const_iterator unvectorize(const Vec &v,
					    bool minimal=true);

  private:
    double sum_, n_, lognc_;  // log nc is the log product of x-factorials
  };
  //----------------------------------------------------------------------//

  class PoissonModel : public ParamPolicy_1<UnivParams>,
		       public SufstatDataPolicy<IntData, PoissonSuf>,
		       public ConjugatePriorPolicy<PoissonGammaSampler>,
		       public NumOptModel,
		       public EmMixtureComponent
  {
  public:

    PoissonModel(double lam=1.0);
    PoissonModel(const std::vector<uint> &);
    PoissonModel(const PoissonModel &m);
    PoissonModel *clone() const;

    virtual void mle();
    virtual double Loglike(Vec &g, Mat &h, uint nd)const;

    Ptr<UnivParams> Lam();
    const Ptr<UnivParams> Lam()const;
    double lam()const;
    void set_lam(double);

    // probability calculations
    virtual double pdf(Ptr<Data> x, bool logscale) const;
    double pdf(uint x, bool logscale) const;

    // moments and summaries:
    double mean()const;
    double var()const;
    double sd()const;
    double simdat() const;

    void set_conjugate_prior(Ptr<GammaModel>);
    void set_conjugate_prior(Ptr<PoissonGammaSampler>);

    virtual void add_mixture_data(Ptr<Data>,  double prob);
  };


}
#endif