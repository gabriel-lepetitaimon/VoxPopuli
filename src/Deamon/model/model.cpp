#include "model.h"
#include <QDir>

#include <QDebug>


JSonNode::JSonNode(QString name, JSonNode *parent)
    : QObject(parent), _name(name), _jsonData(QJsonObject()), _parentNode(parent)
{

}

QVariant JSonNode::get(const QString &name)
{
    return _jsonData.value(name).toVariant();
}

bool JSonNode::getToString(const QString &name, QString &result) const
{
    QJsonValue o = _jsonData.value(name);
    if(o.isObject()){
        foreach(JSonNode* n, _subnodes)
            if(n->name()==name){
                result = n->print();
                return true;
            }
        return false;
    }

    QVariant v = o.toVariant();
    if(!v.isValid() || v.isNull())
        return false;
    switch(v.type()){
    case QVariant::Bool:
        result = v.toBool()?"true":"false";
        break;
    case QVariant::Double:
        result = QString().setNum(v.toDouble());
        break;
    case QVariant::String:
        result = '"'+v.toString()+'"';
        break;
    default:
        return false;
    }
    return true;
}

JSonNode::SetError JSonNode::set(const QString& name, const QString& value)
{
    return setValue(name, value);
}

JSonNode::SetError JSonNode::setValue(QString name, QString )
{
    return _jsonData.contains(name)?ReadOnly:DoesNotExist;
}

JSonNode::SetError JSonNode::setString(QString name, QString value)
{
    if(_jsonData.contains(name)){
        QJsonValue jsonV = _jsonData.value(name);
        if(jsonV.isString() && jsonV.toString() == value)
            return SameValue;
    }

    _jsonData[name] = value;
    valueChanged(name);
    return NoError;
}

JSonNode::SetError JSonNode::setNumber(QString name, QString value)
{
    bool r = true;
    double v = value.toDouble(&r);
    if(!r)
        return WrongArg;

    return setNumber(name, v);
}

JSonNode::SetError JSonNode::setNumber(QString name, double value)
{
    if(_jsonData.contains(name)){
        QJsonValue jsonV = _jsonData.value(name);
        if(jsonV.isDouble() && jsonV.toDouble() == value)
            return SameValue;
    }

    _jsonData[name] = value;
    valueChanged(name);
    return NoError;
}

JSonNode::SetError JSonNode::setBool(QString name, QString value)
{
    bool r = false;
    bool v = false;
    if(value.compare("true",Qt::CaseInsensitive) || value == "1"){
        r = true;
        v = true;
    }else if(value.compare("false",Qt::CaseInsensitive) || value == "0")
        r = true;

    if(!r)
        return WrongArg;

    return setBool(name, v);
}

JSonNode::SetError JSonNode::setBool(QString name, bool value)
{
    if(_jsonData.contains(name)){
        QJsonValue jsonV = _jsonData.value(name);
        if(jsonV.isBool() && jsonV.toBool()==value)
            return SameValue;
    }

    _jsonData[name] = value;
    valueChanged(name);
    return NoError;
}

QString JSonNode::print() const
{
    QString r = "\n{\n";
    foreach(QString name, _jsonData.keys()){
        QString d;
        if(getToString(name,d)){
            d.replace("\n","\n\t");
            r+="\t"+name+": "+d+"\n";
        }
    }
    r+="}";
    return r;
}

void JSonNode::valueChanged(QString name)
{
    updateParentJSon();
    QString addr = address();
    if(!addr.isEmpty()) addr+".";
    QString d;
    getToString(name, d);
    emit out(addr+(addr.isEmpty()?"":".")+name+": "+d);
}

bool JSonNode::call(const QString &functionName, const QStringList &arg, const std::function<void(QString)>& returnCb){
    if(execFunction(functionName, arg, returnCb))
        return true;

    if(functionName == "print"){
        returnCb(print());
        return true;
    }
    return false;
}

JSonNode *JSonNode::nodeAt(QString name)
{
    foreach(JSonNode* node, _subnodes)
        if(node->_name==name)
            return node;

    return 0;
}

QString JSonNode::address() const
{
    if(!_parentNode)
        return "";

    QString r=_name;

    JSonNode* n = _parentNode;
    while(n->_parentNode){
        r.prepend(n->name()+".");
        n = n->_parentNode;
    }

    return r;
}


