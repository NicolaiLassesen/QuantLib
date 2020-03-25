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

#include <ql/instruments/foreignexchangeforward.hpp>
#include <ql/termstructures/fxforwardpointtermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class ForwardPointsEngine : public ForeignExchangeForward::engine {
      public:
        ForwardPointsEngine(
            const ExchangeRate& spotExchangeRate,
            const Handle<FxForwardPointTermStructure>& forwardPointsCurve =
                Handle<FxForwardPointTermStructure>(),
            const Handle<YieldTermStructure>& baseDiscountCurve = Handle<YieldTermStructure>(),
            const Handle<YieldTermStructure>& termDiscountCurve = Handle<YieldTermStructure>());
        void calculate() const;
        Currency valuationCurrency() const { return spotExchangeRate_.source(); }
        const ExchangeRate spotExchangeRate() const { return spotExchangeRate_; }
        Handle<FxForwardPointTermStructure> forwardPointsCurve() const {
            return forwardPointsCurve_;
        }
        Handle<YieldTermStructure> baseDiscountCurve() const { return baseDiscountCurve_; }
        Handle<YieldTermStructure> termDiscountCurve() const { return termDiscountCurve_; }

      private:
        ExchangeRate spotExchangeRate_;
        Handle<FxForwardPointTermStructure> forwardPointsCurve_;
        Handle<YieldTermStructure> baseDiscountCurve_;
        Handle<YieldTermStructure> termDiscountCurve_;
    };

}
#endif