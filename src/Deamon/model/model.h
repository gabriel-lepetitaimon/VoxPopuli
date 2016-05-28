#ifndef JSONMODEL_H
#define JSONMODEL_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QVariant>
#include <functional>

#include <QDebug>

class JSonNode;
class JSonModel;

class JSonNode: public QObject
{
    Q_OBJECT

public:
    enum SetError{
        SameValue = -1,
        NoError = 0,
        DoesNotExist,
        ReadOnly,
        WrongArg
    };

    enum JSonNodeFlag{
        NO_JSON_FLAG = 0,
        RENAMEABLE = 0x01
    };


    const QString&      name()          const {return _name;}
    JSonNode*           nodeAt(QString name);
    JSonNode*           parentNode()    const {return _parentNode;}

    QString             address() const;
    QVariant get(const QString& name);
    bool getToString(const QString& name, QString &result) const;

    void printOut();
    virtual QString print() const;

    bool populateNode(const QJsonObject &data);

    virtual ~JSonNode();

public slots:
    SetError set(const QString& name, const QString& value);
    bool call(const QString& functionName, const QStringList& arg, const std::function<void(QString)> &returnCb=[](QString){});

signals:
    void out(QString data);

protected:
    JSonNode(QString name, JSonNode *parent, const JSonNodeFlag& flags = NO_JSON_FLAG);

    virtual void updateParentJSon();
    virtual bool createSubNode(QString name, const QJsonObject& data);
    void addSubNode(JSonNode* node);
    void removeSubNode(JSonNode* node);
    void clearJsonData();

    virtual SetError setValue(QString name, QString value);
    JSonNode::SetError setString(QString name, QString value);
    SetError setNumber(QString name, QString value);
    SetError setNumber(QString name, double value);
    SetError setBool(QString name, QString value);
    SetError setBool(QString name, bool value);
    virtual SetError parseArray(QString name, QStringList value);
    void valueChanged(QString name);

    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& cb=[](QString){});
    void printOut(QString msg);

    bool rename(QString name);

    QString _name;
    const JSonNodeFlag _flags;
    QJsonObject _jsonData;
    JSonNode* _parentNode;

private:
    QList<JSonNode*> _subnodes;
};


#define JSON QDir::current().filePath(_name+".json")
class JSonModel : public JSonNode
{
    Q_OBJECT

public:
    JSonNode *nodeByAddress(QString address);
    virtual ~JSonModel();


public slots:
    bool loadFile(QString path);
    bool saveToFile(QString path);
    bool resetFile();
    bool initFile();


protected:
    explicit JSonModel(QString name);
    void initModel();
    void updateParentJSon();

protected slots:
    bool parseJSonFile();
    bool writeJSonFile();

signals:
    void startTimer();

private:
    void cleanJSon();

    QJsonDocument _jsonDoc;
    QFileSystemWatcher _watch;
    QTimer timer;

};

#endif // JSONMODEL_H
