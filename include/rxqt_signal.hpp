#pragma once

#ifndef RXQT_SIGNAL_HPP
#define RXQT_SIGNAL_HPP

#include <rxcpp/rx.hpp>
#include <QObject>

namespace rxqt {

namespace signal {

namespace detail {

template <class Signal>
struct from_signal
{
};

template <class R, class Q>
struct from_signal<R(Q::*)()>
{
    using signal_type = R(Q::*)();
    using value_type = long;

    static rxcpp::observable<long> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<long>(
            [qobject, signal](rxcpp::subscriber<long> s){
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
struct from_signal<R(Q::*)(const A0&)>
{
    using signal_type = R(Q::*)(const A0&);
    using value_type = A0;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](rxcpp::subscriber<value_type> s){
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

template <class R, class Q, class A0, class ...Args>
struct from_signal<R(Q::*)(const A0&, const Args&...)>
{
    using signal_type = R(Q::*)(const A0&, const Args&...);
    using value_type = std::tuple<A0, Args...>;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](rxcpp::subscriber<value_type> s){
                QObject::connect(qobject, signal,[s](const A0& v0, const Args&... values){
                    s.on_next(std::make_tuple(v0, values...));
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

template <class Q, class Signal>
rxcpp::observable<typename signal::detail::from_signal<Signal>::value_type>
from_signal(Q* qobject, Signal signal)
{
    return signal::detail::from_signal<Signal>::create(qobject, signal);
}

} // qtrx

#endif // RXQT_SIGNAL_HPP
