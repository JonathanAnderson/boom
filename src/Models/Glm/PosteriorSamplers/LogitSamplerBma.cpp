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

#include <Models/Glm/PosteriorSamplers/LogitSamplerBma.hpp>
#include <distributions.hpp>
#include <cpputil/math_utils.hpp>
#include <LinAlg/Selector.hpp>
#include <cpputil/seq.hpp>
#include <algorithm>
#include <stats/logit.hpp>
#include <Models/Glm/WeightedRegressionModel.hpp>

namespace BOOM{

  typedef LogitSamplerBma LSB;

  LSB::LogitSamplerBma(LogisticRegressionModel *mod,
		       Ptr<MvnBase> pri,
		       Ptr<VariableSelectionPrior> vs)
    : LogitSampler(mod,pri),
      mod_(mod),
      pri_(pri),
      vs_(vs),
      max_nflips_(mod_->xdim())
  {}

  double LSB::logpri()const{
    const Selector & inc(mod_->inc());
    double ans = vs_->logp(inc);
    if(inc.nvars() > 0){
      Spd ivar = inc.select(pri_->siginv());
      Vec mu = inc.select(pri_->mu());
      Vec beta = inc.select(mod_->beta());
      ans += dmvn(beta, mu, ivar, true);
    }
    return ans;
  }

  void LSB::draw(){
    impute_latent_data();
    draw_gamma();
    draw_beta_given_gamma();
  }

  void LSB::limit_model_selection(uint n){ max_nflips_ = n;}


  static inline bool keep_flip(double logp_old, double logp_new){
    if(!finite(logp_new)) return false;
    double pflip = logit_inv(logp_new - logp_old);
    double u = runif(0,1);
    return u < pflip ? true : false;
  }

  void LSB::draw_gamma(){
    // draws vector of 0's and 1's indicating which coefficients are
    // nonzero

    if(max_nflips_ == 0) return;

    Selector inc = mod_->inc();
    uint nv = inc.nvars_possible();
    double logp = log_model_prob(inc);
    if(!finite(logp)){
      ostringstream err;
      err << "LogitSamplerBma did not start with a legal configuration."
	  << endl
	  << "Selector vector:  " << inc << endl
	  << "beta:            " << mod_->beta() <<endl;
      throw_exception<std::runtime_error>(err.str());
    }

    std::vector<uint> flips = seq<uint>(0, nv-1);
    std::random_shuffle(flips.begin(), flips.end());
    uint hi = std::min<uint>(nv, max_nflips_);
    for(uint i=0; i<hi; ++i){
      uint I = flips[i];
      inc.flip(I);
      double logp_new = log_model_prob(inc);
      if( keep_flip(logp, logp_new)) logp = logp_new;
      else inc.flip(I);  // reject the flip, so flip back
    }
    mod_->coef().set_inc(inc);
  }
  //----------------------------------------------------------------------
  double LSB::log_model_prob(const Selector &g)const{
    double num = vs_->logp(g);
    if(num==BOOM::negative_infinity()) return num;

    Ominv = g.select(pri_->siginv());
    num += .5*Ominv.logdet();
    if(num == BOOM::negative_infinity()) return num;

    Vec mu = g.select(pri_->mu());
    Vec Ominv_mu = Ominv * mu;
    num -= .5*mu.dot(Ominv_mu);

    bool ok=true;
    iV_tilde_ = Ominv + g.select(suf()->xtx());
    Mat L = iV_tilde_.chol(ok);
    if(!ok)  return BOOM::negative_infinity();
    double denom = sum(log(L.diag()));  // = .5 log |Ominv|

    Vec S = g.select(suf()->xty()) + Ominv_mu;
    Lsolve_inplace(L,S);
    denom-= .5*S.normsq();  // S.normsq =  beta_tilde ^T V_tilde beta_tilde

    return num-denom;
  }

  //----------------------------------------------------------------------

  void LSB::draw_beta_given_gamma(){
    const Selector & inc(mod_->inc());
    Ominv = inc.select(pri_->siginv());
    Spd ivar = Ominv + inc.select(suf()->xtx());
    Vec b = inc.select(suf()->xty()) + Ominv * inc.select(pri_->mu());
    b = rmvn_suf(ivar, b);
    mod_->set_beta(b);
  }

}
