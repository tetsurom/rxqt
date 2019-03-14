#include <rxqt.hpp>
#include <QtTest/QtTest>

namespace rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}

class RunLoopTest : public QObject
{
    Q_OBJECT

private:
    void flush(const rxqt::run_loop& rl) const
    {
        while(!rl.empty())
        {
            qApp->processEvents();
        }
    }

private slots:

    void empty()
    {
        using namespace std::chrono;
        rxqt::run_loop runLoop;

        QVERIFY(runLoop.empty());
        rx::observable<>::timer(milliseconds(5)).subscribe([&](auto){});
        QVERIFY(!runLoop.empty());
        flush(runLoop);
        QVERIFY(true);
    }

    void thread()
    {
        rxqt::run_loop runLoop;

        auto mainThread = runLoop.observe_on_run_loop();
        auto mainThreadId = QThread::currentThreadId();

        rx::observable<>::range(1)
            .subscribe_on(rx::observe_on_event_loop())
            .take(1)
            .tap([=](auto){ QVERIFY(mainThreadId != QThread::currentThreadId()); })
            .observe_on(mainThread)
            .subscribe([=](auto){ QVERIFY(mainThreadId == QThread::currentThreadId()); });
        
        flush(runLoop);
    }

};

QTEST_GUILESS_MAIN(RunLoopTest)
#include "run_loop_test.moc"
