#pragma once

#ifndef RXQT_SIGNAL_HPP
#define RXQT_SIGNAL_HPP

#include <rxcpp/rx.hpp>
#include <QObject>

#if defined(_MSC_VER) && (_MSC_VER<=1800)//Add some CXX14Support for VS2013
#include "CXX14Support.h"
#endif

namespace rxqt {

namespace signal {

namespace detail {

template <class Q, class T>
struct from_signal;

template <class Q>
struct from_signal<Q, std::tuple<>>
{
    using value_type = long;

    template <class S>
    static rxcpp::observable<long> create(const Q* qobject, S signal)
    {
        if(!qobject) return rxcpp::sources::never<value_type>();

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

template <class Q, class A0>
struct from_signal<Q, std::tuple<A0>>
{
    using value_type = std::remove_cv_t<std::remove_reference_t<A0>>;

    template <class S>
    static rxcpp::observable<value_type> create(const Q* qobject, S signal)
    {
        if (!qobject) return rxcpp::sources::never<value_type>();

        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](const rxcpp::subscriber<value_type>& s) {
                 QObject::connect(qobject, signal, [s](const A0& v0) {
                     s.on_next(v0);
                 });
                 QObject::connect(qobject, &QObject::destroyed, [s]() {
                     s.on_completed();
                 });
             }
        );
    }
};

template <class Q, class ...Args>
struct from_signal<Q, std::tuple<Args...>>
{
    using value_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

    template <class S>
    static rxcpp::observable<value_type> create(const Q* qobject, S signal)
    {
        if(!qobject) return rxcpp::sources::never<value_type>();

        return rxcpp::observable<>::create<value_type>(
            [qobject, signal](const rxcpp::subscriber<value_type>& s){
                QObject::connect(qobject, signal, [s](const Args&... values){
                    s.on_next(std::make_tuple(values...));
                });
                QObject::connect(qobject, &QObject::destroyed, [s](){
                    s.on_completed();
                });
            }
        );
    }
};

template <class T, class U>
struct tuple_subset;

template <class T, size_t... Is>
struct tuple_subset<T, std::index_sequence<Is...>>
{
    using type = std::tuple<std::tuple_element_t<Is, T>...>;
};

template <class T, size_t N>
struct tuple_take
{
    using type = typename tuple_subset<T, std::make_index_sequence<N>>::type;
};

template <class T, size_t N>
using tuple_take_t = typename tuple_take<T, N>::type;

template <class Q, class T>
struct is_private_signal : std::false_type {};

template <class Q>
struct is_private_signal<Q, typename Q::QPrivateSignal> : std::true_type {};

template <class Q, class T>
using has_private_signal = is_private_signal<Q, std::tuple_element_t<std::tuple_size<T>::value - 1, T>>;

template <size_t N, class Q, class ...Args>
struct signal_factory
{
    static_assert(N <= sizeof...(Args), "Cannot take larger number of parameter than the signal has.");
    using type = from_signal<Q, tuple_take_t<std::tuple<Args...>, N>>;
};

template <size_t N, class Q>
struct signal_factory<N, Q>
{
    static_assert(N <= 0, "Cannot take larger number of parameter than the signal has.");
    using type = from_signal<Q, std::tuple<>>;
};

template <size_t N, class Q, class ...Args>
using signal_factory_t = typename signal_factory<N, Q, Args...>::type;

} // detail

} // signal

template <size_t N, class P, class Q, class R, class ...Args>
auto from_signal(const P* qobject, R(Q::*signal)(Args...))
{
    static_assert(std::is_base_of<Q, P>::value, "Given signal is not member of sender class nor it's base class.");
    return signal::detail::signal_factory_t<N, Q, Args...>::create(static_cast<const Q*>(qobject), signal);
}

template <class P, class Q, class R, class ...Args>
auto from_signal(const P* qobject, R(Q::*signal)(Args...))
{
    return from_signal<sizeof...(Args)>(qobject, signal);
}

} // rxqt

#endif // RXQT_SIGNAL_HPP
