Build Status

| Platform | Status |
|:---------|:-------|
|Linux and OS X|[![Linux and OS X Build Status](https://travis-ci.org/tetsurom/rxqt.svg?branch=master)](https://travis-ci.org/tetsurom/rxqt)|
|Windows|[![Windows Build Status](https://ci.appveyor.com/api/projects/status/github/tetsurom/rxqt?svg=true)](https://ci.appveyor.com/api/projects/status/github/tetsurom/rxqt)|

# RxQt
The Reactive Extensions for Qt.

# Example

```cpp
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
```

# APIs

## from_signal

```cpp
observable<T> rxqt::from_signal(const QObject* sender, PointerToMemberFunction signal);
```

Convert Qt signal to a observable. `T` is decided as following.

|Signal parameter type(s)|T |Note |
|:---------------|:-|:----|
|`void`|`long`|Count of signal emission|
|`A0`|`A0`||
|`A0, A1, ..., An` (n > 1)|`tuple<A0, A1, ..., An>`||

```cpp
template<size_t N>
observable<T> rxqt::from_signal(const QObject* sender, PointerToMemberFunction signal);
```

Convert Qt signal, as N-ary function, to a observable. This can be used to convert private signals.

```cpp
auto o = from_signal(q, &QFileSystemWatcher::fileChanged); // ERROR. o is observable<tuple<QString, QFileSystemWatcher::QPrivateSignal>> where last type is private member.
auto o = from_signal<1>(q, &QFileSystemWatcher::fileChanged); // OK. o is observable<QString>
```

## from_event

```cpp
observable<QEvent*> rxqt::from_event(QObject* object, QEvent::Type type);
```

Convert Qt event to a observable.

## run_loop

Provide an interface between the Qt event loop and RxCpp's run loop scheduler. This enables use of timed RxCpp operators (such as `delay` and `debounce`) with RxQt.

Using `run_loop` requires an object of type `run_loop` to be live while the Qt event loop is active. This can be achieved most simply through use of a local variable defined after the Qt application object has been instantiated in `main`, or wherever the applicaiton object is instantiated. The example below gives an example of appropriate instantiation of the `run_loop`, together with a main window class of `MainWindow`:

```cpp
int main(int argc, char* argv[])
{
   QApplication a(argc, argv);
   MainWindow w;
   rxqt::run_loop rxqt_run_loop;
   w.show();
   return a.exec();
}
```

# Contribution

Issues or Pull Requests are welcomed :)

# Requirement

* [RxCpp](https://github.com/Reactive-Extensions/RxCpp) (minimum of v4.0 for use of `rxqt::run_loop`)
