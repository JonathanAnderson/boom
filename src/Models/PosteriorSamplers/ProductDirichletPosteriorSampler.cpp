/*
  Copyright (C) 2005-2009 Steven L. Scott

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
#include <Models/PosteriorSamplers/DirichletPosteriorSampler.hpp>
#include <Models/PosteriorSamplers/ProductDirichletPosteriorSampler.hpp>
#include <Samplers/ScalarSliceSampler.hpp>
#include <cpputil/math_utils.hpp>

namespace BOOM{

typedef ProductDirichletPosteriorSampler PDPS;

void PDPS::draw(){

  const Mat & sumlog(m_->suf()->sumlog());
  double nobs(m_->suf()->n());
  Mat Nu(m_->Nu());
  uint d= nrow(Nu);
  for(uint i=0; i<d; ++i){
    Vec sumlog_i(sumlog.row(i));
    Vec nu(Nu.row(i));
    for(uint j=0; j<d; ++j){
      DirichletLogp logp(j, nu, sumlog_i, nobs,
                         phi_row_prior_[i], alpha_row_prior_[i],
                         min_nu_);
      ScalarSliceSampler sam(logp, true);
      sam.set_lower_limit(min_nu_);
      nu[j]= sam.draw(nu[j]);
    }
    Nu.row(i) = nu;
  }

  m_->set_Nu(Nu);
}

double PDPS::logpri()const{
  const Mat & Nu(m_->Nu());
  uint d = nrow(Nu);
  Vec phi(d);
  double ans=0;
  for(uint i=0; i<d; ++i){
    double a = sum(Nu.row(i));
    if(a <=0) return BOOM::negative_infinity();
    phi = Nu.row(i);
    for(uint j=0; j<d; ++j) {
      if(phi[j] < min_nu_) {
        return BOOM::negative_infinity();
      }
    }
    phi/=a;
    ans += alpha_row_prior_[i]->logp(a);
    ans += phi_row_prior_[i]->logp(phi);
    ans -= (d-1) * log(a);
  }
  return ans;
}

}
