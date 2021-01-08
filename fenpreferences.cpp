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

#include "recordxmlhandler.h"

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
    QObject::connect(ui->btn_AddRecord,&QToolButton::clicked,this,&fenPreferences::addNewRecord);
    QObject::connect(ui->btn_rmRecord,&QToolButton::clicked,this,&fenPreferences::removeRecord);
    QObject::connect(ui->listRecordWidget,&QListWidget::itemClicked,this,&fenPreferences::listRecordSelected);
    QObject::connect(ui->listGroupWidget,&QListWidget::itemClicked,this,&fenPreferences::listGroupSelected);
    QObject::connect(ui->btn_AddGroup,&QToolButton::clicked,this,&fenPreferences::addGroup);
    QObject::connect(ui->btn_EditGroup,&QToolButton::clicked,this,&fenPreferences::editGroup);
    QObject::connect(ui->btn_rmGroup,&QToolButton::clicked,this,&fenPreferences::rmGroup);
    QObject::connect(ui->btn_RecordFichier,&QPushButton::clicked,this,&fenPreferences::clickOnSaveRecordFile);
    QObject::connect(ui->btn_OuvreFichier,&QPushButton::clicked,this,&fenPreferences::clickOnOpenRecordFile);


    mCurrentRecord=new Record;



    mRecordLayout=new QVBoxLayout(ui->gb_Records);
    mRecordLayout->setAlignment(Qt::AlignTop);
    mRecordLayout->setDirection(QBoxLayout::TopToBottom);




    mPathToMissions=mSettings->value("Mission/Path",QDir::homePath()).toString();
    mPathToConfMission=mSettings->value("PathToConfMission",QDir::homePath()).toString();
    mRecordFileName=mSettings->value("RecordFileName","records.xml").toString();
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
    mRecordFileName=mSettings->value("RecordFileName","records.xml").toString();

    ui->le_RepCruises->setText(mPathToMissions);
    ui->le_ConfMission->setText(mPathToConfMission);
    ui->l_FicRecord->setText(mRecordFileName);

    majListItem();
    majGroupListWidget();


    show();
}

QStringList fenPreferences::getDataGroups()
{
    return mGroupList;
}

QList<Record *> fenPreferences::getRecordsList()
{
    return mRecordList;
}

void fenPreferences::setCurrentCruise(QString sCruise)
{
    mCurrentCruise=sCruise;
    mNewCruise=true;

}

void fenPreferences::endMissionEdit()
{
    mNewCruise=false;
}

QList<Record *> fenPreferences::getRecordsListFromGroups(QStringList groupList)
{
    QList<Record*> recordList;

    QListIterator<Record*>itS(mRecordList);
    while (itS.hasNext()) {
        Record* record=itS.next();
        if(groupList.contains(record->getParameters().sGroup))
            recordList.append(record);

    }
    return recordList;

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


    saveRecordList();
    emit editingFinished();
    this->close();


}

void fenPreferences::annuler()
{
    this->close();
}

void fenPreferences::addNewRecord()
{
    Record *record=new Record;
    record->setDeverrouiller(true);
    mbNewRecord=true;
    addRecord(record);



}

void fenPreferences::addRecord(Record *record)
{

    mCurrentRecord->hide();
    record->setGroupList(mGroupList);
    mRecordList.append(record);

    mCurrentRecord=record;
    mRecordLayout->addWidget(mCurrentRecord);

    if(mCurrentRecord->getParameters().sName.isEmpty())
        mCurrentRecord->show();
    else
        mCurrentRecord->hide();

    QObject::connect(record,&Record::editingStatusChanged,this,&fenPreferences::editingStatusControl);
}



void fenPreferences::removeRecord()
{
    QString sName=mCurrentRecord->getParameters().sName;
    int nRes=QMessageBox::question(this,QString("Suppression de l'enregistrement %1").arg(sName),QString("Etes-vous sûrs de vouloir supprimer l'enregistrement %1").arg(sName));
    if(nRes==QMessageBox::Yes)
    {
        mRecordList.removeOne(mCurrentRecord);
        saveRecordList();
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
    foreach(Record* oneRecord,mRecordList)
    {
        if(oneRecord->getEditStatus()==true)
            n++;
    }
    if(n>0)
    {
        ui->btn_Valider->setEnabled(false);
    }
    else
        ui->btn_Valider->setEnabled(true);



    if(!mCurrentRecord->getEditStatus())
    {
        if(mbNewRecord)
        {
            saveRecordList();
            mbNewRecord=false;
        }

        majListItem();
    }
    }
}

bool fenPreferences::listRecordSelected(QListWidgetItem *item)
{
    mCurrentRecord->hide();

    QString sName=item->text();

    foreach(Record* record,mRecordList)
    {
        if(record->getParameters().sName==sName)
        {
            mCurrentRecord=record;
            mCurrentRecord->show();
            ui->btn_rmRecord->setVisible(true);
            QObject::connect(mCurrentRecord,&Record::editingFinished,this,&fenPreferences::recordEditionFinished);
            return true;
        }
    }
    return false;


}

