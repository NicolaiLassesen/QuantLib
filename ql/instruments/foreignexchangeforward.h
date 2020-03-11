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
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    class FxTerms {
      public:
        FxTerms(const DayCounter& dayCounter,
                const Calendar& calendar,
                BusinessDayConvention businessDayConvention,
                Natural settlementDays);
        FxTerms(const Currency& baseCurrency,
                const Currency& termCurrency);

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


    class ForeignExchangeForward: public Forward {
      public:
        ForeignExchangeForward(const Date& deliveryDate,
                               const Currency& baseCurrency,
                               const Currency& termCurrency,
                               Real baseNotionalAmount,
                               Rate contractAllInRate);
        ForeignExchangeForward(const Date& deliveryDate,
                               const Currency& baseCurrency,
                               const Currency& termCurrency,
                               Real baseNotionalAmount,
                               Rate contractAllInRate,
                               const FxTerms& terms);

        //! \name Inspectors
        //@{
        const Date& deliveryDate() const;
        const Currency& baseCurrency() const;
        const Currency& termCurrency() const;
        Rate contractAllInRate() const;
        Real notionalAmountBase() const;
        Real notionalAmountTerm() const;
        //@}

        /*!  Income is zero for a FXFWD */
        Real spotIncome(const Handle<YieldTermStructure>& incomeDiscountCurve) const;
        //! Spot value (NPV) of the underlying loan
        /*! This has always a positive value (asset), even if short the FRA */
        Real spotValue() const;

      protected:
        void performCalculations() const;
        Currency baseCurrency_;
        Currency termCurrency_;
        Rate contractAllInRate_;
        Real notionalAmountBase_;
        Real notionalAmountTerm_;
      private:
    };


    /*! \relates ForeignExchangeForward */
    std::ostream& operator<<(std::ostream&, const ForeignExchangeForward&);


    // inline definitions

    inline const DayCounter& FxTerms::dayCounter() const { 
        return dayCounter_;
    }

    inline const Calendar& FxTerms::calendar() const { 
        return calendar_;
    }

    inline BusinessDayConvention FxTerms::businessDayConvention() const {
        return businessDayConvention_;
    }

    inline Natural FxTerms::settlementDays() const {
        return settlementDays_;
    }

    inline const Date& ForeignExchangeForward::deliveryDate() const { return maturityDate_; }

    inline const Currency& ForeignExchangeForward::baseCurrency() const { return baseCurrency_; }

    inline const Currency& ForeignExchangeForward::termCurrency() const { return termCurrency_; }
    
    inline Rate ForeignExchangeForward::contractAllInRate() const { return contractAllInRate_; }
    
    inline Real ForeignExchangeForward::notionalAmountBase() const { return notionalAmountBase_; }
    
    inline Real ForeignExchangeForward::notionalAmountTerm() const { return notionalAmountTerm_; }

}

#endif