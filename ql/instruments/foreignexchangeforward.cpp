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

#include <ql/instruments/foreignexchangeforward.hpp>
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

    FxTerms::FxTerms(const ExchangeRate& exchangeRate)
    : FxTerms(exchangeRate.source(), exchangeRate.target()) {}


    ForeignExchangeForward::ForeignExchangeForward(const Date& deliveryDate,
                                                   const Money& baseNotionalAmount,
                                                   const ExchangeRate& contractAllInRate,
                                                   const Type& forwardType)
    : ForeignExchangeForward(
          deliveryDate, baseNotionalAmount, contractAllInRate, forwardType, FxTerms(contractAllInRate)) {}

    ForeignExchangeForward::ForeignExchangeForward(const Date& deliveryDate,
                                                   const Money& baseNotionalAmount,
                                                   const ExchangeRate& contractAllInRate,
                                                   const Type& forwardType,
                                                   const FxTerms& terms)
    : deliveryDate_(deliveryDate), baseNotionalAmount_(baseNotionalAmount),
      termNotionalAmount_(contractAllInRate.exchange(baseNotionalAmount)),
      forwardType_(forwardType), foreignExchangeTerms_(terms) {
        QL_ENSURE(baseNotionalAmount_.currency() == contractAllInRate.source() ||
                      baseNotionalAmount_.currency() == contractAllInRate.target(),
                  "currency of base notional differs from all in rate currencies");
        termCurrency_ = baseNotionalAmount_.currency() == contractAllInRate.source() ?
                            contractAllInRate.target() :
                            contractAllInRate.source();
        contractAllInRate_ = baseNotionalAmount_.currency() == contractAllInRate.source() ?
                                 contractAllInRate :
                                 ExchangeRate::inverse(contractAllInRate);
    }

    void QuantLib::ForeignExchangeForward::setupArguments(PricingEngine::arguments* args) const {
        ForeignExchangeForward::arguments* arguments =
            dynamic_cast<ForeignExchangeForward::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
        arguments->deliveryDate = deliveryDate_;
        arguments->baseNotionalAmount = baseNotionalAmount_;
        arguments->contractAllInRate = contractAllInRate_;
        arguments->forwardType = forwardType_;
        arguments->dayCounter = foreignExchangeTerms_.dayCounter();
        arguments->calendar = foreignExchangeTerms_.calendar();
        arguments->businessDayConvention = foreignExchangeTerms_.businessDayConvention();
        arguments->settlementDays = foreignExchangeTerms_.settlementDays();
    }

    void ForeignExchangeForward::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const ForeignExchangeForward::results* results =
            dynamic_cast<const ForeignExchangeForward::results*>(r);
        QL_ENSURE(results != 0, "wrong result type");
        fairForwardPoints_ = results->fairForwardPoints;
        forwardNetValueBase_ = results->forwardNetValueBase;
        forwardNetValueTerm_ = results->forwardNetValueTerm;
        presentNetValueBase_ = results->presentNetValueBase;
        presentNetValueTerm_ = results->presentNetValueTerm;
    }

    Decimal ForeignExchangeForward::fairForwardPoints() const {
        calculate();
        QL_ENSURE(fairForwardPoints_ != Null<Decimal>(), "fairForwardPoints not given");
        return fairForwardPoints_;
    }

    Money ForeignExchangeForward::forwardNetValueBase() const {
        calculate();
        QL_ENSURE(!forwardNetValueBase_.empty(), "forwardNetValueBase not given");
        return forwardNetValueBase_;
    }
    Money ForeignExchangeForward::forwardNetValueTerm() const {
        calculate();
        QL_ENSURE(!forwardNetValueTerm_.empty(), "forwardNetValueTerm not given");
        return forwardNetValueTerm_;
    }

    Money ForeignExchangeForward::presentNetValueBase() const {
        calculate();
        QL_ENSURE(!presentNetValueBase_.empty(), "presentNetValueBase not given");
        return presentNetValueBase_;
    }

    Money ForeignExchangeForward::presentNetValueTerm() const {
        calculate();
        QL_ENSURE(!presentNetValueTerm_.empty(), "presentNetValueBase not given");
        return presentNetValueTerm_;
    }

    Money ForeignExchangeForward::forwardGrossValueBase() const {
        calculate();
        QL_ENSURE(!forwardNetValueBase_.empty(), "forwardValue not given");
        Money fwdGrossBase = forwardNetValueBase_ - baseNotionalAmount_ * this->baseSign();
        return fwdGrossBase;
    }

    Money ForeignExchangeForward::forwardGrossValueTerm() const {
        calculate();
        QL_ENSURE(!forwardNetValueTerm_.empty(), "forwardValue not given");
        Money fwdGrossTerm = forwardNetValueTerm_ + termNotionalAmount_ * this->baseSign();
        return fwdGrossTerm;
    }

    void ForeignExchangeForward::setupExpired() const {
        Instrument::setupExpired();
        forwardNetValueBase_ = Money();
        forwardNetValueTerm_ = Money();
        presentNetValueBase_ = Money();
        presentNetValueTerm_ = Money();
    }

    void ForeignExchangeForward::arguments::validate() const {
        QL_REQUIRE(baseNotionalAmount.currency() == contractAllInRate.source(),
                   "contract all-in rate should have same base currency as notionnal amount");
    }


    std::ostream& operator<<(std::ostream& out, const ForeignExchangeForward& c) {
        return out << c.baseCurrency() << c.termCurrency() << " " << io::iso_date(c.deliveryDate())
                   << " " << c.contractNotionalAmountBase();
    }

    std::ostream& operator<<(std::ostream& out, const ForeignExchangeForward::Type& t) {
        switch (t) {
            case QuantLib::ForeignExchangeForward::Type::SellBaseBuyTermForward:
                return out << "SellBaseBuyTermForward";
            case QuantLib::ForeignExchangeForward::Type::BuyBaseSellTermForward:
                return out << "BuyBaseSellTermForward";
            default:
                QL_FAIL("unknown QuantLib::ForeignExchangeForward::Type(" << QuantLib::Integer(t) << ")");
        }
    }

}
