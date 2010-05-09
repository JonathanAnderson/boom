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

#ifndef BOOM_GAMMA_MODEL_HPP
#define BOOM_GAMMA_MODEL_HPP

#include <iosfwd>
#include <cpputil/Ptr.hpp>
#include <Models/ModelTypes.hpp>
#include <Models/DoubleModel.hpp>
#include "Sufstat.hpp"
#include <Models/Policies/SufstatDataPolicy.hpp>
#include <Models/Policies/ParamPolicy_2.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/EmMixtureComponent.hpp>

//======================================================================
namespace BOOM{
  class GammaSuf: public SufstatDetails<DoubleData>{
  public:

    // constructor
    GammaSuf();
    GammaSuf(const GammaSuf &);
    GammaSuf *clone() const;

    void clear();
    void Update(const DoubleData &dat);
    void update_raw_data(double y);
    void add_mixture_data(double y, double prob);

    double sum()const;
    double sumlog()const;
    double n()const;
    ostream & display(ostream &out)const;

    virtual void combine(Ptr<GammaSuf> s);
    virtual void combine(const GammaSuf & s);
    GammaSuf * abstract_combine(Sufstat *s){
      return abstract_combine_impl(this,s);    }
    virtual Vec vectorize(bool minimal=true)const;
    virtual Vec::const_iterator unvectorize(Vec::const_iterator &v,
					    bool minimal=true);
    virtual Vec::const_iterator unvectorize(const Vec &v,
					    bool minimal=true);
  private:
    double sum_, sumlog_, n_;
  };
  //======================================================================
  class GammaModelBase // Gamma Model, Chi-Square Model, Scaled Chi-Square
    : public SufstatDataPolicy<DoubleData, GammaSuf>,
      public DiffDoubleModel,
      public NumOptModel,
      public EmMixtureComponent
  {
  public:
    GammaModelBase();
    GammaModelBase(const GammaModelBase &);
    virtual GammaModelBase * clone()const=0;

    virtual double alpha()const=0;
    virtual double beta()const=0;
    virtual void add_mixture_data(Ptr<Data>, double prob);
//     double pdf(Ptr<Data> dp, bool logscale) const;
//     double pdf(double x, bool logscale) const;
    double Logp(double x, double &g, double &h, uint nd) const ;
    double sim() const;
  };
  //======================================================================

  class GammaModel
    : public GammaModelBase,
      public ParamPolicy_2<UnivParams, UnivParams>,
      public PriorPolicy
  {
  public:
    // if moments==true then this is a Gamma model with mean b and
    // shape parameter a.  Otherwise the mean is a/b (usual
    // parameterization)
    GammaModel(double a=1.0, double b=1.0, bool moments=false);
    GammaModel(const GammaModel &m);
    GammaModel *clone() const;

    Ptr<UnivParams> Alpha_prm();
    Ptr<UnivParams> Beta_prm();
    const Ptr<UnivParams> Alpha_prm()const;
    const Ptr<UnivParams> Beta_prm()const;

    double alpha()const;
    double beta()const;
    void set_alpha(double);
    void set_beta(double);
    void set_params(double a, double b);

    // probability calculations
    double Loglike(Vec &g, Mat &h, uint lev) const ;
    void mle();
  };

}
#endif  // GAMMA_MODEL_H