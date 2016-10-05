#include <rxqt.hpp>
#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QKeyEvent>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto widget = std::unique_ptr<QWidget>(new QWidget());
    auto layout = new QVBoxLayout;
    widget->setLayout(layout);
    {
        auto e0 = new QLineEdit("Edit here");
        auto e1 = new QLineEdit;
        e1->setEnabled(false);
        layout->addWidget(e0);
        layout->addWidget(e1);

        rxqt::from_signal(e0, &QLineEdit::textChanged)
                .map([](const QString& s){ return "[[["+s+"]]]"; })
                .subscribe([e1](const QString& s){ e1->setText(s); });

        rxqt::from_event(e0, QEvent::KeyPress)
                .subscribe([](const QEvent* e){
                    auto ke = static_cast<const QKeyEvent*>(e);
                    qDebug() << ke->key();
                });
    }
    widget->show();
    return a.exec();
}
