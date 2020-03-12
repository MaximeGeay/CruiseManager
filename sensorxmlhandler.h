#ifndef SENSORXMLHANDLER_H
#define SENSORXMLHANDLER_H

#include <QObject>
#include<QXmlDefaultHandler>

#include"sensor.h"

class SensorXmlHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    explicit SensorXmlHandler(QObject *parent = nullptr);
    virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    virtual bool error(const QXmlParseException &exception);
    virtual bool fatalError(const QXmlParseException &exception);
     virtual bool warning(const QXmlParseException &exception);

 signals:
    void loadSensor (Sensor* S);
    void loadGroup (QString sGroup);
    void sendMessage (QString s);

};

#endif // SENSORXMLHANDLER_H
