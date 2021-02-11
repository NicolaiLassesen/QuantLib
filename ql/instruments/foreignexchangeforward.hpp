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

#include <ql/currency.hpp>
#include <ql/exchangerate.hpp>
#include <ql/instruments/forward.hpp>
#include <ql/money.hpp>
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
        enum class Type { SellBaseBuyTermForward, BuyBaseSellTermForward };

        ForeignExchangeForward(const Date& deliveryDate,
                               const Money& baseNotionalAmount,
                               const ExchangeRate& contractAllInRate,
                               const Type& forwardType = Type::SellBaseBuyTermForward);
        ForeignExchangeForward(const Date& deliveryDate,
                               const Money& baseNotionalAmount,
                               const ExchangeRate& contractAllInRate,
                               const Type& forwardType,
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
        const Type forwardType() const;
        const Date& deliveryDate() const;
        const Currency& baseCurrency() const;
        const Currency& termCurrency() const;
        const ExchangeRate& contractAllInRate() const;
        const Money& contractNotionalAmountBase() const;
        Money contractNotionalAmountTerm() const;
        const FxTerms& foreignExchangeTerms() const;
        //@}

        //! \name Results
        //@{
        Decimal fairForwardPoints() const;
        Money forwardNetValueBase() const;
        Money forwardNetValueTerm() const;
        Money presentNetValueBase() const;
        Money presentNetValueTerm() const;
        Money forwardGrossValueBase() const;
        Money forwardGrossValueTerm() const;
        //@}

      protected:
        void setupExpired() const;
        Decimal baseSign() const {
            return forwardType_ == ForeignExchangeForward::Type::SellBaseBuyTermForward ? -1.0 : 1.0;
        }

        Date deliveryDate_;
        Money baseNotionalAmount_;
        Money termNotionalAmount_;
        ExchangeRate contractAllInRate_;
        Type forwardType_;
        FxTerms foreignExchangeTerms_;
        Currency termCurrency_;

        mutable Decimal fairForwardPoints_;
        mutable Money forwardNetValueBase_;
        mutable Money forwardNetValueTerm_;
        mutable Money presentNetValueBase_;
        mutable Money presentNetValueTerm_;

      private:
    };


    class ForeignExchangeForward::arguments : public PricingEngine::arguments {
      public:
        void validate() const;
        Decimal baseSign() const {
            return forwardType == ForeignExchangeForward::Type::SellBaseBuyTermForward ? -1.0 : 1.0;
        }

        Date deliveryDate;
        Money baseNotionalAmount;
        ExchangeRate contractAllInRate;
        Type forwardType;
        DayCounter dayCounter;
        Calendar calendar;
        BusinessDayConvention businessDayConvention;
        Natural settlementDays;
    };


    class ForeignExchangeForward::results : public Instrument::results {
      public:
        Decimal fairForwardPoints;
        Money forwardNetValueBase;
        Money forwardNetValueTerm;
        Money presentNetValueBase;
        Money presentNetValueTerm;
        void reset() {
            fairForwardPoints = Null<Decimal>();
            forwardNetValueBase = Money();
            forwardNetValueTerm = Money();
            presentNetValueBase = Money();
            presentNetValueTerm = Money();
            Instrument::results::reset();
        }
    };


    class ForeignExchangeForward::engine
    : public GenericEngine<ForeignExchangeForward::arguments, ForeignExchangeForward::results> {};


    /*! \relates ForeignExchangeForward */
    std::ostream& operator<<(std::ostream&, const ForeignExchangeForward&);

    /*! \relates ForeignExchangeForward::Type */
    std::ostream& operator<<(std::ostream&, const ForeignExchangeForward::Type& t);


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

    inline const ForeignExchangeForward::Type ForeignExchangeForward::forwardType() const {
        return forwardType_;
    }

    inline const Date& ForeignExchangeForward::deliveryDate() const { return deliveryDate_; }

    inline const Currency& ForeignExchangeForward::baseCurrency() const {
        return baseNotionalAmount_.currency();
    }

    inline const Currency& ForeignExchangeForward::termCurrency() const { return termCurrency_; }

    inline const ExchangeRate& ForeignExchangeForward::contractAllInRate() const {
        return contractAllInRate_;
    }

    inline const Money& ForeignExchangeForward::contractNotionalAmountBase() const {
        return baseNotionalAmount_;
    }

    inline Money ForeignExchangeForward::contractNotionalAmountTerm() const {
        return contractAllInRate_.exchange(baseNotionalAmount_);
    }

    inline const FxTerms& ForeignExchangeForward::foreignExchangeTerms() const {
        return foreignExchangeTerms_;
    }

}

#endif