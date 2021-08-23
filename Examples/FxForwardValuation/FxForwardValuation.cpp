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
#include <ql/auto_link.hpp>
#endif

#include <ql/currencies/all.hpp>
#include <ql/currency.hpp>
#include <ql/exchangerate.hpp>
#include <ql/forwardexchangerate.hpp>
#include <ql/instruments/foreignexchangeforward.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/fx/forwardpointsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/fxforwardpointtermstructure.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/period.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }
}
#endif

Handle<FxForwardPointTermStructure> usdEurFwdPointStructure(const Date& todaysDate);
Handle<FxForwardPointTermStructure> eurUsdFwdPointStructure(const Date& todaysDate);
Handle<FxForwardPointTermStructure> gbpEurFwdPointStructure(const Date& todaysDate);
Handle<FxForwardPointTermStructure> eurGbpFwdPointStructure(const Date& todaysDate);
Handle<YieldTermStructure> discountingEurCurve(const Date& todaysDate);
Handle<YieldTermStructure> discountingUsdCurve(const Date& todaysDate);
Handle<YieldTermStructure> discountingGbpCurve(const Date& todaysDate);

void runShortUsdEurExample(const Date& todaysDate);
void runShortGbpEurExample(const Date& todaysDate);
void runLongUsdEurExample(const Date& todaysDate);
void runLongGbpEurExample(const Date& todaysDate);

void printResults(const ext::shared_ptr<ForeignExchangeForward> fxFwd);

