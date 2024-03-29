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
#include "MLAuxMixSampler.hpp"
#include <distributions.hpp>       // for rlexp,dnorm,rmvn
#include <Models/Glm/MultinomialLogitModel.hpp>
#include <Models/MvnBase.hpp>
#include <Models/Glm/ChoiceData.hpp>

namespace BOOM{
  typedef MLAuxMixSampler AUX;
  typedef MultinomialLogitModel MLM;

  AUX::MLAuxMixSampler(MLM *Mod, Ptr<MvnBase> Pri, uint nthreads)
    : mod_(Mod),
      pri(Pri)
  {
    Ptr<VariableSelectionPrior> vp(0);
    sam = new MLVS(mod_, pri, vp, nthreads, false);
    sam->supress_model_selection();
  }
  void AUX::draw(){ sam->draw();}
  double AUX::logpri()const{ return pri->logp(mod_->beta());}
}
