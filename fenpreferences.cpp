#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QXmlSimpleReader>
//#include <QXmlStreamWriter>
#include <QVBoxLayout>
#include <QDebug>
#include <QListWidget>
#include <QInputDialog>




#include "fenpreferences.h"
#include "ui_fenpreferences.h"

#include "sensorxmlhandler.h"

fenPreferences::fenPreferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fenPreferences)
{
    ui->setupUi(this);

    mSettings=new QSettings("CruiseManager","Settings");

    QObject::connect(ui->btn_ParcCruises,&QToolButton::clicked,this,&fenPreferences::selectCruisesPath);
    QObject::connect(ui->btn_ParcConfMission,&QToolButton::clicked,this,&fenPreferences::selectConfMissionPath);
    QObject::connect(ui->btn_Valider,&QPushButton::clicked,this,&fenPreferences::valider);
    QObject::connect(ui->btn_annuler,&QPushButton::clicked,this,&fenPreferences::annuler);
    QObject::connect(ui->btn_AddSensor,&QToolButton::clicked,this,&fenPreferences::addNewSensor);
    QObject::connect(ui->btn_rmSensor,&QToolButton::clicked,this,&fenPreferences::removeSensor);
    QObject::connect(ui->listSensorWidget,&QListWidget::itemClicked,this,&fenPreferences::listSensorSelected);
    QObject::connect(ui->listGroupWidget,&QListWidget::itemClicked,this,&fenPreferences::listGroupSelected);
    QObject::connect(ui->btn_AddGroup,&QToolButton::clicked,this,&fenPreferences::addGroup);
    QObject::connect(ui->btn_EditGroup,&QToolButton::clicked,this,&fenPreferences::editGroup);
    QObject::connect(ui->btn_rmGroup,&QToolButton::clicked,this,&fenPreferences::rmGroup);

    mCurrentSensor=new Sensor;



    mSensorLayout=new QVBoxLayout(ui->gb_Sensors);
    mSensorLayout->setAlignment(Qt::AlignTop);
    mSensorLayout->setDirection(QBoxLayout::TopToBottom);


    mPathToMissions=mSettings->value("Mission/Path",QDir::homePath()).toString();
    mPathToConfMission=mSettings->value("PathToConfMission",QDir::homePath()).toString();

    majListItem();
    majGroupListWidget();


}

fenPreferences::~fenPreferences()
{
    delete ui;
}

void fenPreferences::init()
{
    mPathToMissions=mSettings->value("Mission/Path",QDir::homePath()).toString();
    mPathToConfMission=mSettings->value("PathToConfMission",QDir::homePath()).toString();

    ui->le_RepCruises->setText(mPathToMissions);
    ui->le_ConfMission->setText(mPathToConfMission);

    majListItem();
    majGroupListWidget();


    show();
}

QStringList fenPreferences::getDataGroups()
{
    return mGroupList;
}

QList<Sensor *> fenPreferences::getSensorsList()
{
    return mSensorList;
}

void fenPreferences::setCurrentCruise(QString sCruise)
{
    mCurrentCruise=sCruise;
    mNewCruise=true;

    majCurrentCruise();
}

void fenPreferences::endMissionEdit()
{
    mNewCruise=false;
}

QList<Sensor *> fenPreferences::getSensorsListFromGroups(QStringList groupList)
{
    QList<Sensor*> sensorList;

    QListIterator<Sensor*>itS(mSensorList);
    while (itS.hasNext()) {
        Sensor* sensor=itS.next();
        if(groupList.contains(sensor->getParameters().sGroup))
            sensorList.append(sensor);

    }
    return sensorList;

}

