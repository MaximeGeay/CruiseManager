#include <QDebug>
#include "sensorxmlhandler.h"

SensorXmlHandler::SensorXmlHandler(QObject *parent) : QObject(parent)
{

}

bool SensorXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    if(localName.compare(tr("sensorlist"))==0)
    {
        emit sendMessage(tr("Loading sensors list..."));
       // qDebug()<<"Loading sensors list...";
    }
    if(localName.compare(tr("sensor"))==0)
    {

        Sensor::Parameters param;
        param.sName=atts.value("Name");
        param.sSourcePath=atts.value("SourcePath");
        param.sDestPath=atts.value("DestPath");
        param.sExtensions=atts.value("Ext");
        param.sDesc=atts.value("Desc");
        param.sync=static_cast<Sensor::SyncType>(atts.value("Sync").toInt());
      /*
        QString sValid=atts.value("Valid");
        if(sValid=="1")
            param.bValid=true;
        else
            param.bValid=false;
        */
        param.type=static_cast<Sensor::RecordType>(atts.value("Type").toInt());
        param.sGroup=atts.value("Group");
        Sensor* sensor= new Sensor;
        sensor->setParameters(param);

        emit loadSensor(sensor);

    }


    if(localName.compare(tr("group"))==0)
    {
        QString sName=atts.value("Name");
        emit loadGroup(sName);


    }

    return true;
}

bool SensorXmlHandler::error(const QXmlParseException &exception)
{
    QString info=tr("Error at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
}

bool SensorXmlHandler::fatalError(const QXmlParseException &exception)
{
    QString info=tr("Fatal Error at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
}

bool SensorXmlHandler::warning(const QXmlParseException &exception)
{

    QString info=tr("Warning at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
}


