#include <rxqt.hpp>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

using namespace std::chrono;

namespace Rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    rxqt::run_loop rxqt_run_loop;

    auto widget = std::make_unique<QWidget>();
    auto layout = new QVBoxLayout;
    widget->setLayout(layout);

    auto button = new QPushButton("Click me");
    auto label = new QLabel;

    layout->addWidget(button);
    layout->addWidget(label);

    auto count = std::make_shared<int>(0);

    rxqt::from_signal(button, &QPushButton::clicked)
            .map([=](const auto&){ return (*count) += 1; })
            .debounce(milliseconds(QApplication::doubleClickInterval()))
            .tap([=](int){ (*count) = 0; })
            .subscribe([label](int x){ label->setText(QString("%1-ple click.").arg(x)); });

    rxqt::from_signal(button, &QPushButton::pressed)
            .subscribe([=](const auto&){ label->setText(QString()); });

    widget->show();
    return app.exec();
}