void fenPreferences::selectCruisesPath()
{
    QString sChemin=QFileDialog::getExistingDirectory(this,tr("Sélectionner le répertoire Missions"),mPathToMissions,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
        ui->le_RepCruises->setText(sChemin);
}

void fenPreferences::selectConfMissionPath()
{
    QString sChemin=QFileDialog::getExistingDirectory(this,tr("Sélectionner le répertoire ConfMission"),mPathToConfMission,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
        ui->le_ConfMission->setText(sChemin);
}

void fenPreferences::valider()
{
    QDir dir;
    if(dir.exists(ui->le_RepCruises->text()))
    {
        mSettings->setValue("Mission/Path",ui->le_RepCruises->text());
    }
    else
    {
        QMessageBox::warning(this,tr("Répertoire invalide"),QString("Le répertoire Missions est invalide.\r\nLe répertoire %1 n'éxiste pas").arg(ui->le_RepCruises->text()));
        return;
    }

    if(dir.exists(ui->le_ConfMission->text()))
    {
        mSettings->setValue("PathToConfMission",ui->le_ConfMission->text());
    }
    else
    {
        QMessageBox::warning(this,tr("Répertoire invalide"),QString("Le répertoire ConfMission est invalide.\r\nLe répertoire %1 n'éxiste pas").arg(ui->le_ConfMission->text()));
        return;
    }

    saveSensorList();
    emit editingFinished();
    this->close();


}

void fenPreferences::annuler()
{
    this->close();
}

void fenPreferences::addNewSensor()
{
    Sensor *sensor=new Sensor;
    sensor->setDeverrouiller(true);
    mbNewSensor=true;
    addSensor(sensor);



}

void fenPreferences::addSensor(Sensor *sensor)
{

    mCurrentSensor->hide();
    sensor->setGroupList(mGroupList);
    mSensorList.append(sensor);
    //ui->listSensorWidget->addItem(sensor->getParameters().sName);

    mCurrentSensor=sensor;
    mSensorLayout->addWidget(mCurrentSensor);

    if(mCurrentSensor->getParameters().sName.isEmpty())
        mCurrentSensor->show();
    else
        mCurrentSensor->hide();
    //sensor->show();

    QObject::connect(sensor,&Sensor::editingStatusChanged,this,&fenPreferences::editingStatusControl);
}

void fenPreferences::majCurrentCruise()
{
  /*  QString sPathToNewCruise=QString("%1/%2").arg(mPathToMissions).arg(mCurrentCruise);
    QString sRelativePath;

    QList<Sensor*>tempList;
    foreach(Sensor* sensor,mSensorList)
    {
        Sensor::Parameters param=sensor->getParameters();
        sRelativePath=param.sDestPath.section(mPathToMissions,-1).section("/",2,-1);

        param.sDestPath=QString("%1/%2").arg(sPathToNewCruise).arg(sRelativePath);
        sensor->setParameters(param);
        tempList.append(sensor);
    }

    mSensorList=tempList;
    saveSensorList();*/
}

void fenPreferences::removeSensor()
{
    QString sName=mCurrentSensor->getParameters().sName;
    int nRes=QMessageBox::question(this,QString("Suppression de l'enregistrement %1").arg(sName),QString("Etes-vous sûrs de vouloir supprimer l'enregistrement %1").arg(sName));
    if(nRes==QMessageBox::Yes)
    {
        mSensorList.removeOne(mCurrentSensor);
        saveSensorList();
        majListItem();
    }

}

void fenPreferences::showMessage(QString sMsg)
{
    qDebug()<< sMsg;
}

void fenPreferences::editingStatusControl()
{

    if(!mNewCruise)
    {
    int n=0;
    foreach(Sensor* oneSensor,mSensorList)
    {
        if(oneSensor->getEditStatus()==true)
            n++;
    }
    if(n>0)
    {
        ui->btn_Valider->setEnabled(false);
    }
    else
        ui->btn_Valider->setEnabled(true);
    qDebug()<<"SensorList"<<mSensorList.count();


    if(!mCurrentSensor->getEditStatus())
    {
        if(mbNewSensor)
        {
            saveSensorList();
            mbNewSensor=false;
        }

        majListItem();
    }
    }
}

bool fenPreferences::listSensorSelected(QListWidgetItem *item)
{
    mCurrentSensor->hide();

    QString sName=item->text();

    foreach(Sensor* sensor,mSensorList)
    {
        if(sensor->getParameters().sName==sName)
        {
            mCurrentSensor=sensor;
            mCurrentSensor->show();
            ui->btn_rmSensor->setVisible(true);
            QObject::connect(mCurrentSensor,&Sensor::editingFinished,this,&fenPreferences::sensorEditionFinished);
            return true;
        }
    }
    return false;


}

void fenPreferences::majListItem()
{

    mSensorList.clear();
    mGroupList.clear();
    ui->listSensorWidget->clear();

    ui->btn_rmSensor->setVisible(false);

    QString sFileOpen=QString("%1/sensors.xml").arg(mPathToConfMission);
    QFile file(sFileOpen);

    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,tr("Open Xml File"),tr("The xml file is not readable"));
        return;
    }
    QXmlSimpleReader reader;
    SensorXmlHandler handler;
    QObject::connect(&handler,&SensorXmlHandler::loadSensor,this,&fenPreferences::addSensor);
    QObject::connect(&handler,&SensorXmlHandler::sendMessage,this,&fenPreferences::showMessage);
    QObject::connect(&handler,&SensorXmlHandler::loadGroup,this,&fenPreferences::appendGroup);

    reader.setContentHandler(&handler);
    QXmlInputSource source(&file);
    bool result=reader.parse(source);
    file.close();

    majGroupListWidget();

}

void fenPreferences::sensorEditionFinished()
{
    saveSensorList();
}

void fenPreferences::addGroup()
{
    QString sGroup=QInputDialog::getText(this,"Ajout d'un nouveau groupe de données",QString("Veuillez entrer le nom du nouveau groupe de données"));
    if(!sGroup.isEmpty())
    {
        mGroupList.append(sGroup);
        majGroupListWidget();
    }
}

