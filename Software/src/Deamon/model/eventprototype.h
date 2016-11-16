#ifndef EVENTPROTOTYPE_H
#define EVENTPROTOTYPE_H

#include <QObject>

class EventTrigger;

class EventPrototype : public QObject
{
    Q_OBJECT    

   
    virtual ~EventPrototype();

    const QString& definition() const {return _definition;}
    bool isInEvent() const {return _in;}
signals:

public slots:
    virtual bool triggerOut(uint8_t data) = 0;
    bool triggerIn(uint8_t data);

protected:
    explicit EventPrototype(bool inEvent, QString def, EventTrigger* trigger);

    void setupEvent();
    virtual void setupInEvent()   {}
    virtual void setupOutEvent()  {}

    virtual void removeInEvent()  {}
    virtual void removeOutEvent() {}

private:
    QString _definition;
    bool _in;
    EventTrigger* _trigger;
};

#endif // EVENTPROTOTYPE_H
