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

#include "MarkovConjSampler.hpp"
#include <distributions.hpp>

namespace BOOM{

  typedef MarkovConjSampler MCS;

  MCS::MarkovConjSampler(MarkovModel *Mod,
			 Ptr<ProductDirichletModel> Q,
			 Ptr<DirichletModel> pi0)
    : mod_(Mod),
      Q_(Q),
      pi0_(pi0)
    {
      mod_->free_pi0();
    }

  MCS::MarkovConjSampler(MarkovModel * Mod,
			 Ptr<ProductDirichletModel> Q)
    : mod_(Mod),
      Q_(Q)
    {}

  MCS::MarkovConjSampler(MarkovModel *Mod,
                         const Mat & Nu,
                         const Vec & nu)
       : mod_(Mod),
         Q_(new ProductDirichletModel(Nu)),
         pi0_(new DirichletModel(nu))
   {
     mod_->free_pi0();
   }

  MCS::MarkovConjSampler(MarkovModel * Mod,
                         const Mat & Nu)
       : mod_(Mod),
         Q_(new ProductDirichletModel(Nu))
   {}


  MCS * MCS::clone()const{return new MCS(*this);}

  double MCS::logpri()const{
    const Mat &Nu(this->Nu());
    const Mat &Q(mod_->Q());
    assert(Nu.same_dim(Q));
    uint S = Nu.nrow();
    double ans = 0;
    for(uint s=0; s<S; ++s){
      ans += ddirichlet(Q.row(s), Nu.row(s), true);
    }

    if(mod_->pi0_fixed()) return ans;
    check_pi0();

    ans += ddirichlet(mod_->pi0(), this->nu(), true);
    return ans;
  }


  void MCS::draw(){
    const Mat & Nu(this->Nu());
    Mat Q(mod_->Q());
    const Mat & N(mod_->suf()->trans());
    assert(Nu.same_dim(Q));
    uint S = Nu.nrow();
    for(uint s=0; s<S; ++s){
      wsp = Nu.row(s) + N.row(s);
      Q.row(s) = rdirichlet_mt(rng(), wsp);
    }
    mod_->set_Q(Q);

//     cout << "drawing Q:  trans = " << endl
//          << N <<endl;

    if(mod_->pi0_fixed()) return;
    check_pi0();
    wsp = this->nu() + mod_->suf()->init();
    //    cout << "drawing pi0:  nu = " << wsp << endl;
    mod_->set_pi0( rdirichlet_mt(rng(), wsp));
  }


  void MCS::find_posterior_mode(){
    const Mat & Nu(this->Nu());
    Mat Q(mod_->Q());
    const Mat & N(mod_->suf()->trans());
    assert(Nu.same_dim(Q));
    uint S = Nu.nrow();
    for(uint s=0; s<S; ++s){
      wsp = Nu.row(s) + N.row(s);
      Q.row(s) = mdirichlet(wsp);
    }
    mod_->set_Q(Q);

    if(mod_->pi0_fixed()) return;
    check_pi0();
    wsp = this->nu() + mod_->suf()->init();
    mod_->set_pi0( mdirichlet(wsp));
  }


  const Mat & MCS::Nu()const{ return Q_->Nu(); }
  const Vec & MCS::nu()const{
    check_nu();
    return pi0_->nu();
  }

  void MCS::check_pi0()const{
    if(!!pi0_) return;
    ostringstream err;
    err << "A Markov chain model has a free initial distribution "
	<< "parameter (pi0) that was not assigned a prior."
	<< endl
	<< "Prior for transition counts was:" <<endl
	<< Nu() << endl;
    throw_exception<std::runtime_error>(err.str());
  }

  void MCS::check_nu()const{
    if(!!pi0_) return;
    ostringstream err;
    err << "MarkovConjugateSampler::nu()" << endl
	<< "No prior distribution was set" << endl;
    throw_exception<std::runtime_error>(err.str());
  }


}
