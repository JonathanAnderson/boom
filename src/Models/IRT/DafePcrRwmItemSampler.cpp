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

#include "DafePcrRwm.hpp"
#include <Models/MvnModel.hpp>
#include <Models/MvtModel.hpp>
#include <Models/IRT/PartialCreditModel.hpp>

#include <Samplers/MetropolisHastings.hpp>

#include <TargetFun/Loglike.hpp>

#include <cpputil/ParamHolder.hpp>
#include <cpputil/math_utils.hpp>

#include <boost/bind.hpp>

namespace BOOM{
  namespace IRT{
    typedef DafePcrRwmItemSampler ISAM;
    typedef PartialCreditModel PCR;

    class ItemLoglikeTF{
    public:
      ItemLoglikeTF(Ptr<PCR>);
      ItemLoglikeTF * clone()const;
      double operator()(const Vec &beta)const;
    private:
      Ptr<PCR> pcr;
      Ptr<VectorParams> v;
      mutable Vec wsp;
    };

    typedef ItemLoglikeTF TF;
    //------------------------------------------------------------
    TF * TF::clone()const{return new TF(*this);}

    TF::ItemLoglikeTF(Ptr<PCR> item)
      : pcr(item),
	v(item->Beta_prm()),
	wsp(item->beta())
    {}

    double TF::operator()(const Vec &b)const{
      ParamHolder ph(b, v, wsp);
      if(pcr->a()<= 0.0) return BOOM::negative_infinity();
      double ans = pcr->loglike();
      return ans;
    }


    //======================================================================

    struct Logp{
      Logp(const TF &F, Ptr<MvnModel> P) : f(F), pri(P){}
      double operator()(const Vec &x)const{ return f(x) + pri->logp(x);}
      const TF & f;
      Ptr<MvnModel> pri;
    };

    ISAM::DafePcrRwmItemSampler(Ptr<PCR> item, Ptr<MvnModel> Prior, double Tdf)
      : mod(item),
	prior(Prior),
	sigsq(1.644934066848226), // pi^2/6
	xtx(mod->nlevels()),
	ivar(mod->nlevels())
    {
      TF loglike(mod);
      Logp target(loglike, prior);
      uint dim = mod->beta().size();

      prop = new MvtRwmProposal(Spd(dim).Id(), Tdf);
      sampler = new MetropolisHastings(target,prop);
    }

    void ISAM::draw(){
      get_moments();
      prop->set_ivar(ivar);
      b = sampler->draw(mod->beta());
      mod->set_beta(b);
    }

    double ISAM::logpri()const{
      return prior->pdf(mod->beta(), true);}

    void ISAM::get_moments(){
      xtx=0.0;
      const SubjectSet &subjects(mod->subjects());
      for_each(subjects.begin(), subjects.end(),
	       boost::bind(&ISAM::accumulate_moments, this, _1));

      ivar = prior->siginv() + xtx/sigsq;
    }

    void ISAM::accumulate_moments(Ptr<Subject> s){
      const Mat &X(mod->X(s->Theta()));
      xtx.add_inner(X);
    }

  }
}
