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
    using value_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

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
    using value_type = std::remove_cv_t<std::remove_reference_t<A0>>;

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
    using type = from_signal<R, Q, std::tuple_element_t<Is, T>...>;
};

template <class R, class Q, class ...Args>
struct get_signal_factory
{
    using as_tuple = std::tuple<Args...>;
    static constexpr bool has_private_signal =
        is_private_signal<Q, std::tuple_element_t<sizeof...(Args) - 1, as_tuple>>::value;
    static constexpr size_t arg_count = has_private_signal ? sizeof...(Args) - 1 : sizeof...(Args);
    using type = typename construct_signal_type<R, Q, as_tuple, std::make_index_sequence<arg_count>>::type;
};

template <class R, class Q>
struct get_signal_factory<R, Q>
{
    using type = from_signal<R, Q>;
};

} // detail

} // signal

template <class P, class R, class Q, class ...Args>
std::enable_if_t<std::is_base_of<Q, P>::value, rxcpp::observable<typename signal::detail::get_signal_factory<R, Q, Args...>::type::value_type>>
from_signal(const P* qobject, R(Q::*signal)(Args...))
{
    using signal_factory = typename signal::detail::get_signal_factory<R, Q, Args...>::type;
    return signal_factory::create(static_cast<const Q*>(qobject), reinterpret_cast<typename signal_factory::signal_type>(signal));
}

} // qtrx

#endif // RXQT_SIGNAL_HPP
