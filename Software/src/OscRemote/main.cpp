#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>
#include "oscremote.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQuickView view(QUrl("qrc:/main.qml"));
    QObject *item = view.rootObject();

    OSCRemote r(item);
    
    view.show();
    return app.exec();
}
