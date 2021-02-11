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

/*! \file forwardexchangerate.hpp
    \brief forward exchange rate between two currencies
*/

#ifndef quantlib_forward_exchange_rate_hpp
#define quantlib_forward_exchange_rate_hpp

#include <ql/exchangerate.hpp>
#include <ql/money.hpp>
#include <ql/utilities/null.hpp>
#include <ql/time/period.hpp>
#include <utility>

namespace QuantLib {

    //! forward exchange rate between two currencies
    /*! \test TODO
     */
    class ForwardExchangeRate {
      public:
        //! \name Constructors
        //@{
        ForwardExchangeRate(const Period& tenor = Period());
        ForwardExchangeRate(const ExchangeRate& spotRate,
                            const Decimal forwardPoints,
                            const Period& tenor = Period());
        //@}

        //! \name Inspectors
        //@{
        //! the source currency.
        const Currency& source() const;
        //! the target currency.
        const Currency& target() const;
        //! the type
        ExchangeRate::Type type() const;
        //! the spot exchange rate object
        const ExchangeRate& spotExchangeRate() const;
        //! the spot exchange rate
        Decimal spotRate() const;
        //! the forward points
        Decimal forwardPoints() const;
        //! the all-in forward exchange rate
        Decimal forwardRate() const;
        //! the tenor
        const Period& tenor() const;
        //@}

        //! \name Utility methods
        //@{
        //! apply the exchange rate to a cash amount
        Money exchange(const Money& amount) const;
        //! chain two exchange rates
        static ForwardExchangeRate chain(const ForwardExchangeRate& r1,
                                         const ForwardExchangeRate& r2);
        //! get inverse exchange rate
        static ForwardExchangeRate inverse(const ForwardExchangeRate& r);
        //@}
      private:
        ExchangeRate spotRate_;
        Decimal forwardPoints_;
        Period tenor_;
        ExchangeRate::Type type_;
        std::pair<ext::shared_ptr<ForwardExchangeRate>, ext::shared_ptr<ForwardExchangeRate> >
            rateChain_;
    };


    // inline definitions

    inline ForwardExchangeRate::ForwardExchangeRate(const Period& tenor)
    : spotRate_(ExchangeRate()), forwardPoints_(Null<Decimal>()), tenor_(tenor),
      type_(ExchangeRate::Direct) {}

    inline ForwardExchangeRate::ForwardExchangeRate(const ExchangeRate& spotRate,
                                                    Decimal forwardPoints,
                                                    const Period& tenor)
    : spotRate_(spotRate), forwardPoints_(forwardPoints), tenor_(tenor),
      type_(ExchangeRate::Direct) {}

    inline const Currency& ForwardExchangeRate::source() const { return spotRate_.source(); }

    inline const Currency& ForwardExchangeRate::target() const { return spotRate_.target(); }

    inline ExchangeRate::Type ForwardExchangeRate::type() const { return type_; }

    inline const ExchangeRate& ForwardExchangeRate::spotExchangeRate() const { return spotRate_; }

    inline Decimal ForwardExchangeRate::spotRate() const { return spotRate_.rate(); }

    inline Decimal ForwardExchangeRate::forwardPoints() const { return forwardPoints_; }

    inline Decimal QuantLib::ForwardExchangeRate::forwardRate() const {
        return spotRate_.rate() + forwardPoints_ / 10000.0;
    }

    inline const Period& ForwardExchangeRate::tenor() const { return tenor_; }

}

#endif