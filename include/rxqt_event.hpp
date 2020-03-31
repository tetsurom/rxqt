#pragma once

#ifndef RXQT_EVENT_HPP
#define RXQT_EVENT_HPP

#include <QObject>
#include <QEvent>
#include <rxcpp/rx.hpp>

namespace rxqt {

namespace event {

    namespace detail {

        class EventFilter : public QObject {
        public:
            EventFilter(QObject* parent, QEvent::Type type, rxcpp::subscriber<QEvent*> s)
                : QObject(parent)
                , type(type)
                , s(s)
            {
            }
            ~EventFilter()
            {
                s.on_completed();
            }

            bool eventFilter(QObject* obj, QEvent* event)
            {
                if (event->type() == type) {
                    s.on_next(event);
                }
                return QObject::eventFilter(obj, event);
            }

        private:
            QEvent::Type type;
            rxcpp::subscriber<QEvent*> s;
        };

    } // detail

} // event

rxcpp::observable<QEvent*> inline from_event(QObject* qobject, QEvent::Type type)
{
    if (!qobject)
        return rxcpp::sources::never<QEvent*>();

    return rxcpp::observable<>::create<QEvent*>(
        [qobject, type](rxcpp::subscriber<QEvent*> s) {
            auto eventFilter = new event::detail::EventFilter(qobject, type, s);
            qobject->installEventFilter(eventFilter);

            s.add([eventFilter, qobject] {
                qobject->removeEventFilter(eventFilter);
                eventFilter->deleteLater();
            });
        });
}

} // rxqt

#endif // RXQT_EVENT_HPP
