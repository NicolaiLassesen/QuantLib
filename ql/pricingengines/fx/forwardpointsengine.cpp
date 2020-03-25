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

#include <ql/pricingengines/fx/forwardpointsengine.hpp>

namespace QuantLib {

    ForwardPointsEngine::ForwardPointsEngine(
        const ExchangeRate& spotExchangeRate,
        const Handle<FxForwardPointTermStructure>& forwardPointsCurve,
        const Handle<YieldTermStructure>& baseDiscountCurve,
        const Handle<YieldTermStructure>& termDiscountCurve)
    : spotExchangeRate_(spotExchangeRate), forwardPointsCurve_(forwardPointsCurve),
      baseDiscountCurve_(baseDiscountCurve), termDiscountCurve_(termDiscountCurve) {
        QL_REQUIRE(spotExchangeRate_.source() == forwardPointsCurve_->source(),
                   "base currency mismatch");
        QL_REQUIRE(spotExchangeRate_.target() == forwardPointsCurve_->target(),
                   "term currency mismatch");

        registerWith(forwardPointsCurve_);
        registerWith(baseDiscountCurve_);
        registerWith(termDiscountCurve_);
    }

    void ForwardPointsEngine::calculate() const {
        QL_REQUIRE(!baseDiscountCurve_.empty(), "base discounting term structure handle is empty");
        QL_REQUIRE(!termDiscountCurve_.empty(), "term discounting term structure handle is empty");
        QL_REQUIRE(!forwardPointsCurve_.empty(), "forward points curve handle is empty");

        // Collect required inputs to calculation
        Decimal sign = arguments_.baseSign();
        Date valuationDate = (*baseDiscountCurve_)->referenceDate();
        Date deliveryDate = arguments_.deliveryDate;
        Time timeToDelivery = arguments_.dayCounter.yearFraction(valuationDate, deliveryDate);
        Money baseNotional = sign * arguments_.baseNotionalAmount;
        ExchangeRate allInRate = arguments_.contractAllInRate;
        QL_REQUIRE(spotExchangeRate_.source() == allInRate.source() && spotExchangeRate_.target() == allInRate.target(),
                   "forward points curve currencies not valid for valuation of contract");

        DiscountFactor baseDiscount = (*baseDiscountCurve_)->discount(timeToDelivery);
        DiscountFactor termDiscount = (*termDiscountCurve_)->discount(timeToDelivery);
        ForwardExchangeRate fwdExchangeRate = (*forwardPointsCurve_)->forwardExchangeRate(timeToDelivery);
        Decimal termForwardValue = baseNotional.value() * (fwdExchangeRate.forwardRate() - allInRate.rate());
        Decimal baseForwardValue = allInRate.exchange(baseNotional).value() * (1.0 / fwdExchangeRate.forwardRate() - 1.0 / allInRate.rate());
        Decimal termPresentValue = termDiscount * termForwardValue;
        Decimal basePresentValue = baseDiscount * baseForwardValue;

        // Store results
        results_.valuationDate = valuationDate;
        results_.fairForwardPoints = fwdExchangeRate.forwardPoints();
        results_.forwardNetValueBase = Money(baseForwardValue, spotExchangeRate_.source());
        results_.forwardNetValueTerm = Money(termForwardValue, spotExchangeRate_.target());
        results_.presentNetValueBase = Money(basePresentValue, spotExchangeRate_.source());
        results_.presentNetValueTerm = Money(termPresentValue, spotExchangeRate_.target());
        results_.value = termPresentValue;
        results_.errorEstimate = 0.0;
    }

}