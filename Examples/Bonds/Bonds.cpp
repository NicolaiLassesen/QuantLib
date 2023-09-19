/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2008 Florent Grenier

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

/*  This example shows how to set up a term structure and then price
    some simple bonds. The last part is dedicated to peripherical
    computations such as "Yield to Price" or "Price to Yield"
 */

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace QuantLib;

int stdExample() {
    try {

        std::cout << std::endl;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();

        Date settlementDate(18, September, 2008);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 3;
        Natural settlementDays = 3;

        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;

        std::cout << "Today: " << todaysDate.weekday() << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday() << ", " << settlementDate
                  << std::endl;


        // Building of the bonds discounting yield curve

        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // Common data

        // ZC rates for the short end
        Rate zc3mQuote = 0.0096;
        Rate zc6mQuote = 0.0145;
        Rate zc1yQuote = 0.0194;

        auto zc3mRate = ext::make_shared<SimpleQuote>(zc3mQuote);
        auto zc6mRate = ext::make_shared<SimpleQuote>(zc6mQuote);
        auto zc1yRate = ext::make_shared<SimpleQuote>(zc1yQuote);

        DayCounter zcBondsDayCounter = Actual365Fixed();

        auto zc3m = ext::make_shared<DepositRateHelper>(Handle<Quote>(zc3mRate), 3 * Months,
                                                        fixingDays, calendar, ModifiedFollowing,
                                                        true, zcBondsDayCounter);
        auto zc6m = ext::make_shared<DepositRateHelper>(Handle<Quote>(zc6mRate), 6 * Months,
                                                        fixingDays, calendar, ModifiedFollowing,
                                                        true, zcBondsDayCounter);
        auto zc1y = ext::make_shared<DepositRateHelper>(Handle<Quote>(zc1yRate), 1 * Years,
                                                        fixingDays, calendar, ModifiedFollowing,
                                                        true, zcBondsDayCounter);

        // setup bonds
        Real redemption = 100.0;

        const Size numberOfBonds = 5;

        Date issueDates[] = {Date(15, March, 2005), Date(15, June, 2005), Date(30, June, 2006),
                             Date(15, November, 2002), Date(15, May, 1987)};

        Date maturities[] = {Date(31, August, 2010), Date(31, August, 2011), Date(31, August, 2013),
                             Date(15, August, 2018), Date(15, May, 2038)};

        Real couponRates[] = {0.02375, 0.04625, 0.03125, 0.04000, 0.04500};

        Real marketQuotes[] = {100.390625, 106.21875, 100.59375, 101.6875, 102.140625};

        std::vector<ext::shared_ptr<SimpleQuote>> quote;
        for (Real marketQuote : marketQuotes) {
            ext::shared_ptr<SimpleQuote> cp(new SimpleQuote(marketQuote));
            quote.push_back(cp);
        }

        RelinkableHandle<Quote> quoteHandle[numberOfBonds];
        for (Size i = 0; i < numberOfBonds; i++) {
            quoteHandle[i].linkTo(quote[i]);
        }

        // Definition of the rate helpers
        std::vector<ext::shared_ptr<BondHelper>> bondsHelpers;

        for (Size i = 0; i < numberOfBonds; i++) {

            Schedule schedule(issueDates[i], maturities[i], Period(Semiannual),
                              UnitedStates(UnitedStates::GovernmentBond), Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);

            auto bondHelper = ext::make_shared<FixedRateBondHelper>(
                quoteHandle[i], settlementDays, 100.0, schedule,
                std::vector<Rate>(1, couponRates[i]), ActualActual(ActualActual::Bond), Unadjusted,
                redemption, issueDates[i]);

            // the above could also be done by creating a
            // FixedRateBond instance and writing:
            //
            // auto bondHelper = ext::make_shared<BondHelper>(quoteHandle[i], bond);
            //
            // This would also work for bonds that still don't have a
            // specialized helper, such as floating-rate bonds.


            bondsHelpers.push_back(bondHelper);
        }

        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter = ActualActual(ActualActual::ISDA);

        // A depo-bond curve
        std::vector<ext::shared_ptr<RateHelper>> bondInstruments;

        // Adding the ZC bonds to the curve for the short end
        bondInstruments.push_back(zc3m);
        bondInstruments.push_back(zc6m);
        bondInstruments.push_back(zc1y);

        // Adding the Fixed rate bonds to the curve for the long end
        for (Size i = 0; i < numberOfBonds; i++) {
            bondInstruments.push_back(bondsHelpers[i]);
        }

        auto bondDiscountingTermStructure =
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
                settlementDate, bondInstruments, termStructureDayCounter);

        // Building of the Libor forecasting curve
        // deposits
        Rate d1wQuote = 0.043375;
        Rate d1mQuote = 0.031875;
        Rate d3mQuote = 0.0320375;
        Rate d6mQuote = 0.03385;
        Rate d9mQuote = 0.0338125;
        Rate d1yQuote = 0.0335125;
        // swaps
        Rate s2yQuote = 0.0295;
        Rate s3yQuote = 0.0323;
        Rate s5yQuote = 0.0359;
        Rate s10yQuote = 0.0412;
        Rate s15yQuote = 0.0433;


        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // deposits
        auto d1wRate = ext::make_shared<SimpleQuote>(d1wQuote);
        auto d1mRate = ext::make_shared<SimpleQuote>(d1mQuote);
        auto d3mRate = ext::make_shared<SimpleQuote>(d3mQuote);
        auto d6mRate = ext::make_shared<SimpleQuote>(d6mQuote);
        auto d9mRate = ext::make_shared<SimpleQuote>(d9mQuote);
        auto d1yRate = ext::make_shared<SimpleQuote>(d1yQuote);
        // swaps
        auto s2yRate = ext::make_shared<SimpleQuote>(s2yQuote);
        auto s3yRate = ext::make_shared<SimpleQuote>(s3yQuote);
        auto s5yRate = ext::make_shared<SimpleQuote>(s5yQuote);
        auto s10yRate = ext::make_shared<SimpleQuote>(s10yQuote);
        auto s15yRate = ext::make_shared<SimpleQuote>(s15yQuote);

        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits
        DayCounter depositDayCounter = Actual360();

        auto d1w = ext::make_shared<DepositRateHelper>(Handle<Quote>(d1wRate), 1 * Weeks,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);
        auto d1m = ext::make_shared<DepositRateHelper>(Handle<Quote>(d1mRate), 1 * Months,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);
        auto d3m = ext::make_shared<DepositRateHelper>(Handle<Quote>(d3mRate), 3 * Months,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);
        auto d6m = ext::make_shared<DepositRateHelper>(Handle<Quote>(d6mRate), 6 * Months,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);
        auto d9m = ext::make_shared<DepositRateHelper>(Handle<Quote>(d9mRate), 9 * Months,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);
        auto d1y = ext::make_shared<DepositRateHelper>(Handle<Quote>(d1yRate), 1 * Years,
                                                       fixingDays, calendar, ModifiedFollowing,
                                                       true, depositDayCounter);

        // setup swaps
        auto swFixedLegFrequency = Annual;
        auto swFixedLegConvention = Unadjusted;
        auto swFixedLegDayCounter = Thirty360(Thirty360::European);
        auto swFloatingLegIndex = ext::make_shared<Euribor6M>();

        const Period forwardStart(1 * Days);

        auto s2y = ext::make_shared<SwapRateHelper>(
            Handle<Quote>(s2yRate), 2 * Years, calendar, swFixedLegFrequency, swFixedLegConvention,
            swFixedLegDayCounter, swFloatingLegIndex, Handle<Quote>(), forwardStart);
        auto s3y = ext::make_shared<SwapRateHelper>(
            Handle<Quote>(s3yRate), 3 * Years, calendar, swFixedLegFrequency, swFixedLegConvention,
            swFixedLegDayCounter, swFloatingLegIndex, Handle<Quote>(), forwardStart);
        auto s5y = ext::make_shared<SwapRateHelper>(
            Handle<Quote>(s5yRate), 5 * Years, calendar, swFixedLegFrequency, swFixedLegConvention,
            swFixedLegDayCounter, swFloatingLegIndex, Handle<Quote>(), forwardStart);
        auto s10y = ext::make_shared<SwapRateHelper>(Handle<Quote>(s10yRate), 10 * Years, calendar,
                                                     swFixedLegFrequency, swFixedLegConvention,
                                                     swFixedLegDayCounter, swFloatingLegIndex,
                                                     Handle<Quote>(), forwardStart);
        auto s15y = ext::make_shared<SwapRateHelper>(Handle<Quote>(s15yRate), 15 * Years, calendar,
                                                     swFixedLegFrequency, swFixedLegConvention,
                                                     swFixedLegDayCounter, swFloatingLegIndex,
                                                     Handle<Quote>(), forwardStart);


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0

        // A depo-swap curve
        std::vector<ext::shared_ptr<RateHelper>> depoSwapInstruments;
        depoSwapInstruments.push_back(d1w);
        depoSwapInstruments.push_back(d1m);
        depoSwapInstruments.push_back(d3m);
        depoSwapInstruments.push_back(d6m);
        depoSwapInstruments.push_back(d9m);
        depoSwapInstruments.push_back(d1y);
        depoSwapInstruments.push_back(s2y);
        depoSwapInstruments.push_back(s3y);
        depoSwapInstruments.push_back(s5y);
        depoSwapInstruments.push_back(s10y);
        depoSwapInstruments.push_back(s15y);
        auto depoSwapTermStructure = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
            settlementDate, depoSwapInstruments, termStructureDayCounter);

        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<YieldTermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<YieldTermStructure> forecastingTermStructure;

        /*********************
         * BONDS TO BE PRICED *
         **********************/

        // Common data
        Real faceAmount = 100;

        // Pricing engine
        auto bondEngine = ext::make_shared<DiscountingBondEngine>(discountingTermStructure);

        // Zero coupon bond
        ZeroCouponBond zeroCouponBond(settlementDays, UnitedStates(UnitedStates::GovernmentBond),
                                      faceAmount, Date(15, August, 2013), Following, Real(116.92),
                                      Date(15, August, 2003));

        zeroCouponBond.setPricingEngine(bondEngine);

        // Fixed 4.5% US Treasury Note
        Schedule fixedBondSchedule(Date(15, May, 2007), Date(15, May, 2017), Period(Semiannual),
                                   UnitedStates(UnitedStates::GovernmentBond), Unadjusted,
                                   Unadjusted, DateGeneration::Backward, false);

        FixedRateBond fixedRateBond(settlementDays, faceAmount, fixedBondSchedule,
                                    std::vector<Rate>(1, 0.045), ActualActual(ActualActual::Bond),
                                    ModifiedFollowing, 100.0, Date(15, May, 2007));

        fixedRateBond.setPricingEngine(bondEngine);

        // Floating rate bond (3M USD Libor + 0.1%)
        // Should and will be priced on another curve later...

        RelinkableHandle<YieldTermStructure> liborTermStructure;
        const auto libor3m = ext::make_shared<USDLibor>(Period(3, Months), liborTermStructure);
        libor3m->addFixing(Date(17, July, 2008), 0.0278625);

        Schedule floatingBondSchedule(Date(21, October, 2005), Date(21, October, 2010),
                                      Period(Quarterly), UnitedStates(UnitedStates::NYSE),
                                      Unadjusted, Unadjusted, DateGeneration::Backward, true);

        FloatingRateBond floatingRateBond(settlementDays, faceAmount, floatingBondSchedule, libor3m,
                                          Actual360(), ModifiedFollowing, Natural(2),
                                          // Gearings
                                          std::vector<Real>(1, 1.0),
                                          // Spreads
                                          std::vector<Rate>(1, 0.001),
                                          // Caps
                                          std::vector<Rate>(),
                                          // Floors
                                          std::vector<Rate>(),
                                          // Fixing in arrears
                                          true, Real(100.0), Date(21, October, 2005));

        floatingRateBond.setPricingEngine(bondEngine);

        // Coupon pricers
        auto pricer = ext::make_shared<BlackIborCouponPricer>();

        // optionLet volatilities
        Volatility volatility = 0.0;
        Handle<OptionletVolatilityStructure> vol;
        vol = Handle<OptionletVolatilityStructure>(ext::make_shared<ConstantOptionletVolatility>(
            settlementDays, calendar, ModifiedFollowing, volatility, Actual365Fixed()));

        pricer->setCapletVolatility(vol);
        setCouponPricer(floatingRateBond.cashflows(), pricer);

        // Yield curve bootstrapping
        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(bondDiscountingTermStructure);

        // We are using the depo & swap curve to estimate the future Libor rates
        liborTermStructure.linkTo(depoSwapTermStructure);

        /***************
         * BOND PRICING *
         ****************/

        std::cout << std::endl;

        // write column headings
        Size widths[] = {18, 10, 10, 10};

        std::cout << std::setw(widths[0]) << "                 " << std::setw(widths[1]) << "ZC"
                  << std::setw(widths[2]) << "Fixed" << std::setw(widths[3]) << "Floating"
                  << std::endl;

        Size width = widths[0] + widths[1] + widths[2] + widths[3];
        std::string rule(width, '-');

        std::cout << rule << std::endl;

        std::cout << std::fixed;
        std::cout << std::setprecision(2);

        std::cout << std::setw(widths[0]) << "Net present value" << std::setw(widths[1])
                  << zeroCouponBond.NPV() << std::setw(widths[2]) << fixedRateBond.NPV()
                  << std::setw(widths[3]) << floatingRateBond.NPV() << std::endl;

        std::cout << std::setw(widths[0]) << "Clean price" << std::setw(widths[1])
                  << zeroCouponBond.cleanPrice() << std::setw(widths[2])
                  << fixedRateBond.cleanPrice() << std::setw(widths[3])
                  << floatingRateBond.cleanPrice() << std::endl;

        std::cout << std::setw(widths[0]) << "Dirty price" << std::setw(widths[1])
                  << zeroCouponBond.dirtyPrice() << std::setw(widths[2])
                  << fixedRateBond.dirtyPrice() << std::setw(widths[3])
                  << floatingRateBond.dirtyPrice() << std::endl;

        std::cout << std::setw(widths[0]) << "Accrued coupon" << std::setw(widths[1])
                  << zeroCouponBond.accruedAmount() << std::setw(widths[2])
                  << fixedRateBond.accruedAmount() << std::setw(widths[3])
                  << floatingRateBond.accruedAmount() << std::endl;

        std::cout << std::setw(widths[0]) << "Previous coupon" << std::setw(widths[1])
                  << "N/A" // zeroCouponBond
                  << std::setw(widths[2]) << io::rate(fixedRateBond.previousCouponRate())
                  << std::setw(widths[3]) << io::rate(floatingRateBond.previousCouponRate())
                  << std::endl;

        std::cout << std::setw(widths[0]) << "Next coupon" << std::setw(widths[1])
                  << "N/A" // zeroCouponBond
                  << std::setw(widths[2]) << io::rate(fixedRateBond.nextCouponRate())
                  << std::setw(widths[3]) << io::rate(floatingRateBond.nextCouponRate())
                  << std::endl;

        std::cout << std::setw(widths[0]) << "Yield" << std::setw(widths[1])
                  << io::rate(zeroCouponBond.yield(Actual360(), Compounded, Annual))
                  << std::setw(widths[2])
                  << io::rate(fixedRateBond.yield(Actual360(), Compounded, Annual))
                  << std::setw(widths[3])
                  << io::rate(floatingRateBond.yield(Actual360(), Compounded, Annual)) << std::endl;

        std::cout << std::endl;

        // Other computations
        std::cout << "Sample indirect computations (for the floating rate bond): " << std::endl;
        std::cout << rule << std::endl;

        std::cout << "Yield to Clean Price: "
                  << floatingRateBond.cleanPrice(
                         floatingRateBond.yield(Actual360(), Compounded, Annual), Actual360(),
                         Compounded, Annual, settlementDate)
                  << std::endl;

        std::cout << "Clean Price to Yield: "
                  << io::rate(floatingRateBond.yield(floatingRateBond.cleanPrice(), Actual360(),
                                                     Compounded, Annual, settlementDate))
                  << std::endl;

        /* "Yield to Price"
           "Price to Yield" */

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

