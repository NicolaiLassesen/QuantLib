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
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file forward.hpp
    \brief Base forward class
*/

#include <ql/instruments/foreignexchangeforward.h>
#include <ql/time/all.hpp>

namespace QuantLib {

    FxTerms::FxTerms(const DayCounter& dayCounter,
                     const Calendar& calendar,
                     BusinessDayConvention businessDayConvention,
                     Natural settlementDays)
    : dayCounter_(dayCounter), calendar_(calendar), businessDayConvention_(businessDayConvention),
      settlementDays_(settlementDays) {}

    FxTerms::FxTerms(const Currency& baseCurrency, const Currency& termCurrency)
    : dayCounter_(Actual360()), calendar_(NullCalendar()), businessDayConvention_(Following),
      settlementDays_(2) {
        if (baseCurrency.code() == "EUR" && termCurrency.code() == "USD") {
            dayCounter_ = Actual365Fixed(Actual365Fixed::Standard);
            calendar_ = JointCalendar(TARGET(), UnitedStates(UnitedStates::NYSE));
            businessDayConvention_ = Following;
            settlementDays_ = 2;
        }
    }


    ForeignExchangeForward::ForeignExchangeForward(const Date& deliveryDate,
                                                   const Currency& baseCurrency,
                                                   const Currency& termCurrency,
                                                   Real baseNotionalAmount,
                                                   Rate contractAllInRate)
    : ForeignExchangeForward(deliveryDate,
                             baseCurrency,
                             termCurrency,
                             baseNotionalAmount,
                             contractAllInRate,
                             FxTerms(baseCurrency, termCurrency)) {}

    ForeignExchangeForward::ForeignExchangeForward(const Date& deliveryDate,
                                                   const Currency& baseCurrency,
                                                   const Currency& termCurrency,
                                                   Real baseNotionalAmount,
                                                   Rate contractAllInRate,
                                                   const FxTerms& terms)
    : Forward(terms.dayCounter(),
              terms.calendar(),
              terms.businessDayConvention(),
              terms.settlementDays(),
              ext::shared_ptr<Payoff>(),
              Date(),
              deliveryDate),
      baseCurrency_(baseCurrency), termCurrency_(termCurrency),
      contractAllInRate_(contractAllInRate), notionalAmountBase_(baseNotionalAmount),
      notionalAmountTerm_(baseNotionalAmount * contractAllInRate) {}

    void ForeignExchangeForward::performCalculations() const {
        // calculateForwardRate();
        underlyingSpotValue_ = spotValue();
        underlyingIncome_ = 0.0;
        Forward::performCalculations();
    }

    Real ForeignExchangeForward::spotIncome(
        const Handle<YieldTermStructure>& incomeDiscountCurve) const {
        return 0.0;
    }

    Real ForeignExchangeForward::spotValue() const { return 0.0; }


    std::ostream& operator<<(std::ostream& out, const ForeignExchangeForward& c) {
        return out << c.baseCurrency()
                   << c.termCurrency()
                   << " " << io::iso_date(c.deliveryDate())
                   << " " << c.notionalAmountBase()
                   << " - " << c.contractAllInRate();
    }

}