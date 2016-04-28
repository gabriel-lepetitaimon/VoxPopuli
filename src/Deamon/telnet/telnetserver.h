#ifndef TELNETSERVER_H
#define TELNETSERVER_H

#include <QObject>

class TelnetServer : public QObject
{
    Q_OBJECT
public:
    explicit TelnetServer(QObject *parent = 0);

signals:

public slots:
};

#endif // TELNETSERVER_H