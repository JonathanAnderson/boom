/*
  Copyright (C) 2008 Steven L. Scott

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
#include <Models/GaussianModelBase.hpp>
#include <distributions.hpp>
#include <Models/SufstatAbstractCombineImpl.hpp>

namespace BOOM{

  typedef GaussianSuf GS;

  GS::GaussianSuf()
      : sum_(0),
        sumsq_(0),
        n_(0)
  {}

  GS::GaussianSuf(double Sum, double Sumsq, double N)
    : sum_(Sum), sumsq_(Sumsq), n_(N)
  {}
  GS::GaussianSuf(const GS &rhs)
    : Sufstat(rhs),
      SufstatDetails<DataType>(rhs),
      sum_(rhs.sum_),
      sumsq_(rhs.sumsq_),
      n_(rhs.n_)
  {}

  GS* GS::clone()const{return new GS(*this);}

  void GS::Update(const DoubleData &X){
    const double & x = X.value();
    update_raw(x);}

  void GS::update_raw(double y){
    n_ += 1;
    sum_ += y;
    sumsq_ += y*y;
  }

  void GS::add_mixture_data(double y, double prob){
    n_+= prob;
    prob*=y;
    sum_+= prob;
    prob*=y;
    sumsq_ += prob;
  }

  double GS::sum()const{return sum_;}
  double GS::sumsq()const{return sumsq_;}
  double GS::n()const{return n_;}
  double GS::ybar()const{
    if(n_>0) return sum()/n();
    return 0.0; }

  double GS::sample_var()const{
    if(n_<=1)  return 0;
    double ss = sumsq()-n()*pow(ybar(),2);
    return ss/(n_-1);
  }

  void GS::clear(){ sum_ = sumsq_ = n_=0;}

  void GS::combine(Ptr<GS> s){
    n_ += s->n_;
    sum_ += s->sum_;
    sumsq_ += s->sumsq_;
  }

  void GS::combine(const GS & rhs){
    n_ += rhs.n_;
    sum_ += rhs.sum_;
    sumsq_ += rhs.sumsq_;
  }

  GaussianSuf * GS::abstract_combine(Sufstat *s){
    return abstract_combine_impl(this, s);}

  Vec GS::vectorize(bool)const{
    Vec ans(3);
    ans[0] = n_;
    ans[1] = sum_;
    ans[2] = sumsq_;
    return ans;
  }

  Vec::const_iterator GS::unvectorize(Vec::const_iterator &v, bool){
    n_ = *v;     ++v;
    sum_ = *v;   ++v;
    sumsq_ = *v; ++v;
    return v;
  }

  Vec::const_iterator GS::unvectorize(const Vec &v, bool minimal){
    Vec::const_iterator it = v.begin();
    return unvectorize(it,minimal);
  }

  ostream & GS::print(ostream &out)const{
    return out << n_ << " " << sum_ << " " << sumsq_;
  }

  //======================================================================
  GaussianModelBase::GaussianModelBase()
    : DataPolicy(new GaussianSuf())
  {}

  GaussianModelBase::GaussianModelBase(const std::vector<double> &y)
      : DataPolicy(new GaussianSuf())
  {
    DataPolicy::set_data_raw(y.begin(), y.end());
  }

  double GaussianModelBase::pdf(Ptr<Data> dp, bool logscale)const{
    double ans = logp(DAT(dp)->value());
    return logscale ? ans : exp(ans);
  }

  double GaussianModelBase::pdf(const Data * dp, bool logscale)const{
    double ans = logp(DAT(dp)->value());
    return logscale ? ans : exp(ans);
  }

  double GaussianModelBase::Logp(double x, double &g, double &h, uint nd)const{
    double m = mu();
    double ans = dnorm(x, m, sigma(), 1);
    if(nd>0) g = -(x-m)/sigsq();
    if(nd>1) h = -1.0/sigsq();
    return ans;
  }

  double GaussianModelBase::Logp(const Vec &x, Vec &g, Mat &h, uint nd)const{
    double X=x[0];
    double G(0),H(0);
    double ans = Logp(X,G,H,nd);
    if(nd>0) g[0]=G;
    if(nd>1) h(0,0)=H;
    return ans;
  }

  double GaussianModelBase::ybar()const{return suf()->ybar();}
  double GaussianModelBase::sample_var()const{return suf()->sample_var();}


  void GaussianModelBase::add_mixture_data(Ptr<Data> dp, double prob){
    double y = DAT(dp)->value();
    suf()->add_mixture_data(y, prob);
  }

  double GaussianModelBase::sim()const{ return rnorm(mu(), sigma()); }

  void GaussianModelBase::add_data_raw(double x){
    NEW(DoubleData, dp)(x);
    this->add_data(dp);
  }



}
