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

/*! \file forwardpointengine.h
    \brief Engine allowing valuation of ForeignExchangeForward contracts using a forward point curve
*/

#ifndef quantlib_forward_points_engine_hpp
#define quantlib_forward_points_engine_hpp

#include <ql/instruments/foreignexchangeforward.h>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class ForwardPointsEngine : public ForeignExchangeForward::engine {
      public:
        ForwardPointsEngine(
            Currency valuationCurrency,
            Decimal spotExchangeRate,
            Decimal forwardPoints,
            const Handle<YieldTermStructure>& discountCurve = Handle<YieldTermStructure>());
        void calculate() const;
        Currency valuationCurrency() const { return valuationCurrency_; }
        Decimal spotExchangeRate() const { return spotExchangeRate_; }
        Decimal forwardPoints() const { return forwardPoints_; }
        Handle<YieldTermStructure> discountCurve() const { return discountCurve_; }
      private:
        Currency valuationCurrency_;
        // TODO: Create spot exchange rate table with all currencies and use that
        Decimal spotExchangeRate_;
        // TODO: For now just use forward point as simple real - when everything is ready and works
        // create forward point curve and use that
        Decimal forwardPoints_;
        Handle<YieldTermStructure> discountCurve_;
    };

}
#endif