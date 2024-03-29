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
#ifndef BOOM_WEEKLY_CYCLE_POISSON_PROCESS_HPP_
#define BOOM_WEEKLY_CYCLE_POISSON_PROCESS_HPP_

#include <Models/PointProcess/PoissonProcess.hpp>
#include <Models/PointProcess/PointProcess.hpp>
#include <Models/Sufstat.hpp>
#include <Models/Policies/ParamPolicy_4.hpp>
#include <Models/Policies/SufstatDataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>

namespace BOOM{

  class WeeklyCyclePoissonSuf : public SufstatDetails<PointProcess> {
   public:
    WeeklyCyclePoissonSuf();
    virtual WeeklyCyclePoissonSuf * clone()const;
    virtual void clear();

    void Update(const PointProcess &data);
    void add_exposure_window(const DateTime &t0, const DateTime &t1);
    void add_event(const DateTime &t);

    WeeklyCyclePoissonSuf * combine(Ptr<WeeklyCyclePoissonSuf>);
    WeeklyCyclePoissonSuf * combine(const WeeklyCyclePoissonSuf &);
    virtual WeeklyCyclePoissonSuf * abstract_combine(Sufstat *s);

    virtual Vec vectorize(bool minimal = true)const;
    virtual Vec::const_iterator unvectorize(Vec::const_iterator &v,
                                            bool minimal = true);
    virtual Vec::const_iterator unvectorize(const Vec &v,
                                            bool minimal = true);
    virtual ostream & print(ostream &out)const;

    Vec daily_event_count()const;
    Vec weekday_hourly_event_count()const;
    Vec weekend_hourly_event_count()const;

    // Returns a matrix where the (day, hour) element gives the total
    // exposure time (measured in fractoins of a day) for that hour in
    // that day of the week.
    const Mat &exposure()const;
    const Mat &count()const;
   private:
    // Keeps track of the number of events that take place during each
    // hour of the week.  Indexed by (day, hour).
    Mat count_;

    // Keeps track of the number of hours (including fractional hours)
    // exposed during each hour of the week.  Time in each cell is
    // measured in days (not hours).
    Mat exposure_;

    static const Vec one_7;
    static const Vec one_24;
  };

  // A Poisson process containing a day of week and hour of day cycle.
  class WeeklyCyclePoissonProcess
      : public PoissonProcess,
        public ParamPolicy_4<UnivParams,     // Weekly rate
                             VectorParams,   // Daily factor
                             VectorParams,   // Weekday hourly factor
                             VectorParams>,  // Weekend hourly factor
        public SufstatDataPolicy<PointProcess, WeeklyCyclePoissonSuf>,
        public PriorPolicy,
        public LoglikeModel
  {
   public:
    WeeklyCyclePoissonProcess();
    WeeklyCyclePoissonProcess * clone()const;

    virtual double loglike()const;
    virtual void mle();

    virtual double event_rate(const DateTime &t)const;
    double event_rate(DayNames day, int hour)const;

    virtual double expected_number_of_events(const DateTime &t0,
                                             const DateTime &t1)const;
    double average_daily_rate()const;
    void set_average_daily_rate(double lambda);

    const Vec &day_of_week_pattern()const;        // sums to 7
    void set_day_of_week_pattern(const Vec &pattern);

    const Vec &weekday_hourly_pattern()const;     // sums to 24
    void set_weekday_hourly_pattern(const Vec &pattern);

    const Vec &weekend_hourly_pattern()const;     // sums to 24
    void set_weekend_hourly_pattern(const Vec &pattern);

    virtual PointProcess simulate(
        const DateTime &t0,
        const DateTime &t1,
        boost::function<Data*()> mark_generator = NullDataGenerator())const;

    Ptr<UnivParams> average_daily_event_rate_prm();
    const Ptr<UnivParams> average_daily_event_rate_prm()const;
    Ptr<VectorParams> day_of_week_cycle_prm();
    const Ptr<VectorParams> day_of_week_cycle_prm()const;
    Ptr<VectorParams> weekday_hour_of_day_cycle_prm();
    const Ptr<VectorParams> weekday_hour_of_day_cycle_prm()const;
    Ptr<VectorParams> weekend_hour_of_day_cycle_prm();
    const Ptr<VectorParams> weekend_hour_of_day_cycle_prm()const;

    void add_data_raw(const PointProcess &);
    virtual void add_exposure_window(const DateTime &t0, const DateTime &t1);
    virtual void add_event(const DateTime &t);
   private:
    const Vec &hourly_pattern(int day)const;
    void maximize_average_daily_rate();
    void maximize_daily_pattern();
    void maximize_hourly_pattern();
  };


}
#endif  // BOOM_WEEKLY_CYCLE_POISSON_PROCESS_HPP_
