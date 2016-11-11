#include "model.h"
#include <QDir>
#include <QJsonArray>
#include <QDebug>


JSonNode::JSonNode(QString name, JSonNode *parent, const JSonNodeFlag& flags )
    : QObject(parent), _name(name), _flags(flags), _jsonData(QJsonObject()), _parentNode(parent)
{

}

QVariant JSonNode::get(const QString &name) const
{
    QJsonValue v = _jsonData.value(name);
    if(v.isUndefined())
        return QVariant();
    return v.toVariant();
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

    QList<QVariant> l;

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
    case QVariant::List:
         l = v.toList();
         if(l.isEmpty()){
             result = "[]";
             return true;
         }
        result = '[';
        foreach(QVariant var, l){
            if(var.type() == QVariant::Bool)
                result += var.toBool()?"true":"false";
            else if(var.type() == QVariant::Double)
                result += QString().setNum(var.toDouble());
            else if(var.type() == QVariant::String)
                result += '"'+var.toString()+'"';
            result+=", ";
        }
        result.remove(result.size()-2, 2);
        result+=']';
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

JSonNode::SetError JSonNode::parseArray(QString name, QStringList )
{
    return _jsonData.contains(name)?ReadOnly:DoesNotExist;
}

bool JSonNode::execFunction(QString , QStringList , const std::function<void (QString)> &)
{
    return false;
}

QString JSonNode::print() const
{
    QString r = "{\n";
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

QStringList JSonNode::variablesNames() const
{
    return _jsonData.keys();
}

void JSonNode::printOut(QString msg)
{
    QString addr = address();
    if(msg.startsWith('.')){
        if(addr.isEmpty())
            msg.remove(0,1);
    }else if(!msg.startsWith(' '))
        msg.prepend(" ");

    emit out(addr+msg);
}

void JSonNode::addHelp(QString name, QString doc, bool function)
{
    if(function)
        _helpFunction->insert(name,doc);
    else
        _helpVariables->insert(name,doc);
}

QString JSonNode::helpString()
{
    QString r;
    const int maxColumn = 80;

    //function
    {
        r += " -- " + name() + " Functions  --\n";
        int definitionColumn = 0;
        foreach(QString f, getHelp(true).keys()) {
            if(f.length() > definitionColumn)
                definitionColumn = f.length();
        }
        definitionColumn += 3;


        for(auto it = getHelp(true).constBegin(); it!=getHelp(true).constEnd(); it++) {
            if(it.key()=="help()")
                continue;

            r += it.key();
            r += QString(' ').repeated(definitionColumn-it.key().length());

            QStringList docs = it.value().split(' ');
            int column = 0;
            for(int i=0; i<docs.size(); i++){
                if(docs.at(i).length() + column + definitionColumn > maxColumn){
                    if(column==0)
                        r+= docs.at(i);
                    r+="\n" + QString(' ').repeated(definitionColumn);
                    if(column!=0){
                        r+= docs.at(i)+' ';
                        column = docs.at(i).length() + 1;
                    }
                }else{
                    r+=docs.at(i)+' ';
                    column += docs.at(i).length() + 1;
                }
            }
            r+='\n';
        }
    }

    //variables
    if(!getHelp(false).isEmpty()){
        r += "\n -- " + name() + " Variables  --\n";
        int definitionColumn = 0;
        foreach(QString f, getHelp(false).keys()) {
            if(f.length() > definitionColumn)
                definitionColumn = f.length();
        }
        definitionColumn += 3;

        for(auto it = getHelp(false).constBegin(); it!=getHelp(false).constEnd(); it++) {
            r += it.key();
            r += QString(' ').repeated(definitionColumn-it.key().length());

            QStringList docs = it.value().split(' ');
            int column = 0;
            for(int i=0; i<docs.size(); i++){
                if(docs.at(i).length() + column + definitionColumn > maxColumn){
                    if(column==0)
                        r+= docs.at(i);
                    r+="\n" + QString(' ').repeated(definitionColumn);
                    if(column!=0){
                        r+= docs.at(i)+' ';
                        column = docs.at(i).length() + 1;
                    }
                }else{
                    r+=docs.at(i)+' ';
                    column = docs.at(i).length() + 1;
                }
            }
            r+='\n';
        }
    }

    return r;
}

bool JSonNode::rename(QString name)
{
    if(!_parentNode)
        return false;

    if(_parentNode->_jsonData.contains(name))
        return false;

    printOut("renamed \""+name+"\"");

    _parentNode->_jsonData.remove(_name);
    _name = name;
    updateParentJSon();

    return true;
}

void JSonNode::printOut(){
    printOut(print());
}

void JSonNode::valueChanged(QString name)
{
    updateParentJSon();
    QString addr = address();
    if(!addr.isEmpty()) addr+".";
    QString d;
    getToString(name, d);
    printOut("."+name+": "+d);
}

bool JSonNode::call(const QString &functionName, const QStringList &arg, const std::function<void(QString)>& returnCb){
    if(execFunction(functionName, arg, returnCb)){
        returnCb("");
        return true;
    }

    if(functionName == "print"){
        returnCb(print());
        return true;
    }else if(functionName == "rename" && (_flags&RENAMEABLE) && arg.size()==1){
        rename(arg.at(0));
        returnCb("");
        return true;
    }else if(functionName == "help"){
        returnCb(helpString());
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

    if(_name.contains(' '))
        return false;

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
        }else if(v.isArray()){
            QJsonArray array = v.toArray();
            QStringList r;
            for(int i=0; i<array.size(); i++){
                QJsonValue itV = array.at(i);
                if(itV.isString())
                    r+= itV.toString();
                else if(itV.isBool())
                    r+= itV.toBool()?"true":"false";
                else if(itV.isDouble())
                    r+= QString().setNum(v.toDouble());
            }
            parseArray(k, r);

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

    return true;
}

const QMap<QString, QString> &JSonNode::getHelp(bool function)
{
    if(function){
        if(!_helpFunction){
            _helpFunction = new QMap<QString,QString>();
            generateHelp(true);
            _helpFunction->insert("print()", "Print the json contents of this object.");
            _helpFunction->insert("help()",  "Print the help string of this object.");
            if(_flags&RENAMEABLE)
                _helpFunction->insert("rename( string )", "Rename this object.");
        }
        return *_helpFunction;
    }


    if(!_helpVariables){
        _helpVariables = new QMap<QString,QString>();
        generateHelp(false);
    }
    return *_helpVariables;
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
    printOut('.'+node->name()+" added");
    connect(node, SIGNAL(out(QString)), this, SIGNAL(out(QString)));
    node->printOut();
    updateParentJSon();
}

void JSonNode::removeSubNode(JSonNode *node)
{
    printOut('.'+node->name()+" removed");
    _jsonData.remove(node->name());
    _subnodes.removeOne(node);
    node->deleteLater();
    updateParentJSon();
}

void JSonNode::clearJsonData()
{
    foreach(JSonNode* n, _subnodes)
        n->deleteLater();
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
        if(!QFile::copy(":/JSON/"+_name+".json", JSON))
            return false;
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

void JSonModel::init()
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
