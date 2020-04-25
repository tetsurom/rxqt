#pragma once

#ifndef RXQT_RUN_LOOP_HPP

#include <QtGlobal>
#if QT_VERSION < 0x051000
#include <QCoreApplication>
#else
#include <QMetaObject>
#endif
#include <QThread>
#include <QTimer>
#include <rxcpp/rx.hpp>

namespace rxqt {

class run_loop : public QObject {
public:
    explicit run_loop(QObject* parent = nullptr)
        : QObject(parent)
    {
        timer = new QTimer(this);
        // Give the RxCpp run loop a a function to let us schedule a wakeup in order to dispatch run loop events
        rxcpp_run_loop.set_notify_earlier_wakeup([this](auto const& when) { this->on_earlier_wakeup(this->ms_until(when).count()); });
        timer->setSingleShot(true);
        timer->setTimerType(Qt::PreciseTimer);
        // When the timer expires, we'll flush the run loop
        timer->connect(timer, &QTimer::timeout, this, &run_loop::on_event_scheduled);
    }

    rxcpp::schedulers::scheduler get_scheduler() const
    {
        return rxcpp_run_loop.get_scheduler();
    }

    auto observe_on_run_loop() const
    {
        return rxcpp::observe_on_run_loop(rxcpp_run_loop);
    }

    bool empty() const
    {
        return rxcpp_run_loop.empty();
    }

private:
#if QT_VERSION < 0x051000
    class on_earlier_wakeup_event : public QEvent {
    public:
        on_earlier_wakeup_event(run_loop* run_loop, int msec)
            : QEvent(QEvent::None)
            , msec(msec)
            , run_loop_p(run_loop)
        {
        }

        ~on_earlier_wakeup_event() override
        {
            run_loop_p->on_earlier_wakeup(msec);
        }

    private:
        const int msec = 0;
        run_loop* run_loop_p = nullptr;
    };
#endif

    void on_earlier_wakeup(int msec)
    {
        // Tell the timer to wake-up at `when` if its not already waking up earlier
        if (this->thread() == QThread::currentThread()) {
            const int remainingTime = timer->remainingTime();
            if (remainingTime < 0 || msec < remainingTime) {
                timer->start(msec);
            }
        } else {
#if QT_VERSION < 0x051000
            QCoreApplication::postEvent(this, new on_earlier_wakeup_event(this, msec));
#else
            QMetaObject::invokeMethod(this, [this, msec] { on_earlier_wakeup(msec); });
#endif
        }
    }

    // Flush the RxCpp run loop
    void on_event_scheduled()
    {
        // Dispatch outstanding RxCpp events
        while (!rxcpp_run_loop.empty() && rxcpp_run_loop.peek().when < rxcpp_run_loop.now()) {
            rxcpp_run_loop.dispatch();
        }
        // If there are outstanding events, set the timer to wakeup for the first one
        if (!rxcpp_run_loop.empty()) {
            const auto time_till_next_event = ms_until(rxcpp_run_loop.peek().when);
            timer->start(static_cast<int>(time_till_next_event.count()));
        }
    }

    // Calculate milliseconds from now until `when`
    std::chrono::milliseconds ms_until(rxcpp::schedulers::run_loop::clock_type::time_point const& when) const
    {
        return (std::max)(ceil<std::chrono::milliseconds>(when - rxcpp_run_loop.now()), std::chrono::milliseconds::zero());
    }

    // Round the specified duration to the smallest number of `To` ticks that's not less than `duration`
    template <class To, class Rep, class Period>
    static inline To ceil(const std::chrono::duration<Rep, Period>& duration)
    {
        const auto as_To = std::chrono::duration_cast<To>(duration);
        return (as_To < duration) ? (as_To + To { 1 }) : as_To;
    }

    rxcpp::schedulers::run_loop rxcpp_run_loop;
    QTimer* timer = nullptr;
};

} // namespace rxqt

#endif // RXQT_RUN_LOOP_HPP
