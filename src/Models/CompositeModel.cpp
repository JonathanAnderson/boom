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

#include "CompositeModel.hpp"

namespace BOOM{

  typedef CompositeModel CM;

  CM::CompositeModel(){}

  void CM::setup(){
    ParamPolicy::set_models(m_.begin(), m_.end());
  }

  CM::CompositeModel(const CM & rhs)
    : Model(rhs),
      ParamPolicy(rhs),
      DataPolicy(rhs),
      PriorPolicy(rhs)
  {
    uint S = rhs.m_.size();
    for(uint s=0; s<S; ++s) m_.push_back(rhs.m_[s]->clone());
    setup();
  }


  CM * CM::clone()const{return new CM(*this);}

//   void CM::initialize_params(){
//     for(uint s=0; s<m_.size(); ++s) m_[s]->initialize_params();}



  void CM::add_data(Ptr<CompositeData> dp){
    DataPolicy::add_data(dp);
    uint n = dp->dim();
    assert(n == m_.size());
    for(uint i=0; i<n; ++i) m_[i]->add_data(dp->get(i));
  }

  void CM::add_data(Ptr<Data> dp){
    Ptr<CompositeData> d = DAT(dp);
    add_data(d);
  }

  double CM::pdf(Ptr<Data> dp, bool logscale)const{
    return pdf(DAT(dp), logscale); }

  double CM::pdf(Ptr<CompositeData> dp, bool logscale)const{
    uint n = dp->dim();
    assert(n==m_.size());
    double ans=0;
    for(uint i=0; i<n; ++i) ans+= m_[i]->pdf(dp->get(i), true);
    return logscale ? ans : exp(ans);
  }

}