void fenPreferences::majListItem()
{

    mRecordList.clear();
    mGroupList.clear();
    ui->listRecordWidget->clear();

    ui->btn_rmRecord->setVisible(false);

    QString sFileOpen=QString("%1/%2").arg(mPathToConfMission,mRecordFileName);
    QFile file(sFileOpen);

    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,tr("Open Xml File"),tr("The xml file is not readable"));
        return;
    }
    QXmlSimpleReader reader;
    RecordXmlHandler handler;
    QObject::connect(&handler,&RecordXmlHandler::loadRecord,this,&fenPreferences::addRecord);
    QObject::connect(&handler,&RecordXmlHandler::sendMessage,this,&fenPreferences::showMessage);
    QObject::connect(&handler,&RecordXmlHandler::loadGroup,this,&fenPreferences::appendGroup);

    reader.setContentHandler(&handler);
    QXmlInputSource source(&file);
    reader.parse(source);
    file.close();

    majGroupListWidget();

}

void fenPreferences::recordEditionFinished()
{
    saveRecordList();
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

            QList<Record*>tempList;
            foreach(Record* record,mRecordList)
            {
                Record::Parameters param=record->getParameters();
                if(param.sGroup==sPrevious)
                {
                    param.sGroup=sGroup;
                    record->setParameters(param);

                }
                tempList.append(record);
            }

            mRecordList=tempList;



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
        QList<Record*>tempList;
        foreach(Record* record,mRecordList)
        {
            Record::Parameters param=record->getParameters();
            if(param.sGroup==sGroup)
            {
                param.sGroup=QString("Tous");
                record->setParameters(param);
            }

            tempList.append(record);
        }
        mRecordList=tempList;
        saveRecordList();
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
    saveRecordList();
    foreach(Record* record,mRecordList)
    {
        record->setGroupList(mGroupList);
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
    ui->listRecordWidget->clear();
    mCurrentRecord->close();
    if(item->text()=="Tous")
    {
        ui->btn_rmGroup->setEnabled(false);
        ui->btn_EditGroup->setEnabled(false);
        foreach(Record* record,mRecordList)
        {
            ui->listRecordWidget->addItem(record->getParameters().sName);
        }

    }
    else
    {
        ui->btn_rmGroup->setEnabled(true);
        ui->btn_EditGroup->setEnabled(true);

        foreach(Record* record,mRecordList)
        {
            Record::Parameters param=record->getParameters();
            if(param.sGroup==item->text())
                ui->listRecordWidget->addItem(record->getParameters().sName);
        }
    }
}

void fenPreferences::clickOnSaveRecordFile()
{
    bool ok;
         QString text = QInputDialog::getText(this, tr("Sauvegarder le fichier courant"),
                                              tr("Entrer un nouveau nom"), QLineEdit::Normal,
                                              mRecordFileName, &ok);
         if (ok && !text.isEmpty())
         {
             if(!text.endsWith(".xml"))
                 text=QString("%1.xml").arg(text);

             if(QFile::exists(QString("%1/%2").arg(mPathToConfMission,text)))
             {
                 int nRes=QMessageBox::question(this,"Le fichier existe déjà",QString("Le fichier %1 existe déjà.\n"
                                                                             "Souhaitez-vous l'écraser?").arg(text),QMessageBox::Yes|QMessageBox::No);
                 if(nRes==QMessageBox::No)
                 {
                     clickOnSaveRecordFile();
                     return;
                 }
                 else
                 {
                     QFile::remove(QString("%1/%2").arg(mPathToConfMission,text));
                 }

             }
             bool bRes=QFile::copy(QString("%1/%2").arg(mPathToConfMission,mRecordFileName),QString("%1/%2").arg(mPathToConfMission,text));
             if(bRes)
             {
                 mRecordFileName=text;
                 mSettings->setValue("RecordFileName",mRecordFileName);
                 init();
             }

         }
}

void fenPreferences::clickOnOpenRecordFile()
{
    QString sFilePath=QFileDialog::getOpenFileName(this,"Sélectionner un fichier de configuration .xml",mPathToConfMission,tr("Fichier de configuration XML (*.xml)"));

    if(!sFilePath.isEmpty())
    {
        QString sFileName=sFilePath.section("/",-1);
        int nRes=QMessageBox::question(this,"Changement de fichier de configuration",QString("Etes-vous sûr de vouloir utiliser le fichier %1 à la place du fichier %2")
                                       .arg(sFileName,mRecordFileName),QMessageBox::Yes|QMessageBox::No);
        if(nRes==QMessageBox::Yes)
        {
            if (!QFile::exists(QString("%1/%2").arg(mPathToConfMission,sFileName)))
            {
                if(!QFile::copy(sFilePath,QString("%1/%2").arg(mPathToConfMission,sFileName)))
                { QMessageBox::critical(this,"Echec",QString("Echec de l'import du fichier %1 dans le répertoire ConfMission").arg(sFileName));
                    return;
                }
            }

            mRecordFileName=sFileName;
            mSettings->setValue("RecordFileName",mRecordFileName);
            init();
        }
    }
}





void fenPreferences::saveRecordList()
{

    QString sXmlPath=QString("%1/%2").arg(mPathToConfMission,mRecordFileName);
    QFile xmlFile(sXmlPath);
    if(!xmlFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,QString("Impossible d'écrire le fichier %1").arg(mRecordFileName),tr("Le fichier n'est pas inscriptible"));
        return;
    }
    QXmlStreamWriter writer(&xmlFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(tr("http://cqt.cruisemanager.fr/recordlist"),"recordlist");
    writer.writeComment(tr("%1 enregistrements sont configurés").arg(mRecordList.count()));

    foreach(Record* oneRecord,mRecordList)
    {
        oneRecord->write(writer);
    }

    QString uri="http://cqt.cruisemanager.fr/recordlist";
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
