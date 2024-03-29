/*
  Copyright (C) 2006 Steven L. Scott

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
#include "MnpBetaSampler.hpp"
#include <Models/MvnModel.hpp>
#include <Models/Glm/MultinomialProbitModel.hpp>
#include <distributions.hpp>

namespace BOOM{
  typedef MnpBetaSampler MBS;
  typedef MultinomialProbitModel MNP;
  MBS::MnpBetaSampler(MNP *Mod, Ptr<MvnModel> Pri)
    : mnp(Mod),
      pri(Pri),
      b0_fixed(true)
  {}

  void MBS::draw(){
    Spd ivar = mnp->xtx() + pri->siginv();
    Vec mean = mnp->xty() + pri->siginv()*pri->mu();
    mean = ivar.solve(mean);
    Vec beta = rmvn_ivar(mean, ivar);
    if(b0_fixed){
      uint start = 0;
      uint p = mnp->subject_nvars();
      Vec b0(beta.begin(), beta.begin()+p);
      for(uint i=0; i<mnp->Nchoices(); ++i){
	subvector(beta, start, start+p) -= b0;
	start+=p;}}
    mnp->set_beta(beta);
  }

  double MBS::logpri()const{return pri->logp(mnp->beta());}

  void MBS::fix_beta0(bool yn){b0_fixed = yn;}
}
