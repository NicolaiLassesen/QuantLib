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

#ifndef quantlib_foreignexchangeforward_hpp
#define quantlib_foreignexchangeforward_hpp

#include <ql/instruments/forward.hpp>
#include <ql/currency.hpp>
#include <ql/money.hpp>
#include <ql/exchangerate.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    class FxTerms {
      public:
        FxTerms(const DayCounter& dayCounter,
                const Calendar& calendar,
                BusinessDayConvention businessDayConvention,
                Natural settlementDays);
        FxTerms(const Currency& baseCurrency, const Currency& termCurrency);
        FxTerms(const ExchangeRate& exchangeRate);

        //! \name Inspectors
        //@{
        const DayCounter& dayCounter() const;
        const Calendar& calendar() const;
        BusinessDayConvention businessDayConvention() const;
        Natural settlementDays() const;
        //@}

      private:
        DayCounter dayCounter_;
        Calendar calendar_;
        BusinessDayConvention businessDayConvention_;
        Natural settlementDays_;
    };


    class ForeignExchangeForward : public Instrument {
      public:
        ForeignExchangeForward(const Date& deliveryDate,
                               const Money& baseNotionalAmount,
                               const ExchangeRate& contractAllInRate);
        ForeignExchangeForward(const Date& deliveryDate,
                               const Money& baseNotionalAmount,
                               const ExchangeRate& contractAllInRate,
                               const FxTerms& terms);

        class arguments;
        class results;
        class engine;

        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
        void fetchResults(const PricingEngine::results*) const;
        //@]

        //! \name Inspectors
        //@{
        const Date& deliveryDate() const;
        const Currency& baseCurrency() const;
        const Currency& termCurrency() const;
        ExchangeRate contractAllInRate() const;
        Real notionalAmountBase() const;
        Real notionalAmountTerm() const;
        const FxTerms& foreignExchangeTerms() const;
        //@}

        //! \name Results
        //@{
        Money forwardValue() const;
        //@}

      protected:
        void setupExpired() const;

        Date deliveryDate_;
        Money baseNotionalAmount_;
        Money termNotionalAmount_;
        ExchangeRate contractAllInRate_;
        FxTerms foreignExchangeTerms_;
        Currency termCurrency_;

        mutable Money forwardValue_;

      private:
    };


    class ForeignExchangeForward::arguments : public PricingEngine::arguments {
      public:
        void validate() const;

        Date deliveryDate;
        Money baseNotionalAmount;
        ExchangeRate contractAllInRate;
        DayCounter dayCounter;
        Calendar calendar;
        BusinessDayConvention businessDayConvention;
        Natural settlementDays;
    };


    class ForeignExchangeForward::results : public Instrument::results {
      public:
        Currency valuationCurrency;
        Money forwardValue;
        void reset() {
            forwardValue = Money();
            Instrument::results::reset();
        }
    };


    class ForeignExchangeForward::engine
    : public GenericEngine<ForeignExchangeForward::arguments, ForeignExchangeForward::results> {};


    /*! \relates ForeignExchangeForward */
    std::ostream& operator<<(std::ostream&, const ForeignExchangeForward&);


    // inline definitions

    inline const DayCounter& FxTerms::dayCounter() const { return dayCounter_; }

    inline const Calendar& FxTerms::calendar() const { return calendar_; }

    inline BusinessDayConvention FxTerms::businessDayConvention() const {
        return businessDayConvention_;
    }

    inline Natural FxTerms::settlementDays() const { return settlementDays_; }

    inline bool ForeignExchangeForward::isExpired() const {
        return deliveryDate_ < Settings::instance().evaluationDate();
    }

    inline const Date& ForeignExchangeForward::deliveryDate() const { return deliveryDate_; }

    inline const Currency& ForeignExchangeForward::baseCurrency() const {
        return baseNotionalAmount_.currency();
    }

    inline const Currency& ForeignExchangeForward::termCurrency() const { return termCurrency_; }

    inline ExchangeRate ForeignExchangeForward::contractAllInRate() const {
        return contractAllInRate_;
    }

    inline Real ForeignExchangeForward::notionalAmountBase() const {
        return baseNotionalAmount_.value();
    }

    inline Real ForeignExchangeForward::notionalAmountTerm() const {
        return contractAllInRate_.exchange(baseNotionalAmount_).value();
    }

    inline const FxTerms& ForeignExchangeForward::foreignExchangeTerms() const {
        return foreignExchangeTerms_;
    }

}

#endif