#pragma once

#ifndef RXQT_RUN_LOOP_HPP

#include <rxcpp/rx.hpp>
#include <QTimer>
#include <QThread>
#include <QSignalMapper>

namespace rxqt {

class run_loop: public QObject
{
 public:
   run_loop(QObject *parent = Q_NULLPTR) : QObject(parent), threadId(QThread::currentThreadId())
   {
      timer = new QTimer(this);
      mapper = new QSignalMapper(this);
      // Give the RxCpp run loop a a function to let us schedule a wakeup in order to dispatch run loop events
      rxcpp_run_loop.set_notify_earlier_wakeup([this](auto const& when) { this->on_earlier_wakeup(this->ms_until(when).count()); });
      timer->setSingleShot(true);
      timer->setTimerType(Qt::PreciseTimer);
      // When the timer expires, we'll flush the run loop
      timer->connect(timer, &QTimer::timeout, this, &run_loop::on_event_scheduled);
      mapper->connect(mapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &run_loop::on_earlier_wakeup);
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
   void on_earlier_wakeup(int msec)
   {
      // Tell the timer to wake-up at `when` if its not already waking up earlier
      if (threadId == QThread::currentThreadId())
      {
         const int remainingTime = timer->remainingTime();
         if (remainingTime < 0 || msec < remainingTime)
         {
            timer->start(msec);
         }
      }
      else
      {
         mapper->setMapping(this, msec);
         mapper->map(this);
         mapper->removeMappings(this);
      }
   }

   // Flush the RxCpp run loop
   void on_event_scheduled()
   {
      // Dispatch outstanding RxCpp events
      while (!rxcpp_run_loop.empty() && rxcpp_run_loop.peek().when < rxcpp_run_loop.now())
      {
         rxcpp_run_loop.dispatch();
      }
      // If there are outstanding events, set the timer to wakeup for the first one
      if (!rxcpp_run_loop.empty())
      {
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
      return (as_To < duration) ? (as_To + To{1}) : as_To;
   }

   rxcpp::schedulers::run_loop rxcpp_run_loop;
   QTimer* timer;
   Qt::HANDLE threadId;
   QSignalMapper* mapper;
};

} // namespace rxqt

#endif // RXQT_RUN_LOOP_HPP
