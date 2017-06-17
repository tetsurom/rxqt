#pragma once

#ifndef RXQT_UTIL_HPP
#define RXQT_UTIL_HPP

#include <rxcpp/rx.hpp>
#include <QObject>

namespace rxqt {

struct add_to {

    explicit add_to(const QObject* qobject): qobject(qobject) {};

    rxcpp::composite_subscription operator()(rxcpp::composite_subscription source) const
    {
        QObject::connect(qobject, &QObject::destroyed, [=](){
            source.unsubscribe();
        });
        return source;
    }

private:
    const QObject* qobject;
};

inline auto operator|(rxcpp::composite_subscription source, const add_to& func)
{
    return func(source);
}

} // rxqt

#endif // RXQT_UTIL_HPP
