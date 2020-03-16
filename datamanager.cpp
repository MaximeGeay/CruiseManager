#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSysInfo>
#include "datamanager.h"
#include "ui_datamanager.h"

DataManager::DataManager(QWidget *parent, Sensor *sensor) :
    QWidget(parent),
    ui(new Ui::DataManager)
{
    ui->setupUi(this);

    QObject::connect(ui->btn_AddFiles,&QPushButton::clicked,this,&DataManager::clickOnAddFiles);
    QObject::connect(ui->btn_Synchro,&QPushButton::clicked,this,&DataManager::clickOnSynchro);

    setSensor(sensor);

    mProcessSynchro=new QProcess(this);
    QObject::connect(mProcessSynchro,&QProcess::readyReadStandardOutput,this,&DataManager::rsyncReponse);
   // QObject::connect(mProcessSynchro,&QProcess::readyRead,this,&DataManager::rsyncReponse);
    QObject::connect(mProcessSynchro,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),this,&DataManager::rsyncEnd);

    if(QSysInfo::productType().compare("windows",Qt::CaseInsensitive)==0)
        mSystemIsWindows=true;

    ui->pb_File->setVisible(false);
    ui->pb_Total->setVisible(false);


}

DataManager::~DataManager()
{
    delete ui;
}

void DataManager::setSensor(Sensor *sensor)
{
    mSensor=sensor;
    Sensor::Parameters param=mSensor->getParameters();
    ui->l_Nom->setText(param.sName);
    ui->l_Etat->setText(param.sDesc);

}

bool DataManager::getSynchroStatus()
{
    return mSynchroStatus;
}

void DataManager::execSynchro()
{

}

QString DataManager::getSensorName()
{
    return mSensor->getParameters().sName;
}

void DataManager::clickOnAddFiles()
{
    Sensor::Parameters param=mSensor->getParameters();
    QStringList sSources=QFileDialog::getOpenFileNames(this,QString("Sélectionner les fichiers à importer"),param.sSourcePath,"Tous fichiers(*.*)");
    QStringList sErrors;
    if(!sSources.isEmpty())
    {
        QDir dir;
        if(dir.exists(param.sDestPath))
        {
            QString sUneSource;
            QString sNomFichier;
            QStringListIterator it(sSources);
            while (it.hasNext()) {
                sUneSource=it.next();
                sNomFichier=sUneSource.section("/",-1);

                QFile file;
                if(!file.copy(sUneSource,QString("%1/%2").arg(param.sDestPath).arg(sNomFichier)))
                {
                    QString sMsg=QString("Impossible de copier le fichier %1 de %2 dans %3").arg(sNomFichier).arg(sUneSource).arg(param.sDestPath);
                    sErrors.append(sMsg);
                }


            }
        }
        else
        {
            QString sMsg=QString("Le répertoire Destination %1 n'existe pas").arg(param.sDestPath);
            sErrors.append(sMsg);
        }
    }

    if(sErrors.count()>0)
    {
        QStringListIterator itE(sErrors);
        while(itE.hasNext())
        {
            QString sErr=itE.next();
            qDebug()<<sErr;
            emit erreur(sErr);

        }
    }
    else
    {
        QMessageBox::information(this,QString("Fichiers copiés"),QString("Les fichiers ont été copiés avec succès"));
    }

}

void DataManager::clickOnSynchro()
{
    Sensor::Parameters param=mSensor->getParameters();
    int nRes=QMessageBox::No;
    if(param.type==Sensor::RecordType::Files)
    {
    nRes=QMessageBox::question(this,QString("Synchronisation de fichier"),QString("Etes-vous sûr de vouloir synchroniser les données %1\n"
                                                                                      "Source : %2\n"
                                                                                      "Destination : %3\n"
                                                                                      "Type : Fichiers\n"
                                                                                      "Extensions : %4").arg(param.sName).arg(param.sSourcePath).arg(param.sDestPath).arg(param.sExtensions));

    }
    else
    {
        nRes=QMessageBox::question(this,QString("Synchronisation de fichier"),QString("Etes-vous sûr de vouloir synchroniser les données %1\n"
                                                                                          "Source : %2\n"
                                                                                          "Destination : %3\n"
                                                                                          "Type : Dossier").arg(param.sName).arg(param.sSourcePath).arg(param.sDestPath));

    }

    if(nRes==QMessageBox::Yes)
    {
        mManualSync=true;
        synchro();
    }
}

void DataManager::synchroAutoStatusChanged(bool bStatus)
{

}

