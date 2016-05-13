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

JSonNode::SetError JSonNode::setValue(QString , QString )
{
    return DoesNotExist;
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

    if(_jsonData.contains(name)){
        QJsonValue jsonV = _jsonData.value(name);
        if(jsonV.isDouble() && jsonV.toDouble() == v)
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

    if(_jsonData.contains(name)){
        QJsonValue jsonV = _jsonData.value(name);
        if(jsonV.isBool() && jsonV.toBool()==v)
            return SameValue;
    }

    _jsonData[name] = v;
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
    emit(out(addr+name+": "+d));
}

bool JSonNode::call(const QString &functionName, const QStringList &arg, QString& result){
    if(execFunction(functionName, arg, result))
        return true;

    if(functionName == "print"){
        result = print();
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
    QString r=_name;
    if(!_parentNode)
        return r;

    JSonNode* n = _parentNode;
    while(n->_parentNode){
        r.prepend(n->name()+".");
        n = n->_parentNode;
    }

    return r;
}



bool JSonNode::populateNode(QJsonValueRef& data)
{
    QJsonObject o = data.toObject();
    return populateNode(o);
}

bool JSonNode::populateNode(QJsonObject data)
{
    for(auto it = data.begin(); it!=data.end(); it++){
        QString k = it.key();
        QJsonValueRef v = it.value();
        if(v.isObject()){
            if(!createSubNode(k, v)){
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
    else
        updateParentJSon();

    return true;
}

bool JSonNode::createSubNode(QString, QJsonValueRef)
{
    return false;
}

void JSonNode::updateParentJSon()
{
    if(!_parentNode)
        return;

    _parentNode->_jsonData.find(_name).value() = _jsonData;
    _parentNode->updateParentJSon();
}

void JSonNode::addSubNode(JSonNode *node)
{
    _subnodes.append(node);
    connect(node, SIGNAL(out(QString)), this, SIGNAL(out(QString)));
}

void JSonNode::clearJsonData()
{
    foreach(JSonNode* n, _subnodes)
        delete n;
    _jsonData = QJsonObject();
}

JSonNode::~JSonNode()
{
    clearJsonData();
}



/****************************************************
 *                  JSonModel                       *
 ****************************************************/


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

    if(!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;
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
    if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
        return false;


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

JSonModel::JSonModel(QString name)
    :JSonNode(name, 0)
{
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
        timer.singleShot(5000, this, SLOT(writeJSonFile()));
}
