#ifndef OSCMODEL_H
#define OSCMODEL_H

#include "eventmodel.h"
#include "qosc.h"

class QOscListener;

class OscInterface: public JSonNode
{
    Q_OBJECT

    QString _oscAddress;
    qosc::Listener* _oscListener;
    
public:
    OscInterface(EventModel * eventModel);
    virtual ~OscInterface();
    
    QString oscAddress() const {return _oscAddress;}
    void    setOscAddress(const QString& s);
    
    int port() const {return _oscListener->port();}
    void setPort(int port);
    
    qosc::Listener* oscListener() {return _oscListener;}
    
public slots:
    void readOscMsg(QString addr, QVariantList args);
    
protected:
    virtual JSonNode::SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual void generateHelp(bool function);
    
};


 

#endif // OSCMODEL_H
