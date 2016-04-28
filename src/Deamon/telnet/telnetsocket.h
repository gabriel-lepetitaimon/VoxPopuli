#ifndef TELNETSOCKET_H
#define TELNETSOCKET_H

#include <QObject>

class TelnetSocket : public QObject
{
    Q_OBJECT
public:
    explicit TelnetSocket(QObject *parent = 0);

signals:

public slots:
};

#endif // TELNETSOCKET_H