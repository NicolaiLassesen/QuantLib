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

#include <ql/forwardexchangerate.hpp>

namespace QuantLib {

    Money ForwardExchangeRate::exchange(const Money& amount) const {
        switch (type_) {
            case ExchangeRate::Direct:
                if (amount.currency() == spotRate_.source())
                    return Money(amount.value() * forwardRate(), spotRate_.target());
                else if (amount.currency() == spotRate_.target())
                    return Money(amount.value() / forwardRate(), spotRate_.source());
                else
                    QL_FAIL("exchange rate not applicable");
            case ExchangeRate::Derived:
                if (amount.currency() == rateChain_.first->source() ||
                    amount.currency() == rateChain_.first->target())
                    return rateChain_.second->exchange(rateChain_.first->exchange(amount));
                else if (amount.currency() == rateChain_.second->source() ||
                         amount.currency() == rateChain_.second->target())
                    return rateChain_.first->exchange(rateChain_.second->exchange(amount));
                else
                    QL_FAIL("exchange rate not applicable");
            default:
                QL_FAIL("unknown exchange-rate type");
        }
    }

    ForwardExchangeRate ForwardExchangeRate::chain(const ForwardExchangeRate& r1,
                                                   const ForwardExchangeRate& r2) {
        QL_ENSURE(r1.tenor() == r2.tenor(),
                  "forward exchange rates must have same tenor in order to chain");

        ExchangeRate chainedSpot = ExchangeRate::chain(r1.spotRate_, r2.spotRate_);
        ForwardExchangeRate result = ForwardExchangeRate(chainedSpot, Null<Decimal>(), r1.tenor());
        result.type_ = ExchangeRate::Derived;
        result.rateChain_ = std::make_pair(ext::make_shared<ForwardExchangeRate>(r1),
                                           ext::make_shared<ForwardExchangeRate>(r2));
        if (r1.source() == r2.source()) {
            result.forwardPoints_ =
                (r2.forwardRate() / r1.forwardRate() - r2.spotRate() / r1.spotRate()) * 10000.0;
            // result.rate_ = r2.rate_ / r1.rate_;
        } else if (r1.source() == r2.target()) {
            result.forwardPoints_ = (1.0 / (r1.forwardRate() * r2.forwardRate()) -
                                     1.0 / (r1.spotRate() * r2.spotRate())) *
                                    10000.0;
            // result.rate_ = 1.0 / (r1.rate_ * r2.rate_);
        } else if (r1.target() == r2.source()) {
            result.forwardPoints_ = r1.spotRate_.rate() * r2.forwardPoints_ +
                                    r2.spotRate_.rate() * r1.forwardPoints_ +
                                    r1.forwardPoints_ * r2.forwardPoints_ / 10000.0;
            // result.rate_ = r1.rate_ * r2.rate_;
        } else if (r1.target() == r2.target()) {
            result.forwardPoints_ =
                (r1.forwardRate() / r2.forwardRate() - r1.spotRate() / r2.spotRate()) * 10000.0;
            // result.rate_ = r1.rate_ / r2.rate_;
        } else {
            QL_FAIL("exchange rates not chainable");
        }
        return result;
    }

    ForwardExchangeRate ForwardExchangeRate::inverse(const ForwardExchangeRate& r) {
        ExchangeRate inverseSpot = ExchangeRate::inverse(r.spotExchangeRate());
        Decimal inverseFwd = 1.0/ r.forwardRate();
        return ForwardExchangeRate(inverseSpot, (inverseFwd - inverseSpot.rate()) / 10000.0,
                                   r.tenor_);
    }

}