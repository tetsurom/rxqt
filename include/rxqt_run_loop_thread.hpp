#pragma once
#ifndef RXQT_RUN_LOOP_THREAD_HPP

#include "rxqt_run_loop.hpp"

namespace rxqt {
class RunLoopThread : public QThread {
public:
    using QThread::QThread;

    rxqt::run_loop* runLoop() const
    {
        return m_runLoop.get();
    }

protected:
    void run() override
    {
        m_runLoop = std::make_unique<rxqt::run_loop>();
        QThread::run();
        m_runLoop = nullptr;
    }

private:
    std::unique_ptr<rxqt::run_loop> m_runLoop;
};

} // namespace rxqt

#endif // RXQT_RUN_LOOP_THREAD_HPP
