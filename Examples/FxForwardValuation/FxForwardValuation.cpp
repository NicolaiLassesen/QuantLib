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
#include <ql/utilities/dataformatters.hpp>

#include <iomanip>
#include <iostream>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }
}
#endif

int main(int, char*[]) {

    try {

        std::cout << std::endl;

        Date todaysDate(11, March, 2020);
        Settings::instance().evaluationDate() = todaysDate;

        std::cout << "Today: " << todaysDate.weekday() << ", " << todaysDate << std::endl;

        Date deliveryDate = TARGET().adjust(todaysDate + Period(3 * Months), Following);
        Currency baseCurrency = EURCurrency(),
                 termCurrency = USDCurrency();
        Real baseNotionalAmount = 10000;
        Rate contractAllInRate = 1.1389;

        ext::shared_ptr<ForeignExchangeForward> fxFwd(new ForeignExchangeForward(
            deliveryDate, baseCurrency, termCurrency, baseNotionalAmount, contractAllInRate));

        std::cout << "Valuation of FxFwd: " << *fxFwd << std::endl;

        Rate spotExchangeRate = 1.1351;
         
        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}
