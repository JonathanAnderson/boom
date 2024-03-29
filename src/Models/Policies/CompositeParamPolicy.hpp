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
#ifndef BOOM_COMPOSITE_MODEL_PARAM_POLICY
#define BOOM_COMPOSITE_MODEL_PARAM_POLICY


/*======================================================================
  Use this policy when the Model is defined as a composite of several
  sub-models.  E.g. latent variable models.  If the model just happens
  to have many paramters use ManyParamPolicy instead.
  ======================================================================*/

#include<cpputil/Ptr.hpp>
#include <Models/ModelTypes.hpp>
#include <Models/ParamTypes.hpp>
namespace BOOM{

  class CompositeParamPolicy
    : virtual public Model
  {
  public:
    typedef CompositeParamPolicy ParamPolicy;
    CompositeParamPolicy();

    template <class FwdIt>                // *FwdIt is a Ptr<Model>
    CompositeParamPolicy(FwdIt b, FwdIt e);

    CompositeParamPolicy(const CompositeParamPolicy &rhs);  // components not copied
    CompositeParamPolicy * clone()const=0;
    CompositeParamPolicy & operator=(const CompositeParamPolicy &);

    void add_model(Ptr<Model>);
    void drop_model(Ptr<Model>);
    void clear();

    template<class Fwd>
    void set_models(Fwd b, Fwd e);

    ParamVec t();
    const ParamVec t()const;

    void add_params(Ptr<Params>);

  private:
    bool have_model(Ptr<Model>)const;
    std::vector<Ptr<Model> > models_;
    ParamVec t_;
  };

  template <class Fwd>
  void CompositeParamPolicy::set_models(Fwd b, Fwd e){
    models_.clear();
    std::copy(b,e, back_inserter(models_));
    t_.clear();
    for(uint i =0; i<models_.size(); ++i){
      ParamVec tmp(models_[i]->t());
      std::copy(tmp.begin(), tmp.end(), back_inserter(t_));}}


  template <class Fwd>
  CompositeParamPolicy::CompositeParamPolicy(Fwd b, Fwd e)
    : models_(),
      t_()
  {
    set_models(b,e);
  }

}
#endif// BOOM_COMPOSITE_MODEL_PARAM_POLICY