void DataManager::synchro()
{
    ui->te_rsync->clear();


    Sensor::Parameters param=mSensor->getParameters();
    QDir dir;
    if(!dir.exists(param.sSourcePath))
    {
        QString sMsg=QString("Le répertoire Source %1 n'existe pas").arg(param.sSourcePath);
        qDebug()<<sMsg;
        emit erreur(sMsg);
        return;
    }
    if(!dir.exists(param.sDestPath))
    {
        QString sMsg=QString("Le répertoire Destination %1 n'existe pas").arg(param.sDestPath);
        qDebug()<<sMsg;
        emit erreur(sMsg);
        return;
    }


    QString sCmd;
    QString sSource,sDest,sProgram;
    QString sOption;
    if(mSystemIsWindows)
    {
        sSource=QString("/cygdrive/%1%2").arg(param.sSourcePath.section(":",0,0)).arg(param.sSourcePath.section(":",1,-1));
        sDest=QString("/cygdrive/%1%2").arg(param.sDestPath.section(":",0,0)).arg(param.sDestPath.section(":",1,-1));
        sProgram=QString("E:/Developpement/CruiseManager/rsync/rsync.exe");
    }
    else {
        sSource=param.sSourcePath;
        sDest=param.sDestPath;
        sProgram=QString("rsync");
    }


    ui->pb_File->setRange(0,100);
   ui->pb_File->setValue(0);
    ui->pb_File->setVisible(true);

   ui->pb_Total->setValue(0);
    ui->pb_Total->setVisible(true);


    if(mSimulate)
        sOption=QString("-navz");
    else
    {
        sOption=QString("-avz");



    }


   if(param.type==Sensor::RecordType::Files)
   {

       int nbExt=param.sExtensions.count(",");
       QString sInclude;
       for(int n=0;n<=nbExt;n++)
       {
           sInclude=sInclude.append(QString("--include=\"%1\" ").arg(param.sExtensions.section(",",n,n)));

       }

       qDebug()<<"sInclude"<<sInclude;

       sCmd=QString("%1 %2 %3 --exclude=\"*\" --progress --delete-after %4/ %5/").arg(sProgram).arg(sOption).arg(sInclude).arg(sSource).arg(sDest);
   }
   else
   {
       sCmd=QString("%1 %2 --progress --delete-after %3/ %4/").arg(sProgram).arg(sOption).arg(sSource).arg(sDest);


   }

   if(mSystemIsWindows)
        mProcessSynchro->setWorkingDirectory("E:/Developpement/CruiseManager/rsync");

   qDebug()<<"sCmd"<<sCmd;
   ui->l_Etat->setText(sCmd);

    mCumulSize=0;
   mProcessSynchro->start(sCmd);


   //ui->l_Etat->setText(sCmd);

}

void DataManager::rsyncReponse()
{
    QString sReponse=mProcessSynchro->readAllStandardOutput();


    ui->te_rsync->append(sReponse);
    if(mSimulate&&sReponse.contains("total size"))
    {
        mSizeToCopy=sReponse.section("total size is",1,1).section("speedup",0,0).remove(" ").remove(",").toInt()/1000;

        ui->pb_Total->setRange(0,mSizeToCopy);


    }


    if(sReponse.contains("%"))
    {
        //qDebug()<<"value"<<sReponse.section("% ",0,0).section(" ",-1).toInt();
        int nPourcent=sReponse.section("% ",0,0).section(" ",-1).toInt();
        ui->pb_File->setValue(nPourcent);

        int nSize=sReponse.section(",",0,1).section("\r",1,1).remove(" ").remove(",").toInt();
        ui->pb_Total->setValue(mCumulSize+nSize);
        emit copyPourcent(ui->pb_Total->text().remove("%").toInt());

        if(nPourcent==100)
            mCumulSize+=nSize;

       // int nTemp=mCumulSize+nSize;


    }


}

void DataManager::rsyncEnd(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug()<<"exitCode"<<exitCode<<exitStatus;
    int nRes;
    if(exitCode==0&&mSimulate)
    {
       if(mManualSync)
       {
           double dSize=mSizeToCopy;
                   dSize=dSize/1000;
           QString sMoToCopy=QString::number(dSize,'f',1);
           nRes=QMessageBox::question(this,"Confirmation de la synchronisation",QString("%1 Mo à copier\n"
                                                                                "Souhaitez-vous procéder à la synchronisation?").arg(sMoToCopy));
       }
       else
       {
           nRes=QMessageBox::Yes;
       }

        if(nRes==QMessageBox::Yes)
        {
             mSimulate=false;
             ui->pb_File->setVisible(true);
             ui->pb_Total->setVisible(true);
            synchro();
        }


    }
    else
    {
        mSimulate=true;
        ui->pb_File->setVisible(false);
        ui->pb_Total->setVisible(false);
    }

    mManualSync=false;
  //
}
