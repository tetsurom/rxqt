#pragma once

#ifndef RXQT_RUN_LOOP_HPP

#include <rxcpp/rx.hpp>
#include <QTimer>

namespace rxqt {

class run_loop
{
 public:
   run_loop(QObject *parent = Q_NULLPTR) : timer(parent)
   {
      // Give the RxCpp run loop a a function to let us schedule a wakeup in order to dispatch run loop events
      rxcpp_run_loop.set_notify_earlier_wakeup([&](std::chrono::steady_clock::time_point when)
      {
         // Tell the timer to wake-up at `when` if its not already waking up earlier
         const auto ms_till_task = ms_until(when);
         if (!timer.isActive() || ms_till_task.count() < timer.remainingTime())
         {
            timer.start(ms_till_task.count());
         }
      });
      timer.setSingleShot(true);
      timer.setTimerType(Qt::PreciseTimer);
      // When the timer expires, we'll flush the run loop
      timer.connect(&timer, &QTimer::timeout, [&]() { onEventScheduled(); });
   }

private:
   // Flush the RxCpp run loop
   void onEventScheduled()
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
         timer.start(static_cast<int>(time_till_next_event.count()));
      }
   }
   // Calculate milliseconds from now until `when`
   std::chrono::milliseconds ms_until(rxcpp::schedulers::run_loop::clock_type::time_point const& when) const
   {
      return ceil<std::chrono::milliseconds>(when - rxcpp_run_loop.now());
   }
   // Round the specified duration to the smallest number of `To` ticks that's not less than `duration`
   template <class To, class Rep, class Period>
   static inline To ceil(const std::chrono::duration<Rep, Period>& duration)
   {
      const auto as_To = std::chrono::duration_cast<To>(duration);
      return (as_To < duration) ? (as_To + To{1}) : as_To;
   }
   rxcpp::schedulers::run_loop rxcpp_run_loop;
   QTimer timer;
};

} // namespace rxqt

#endif // RXQT_RUN_LOOP_HPP
