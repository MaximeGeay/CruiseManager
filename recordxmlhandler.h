#ifndef RECORDXMLHANDLER_H
#define RECORDXMLHANDLER_H

#include <QObject>
#include<QXmlDefaultHandler>

#include"record.h"

class RecordXmlHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    explicit RecordXmlHandler(QObject *parent = nullptr);
    virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    virtual bool error(const QXmlParseException &exception);
    virtual bool fatalError(const QXmlParseException &exception);
     virtual bool warning(const QXmlParseException &exception);

 signals:
    void loadRecord (Record* S);
    void loadGroup (QString sGroup);
    void sendMessage (QString s);

};

#endif // RECORDXMLHANDLER_H
