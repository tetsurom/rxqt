#include <rxqt.hpp>
#include <QtTest/QtTest>

class TestObservable : public QObject
{
    Q_OBJECT
private slots:
    void fromsignal_nullary()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal(&subject, &TestObservable::signal_nullary).subscribe([&](long c) {
                QVERIFY(c == 0);
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_nullary();
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromSignal_unary_int()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal(&subject, &TestObservable::signal_unary_int).subscribe([&](int c) {
                QVERIFY(c == 1);
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_unary_int(1);
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromSignal_unary_string()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal(&subject, &TestObservable::signal_unary_string).subscribe([&](const QString& s) {
                QVERIFY(s == "string");
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_unary_string(QString("string"));
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromSignal_binary()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal(&subject, &TestObservable::signal_binary).subscribe([&](const std::tuple<int, QString>& t) {
                QVERIFY(std::get<0>(t) == 1);
                QVERIFY(std::get<1>(t) == "string");
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_binary(1, QString("string"));
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromPrivateSignal_nullary()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal<0>(&subject, &TestObservable::signal_private_nullary).subscribe([&](long c) {
                QVERIFY(c == 0);
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_private_nullary(QPrivateSignal());
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromPrivateSignal_unary_int()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal<1>(&subject, &TestObservable::signal_private_unary_int).subscribe([&](int c) {
                QVERIFY(c == 1);
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_private_unary_int(1, QPrivateSignal());
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void fromPrivateSignal_binary()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            rxqt::from_signal<2>(&subject, &TestObservable::signal_private_binary).subscribe([&](const std::tuple<int, const QString>& t) {
                QVERIFY(std::get<0>(t) == 1);
                QVERIFY(std::get<1>(t) == "string");
                called = true;
            }, [&]() { completed = true; });
            emit subject.signal_private_binary(1, QString("string"), QPrivateSignal());
        }
        QVERIFY(called);
        QVERIFY(completed);
    }

    void add_to()
    {
        bool called = false;
        bool completed = false;
        {
            TestObservable subject;
            QObject* dummy = new TestObservable();
            rxqt::from_signal(&subject, &TestObservable::signal_nullary).subscribe([&](long c) {
                QVERIFY(c == 0);
                called = true;
            }, [&]() { completed = true; }) | rxqt::add_to(dummy);
            delete dummy; // result into unsubscribe
            emit subject.signal_nullary();
        }
        QVERIFY(!called);
        QVERIFY(!completed);
    }

signals:
    void signal_nullary();
    void signal_unary_int(int);
    void signal_unary_string(const QString&);
    void signal_binary(int, const QString&);

    void signal_private_nullary(QPrivateSignal);
    void signal_private_unary_int(int, QPrivateSignal);
    void signal_private_binary(int, const QString&, QPrivateSignal);
};

QTEST_GUILESS_MAIN(TestObservable)
#include "signaltest.moc"