int main(int, char*[]) {

    try {

        std::cout << std::endl;

        Date todaysDate(28, February, 2020);
        Settings::instance().evaluationDate() = todaysDate;
        Money::conversionType = Money::ConversionType::AutomatedConversion;
        std::cout << "Today: " << todaysDate.weekday() << ", " << todaysDate << std::endl << std::endl;

        ExchangeRateManager::instance().add(ExchangeRate(USDCurrency(), EURCurrency(), 0.9103736341));
        ExchangeRateManager::instance().add(ExchangeRate(GBPCurrency(), EURCurrency(), 1.1628202171));
        ExchangeRateManager::instance().add(ExchangeRate(CHFCurrency(), EURCurrency(), 0.9405171323));

        runShortUsdEurExample(todaysDate);
        std::cout << std::endl;
        runShortGbpEurExample(todaysDate);
        std::cout << std::endl;
        runLongUsdEurExample(todaysDate);
        std::cout << std::endl;
        runLongGbpEurExample(todaysDate);

        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

void runShortUsdEurExample(const Date& todaysDate) {
    Date deliveryDate = Date(4, March, 2020);
    Money baseNotionalAmount = Money(12925000, USDCurrency());
    ExchangeRate contractAllInRate = ExchangeRate(USDCurrency(), EURCurrency(), 0.897487215294618);

    ext::shared_ptr<ForeignExchangeForward> fxFwd(
        new ForeignExchangeForward(deliveryDate, baseNotionalAmount, contractAllInRate));

    std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

    // TODO: This is crap - the ExchangeRateManager should return a shared_ptr to the exchange rate instead of the actual instance
    ExchangeRate spotUsdEurRate = ExchangeRateManager::instance().lookup(USDCurrency(), EURCurrency());
    Handle<FxForwardPointTermStructure> eurUsdFwdCurve = eurUsdFwdPointStructure(todaysDate);
    Handle<FxForwardPointTermStructure> usdEurFwdCurve = usdEurFwdPointStructure(todaysDate);
    Handle<YieldTermStructure> eurDiscountCurve = discountingEurCurve(todaysDate);
    Handle<YieldTermStructure> usdDiscountCurve = discountingUsdCurve(todaysDate);

    ext::shared_ptr<ForwardPointsEngine> engine(new ForwardPointsEngine(
        spotUsdEurRate, usdEurFwdCurve, usdDiscountCurve, eurDiscountCurve));
    fxFwd->setPricingEngine(engine);

    printResults(fxFwd);

    // Base Leg:  11,600,022.36 EUR
    // Term Leg: -11,762,835.05 EUR
    // ----------------------------
    // NPV:         -162,812.69 EUR
    // ============================
}

void runShortGbpEurExample(const Date& todaysDate) {
    Date deliveryDate = Date(11, March, 2020);
    Money baseNotionalAmount = Money(40300000, GBPCurrency());
    ExchangeRate contractAllInRate = ExchangeRate(GBPCurrency(), EURCurrency(), 1.16992588519517);

    ext::shared_ptr<ForeignExchangeForward> fxFwd(
        new ForeignExchangeForward(deliveryDate, baseNotionalAmount, contractAllInRate));

    std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

    ExchangeRate spotBaseTermRate = ExchangeRateManager::instance().lookup(GBPCurrency(), EURCurrency());
    Handle<FxForwardPointTermStructure> termBaseFwdCurve = eurGbpFwdPointStructure(todaysDate);
    Handle<FxForwardPointTermStructure> baseTermFwdCurve = gbpEurFwdPointStructure(todaysDate);
    Handle<YieldTermStructure> termDiscountCurve = discountingEurCurve(todaysDate);
    Handle<YieldTermStructure> baseDiscountCurve = discountingGbpCurve(todaysDate);

    ext::shared_ptr<ForwardPointsEngine> engine(new ForwardPointsEngine(
        spotBaseTermRate, baseTermFwdCurve, baseDiscountCurve, termDiscountCurve));
    fxFwd->setPricingEngine(engine);

    printResults(fxFwd);

    // Base Leg:  47,148,013.17 EUR
    // Term Leg: -46,843,587.57 EUR
    // ----------------------------
    // NPV:         -304,425.60 EUR
    // ============================
}

void runLongUsdEurExample(const Date& todaysDate) {
    Date deliveryDate = Date(28, May, 2020);
    Money baseNotionalAmount = Money(24750000, USDCurrency());
    ExchangeRate contractAllInRate = ExchangeRate(USDCurrency(), EURCurrency(), 0.919214806712107);

    ext::shared_ptr<ForeignExchangeForward> fxFwd(
        new ForeignExchangeForward(deliveryDate, baseNotionalAmount, contractAllInRate));

    std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

    ExchangeRate spotUsdEurRate = ExchangeRateManager::instance().lookup(USDCurrency(), EURCurrency());
    Handle<FxForwardPointTermStructure> eurUsdFwdCurve = eurUsdFwdPointStructure(todaysDate);
    Handle<FxForwardPointTermStructure> usdEurFwdCurve = usdEurFwdPointStructure(todaysDate);
    Handle<YieldTermStructure> eurDiscountCurve = discountingEurCurve(todaysDate);
    Handle<YieldTermStructure> usdDiscountCurve = discountingUsdCurve(todaysDate);

    ext::shared_ptr<ForwardPointsEngine> engine(new ForwardPointsEngine(
        spotUsdEurRate, usdEurFwdCurve, usdDiscountCurve, eurDiscountCurve));
    fxFwd->setPricingEngine(engine);

    printResults(fxFwd);

    // Base Leg:  22,750,566.47 EUR
    // Term Leg: -22,412,996.84 EUR
    // ----------------------------
    // NPV:         -337,569.62 EUR
    // ============================
}

void runLongGbpEurExample(const Date& todaysDate) {
    Date deliveryDate = Date(28, May, 2020);
    Money baseNotionalAmount = Money(16925000, GBPCurrency());
    ExchangeRate contractAllInRate = ExchangeRate(GBPCurrency(), EURCurrency(), 1.19394431443717);

    ext::shared_ptr<ForeignExchangeForward> fxFwd(
        new ForeignExchangeForward(deliveryDate, baseNotionalAmount, contractAllInRate));

    std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

    ExchangeRate spotBaseTermRate = ExchangeRateManager::instance().lookup(GBPCurrency(), EURCurrency());
    Handle<FxForwardPointTermStructure> termBaseFwdCurve = eurGbpFwdPointStructure(todaysDate);
    Handle<FxForwardPointTermStructure> baseTermFwdCurve = gbpEurFwdPointStructure(todaysDate);
    Handle<YieldTermStructure> termDiscountCurve = discountingEurCurve(todaysDate);
    Handle<YieldTermStructure> baseDiscountCurve = discountingGbpCurve(todaysDate);

    ext::shared_ptr<ForwardPointsEngine> engine(new ForwardPointsEngine(
        spotBaseTermRate, baseTermFwdCurve, baseDiscountCurve, termDiscountCurve));
    fxFwd->setPricingEngine(engine);

    printResults(fxFwd);

    // Base Leg:  20,207,507.52 EUR
    // Term Leg: -19,621,824.42 EUR
    // ----------------------------
    // NPV:         -585,683.10 EUR
    // ============================
}

void printResults(const ext::shared_ptr<ForeignExchangeForward> fxFwd) {
    Money contractTermNotional = fxFwd->contractNotionalAmountTerm();
    Money forwardTermGross = fxFwd->forwardGrossValueTerm();
    Money forwardNetValue = fxFwd->forwardNetValueTerm();
    Money presentNetValue = fxFwd->presentNetValueTerm();
    std::cout << "Fair forward points: " << fxFwd->fairForwardPoints() << std::endl;
    std::cout << "Forward base leg value: " << contractTermNotional << std::endl;
    std::cout << "Forward term leg value: " << forwardTermGross << std::endl;
    std::cout << "Forward net value: " << forwardNetValue << std::endl;
    std::cout << "Present net value: " << presentNetValue << std::endl;
}

Handle<FxForwardPointTermStructure> usdEurFwdPointStructure(const Date& todaysDate) {

    Calendar calendar = JointCalendar(TARGET(), UnitedStates(UnitedStates::Market::FederalReserve));
    DayCounter dayCounter = Actual360();
    ExchangeRate spotExchRate =
        ExchangeRateManager::instance().lookup(USDCurrency(), EURCurrency());
    if (spotExchRate.source() != USDCurrency())
        spotExchRate = ExchangeRate::inverse(spotExchRate);

    std::vector<ForwardExchangeRate> fwdExchRates;
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -4.051701, Period(1, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -7.906924, Period(2, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -11.743311, Period(3, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -17.395392, Period(1, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -33.074375, Period(2, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -47.207796, Period(3, Months)));

    ext::shared_ptr<FxForwardPointTermStructure> fwdPtCurve(
        new InterpolatedFxForwardPointTermStructure<Linear>(todaysDate, fwdExchRates, dayCounter,
                                                            calendar));

    return Handle<FxForwardPointTermStructure>(fwdPtCurve);
}

Handle<FxForwardPointTermStructure> eurUsdFwdPointStructure(const Date& todaysDate) {

    Calendar calendar = JointCalendar(TARGET(), UnitedStates(UnitedStates::Market::FederalReserve));
    DayCounter dayCounter = Actual360();
    ExchangeRate spotExchRate =
        ExchangeRateManager::instance().lookup(EURCurrency(), USDCurrency());
    if (spotExchRate.source() != EURCurrency())
        spotExchRate = ExchangeRate::inverse(spotExchRate);

    std::vector<ForwardExchangeRate> fwdExchRates;
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 4.9, Period(1, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 9.625, Period(2, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 14.305, Period(3, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 21.155, Period(1, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 40.669, Period(2, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 57.975, Period(3, Months)));

    ext::shared_ptr<FxForwardPointTermStructure> fwdPtCurve(
        new InterpolatedFxForwardPointTermStructure<Linear>(todaysDate, fwdExchRates, dayCounter,
                                                            calendar));

    return Handle<FxForwardPointTermStructure>(fwdPtCurve);
}

Handle<FxForwardPointTermStructure> gbpEurFwdPointStructure(const Date& todaysDate) {

    Calendar calendar = JointCalendar(TARGET(), UnitedKingdom(UnitedKingdom::Market::Settlement));
    DayCounter dayCounter = Actual360();
    ExchangeRate spotExchRate = ExchangeRateManager::instance().lookup(GBPCurrency(), EURCurrency());
    if (spotExchRate.source() != GBPCurrency())
        spotExchRate = ExchangeRate::inverse(spotExchRate);

    std::vector<ForwardExchangeRate> fwdExchRates;
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -2.8, Period(1, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -6.91, Period(2, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -9.74, Period(3, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -12.13, Period(1, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -24.16, Period(2, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, -34.99, Period(3, Months)));

    ext::shared_ptr<FxForwardPointTermStructure> fwdPtCurve(
        new InterpolatedFxForwardPointTermStructure<Linear>(todaysDate, fwdExchRates, dayCounter,
                                                            calendar));

    return Handle<FxForwardPointTermStructure>(fwdPtCurve);
}

Handle<FxForwardPointTermStructure> eurGbpFwdPointStructure(const Date& todaysDate) {

    Calendar calendar = JointCalendar(TARGET(), UnitedKingdom(UnitedKingdom::Market::Settlement));
    DayCounter dayCounter = Actual360();
    ExchangeRate spotExchRate = ExchangeRateManager::instance().lookup(EURCurrency(), GBPCurrency());
    if (spotExchRate.source() != EURCurrency())
        spotExchRate = ExchangeRate::inverse(spotExchRate);

    std::vector<ForwardExchangeRate> fwdExchRates;
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 2.06, Period(1, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 4.01, Period(2, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 6.19, Period(3, Weeks)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 8.98, Period(1, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 17.85, Period(2, Months)));
    fwdExchRates.push_back(ForwardExchangeRate(spotExchRate, 25.97, Period(3, Months)));

    ext::shared_ptr<FxForwardPointTermStructure> fwdPtCurve(
        new InterpolatedFxForwardPointTermStructure<Linear>(todaysDate, fwdExchRates, dayCounter,
                                                            calendar));

    return Handle<FxForwardPointTermStructure>(fwdPtCurve);
}


Handle<YieldTermStructure> discountingEurCurve(const Date& todaysDate) {

    DayCounter termStructureDayCounter = ActualActual(ActualActual::ISDA);

    // deposits
    Integer fixingDays = 0;
    Calendar calendar = TARGET();
    Date settlementDate = calendar.advance(todaysDate, fixingDays, TimeUnit::Days);
    DayCounter depositDayCounter = Actual360();

    ext::shared_ptr<Quote> d1wRate(new SimpleQuote(-0.00518));
    ext::shared_ptr<Quote> d1mRate(new SimpleQuote(-0.00488));
    ext::shared_ptr<Quote> d3mRate(new SimpleQuote(-0.00424));
    ext::shared_ptr<Quote> d6mRate(new SimpleQuote(-0.00386));
    ext::shared_ptr<Quote> d1yRate(new SimpleQuote(-0.00311));

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

Handle<YieldTermStructure> discountingUsdCurve(const Date& todaysDate) {

    DayCounter termStructureDayCounter = ActualActual(ActualActual::ISDA);

    // deposits
    Integer fixingDays = 0;
    Calendar calendar = UnitedStates(UnitedStates::Market::FederalReserve);
    Date settlementDate = calendar.advance(todaysDate, fixingDays, TimeUnit::Days);
    DayCounter depositDayCounter = Actual360();

    ext::shared_ptr<Quote> d1wRate(new SimpleQuote(0.01568  ));
    ext::shared_ptr<Quote> d1mRate(new SimpleQuote(0.0151525));
    ext::shared_ptr<Quote> d3mRate(new SimpleQuote(0.0146275));
    ext::shared_ptr<Quote> d6mRate(new SimpleQuote(0.0139725));
    ext::shared_ptr<Quote> d1yRate(new SimpleQuote(0.013815 ));

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

Handle<YieldTermStructure> discountingGbpCurve(const Date& todaysDate) {

    DayCounter termStructureDayCounter = ActualActual(ActualActual::ISDA);

    // deposits
    Integer fixingDays = 0;
    Calendar calendar = UnitedKingdom(UnitedKingdom::Market::Settlement);
    Date settlementDate = calendar.advance(todaysDate, fixingDays, TimeUnit::Days);
    DayCounter depositDayCounter = Actual365Fixed(Actual365Fixed::Convention::Standard);

    ext::shared_ptr<Quote> d1wRate(new SimpleQuote(0.00681  ));
    ext::shared_ptr<Quote> d1mRate(new SimpleQuote(0.0067675));
    ext::shared_ptr<Quote> d3mRate(new SimpleQuote(0.0067275));
    ext::shared_ptr<Quote> d6mRate(new SimpleQuote(0.0068675));
    ext::shared_ptr<Quote> d1yRate(new SimpleQuote(0.0075038));

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