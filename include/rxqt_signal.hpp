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
    using signal_type = R(Q::*)(Args...);
    using value_type = std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...>;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        if(!qobject) return rxcpp::sources::never<value_type>();

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

template <class R, class Q, class A0>
struct from_signal<R, Q, A0>
{
    using signal_type = R(Q::*)(A0);
    using value_type = typename std::remove_const<typename std::remove_reference<A0>::type>::type;

    static rxcpp::observable<value_type> create(const Q* qobject, signal_type signal)
    {
        if(!qobject) return rxcpp::sources::never<value_type>();

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

template <class Q, class T>
struct is_private_signal : std::false_type {};

template <class Q>
struct is_private_signal<Q, typename Q::QPrivateSignal> : std::true_type {};

template <class R, class Q, class T, class U>
struct construct_signal_type;

template <class R, class Q, class T, std::size_t... Is>
struct construct_signal_type<R, Q, T, std::index_sequence<Is...>>
{
    using type = from_signal<R, Q, typename std::tuple_element<Is, T>::type...>;
};

template <class R, class Q, class ...Args>
struct get_signal_factory
{
    using as_tuple = std::tuple<Args...>;
    static constexpr bool has_private_signal =
        is_private_signal<Q, typename std::tuple_element<sizeof...(Args) - 1, as_tuple>::type>::value;
    static constexpr size_t arg_count = has_private_signal ? sizeof...(Args) - 1 : sizeof...(Args);
    using type = typename construct_signal_type<R, Q, as_tuple, std::make_index_sequence<arg_count>>::type;
};

} // detail

} // signal

template <class R, class Q, class ...Args>
rxcpp::observable<typename signal::detail::get_signal_factory<R, Q, Args...>::type::value_type>
from_signal(const Q* qobject, R(Q::*signal)(Args...))
{
    using signal_factory = typename signal::detail::get_signal_factory<R, Q, Args...>::type;
    return typename signal_factory::create(qobject, reinterpret_cast<typename signal_factory::signal_type>(signal));
}

} // qtrx

#endif // RXQT_SIGNAL_HPP