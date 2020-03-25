/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Nicolai Lassesen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file fxforwardpointtermstructure.h
    \brief Foreign exchange forward points term structure
*/

#ifndef quantlib_fx_fwd_points_term_structure_h
#define quantlib_fx_fwd_points_term_structure_h

#include <ql/exchangerate.hpp>
#include <ql/forwardexchangerate.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <vector>

namespace QuantLib {

    //! Foreign exchange forward points term structure
    /*! This class performs interpolation of forward points for valution of
        foreign exchange forwards.

        \ingroup fxtermstructures

        \test TODO.
    */

    class FxForwardPointTermStructure : public TermStructure {
      public:
        FxForwardPointTermStructure(const Date& referenceDate,
                                    const ExchangeRate& spotExchangeRate,
                                    const DayCounter& dayCounter,
                                    const Calendar& calendar);
        const Currency& source() const;
        const Currency& target() const;

        /*! \name Forward points
        These methods return the interpolated forward points for
        a given date or time. In the former case, the time is
        calculated as a fraction of year from the reference date.
        */
        //@{
        /*! The resulting forward points has the curve's daycounting rule.
         */
        Decimal forwardPoints(const Date& d, bool extrapolate = false) const;

        /*! The resulting forward points has the same day-counting rule
            used by the term structure. The same rule should be used
            for calculating the passed time t.
        */
        Decimal forwardPoints(Time t, bool extrapolate = false) const;
        //@}

        /*! \name Forward exchange rates
        These methods return the interpolated forward exchange rate for
        a given date or time. In the former case, the time is calculated
        as a fraction of year from the reference date.
        */
        //@{
        /*! The resulting exchange rate has the curve's daycounting rule.
         */
        ForwardExchangeRate forwardExchangeRate(const Date& d, bool extrapolate = false) const;

        /*! The resulting exchange rate has the same day-counting rule
            used by the term structure. The same rule should be used
            for calculating the passed time t.
        */
        ForwardExchangeRate forwardExchangeRate(Time t, bool extrapolate = false) const;
        //@}

      protected:
        virtual Decimal forwardPointsImpl(Time) const = 0;
        ExchangeRate spotExchangeRate_;
    };


    template <class Interpolator>
    class InterpolatedFxForwardPointTermStructure : public FxForwardPointTermStructure,
                                                    protected InterpolatedCurve<Interpolator> {
      public:
        //! \name Constructors
        //@{
        InterpolatedFxForwardPointTermStructure(const Date& referenceDate,
                                                const ExchangeRate& spotExchangeRate,
                                                const std::vector<Date>& dates,
                                                const std::vector<Decimal>& forwardPoints,
                                                const DayCounter& dayCounter,
                                                const Calendar& calendar = Calendar(),
                                                const Interpolator& interpolator = Interpolator());
        InterpolatedFxForwardPointTermStructure(const Date& referenceDate,
                                                const std::vector<ForwardExchangeRate>& fwdXRates,
                                                const DayCounter& dayCounter,
                                                const Calendar& calendar = Calendar(),
                                                const Interpolator& interpolator = Interpolator());
        //@}
        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name other inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        const std::vector<Decimal>& forwardPointsVector() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}

