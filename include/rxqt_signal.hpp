#pragma once

#ifndef RXQT_SIGNAL_HPP
#define RXQT_SIGNAL_HPP

#include <rxcpp/rx.hpp>
#include <QObject>

namespace rxqt {

namespace signal {

namespace detail {

template <class R, class Q, class ...Args>
struct from_signal
{
    using signal_type = R(Q::*)(const Args&...);
    using value_type = std::tuple<Args...>;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](const rxcpp::subscriber<value_type>& s){
                QObject::connect(qobject, signal,[s](const Args&... values){
                    s.on_next(std::make_tuple(values...));
                });
                QObject::connect(qobject, &QObject::destroyed, [s](){
                    s.on_completed();
                });
            }
        );
    }
};

template <class R, class Q>
struct from_signal<R, Q>
{
    using signal_type = R(Q::*)();
    using value_type = long;

    static rxcpp::observable<long> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<long>(
            [qobject, signal](const rxcpp::subscriber<long>& s){
                long counter = 0;
                QObject::connect(qobject, signal, [s, counter]() mutable {
                    s.on_next(counter++);
                });
                QObject::connect(qobject, &QObject::destroyed, [s](){
                    s.on_completed();
                });
            }
        );
    }
};

template <class R, class Q, class A0>
struct from_signal<R, Q, A0>
{
    using signal_type = R(Q::*)(const A0&);
    using value_type = A0;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](const rxcpp::subscriber<value_type>& s){
                QObject::connect(qobject, signal, [s](const A0& v0){
                    s.on_next(v0);
                });
                QObject::connect(qobject, &QObject::destroyed, [s](){
                    s.on_completed();
                });
            }
        );
    }
};

} // detail

} // signal

template <class R, class Q, class ...Args>
rxcpp::observable<typename signal::detail::from_signal<R, Q, Args...>::value_type>
from_signal(const Q* qobject, R(Q::*signal)(const Args&...))
{
    return signal::detail::from_signal<R, Q, Args...>::create(qobject, signal);
}

} // qtrx

#endif // RXQT_SIGNAL_HPP
