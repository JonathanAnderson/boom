/*
  Copyright (C) 2005-2013 Steven L. Scott

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

#ifndef BOOM_HIERARCHICAL_GAMMA_MODEL_HPP_
#define BOOM_HIERARCHICAL_GAMMA_MODEL_HPP_

#include <ostream>
#include <vector>

#include <Models/GammaModel.hpp>
#include <Models/Policies/CompositeParamPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>

namespace BOOM {

  class HierarchicalGammaData : public Data {
   public:
    HierarchicalGammaData(const GammaSuf &suf) : suf_(suf){}
    virtual HierarchicalGammaData * clone() const {
      return new HierarchicalGammaData(*this);
    }
    virtual ostream &display(ostream &out) const {
      return out << suf_;
    }
    const GammaSuf &suf()const {return suf_;}

   private:
    GammaSuf suf_;
  };

  class HierarchicalGammaModel
      : public CompositeParamPolicy,
        public PriorPolicy {
   public:
    HierarchicalGammaModel(
        const std::vector<int> &number_of_observations_per_group,
        const std::vector<double> &sum_of_observations_per_group,
        const std::vector<double> &sum_of_logs_of_positives_per_group);

    HierarchicalGammaModel(const HierarchicalGammaModel &rhs);
    virtual HierarchicalGammaModel * clone()const;

    virtual void clear_methods();

    // Removes all data_level_models and their associated parameters
    // and data.
    virtual void clear_data();

    // Adds the data_level_models from rhs to this.
    virtual void combine_data(const Model &rhs, bool just_suf = true);

    // Creates a new data_level_model with data assigned.
    virtual void add_data(Ptr<Data>);

    // Returns the number of data_level_models managed by this model.
    int number_of_groups()const;

    GammaModel * prior_for_mean_parameters();
    GammaModel * prior_for_shape_parameters();
    GammaModel * data_model(int i);

    double mean_parameter_prior_mean()const;
    double mean_parameter_prior_shape()const;
    double shape_parameter_prior_mean()const;
    double shape_parameter_prior_shape()const;

   private:
    // Adds the prior models to ParamPolicy
    void initialize();
    void get_initial_parameter_estimates(Ptr<GammaModel>)const;
    void add_data_level_model(Ptr<GammaModel> data_model);

    Ptr<GammaModel> prior_for_mean_parameters_;
    Ptr<GammaModel> prior_for_shape_parameters_;
    std::vector<Ptr<GammaModel> > data_models_;
  };

}  // namespace BOOM
#endif  //  BOOM_HIERARCHICAL_GAMMA_MODEL_HPP_