      protected:
        InterpolatedFxForwardPointTermStructure(
            const DayCounter& dc = DayCounter(),
            const ExchangeRate& spotExchangeRate = ExchangeRate(),
            const std::vector<Date>& dates = std::vector<Date>(),
            const std::vector<Decimal>& forwardPoints = std::vector<Decimal>());
        InterpolatedFxForwardPointTermStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dc = DayCounter(),
            const ExchangeRate& spotExchangeRate = ExchangeRate(),
            const std::vector<Date>& dates = std::vector<Date>(),
            const std::vector<Decimal>& forwardPoints = std::vector<Decimal>());
        InterpolatedFxForwardPointTermStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dc = DayCounter(),
            const ExchangeRate& spotExchangeRate = ExchangeRate(),
            const std::vector<Date>& dates = std::vector<Date>(),
            const std::vector<Decimal>& forwardPoints = std::vector<Decimal>());

        void initialize();

        //! \name FxForwardPointTermStructure implementation
        //@{
        Decimal forwardPointsImpl(Time t) const;
        //@}

        std::vector<Date> dates_;
        std::vector<Decimal> fwdPoints_;

      private:
    };


    // inline definitions

    inline FxForwardPointTermStructure::FxForwardPointTermStructure(
        const Date& referenceDate,
        const ExchangeRate& spotExchangeRate,
        const DayCounter& dayCounter,
        const Calendar& calendar)
    : TermStructure(referenceDate, calendar, dayCounter), spotExchangeRate_(spotExchangeRate) {}

    inline const Currency& FxForwardPointTermStructure::source() const {
        return spotExchangeRate_.source();
    }

    inline const Currency& FxForwardPointTermStructure::target() const {
        return spotExchangeRate_.target();
    }

    inline Decimal FxForwardPointTermStructure::forwardPoints(const Date& d,
                                                              bool extrapolate) const {
        return forwardPoints(timeFromReference(d), extrapolate);
    }

    inline Decimal FxForwardPointTermStructure::forwardPoints(Time t, bool extrapolate) const {
        checkRange(t, extrapolate);
        return forwardPointsImpl(t);
    }

    inline ForwardExchangeRate
    FxForwardPointTermStructure::forwardExchangeRate(const Date& d, bool extrapolate) const {
        return forwardExchangeRate(timeFromReference(d), extrapolate);
    }

    inline ForwardExchangeRate
    FxForwardPointTermStructure::forwardExchangeRate(Time t, bool extrapolate) const {
        Decimal fwdPoints = forwardPointsImpl(t);
        return ForwardExchangeRate(spotExchangeRate_, fwdPoints, Period());
    }


    template <class Interpolator>
    inline InterpolatedFxForwardPointTermStructure<Interpolator>::
        InterpolatedFxForwardPointTermStructure(const Date& referenceDate,
                                                const ExchangeRate& spotExchangeRate,
                                                const std::vector<Date>& dates,
                                                const std::vector<Decimal>& forwardPoints,
                                                const DayCounter& dayCounter,
                                                const Calendar& calendar,
                                                const Interpolator& interpolator)
    : FxForwardPointTermStructure(referenceDate, spotExchangeRate, dayCounter, calendar),
      InterpolatedCurve<Interpolator>(dates.size() + 1, interpolator), dates_(dates),
      fwdPoints_(forwardPoints) {
        initialize();
    }

    template <class Interpolator>
    inline InterpolatedFxForwardPointTermStructure<Interpolator>::
        InterpolatedFxForwardPointTermStructure(
            const Date& referenceDate,
            const std::vector<ForwardExchangeRate>& fwdExchangeRates,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const Interpolator& interpolator)
    : FxForwardPointTermStructure(
          referenceDate, fwdExchangeRates.begin()->spotExchangeRate(), dayCounter, calendar),
      InterpolatedCurve<Interpolator>(fwdExchangeRates.size() + 1, interpolator) {
        // Fill dates/fwdPoints vectors
        dates_.resize(fwdExchangeRates.size());
        fwdPoints_.resize(fwdExchangeRates.size());
        for (Size i = 0; i < fwdExchangeRates.size(); i++) {
            dates_[i] = referenceDate + fwdExchangeRates[i].tenor();
            fwdPoints_[i] = fwdExchangeRates[i].forwardPoints();
        }
        // Run initialization
        initialize();
    }

    template <class Interpolator>
    inline InterpolatedFxForwardPointTermStructure<Interpolator>::
        InterpolatedFxForwardPointTermStructure(const DayCounter& dc,
                                                const ExchangeRate& spotExchangeRate,
                                                const std::vector<Date>& dates,
                                                const std::vector<Decimal>& forwardPoints)
    : TermStructure(dc), spotExchangeRate_(spotExchangeRate), dates_(dates),
      fwdPoints_(forwardPoints) {
        // Derived class should call initialize()
    }

    template <class Interpolator>
    inline InterpolatedFxForwardPointTermStructure<Interpolator>::
        InterpolatedFxForwardPointTermStructure(const Date& referenceDate,
                                                const Calendar& cal,
                                                const DayCounter& dc,
                                                const ExchangeRate& spotExchangeRate,
                                                const std::vector<Date>& dates,
                                                const std::vector<Decimal>& forwardPoints)
    : TermStructure(referenceDate, cal, dc), spotExchangeRate_(spotExchangeRate), dates_(dates),
      fwdPoints_(forwardPoints) {
        // Derived class should call initialize()
    }

    template <class Interpolator>
    inline InterpolatedFxForwardPointTermStructure<Interpolator>::
        InterpolatedFxForwardPointTermStructure(Natural settlementDays,
                                                const Calendar& cal,
                                                const DayCounter& dc,
                                                const ExchangeRate& spotExchangeRate,
                                                const std::vector<Date>& dates,
                                                const std::vector<Decimal>& forwardPoints)
    : TermStructure(settlementDays, cal, dc), spotExchangeRate_(spotExchangeRate), dates_(dates),
      fwdPoints_(forwardPoints) {
        // Derived class should call initialize()
    }

    template <class Interpolator>
    inline void InterpolatedFxForwardPointTermStructure<Interpolator>::initialize() {
        QL_REQUIRE(dates_.size() >= Interpolator::requiredPoints - 1,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == fwdPoints_.size() + 1, "data count mismatch");

        Decimal spotExchangeRate = spotExchangeRate_.rate();
        Date refDate = this->referenceDate();

        this->times_[0] = 0.0;
        this->data_[0] = 0.0;
        QL_REQUIRE(dates_[0] > refDate, "invalid date (" << dates_[0] << ", vs " << refDate << ")");
        this->times_[1] = dayCounter().yearFraction(refDate, dates_[0]);
        QL_REQUIRE(!close(this->times_[1], this->times_[0]),
                   "two dates correspond to the same time "
                   "under this curve's day count convention");
        this->data_[1] = this->fwdPoints_[0];
        for (Size i = 1; i < dates_.size(); ++i) {
            QL_REQUIRE(dates_[i] > dates_[i - 1],
                       "invalid date (" << dates_[i] << ", vs " << dates_[i - 1] << ")");
            this->times_[i + 1] = dayCounter().yearFraction(refDate, dates_[i]);
            QL_REQUIRE(!close(this->times_[i + 1], this->times_[i]),
                       "two dates correspond to the same time "
                       "under this curve's day count convention");
            this->data_[i + 1] = this->fwdPoints_[i];
        }

        this->interpolation_ = this->interpolator_.interpolate(
            this->times_.begin(), this->times_.end(), this->data_.begin());
        this->interpolation_.update();
    }

    template <class T>
    inline Date InterpolatedFxForwardPointTermStructure<T>::maxDate() const {
        if (this->maxDate_ != Date())
            return this->maxDate_;
        return dates_.back();
    }

    template <class T>
    inline const std::vector<Time>& InterpolatedFxForwardPointTermStructure<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>& InterpolatedFxForwardPointTermStructure<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>& InterpolatedFxForwardPointTermStructure<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Decimal>&
    InterpolatedFxForwardPointTermStructure<T>::forwardPointsVector() const {
        return fwdPoints_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real> >
    InterpolatedFxForwardPointTermStructure<T>::nodes() const {
        std::vector<std::pair<Date, Real> > results(dates_.size());
        for (Size i = 0; i < dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], this->data_[i]);
        return results;
    }

    template <class Interpolator>
    inline Decimal
    InterpolatedFxForwardPointTermStructure<Interpolator>::forwardPointsImpl(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);
        // constant extrapolation
        return this->data_.back();
        //return this->interpolation_(t, extrapolate);
    }

}

#endif