int bootstrapSwap() {

    Date todaysDate(28, August, 2020);
    Settings::instance().evaluationDate() = todaysDate;

    Calendar rateHelperCalendar = UnitedStates(UnitedStates::LiborImpact);

    ext::shared_ptr<DepositRateHelper> dp1m = ext::make_shared<DepositRateHelper>(
        0.0017025, 1 * Months, 2, rateHelperCalendar, ModifiedFollowing, false, Actual360());
    ext::shared_ptr<DepositRateHelper> dp2m = ext::make_shared<DepositRateHelper>(
        0.0019038, 2 * Months, 2, rateHelperCalendar, ModifiedFollowing, false, Actual360());
    ext::shared_ptr<DepositRateHelper> dp3m = ext::make_shared<DepositRateHelper>(
        0.00251, 3 * Months, 2, rateHelperCalendar, ModifiedFollowing, false, Actual360());
    ext::shared_ptr<DepositRateHelper> dp6m = ext::make_shared<DepositRateHelper>(
        0.0030813, 6 * Months, 2, rateHelperCalendar, ModifiedFollowing, false, Actual360());
    ext::shared_ptr<DepositRateHelper> dp12m = ext::make_shared<DepositRateHelper>(
        0.0044, 12 * Months, 2, rateHelperCalendar, ModifiedFollowing, false, Actual360());

    // intentionally we do not provide a fixing for the euribor index used for
    // bootstrapping in order to be compliant with the ISDA specification

    ext::shared_ptr<IborIndex> usdLibor3M = ext::make_shared<USDLibor>(USDLibor(3 * Months));

    // check if indexed coupon is defined (it should not to be 100% consistent with
    // the ISDA spec)
    if (!IborCoupon::Settings::instance().usingAtParCoupons()) {
        std::cout << "Warning: IborCoupon::usingAtParCoupons() == false is used, "
                  << "which is not precisely consistent with the specification "
                  << "of the ISDA rate curve." << std::endl;
    }

    DayCounter fixedDayCount = Thirty360(Thirty360::Convention::BondBasis);

    ext::shared_ptr<SwapRateHelper> sw2y =
        ext::make_shared<SwapRateHelper>(0.002473, 2 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw3y =
        ext::make_shared<SwapRateHelper>(0.0026516, 3 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw4y =
        ext::make_shared<SwapRateHelper>(0.0030825, 4 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw5y =
        ext::make_shared<SwapRateHelper>(0.00372, 5 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw6y =
        ext::make_shared<SwapRateHelper>(0.000452, 6 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw7y =
        ext::make_shared<SwapRateHelper>(0.005357, 7 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw8y =
        ext::make_shared<SwapRateHelper>(0.0061475, 8 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw9y =
        ext::make_shared<SwapRateHelper>(0.006874, 9 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw10y =
        ext::make_shared<SwapRateHelper>(0.00753, 10 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw11y =
        ext::make_shared<SwapRateHelper>(0.008103, 11 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw12y =
        ext::make_shared<SwapRateHelper>(0.008611, 12 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw15y =
        ext::make_shared<SwapRateHelper>(0.0097065, 15 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw20y =
        ext::make_shared<SwapRateHelper>(0.0107923, 20 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);
    ext::shared_ptr<SwapRateHelper> sw30y =
        ext::make_shared<SwapRateHelper>(0.011445, 30 * Years, rateHelperCalendar, Semiannual,
                                         ModifiedFollowing, fixedDayCount, usdLibor3M);

    std::vector<ext::shared_ptr<RateHelper>> bootstrap_helpers;

    bootstrap_helpers.push_back(dp1m);
    bootstrap_helpers.push_back(dp2m);
    bootstrap_helpers.push_back(dp3m);
    bootstrap_helpers.push_back(dp6m);
    bootstrap_helpers.push_back(dp12m);
    bootstrap_helpers.push_back(sw2y);
    bootstrap_helpers.push_back(sw3y);
    bootstrap_helpers.push_back(sw4y);
    bootstrap_helpers.push_back(sw5y);
    bootstrap_helpers.push_back(sw6y);
    bootstrap_helpers.push_back(sw7y);
    bootstrap_helpers.push_back(sw8y);
    bootstrap_helpers.push_back(sw9y);
    bootstrap_helpers.push_back(sw10y);
    bootstrap_helpers.push_back(sw11y);
    bootstrap_helpers.push_back(sw12y);
    bootstrap_helpers.push_back(sw15y);
    bootstrap_helpers.push_back(sw20y);
    bootstrap_helpers.push_back(sw30y);

    Handle<YieldTermStructure> rateTs(ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
        todaysDate, bootstrap_helpers, Actual365Fixed()));
    rateTs->enableExtrapolation();

    // output rate curve
    std::cout << "Rate curve: " << std::endl;
    for (auto& bootstrap_helper : bootstrap_helpers) {
        Date d = bootstrap_helper->latestDate();
        std::cout << d << "\t" << setprecision(6)
                  << rateTs->zeroRate(d, Actual365Fixed(), Continuous).rate() << "\t"
                  << rateTs->discount(d) << std::endl;
    }

    return 0;
}


int main(int, char*[]) {
    // return stdExample();
    return bootstrapSwap();
}
