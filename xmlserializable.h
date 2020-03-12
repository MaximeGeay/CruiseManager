#ifndef XMLSERIALIZABLE_H
#define XMLSERIALIZABLE_H
class XMLSerializable;
#include<QXmlStreamWriter>


//Classe abstraite
class XMLSerializable
{
public:
    XMLSerializable();
    virtual void write(QXmlStreamWriter& writer)=0; //méthode virtuelle pure

};

#endif // XMLSERIALIZABLE_H
