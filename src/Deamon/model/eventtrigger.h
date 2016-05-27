#ifndef EVENTTRIGGER_H
#define EVENTTRIGGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <functional>
#include <QMutex>

typedef std::function<void(uint8_t)> EventCb;
typedef std::function<void()> SimplifiedEventCb;

class EventTrigger : public QObject
{
    Q_OBJECT

    QString _name;
    bool _inEvent;
    QMap<QString, EventCb> _callbacks;

    QMutex mutex;

public:
    explicit EventTrigger(QString name, bool inEvent, QObject* parent=0);
    QString name() const {return _name;}
    QStringList definitions() const;
    bool isInEvent() const {return _inEvent;}

    void addEvent(QString definition);
    void addEvent(QString definition, EventCb cb);
    void addEvent(QString definition, SimplifiedEventCb cb);
    bool removeEvent(QString definition);

signals:
    void triggrered(uint8_t data);
public slots:
    void trigger(uint8_t data = 0x00);
};

#endif // EVENTTRIGGER_H
