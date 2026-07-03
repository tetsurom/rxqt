#pragma once
#ifndef RXQT_RUN_LOOP_THREAD_HPP

#include "rxqt_run_loop.hpp"

#include <QSemaphore>

namespace rxqt {
class RunLoopThread : public QThread {
public:
    using QThread::QThread;

    // Returns the run loop, blocking until run() has created it. The returned
    // pointer is only valid while the thread is running; callers must not use it
    // during or after the thread's destruction.
    rxqt::run_loop* runLoop() const
    {
        m_started.acquire();
        m_started.release(); // re-open the gate for other callers
        return m_runLoop.get();
    }

protected:
    void run() override
    {
        m_runLoop = std::make_unique<rxqt::run_loop>();
        m_started.release();
        QThread::run();
        m_runLoop = nullptr;
    }

private:
    std::unique_ptr<rxqt::run_loop> m_runLoop;
    mutable QSemaphore m_started;
};

} // namespace rxqt

#endif // RXQT_RUN_LOOP_THREAD_HPP
