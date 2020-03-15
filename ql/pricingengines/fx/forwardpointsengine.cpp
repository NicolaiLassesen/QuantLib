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

#include <ql/pricingengines/fx/forwardpointsengine.h>

namespace QuantLib {

    ForwardPointsEngine::ForwardPointsEngine(Currency valuationCurrency,
                                             Decimal spotExchangeRate,
                                             Decimal forwardPoints,
                                             const Handle<YieldTermStructure>& discountCurve)
    : valuationCurrency_(valuationCurrency), spotExchangeRate_(spotExchangeRate),
      forwardPoints_(forwardPoints), discountCurve_(discountCurve) {
        registerWith(discountCurve_);
    }

    void ForwardPointsEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(), "discounting term structure handle is empty");

        // Collect required inputs to calculation
        Date valuationDate = (*discountCurve_)->referenceDate();
        Date deliveryDate = arguments_.deliveryDate;
        Time timeToDelivery = arguments_.dayCounter.yearFraction(valuationDate, deliveryDate);
        ExchangeRate allInRate = arguments_.contractAllInRate;
        DiscountFactor discount = (*discountCurve_)->discount(timeToDelivery);
        ExchangeRate fwdExchangeRate = ExchangeRate(allInRate.source(), allInRate.target(),
                                                    spotExchangeRate_ + forwardPoints_ / 10000.0); 
        Money forwardValue =
            arguments_.baseNotionalAmount * (fwdExchangeRate.rate() - allInRate.rate());

        // Store results
        results_.valuationCurrency = valuationCurrency_;
        results_.valuationDate = valuationDate;
        results_.forwardValue = forwardValue;
        results_.value = discount * forwardValue.value();
        results_.errorEstimate = 0.0;
    }

}