#include <QDebug>
#include "recordxmlhandler.h"

RecordXmlHandler::RecordXmlHandler(QObject *parent) : QObject(parent)
{

}

bool RecordXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    if(localName.compare(tr("recordlist"))==0)
    {
        emit sendMessage(tr("Loading records list..."));
    }
    if(localName.compare(tr("record"))==0)
    {

        Record::Parameters param;
        param.sName=atts.value("Name");
        param.sSourcePath=atts.value("SourcePath");
        param.sDestPath=atts.value("DestPath");
        param.sExtensions=atts.value("Ext");
        param.sDesc=atts.value("Desc");
        param.sync=static_cast<Record::SyncType>(atts.value("Sync").toInt());
        param.type=static_cast<Record::RecordType>(atts.value("Type").toInt());
        param.sGroup=atts.value("Group");
        param.bSyncAuto=static_cast<bool>(atts.value("SyncAuto").toInt());
        param.nRecurrence=atts.value("Recurrence").toInt();
        Record* record= new Record;
        record->setParameters(param);

        emit loadRecord(record);

    }


    if(localName.compare(tr("group"))==0)
    {
        QString sName=atts.value("Name");
        emit loadGroup(sName);


    }

    return true;
}

bool RecordXmlHandler::error(const QXmlParseException &exception)
{
    QString info=tr("Error at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
    return true;
}

bool RecordXmlHandler::fatalError(const QXmlParseException &exception)
{
    QString info=tr("Fatal Error at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
     return true;
}

bool RecordXmlHandler::warning(const QXmlParseException &exception)
{

    QString info=tr("Warning at %1, %2").arg(exception.lineNumber()).arg(exception.columnNumber());
    emit sendMessage(info);
    qDebug()<<info<<exception.message();
     return true;
}