void fenPreferences::editGroup()
{
    QString sPrevious=ui->listGroupWidget->currentItem()->text();
    int nRes=QMessageBox::question(this,QString("Modification du groupe %1").arg(sPrevious),QString("Vous vous apprêtez à modifier le groupe %1.\n"
                                                                                                    "L'ensemble des enregistrements affectés à ce groupe seront modifiés.\n"
                                                                                                    " Etes-vous sûr de vouloir modifier le groupe %1?").arg(sPrevious));
    if(nRes==QMessageBox::Yes)
    {
        QString sGroup=QInputDialog::getText(this,QString("Modification d'un groupe"),QString("Veuillez entrer le nouveau nom du groupe de données"),QLineEdit::Normal,sPrevious);
        if(!sGroup.isEmpty())
        {

            int nGroupIndex=mGroupList.indexOf(sPrevious);
            mGroupList.replace(nGroupIndex,sGroup);
            majGroupListWidget();

            QList<Sensor*>tempList;
            foreach(Sensor* sensor,mSensorList)
            {
                Sensor::Parameters param=sensor->getParameters();
                if(param.sGroup==sPrevious)
                {
                    param.sGroup=sGroup;
                    sensor->setParameters(param);

                }
                tempList.append(sensor);
            }

            mSensorList=tempList;



        }
    }
}

void fenPreferences::rmGroup()
{
    QString sGroup=ui->listGroupWidget->currentItem()->text();
    int nRes=QMessageBox::question(this,QString("Suppression du groupe %1").arg(sGroup),QString("Vous vous apprêtez à modifier le groupe %1.\n"
                                                                                               "L'ensemble des enregistrements liés à ce groupe seront affectés au groupe [Tous].\n"
                                                                                                "Etes-vous sûr de vouloir supprimer le groupe %1?").arg(sGroup));
    if(nRes==QMessageBox::Yes)
    {
        mGroupList.removeOne(sGroup);
        majGroupListWidget();
        QList<Sensor*>tempList;
        foreach(Sensor* sensor,mSensorList)
        {
            Sensor::Parameters param=sensor->getParameters();
            if(param.sGroup==sGroup)
            {
                param.sGroup=QString("Tous");
                sensor->setParameters(param);
            }

            tempList.append(sensor);
        }
        mSensorList=tempList;
        saveSensorList();
        majListItem();

    }
}

void fenPreferences::majGroupListWidget()
{
    ui->listGroupWidget->clear();
    ui->btn_rmGroup->setEnabled(false);
    ui->btn_EditGroup->setEnabled(false);


    if(mGroupList.isEmpty())
        mGroupList.append("Tous");

    ui->listGroupWidget->addItems(mGroupList);
    saveSensorList();
    foreach(Sensor* sensor,mSensorList)
    {
        sensor->setGroupList(mGroupList);
    }

    ui->listGroupWidget->setCurrentItem(ui->listGroupWidget->item(0));
    listGroupSelected(ui->listGroupWidget->currentItem());

}

void fenPreferences::appendGroup(QString sGroup)
{
    mGroupList.append(sGroup);
}

void fenPreferences::listGroupSelected(QListWidgetItem *item)
{
    ui->listSensorWidget->clear();
    mCurrentSensor->close();
    if(item->text()=="Tous")
    {
        ui->btn_rmGroup->setEnabled(false);
        ui->btn_EditGroup->setEnabled(false);
        foreach(Sensor* sensor,mSensorList)
        {
            ui->listSensorWidget->addItem(sensor->getParameters().sName);
        }

    }
    else
    {
        ui->btn_rmGroup->setEnabled(true);
        ui->btn_EditGroup->setEnabled(true);

        foreach(Sensor* sensor,mSensorList)
        {
            Sensor::Parameters param=sensor->getParameters();
            if(param.sGroup==item->text())
                ui->listSensorWidget->addItem(sensor->getParameters().sName);
        }
    }
}





void fenPreferences::saveSensorList()
{

    QString sXmlPath=QString("%1/sensors.xml").arg(mPathToConfMission);
    QFile xmlFile(sXmlPath);
    if(!xmlFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,QString("Impossible d'écrire le fichier sensors.xml"),tr("Le fichier n'est pas inscriptible"));
        return;
    }
    QXmlStreamWriter writer(&xmlFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(tr("http://cqt.cruisemanager.fr/sensorlist"),"sensorlist");
    writer.writeComment(tr("%1 capteurs sont configurés").arg(mSensorList.count()));

    foreach(Sensor* oneSensor,mSensorList)
    {
        oneSensor->write(writer);
    }

    QString uri="http://cqt.cruisemanager.fr/sensorlist";
    QStringListIterator it(mGroupList);
    while (it.hasNext())
    {
        writer.writeEmptyElement(uri,"group");

        writer.writeAttribute("Name",QString("%1").arg(it.next()));
    }

    writer.writeEndElement();


    writer.writeEndDocument();
    xmlFile.close();
}
