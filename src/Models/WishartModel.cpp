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

#include "WishartModel.hpp"
#include <distributions.hpp>
#include <cpputil/math_utils.hpp>
#include <cmath>
#include <numopt.hpp>
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <TargetFun/Loglike.hpp>
#include <Models/SufstatAbstractCombineImpl.hpp>


namespace BOOM{
  typedef WishartSuf WS;

  WS::WishartSuf(uint dim)
    : n_(0),
      sumldw_(0),
      sumW_(dim, 0.0)
  {}

  WS::WishartSuf(const WishartSuf &rhs)
    : Sufstat(rhs),
      SufstatDetails<SpdData>(rhs),
      n_(rhs.n_),
      sumldw_(rhs.sumldw_),
      sumW_(rhs.sumW_)
  {}

  WishartSuf *WS::clone() const{ return new WishartSuf(*this);}


  void WishartSuf::clear(){
    sumldw_=0.0;
    sumW_ =0.0;
    n_ = 0.0;  }

  void WishartSuf::Update(const SpdData &dp){
    const Spd &W(dp.value());
    sumldw_ += W.logdet();
    sumW_ += W;
    n_+= 1.0; }

  void WishartSuf::combine(Ptr<WishartSuf> s){
    n_ += s->n_;
    sumldw_ += s->sumldw_;
    sumW_ += s->sumW_;
  }

  void WishartSuf::combine(const WishartSuf & s){
    n_ += s.n_;
    sumldw_ += s.sumldw_;
    sumW_ += s.sumW_;
  }

  WishartSuf * WishartSuf::abstract_combine(Sufstat *s){
    return abstract_combine_impl(this,s);}

  Vec WishartSuf::vectorize(bool minimal)const{
    Vec ans = sumW_.vectorize(minimal);
    ans.push_back(n_);
    ans.push_back(sumldw_);
    return ans;
  }

  Vec::const_iterator WishartSuf::unvectorize(Vec::const_iterator &v,
                                              bool minimal){
    sumW_.unvectorize(v, minimal);
    n_ = *v;      ++v;
    sumldw_ = *v; ++v;
    return v;
  }

  Vec::const_iterator WishartSuf::unvectorize(const Vec &v, bool minimal){
    Vec::const_iterator it = v.begin();
    return unvectorize(it, minimal);
  }

  ostream &WishartSuf::print(ostream &out)const{
    return out << "n_ = " << n_ << endl
               << "sumldw_ = " << sumldw_ << endl
               << "sumW_ = " << endl << sumW_;
  }


  //======================================================================

  typedef WishartModel WM;

  WM::WishartModel(uint p)
    : ParamPolicy(new UnivParams(p+1), new SpdParams(p)),
      DataPolicy(new WS(p)),
      PriorPolicy()
  {}

  WM::WishartModel(uint p, double pri_df, double v)
    : ParamPolicy(new UnivParams(pri_df), new SpdParams(p, v*pri_df)),
      DataPolicy(new WS(p)),
      PriorPolicy()
  {}

  WM::WishartModel(double pri_df, const Spd &PriVarEst)
    : ParamPolicy(new UnivParams(pri_df), new SpdParams(PriVarEst*pri_df)),
      DataPolicy(new WS(PriVarEst.nrow())),
      PriorPolicy()
  {}

  WM::WishartModel(const WM &rhs)
    : Model(rhs),
      MLE_Model(rhs),
      ParamPolicy(rhs),
      DataPolicy(rhs),
      PriorPolicy(rhs),
      dLoglikeModel(rhs),
      SpdModel(rhs)
  {}

  WM *WM::clone() const{return new WM(*this);}

  void WishartModel::mle0(){
    Vec theta = vectorize_params();
    LoglikeTF target(this);
    max_nd0(theta, Target(target));
    unvectorize_params(theta);
  }

  void WishartModel::mle1(){
    Vec theta = vectorize_params();
    dLoglikeTF target(this);
    max_nd1(theta, Target(target), dTarget(target));
    unvectorize_params(theta);
  }

  double WishartModel::logp(const Spd &W) const{
    return dWish(W, sumsq(), nu(), true); }

  void WishartModel::initialize_params(){
    Spd mean(suf()->sumW());
    mean/= suf()->n();
    set_nu(2*mean.nrow()); // out of thin air
    set_sumsq((mean/nu()).inv());
  };

  Ptr<UnivParams> WM::Nu_prm(){return ParamPolicy::prm1();}
  const Ptr<UnivParams> WM::Nu_prm()const{
    return ParamPolicy::prm1();}

  Ptr<SpdParams> WM::Sumsq_prm(){return ParamPolicy::prm2();}
  const Ptr<SpdParams> WM::Sumsq_prm()const{
    return ParamPolicy::prm2();}

  const double & WM::nu() const{return Nu_prm()->value();}
  const Spd &WM::sumsq() const{return Sumsq_prm()->value();}
  void WM::set_nu(double Nu){Nu_prm()->set(Nu);}
  void WM::set_sumsq(const Spd &S){Sumsq_prm()->set(S);}



  Spd WishartModel::simdat(){ return rWish(nu(), sumsq()); }

  double WishartModel::Loglike(Vec &g, uint nd)const{
    const double log2 = 0.69314718055994529;
    const double logpi = 1.1447298858494002;
    int k=dim();
    const Spd  &SS(sumsq());
    double nu = this->nu();
    if(nu <k) return negative_infinity();
    double ldSS = 0;

    bool ok=true;
    ldSS = SS.logdet(ok);
    if(!ok) return negative_infinity();

    double n = suf()->n();
    double sumldw = suf()->sumldw();
    const Spd &sumW(suf()->sumW());

    double tab = traceAB(SS, sumW);
    double tmp1(0), tmp2(0);
    for(int i = 1; i<=k; ++i){
      double tmp = .5*(nu-i+1);
      tmp1+= lgamma(tmp);
      if(nd>0) tmp2+= digamma(tmp);
    }

    double ans = .5*( n*(-nu*k*log2 - .5*k*(k-1)*logpi -2*tmp1 + nu*ldSS)
 		      +(nu-k-1)*sumldw - tab);
    if(nd>0){
      double dnu = .5*( n*(-k*log2 - tmp2+ldSS) + sumldw);

      Spd SSinv(SS.inv());
      int m=0;
      for(int i=0; i<k; ++i){
 	for(int j=0; j<=i; ++j){
 	  g[m] = .5*n*nu * (i==j? SSinv(i,i) : 2*SSinv(i,j));
 	  g[m] -= .5*(i==j ? sumW(i,i) : 2* sumW(i,j));
 	  ++m; }}
      g[m] = dnu;
    }
    return ans;
  }


  double WishartModel::loglike() const{
    Vec g;
    return Loglike(g, 0);
  }

  double WishartModel::dloglike(Vec &g)const{
    return Loglike(g, 1);}

}
