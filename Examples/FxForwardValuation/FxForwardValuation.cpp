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

#include <ql/qldefines.hpp>
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif

#include <ql/settings.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/currency.hpp>
#include <ql/currencies/all.hpp>
#include <ql/instruments/foreignexchangeforward.h>
#include <ql/pricingengines/fx/forwardpointsengine.h>
#include <ql/utilities/dataformatters.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>

#include <iomanip>
#include <iostream>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }
}
#endif

Handle<YieldTermStructure> getDiscountingCurve(Date& todaysDate);

int main(int, char*[]) {

    try {

        std::cout << std::endl;

        Date todaysDate(11, March, 2020);
        Settings::instance().evaluationDate() = todaysDate;

        std::cout << "Today: " << todaysDate.weekday() << ", " << todaysDate << std::endl;

        Date deliveryDate = TARGET().adjust(todaysDate + Period(3 * Months), Following);
        Currency baseCurrency = EURCurrency(), termCurrency = USDCurrency();
        Money baseNotionalAmount = Money(10000, baseCurrency);
        ExchangeRate contractAllInRate = ExchangeRate(baseCurrency, termCurrency, 1.1389);

        ext::shared_ptr<ForeignExchangeForward> fxFwd(
            new ForeignExchangeForward(deliveryDate, baseNotionalAmount, contractAllInRate));

        std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

        Rate spotExchangeRate = 1.1351;
        Rate forwardPoints = 45.0;
        Rate allInRate = spotExchangeRate + forwardPoints / 10000.0;

        Handle<YieldTermStructure> discountingCurve = getDiscountingCurve(todaysDate);

        ext::shared_ptr<ForwardPointsEngine> engine(new ForwardPointsEngine(
            baseCurrency, spotExchangeRate, forwardPoints, discountingCurve));
        fxFwd->setPricingEngine(engine);

        Real npv = fxFwd->NPV();

        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}


Handle<YieldTermStructure> getDiscountingCurve(Date& todaysDate) {

    DayCounter termStructureDayCounter = ActualActual(ActualActual::ISDA);

    // deposits
    Integer fixingDays = 2;
    Calendar calendar = TARGET();
    Date settlementDate = calendar.advance(todaysDate, fixingDays, TimeUnit::Days);
    DayCounter depositDayCounter = Actual360();

    ext::shared_ptr<Quote> d1wRate(new SimpleQuote(-0.00523));
    ext::shared_ptr<Quote> d1mRate(new SimpleQuote(-0.00503));
    ext::shared_ptr<Quote> d3mRate(new SimpleQuote(-0.00473));
    ext::shared_ptr<Quote> d6mRate(new SimpleQuote(-0.00429));
    ext::shared_ptr<Quote> d1yRate(new SimpleQuote(-0.00339));

    ext::shared_ptr<RateHelper> d1w(new DepositRateHelper(Handle<Quote>(d1wRate), 1 * Weeks,
                                                          fixingDays, calendar, ModifiedFollowing,
                                                          true, depositDayCounter));
    ext::shared_ptr<RateHelper> d1m(new DepositRateHelper(Handle<Quote>(d1mRate), 1 * Months,
                                                          fixingDays, calendar, ModifiedFollowing,
                                                          true, depositDayCounter));
    ext::shared_ptr<RateHelper> d3m(new DepositRateHelper(Handle<Quote>(d3mRate), 3 * Months,
                                                          fixingDays, calendar, ModifiedFollowing,
                                                          true, depositDayCounter));
    ext::shared_ptr<RateHelper> d6m(new DepositRateHelper(Handle<Quote>(d6mRate), 6 * Months,
                                                          fixingDays, calendar, ModifiedFollowing,
                                                          true, depositDayCounter));
    ext::shared_ptr<RateHelper> d1y(new DepositRateHelper(Handle<Quote>(d1yRate), 1 * Years,
                                                          fixingDays, calendar, ModifiedFollowing,
                                                          true, depositDayCounter));

    std::vector<ext::shared_ptr<RateHelper> > depoSwapInstruments;
    depoSwapInstruments.push_back(d1w);
    depoSwapInstruments.push_back(d1m);
    depoSwapInstruments.push_back(d3m);
    depoSwapInstruments.push_back(d6m);
    depoSwapInstruments.push_back(d1y);

    ext::shared_ptr<YieldTermStructure> depoTermStructure(
        new PiecewiseYieldCurve<Discount, LogLinear>(settlementDate, depoSwapInstruments,
                                                     termStructureDayCounter));

    return Handle<YieldTermStructure>(depoTermStructure);
}
