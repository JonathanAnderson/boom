/*
  Copyright (C) 2005-2012 Steven L. Scott

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

#ifndef BOOM_COSINE_POISSON_PROCESS_HPP_
#define BOOM_COSINE_POISSON_PROCESS_HPP_

#include <Models/PointProcess/PoissonProcess.hpp>
#include <Models/Policies/ParamPolicy_2.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>

namespace BOOM {
  // The CosinePoissonProcess is an inhomogeneous Poisson process with
  // rate function lambda * (1 + cos(frequency * t)), where t is the
  // time in days since Jan 1 1970..  It is mainly useful for testing
  // code involving inhomogeneous processes.
  class CosinePoissonProcess
      : public PoissonProcess,
        public ParamPolicy_2<UnivParams, UnivParams>,
        public IID_DataPolicy<PointProcess>,
        public PriorPolicy
  {
   public:
    CosinePoissonProcess(double lambda = 1.0, double frequency = 1.0);
    virtual CosinePoissonProcess * clone()const;

    virtual double event_rate(const DateTime &t)const;
    virtual double expected_number_of_events(
        const DateTime &t0, const DateTime &t1)const;

    // Adding data is a no-op since this
    virtual void add_exposure_window(const DateTime &t0, const DateTime &t1){}
    virtual void add_event(const DateTime &t){}

    double lambda() const;
    double frequency() const;

    virtual PointProcess simulate(
        const DateTime &t0,
        const DateTime &t1,
        boost::function<Data*()> mark_generator = NullDataGenerator()) const;

   private:
    DateTime origin_;
  };

}  // namespace BOOM
#endif// BOOM_COSINE_POISSON_PROCESS_HPP_
