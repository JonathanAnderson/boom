/*
  Copyright (C) 2007 Steven L. Scott

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

#include <Models/Glm/PosteriorSamplers/LogitSampler.hpp>
#include <Models/Glm/PosteriorSamplers/draw_logit_lambda.hpp>
#include <Models/Glm/WeightedRegressionModel.hpp>
#include <distributions.hpp>

namespace BOOM{

  namespace{
    const double small_number = 1e-5;
  }

  typedef LogitSampler LS;
  LS::LogitSampler(LogisticRegressionModel *mod,
		   Ptr<MvnBase> pri)
    : mod_(mod),
      pri_(pri),
      suf_(new WeightedRegSuf(pri->dim()))
  {}

  void LS::draw(){
    impute_latent_data();
    draw_beta();
  }

  double LS::logpri()const{
    return pri_->logp(mod_->beta());
  }

  typedef BinaryRegressionData BRD;

  void LS::impute_latent_data(){
    double log_alpha = mod_->log_alpha();
    const std::vector<Ptr<BRD> > & dat(mod_->dat());
    uint n = dat.size();
    suf_->clear();
    Vec beta(mod_->beta());
    for(uint i=0; i<n; ++i){
      Ptr<BRD> dp = dat[i];
      const Vec &x(dp->x());
      double eta = mod_->predict(x) + log_alpha;
      double z = draw_z(dp->y(), eta);
      double lam = draw_lambda(fabs(z- eta));
      suf_->add_data(x,z, 1.0/lam);
    }
  }

  void LS::draw_beta(){
    ivar = pri_->siginv() + suf_->xtx();
    ivar_mu = pri_->siginv() * pri_->mu() + suf_->xty();
    ivar_mu = rmvn_suf(ivar, ivar_mu);
    mod_->set_beta(ivar_mu);
  }

  double LS::draw_z(bool y, double eta)const{
    double trun_prob = plogis(0, eta);
    double u = y ? runif(trun_prob,1) : runif(0,trun_prob);
    return qlogis(u,eta);
  }

  double LS::draw_lambda(double r)const{
    return Logit::draw_lambda_mt(rng(), r);
  }

}
