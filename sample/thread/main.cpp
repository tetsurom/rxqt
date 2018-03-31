#include <rxqt.hpp>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QThread>

#include <rxcpp/operators/rx-subscribe_on.hpp>
#include <rxcpp/operators/rx-observe_on.hpp>

using namespace std::chrono;

namespace rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::schedulers;
    using namespace rxcpp::util;
}

QString threadId(const QString& name)
{
    return name + QString(": %1\n").arg((int)QThread::currentThreadId());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    rxqt::run_loop rxqt_run_loop;

    auto widget = std::make_unique<QWidget>();
    auto layout = new QVBoxLayout();

    auto label = new QLabel(widget.get());
    layout->addWidget(label);

    widget->setLayout(layout);
    widget->show();

    label->setText(label->text() + threadId("main"));

    auto main_thread = rxqt_run_loop.observe_on_run_loop();

    rx::observable<>::range(1)
        .subscribe_on(rx::observe_on_event_loop())
        .take(1)
        .tap([&](int x){ label->setText(label->text() + threadId("subscrive_on")); })
        .observe_on(main_thread)
        .subscribe([&](int x){ label->setText(label->text() + threadId("observe_on")); });

    return app.exec();
}
