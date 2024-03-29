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
#ifndef BOOM_MVN_MEAN_SAMPLER_HPP
#define BOOM_MVN_MEAN_SAMPLER_HPP
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/ParamTypes.hpp>
#include <Models/MvnBase.hpp>

namespace BOOM{
  class MvnModel;
  class VectorParams;
  class SpdParams;

  //____________________________________________________________
  class MvnConjMeanSampler : public PosteriorSampler{
    // assumes y~N(mu, Sig) with mu~N(mu0, Sig/kappa)
    // draws mu given y, Sigma, mu0, kappa
  public:
    MvnConjMeanSampler(MvnModel *Mod);  // improper: mu0 = 0 kappa = 0;
    MvnConjMeanSampler(MvnModel *Mod,
                       Ptr<VectorParams> Mu0,
                       Ptr<UnivParams> Kappa);
    MvnConjMeanSampler(MvnModel *Mod,
                       const Vec &Mu0,
                       double Kappa);

    virtual double logpri()const;  // p(mu|Sig)
    virtual void draw();
  private:
    MvnModel *mvn;
    Ptr<VectorParams> mu0;
    Ptr<UnivParams> kappa;
  };
  //____________________________________________________________

  class MvnMeanSampler : public PosteriorSampler{
    // assumes y~N(mu, Sigma) with mu~N(mu0, Omega)
  public:

    MvnMeanSampler(MvnModel *Mod,
		   Ptr<VectorParams> Mu0,
		   Ptr<SpdParams> Omega);

    MvnMeanSampler(MvnModel *Mod,
		   Ptr<MvnBase> Pri);

    MvnMeanSampler(MvnModel *Mod,
		   const Vec & Mu0,
		   const Spd & Omega);
    double logpri()const;
    void draw();
  private:
    MvnModel *mvn;
    Ptr<MvnBase> mu_prior_;
  };
  //____________________________________________________________
}
#endif// BOOM_MVN_MEAN_SAMPLER_HPP