bool JSonNode::populateNode(const QJsonObject& data)
{
    for(auto it = data.begin(); it!=data.end(); it++){
        QString k = it.key();
        QJsonValue v = it.value();
        if(v.isObject()){
            if(!createSubNode(k, v.toObject())){
                if(!_jsonData.isEmpty()){
                    foreach(JSonNode* n, _subnodes)
                        delete n;
                    populateNode(_jsonData);
                }
                return false;
            }
        }else if(v.isBool())
            setValue(k, v.toBool()?"true":"false");
        else if(v.isString())
            setValue(k,v.toString());
        else if(v.isDouble())
            setValue(k, QString().setNum(v.toDouble()));

    }
    _jsonData = data;

    if(_parentNode)
        _parentNode->addSubNode(this);

    updateParentJSon();

    return true;
}

bool JSonNode::createSubNode(QString , const QJsonObject &)
{
    return false;
}

void JSonNode::updateParentJSon()
{
    if(!_parentNode)
        return;

    _parentNode->_jsonData[_name] = _jsonData;
    _parentNode->updateParentJSon();
}

void JSonNode::addSubNode(JSonNode *node)
{
    _subnodes.append(node);
    _jsonData[node->name()] = node->_jsonData;
    connect(node, SIGNAL(out(QString)), this, SIGNAL(out(QString)));
}

void JSonNode::clearJsonData()
{
    foreach(JSonNode* n, _subnodes)
        delete n;
    _subnodes.clear();
    _jsonData = QJsonObject();
}

JSonNode::~JSonNode()
{
    clearJsonData();
}



/****************************************************
 *                  JSonModel                       *
 ****************************************************/
JSonModel::JSonModel(QString name)
    :JSonNode(name, 0)
{
    timer.setSingleShot(true);
    timer.setInterval(5000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(writeJSonFile()));
    connect(this, SIGNAL(startTimer()), &timer, SLOT(start()));
}

JSonModel::~JSonModel(){
    if(QFile(JSON).exists())
        QFile::remove(JSON);
}

bool JSonModel::loadFile(QString path)
{
    _watch.removePath(JSON);
    bool r = QFile::copy(path, JSON);
    _watch.addPath(JSON);

    if(!r)
        return false;

    return parseJSonFile();
}

bool JSonModel::saveToFile(QString path)
{
    if(timer.isActive()){
        timer.stop();
        writeJSonFile();
    }
    return QFile::copy(JSON, path);
}

bool JSonModel::resetFile()
{
    if(_watch.files().size())
        _watch.removePath(JSON);

    if(QFile(JSON).exists())
        QFile::remove(JSON);
    return initFile();
}

bool JSonModel::initFile()
{
    if(!QFile(JSON).exists()){
        if(_watch.files().size())
            _watch.removePath(JSON);
        bool r = QFile::copy(":/JSON/"+_name+".json", JSON);
        _watch.addPath(JSON);
    }
    return parseJSonFile();
}

bool JSonModel::parseJSonFile()
{
    QFile f(JSON);

    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){
        f.close();
        return false;
    }
        ///TODO: error return

    QByteArray jsonData = f.readAll();
    f.close();

    QJsonDocument data = QJsonDocument::fromJson(jsonData);

        ///TODO: error return;

    cleanJSon();
    _jsonDoc = data;

    return populateNode(_jsonDoc.object());
}

bool JSonModel::writeJSonFile()
{
    QFile f(JSON);
    if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)){
        if(!f.setPermissions(QFile::WriteUser|f.permissions()))
            return false;
        if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
            return false;
    }


    _watch.removePath(JSON);
    f.write(_jsonDoc.toJson());
    f.close();
    _watch.addPath(JSON);

    return true;
}

void JSonModel::cleanJSon()
{
    clearJsonData();
    _jsonDoc = QJsonDocument();
}

JSonNode *JSonModel::nodeByAddress(QString address)
{
    if(address.isEmpty())
        return this;

    QStringList path = address.split(".");
    JSonNode* n = this;
    for(int i=0; i<path.size(); i++){
        if(! (n = n->nodeAt(path[i])) )
            return 0;
    }
    return n;
}

void JSonModel::initModel()
{
    initFile();
    connect(&_watch, SIGNAL(fileChanged(QString)), this, SLOT(parseJSonFile()));
}

void JSonModel::updateParentJSon()
{
    _jsonDoc.setObject(_jsonData);
    if(!timer.isActive())
        emit(startTimer());
}
