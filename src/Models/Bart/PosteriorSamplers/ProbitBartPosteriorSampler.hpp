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

#ifndef BOOM_PROBIT_BART_POSTERIOR_SAMPLER_HPP_
#define BOOM_PROBIT_BART_POSTERIOR_SAMPLER_HPP_

#include <Models/Bart/ProbitBartModel.hpp>
#include <Models/Bart/ResidualRegressionData.hpp>
#include <Models/Bart/PosteriorSamplers/BartPosteriorSampler.hpp>

namespace BOOM {
  namespace Bart {
    class ProbitSufficientStatistics;

    // One instance of ProbitResidualData considers the local
    // sufficient statistics for N success failure trials where y()
    // successes were observed.  N == 1 is an important special case.
    // Each trial is associated with a latent variable z[i] ~
    // N(sum_of_trees, 1).  The trial is a success if z[i] > 0 and a
    // failure otherwise.
    //
    // This class maintains the locally sufficient statistics n and
    // sum_i z[i].
    class ProbitResidualData : public ResidualRegressionData {
     public:
      ProbitResidualData(Ptr<BinomialRegressionData> data_point,
                         double original_prediction);
      int y() const {return original_data_->y();}
      int n() const {return original_data_->n();}
      virtual void add_to_residual(double value);
      virtual void add_to_probit_suf(ProbitSufficientStatistics &suf)const;
      double sum_of_residuals() const;
      void set_sum_of_residuals(double sum_of_residuals);

      // The value of the sum-of-trees for this data point.  It is
      // cheaper to maintain prediction here and adjust it as needed
      // than to recompute it each time it is needed.  The value of
      // the prediction is adjusted each time add_to_residual or
      // subtract_from_residual is called.
      double prediction() const {return prediction_;}
      void set_prediction(double value) { prediction_ = value; }

     private:
      const BinomialRegressionData *original_data_;

      // Sum(z[i]) - n * tree[i]
      double sum_of_latent_probit_residuals_;

      double prediction_;
    };

    //======================================================================
    class ProbitSufficientStatistics : public SufficientStatisticsBase {
     public:
      virtual ProbitSufficientStatistics * clone() const;
      virtual void clear();
      virtual void update(const ResidualRegressionData &abstract_data);
      virtual void update(const ProbitResidualData &data);
      int sample_size() const;
      double sum() const;
     private:
      // n_ is the number of Bernoulli observations.
      double n_;
      // sum_ is the sum of residuals of latent probits.
      double sum_;
    };
  } // namespace Bart

  class ProbitBartPosteriorSampler : public BartPosteriorSamplerBase {
   public:
    typedef Bart::ProbitResidualData DataType;

    ProbitBartPosteriorSampler(ProbitBartModel *model,
                               double prior_mean_guess,
                               double prior_mean_sd,
                               double prior_tree_depth_alpha,
                               double prior_tree_depth_beta);

    virtual void draw();
    virtual double draw_mean(Bart::TreeNode *leaf);
    virtual double log_integrated_likelihood(
        const Bart::SufficientStatisticsBase *suf)const;
    virtual void clear_residuals();
    virtual int residual_size()const;
    virtual Bart::ProbitResidualData * create_and_store_residual(int i);
    virtual Bart::ProbitSufficientStatistics * create_suf()const;

    void impute_latent_data();
    void impute_latent_data_point(DataType *data);
    virtual double log_integrated_probit_likelihood(
        const Bart::ProbitSufficientStatistics *suf)const;
   private:
    ProbitBartModel *model_;
    std::vector<boost::shared_ptr<DataType> > residuals_;
  };

}  // namespace BOOM

#endif  //  BOOM_PROBIT_BART_POSTERIOR_SAMPLER_HPP_
