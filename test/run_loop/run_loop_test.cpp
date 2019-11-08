#include <QtTest/QtTest>
#include <rxqt.hpp>

namespace rx {
using namespace rxcpp;
using namespace rxcpp::subjects;
using namespace rxcpp::sources;
using namespace rxcpp::operators;
using namespace rxcpp::schedulers;
using namespace rxcpp::util;
}

using namespace std::chrono;

class RunLoopTest : public QObject {
    Q_OBJECT

private:
    void flush(const rxqt::run_loop& runLoop) const
    {
        while (!runLoop.empty()) {
            qApp->processEvents();
        }
    }

    template <class F>
    void wait_until(F f, milliseconds timeout = milliseconds(1000))
    {
        QTimer timer;
        timer.setSingleShot(true);
        timer.setTimerType(Qt::PreciseTimer);
        timer.start(timeout.count());
        while (!f() && timer.remainingTime() >= 0) {
            qApp->processEvents();
        }
        QVERIFY(timer.remainingTime() >= 0);
    }

private slots:

    void empty()
    {
        rxqt::run_loop runLoop;

        QVERIFY(runLoop.empty());
        rx::observable<>::timer(milliseconds(5)).subscribe([&](auto) {});
        QVERIFY(!runLoop.empty());
        flush(runLoop);
        QVERIFY(true);
    }

    void timer()
    {
        rxqt::run_loop runLoop;
        bool called = false;

        rx::observable<>::timer(milliseconds(5)).subscribe([&](auto) { called = true; });
        QVERIFY(!called);

        flush(runLoop);
        QVERIFY(called);
    }

    void thread()
    {
        rxqt::run_loop runLoop;

        auto mainThread = runLoop.observe_on_run_loop();
        auto mainThreadId = QThread::currentThreadId();
        bool called = false;

        rx::observable<>::range(1)
            .subscribe_on(rx::observe_on_event_loop())
            .take(1)
            .tap([=](auto) { QVERIFY(mainThreadId != QThread::currentThreadId()); })
            .observe_on(mainThread)
            .subscribe([&](auto) { called = true; QVERIFY(mainThreadId == QThread::currentThreadId()); });

        wait_until([&]() { return called; });
        flush(runLoop);
    }

    void subscribe_on_synchronize_new_thread()
    {
        rxqt::run_loop runLoop;
        bool called = false;

        auto mainthread = runLoop.observe_on_run_loop();
        auto workthread = rx::synchronize_new_thread();

        rx::observable<>::timer(milliseconds(5))
            .subscribe_on(workthread)
            .observe_on(mainthread)
            .subscribe([&](auto) { called = true; });

        QVERIFY(runLoop.empty());
        QVERIFY(!called);

        wait_until([&]() { return called; });

        flush(runLoop);
    }

    void subscribe_on_observe_on_event_loop()
    {
        rxqt::run_loop runLoop;
        bool called = false;

        auto mainthread = runLoop.observe_on_run_loop();
        auto workthread = rx::observe_on_event_loop();

        rx::observable<>::timer(milliseconds(5))
            .subscribe_on(workthread)
            .observe_on(mainthread)
            .subscribe([&](auto) { called = true; });

        QVERIFY(runLoop.empty());
        QVERIFY(!called);

        wait_until([&]() { return called; });

        flush(runLoop);
    }

    void observe_on_synchronize_new_thread()
    {
        rxqt::run_loop runLoop;
        bool called = false;

        auto workthread = rx::synchronize_new_thread();

        rx::observable<>::timer(milliseconds(5))
            .observe_on(workthread)
            .subscribe([&](auto) { called = true; });

        QVERIFY(!runLoop.empty());
        QVERIFY(!called);

        wait_until([&]() { return called; });

        flush(runLoop);
    }

    void observe_on_observe_on_event_loop()
    {
        rxqt::run_loop runLoop;
        bool called = false;

        auto workthread = rx::observe_on_event_loop();

        rx::observable<>::timer(milliseconds(5))
            .observe_on(workthread)
            .subscribe([&](auto) { called = true; });

        QVERIFY(!runLoop.empty());
        QVERIFY(!called);

        wait_until([&]() { return called; });

        flush(runLoop);
    }

    void sample_with_time()
    {
        rxqt::run_loop runLoop;

        rx::observable<>::range(1, 10000)
            .subscribe_on(rx::synchronize_new_thread())
            .sample_with_time(milliseconds(5), runLoop.observe_on_run_loop())
            .subscribe([](int) {});

        flush(runLoop);
    }
};

QTEST_GUILESS_MAIN(RunLoopTest)
#include "run_loop_test.moc"
