#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSysInfo>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QSettings>
#include <QDateTime>

#include "datamanager.h"
#include "ui_datamanager.h"

DataManager::DataManager(QWidget *parent, Sensor *sensor) :
    QWidget(parent),
    ui(new Ui::DataManager)
{
    ui->setupUi(this);

    mAutoSync=new SynchroAuto;


    treeModel=new QFileSystemModel;
    ui->treeContenu->setModel(treeModel);
    ui->treeContenu->header()->setSectionResizeMode(0,QHeaderView::Stretch);



    setSensor(sensor);

    mProcessSynchro=new QProcess(this);
    //QObject::connect(mProcessSynchro,&QProcess::readyReadStandardOutput,this,&DataManager::rsyncReponse);
    QObject::connect(ui->btn_AddFiles,&QPushButton::clicked,this,&DataManager::clickOnAddFiles);
    QObject::connect(ui->btn_Synchro,&QPushButton::clicked,this,&DataManager::clickOnSynchro);

    QObject::connect(mProcessSynchro,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),this,&DataManager::rsyncEnd);
    QObject::connect(ui->treeContenu,&QTreeView::doubleClicked,this,&DataManager::treeView_doubleClicked);
    QObject::connect(ui->cb_SynchroAuto,&QCheckBox::stateChanged,this,&DataManager::clickOnAutoSync);
    QObject::connect(mAutoSync,&SynchroAuto::runSynchro,this,&DataManager::synchro);

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
    ui->cb_SynchroAuto->setChecked(param.bSyncAuto);

    mAutoSync->setSensor(mSensor);
    if(param.bSyncAuto)
        mAutoSync->show();
    else
        mAutoSync->hide();

    if(!param.sDestPath.isEmpty())
        ui->treeContenu->setRootIndex(treeModel->setRootPath(getCompletePath(mSensor)));
}

bool DataManager::getSynchroStatus()
{
    return mSynchroIsRunning;
}

void DataManager::execSynchro()
{

}

QString DataManager::getSensorName()
{
    return mSensor->getParameters().sName;
}

SynchroAuto *DataManager::getAutoSyncWidget()
{
    return mAutoSync;
}

void DataManager::killSyncAuto()
{
    delete mAutoSync;
}

void DataManager::clickOnAddFiles()
{
    Sensor::Parameters param=mSensor->getParameters();
    QStringList sSources=QFileDialog::getOpenFileNames(this,QString("Sélectionner les fichiers à importer"),param.sSourcePath,"Tous fichiers(*.*)");
    QStringList sErrors;
    if(!sSources.isEmpty())
    {
        QDir dir;
        if(dir.exists(getCompletePath(mSensor)))
        {
            QString sUneSource;
            QString sNomFichier;
            QStringListIterator it(sSources);
            while (it.hasNext()) {
                sUneSource=it.next();
                sNomFichier=sUneSource.section("/",-1);

                QFile file;
                if(!file.copy(sUneSource,QString("%1/%2").arg(getCompletePath(mSensor)).arg(sNomFichier)))
                {
                    QString sMsg=QString("Impossible de copier le fichier %1 de %2 dans %3").arg(sNomFichier).arg(sUneSource).arg(getCompletePath(mSensor));
                    sErrors.append(sMsg);
                }


            }
        }
        else
        {
            QString sMsg=QString("Le répertoire Destination %1 n'existe pas").arg(getCompletePath(mSensor));
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
                                                                                      "Type : Dossier").arg(param.sName).arg(param.sSourcePath).arg(getCompletePath(mSensor)));

    }

    if(nRes==QMessageBox::Yes)
    {
        mManualSync=true;
        synchro();
    }
}

void DataManager::clickOnAutoSync()
{
    Sensor::Parameters param=mSensor->getParameters();
    bool bPrevious=param.bSyncAuto;
    param.bSyncAuto=ui->cb_SynchroAuto->isChecked();

    if(param.bSyncAuto)
    {
        mAutoSync->show();
    }
    else
    {
        mAutoSync->hide();
    }
    if(bPrevious!=param.bSyncAuto)
        mSensor->setParameters(param);
}


void DataManager::synchro()
{
    ui->te_rsync->clear();

    mFilesList.clear();
    Sensor::Parameters param=mSensor->getParameters();
    QDir dir;
    if(!dir.exists(param.sSourcePath))
    {
        QString sMsg=QString("Le répertoire Source %1 n'existe pas").arg(param.sSourcePath);
        qDebug()<<sMsg;
        emit erreur(QString("%1 -> %2: %3").arg(QDateTime::currentDateTimeUtc().toString("HH:mm:ss")).arg(param.sName).arg(sMsg));
        mAutoSync->setMessage(sMsg);
        mAutoSync->synchroEnd();

        return;
    }
    if(!dir.exists(getCompletePath(mSensor)))
    {
        QString sMsg=QString("Le répertoire Destination %1 n'existe pas").arg(getCompletePath(mSensor));
        qDebug()<<sMsg;
        emit erreur(QString("%1 -> %2: %3").arg(QDateTime::currentDateTimeUtc().toString("HH:mm:ss")).arg(param.sName).arg(sMsg));

        mAutoSync->setMessage(sMsg);
        mAutoSync->synchroEnd();
        return;
    }


    QString sCmd;
    QString sSource,sDest,sProgram;
    QString sOption;
    mSourcePath=param.sSourcePath;
    if(mSystemIsWindows)
    {
        sSource=QString("/cygdrive/%1%2").arg(param.sSourcePath.section(":",0,0)).arg(param.sSourcePath.section(":",1,-1));
        sDest=QString("/cygdrive/%1%2").arg(getCompletePath(mSensor).section(":",0,0)).arg(getCompletePath(mSensor).section(":",1,-1));
        sProgram=QString("E:/Developpement/CruiseManager/rsync/rsync.exe");
    }
    else {
        sSource=param.sSourcePath;
        sDest=getCompletePath(mSensor);
        sProgram=QString("rsync");
    }


    ui->pb_File->setRange(0,100);
    ui->pb_File->setValue(0);
    ui->pb_File->setVisible(true);

    ui->pb_Total->setValue(0);
    ui->pb_Total->setVisible(true);


    if(mSimulate)
        sOption=QString("-nrlptoDvz");
    else
    {
        sOption=QString("-rlptoDvz");



    }


    QString sSync="";
    if(param.sync==Sensor::SyncType::Mirror)
        sSync="--delete-after ";

    if(param.type==Sensor::RecordType::Files)
    {

        int nbExt=param.sExtensions.count(",");
        QString sInclude;
        for(int n=0;n<=nbExt;n++)
        {
            sInclude=sInclude.append(QString(" --include=\"%1\"").arg(param.sExtensions.section(",",n,n)));

        }


        sCmd=QString("%1 %2 %3 --exclude=\"*\" --progress %4%5/ %6/").arg(sProgram).arg(sOption).arg(sInclude).arg(sSync).arg(sSource).arg(sDest);
    }
    else
    {
        sCmd=QString("%1 %2 --progress %3%4/ %5/").arg(sProgram).arg(sOption).arg(sSync).arg(sSource).arg(sDest);


    }

    if(mSystemIsWindows)
        mProcessSynchro->setWorkingDirectory("E:/Developpement/CruiseManager/rsync");

    //ui->l_Etat->setText(sCmd);

    mCumulSize=0;
    mProcessSynchro->start(sCmd);
    mSynchroIsRunning=true;

    //ui->l_Etat->setText(sCmd);
    ui->te_rsync->append(">> "+sCmd);
}

void DataManager::rsyncReponse()
{
    QString sReponse=mProcessSynchro->readAllStandardOutput();
    // qDebug()<<"sReponse"<<sReponse;

    ui->te_rsync->append(sReponse);


    if(sReponse.contains("%"))
    {
        //qDebug()<<"value"<<sReponse.section("% ",0,0).section(" ",-1).toInt();
        int nPourcent=sReponse.section("% ",0,0).section(" ",-1).toInt();
        ui->pb_File->setValue(nPourcent);

        int nSize=sReponse.section(",",0,1).section("\r",1,1).remove(" ").remove(",").toInt();
        ui->pb_Total->setValue(mCumulSize+nSize);
        //emit copyPourcent(ui->pb_Total->text().remove("%").toInt());
        mAutoSync->percent(ui->pb_Total->text().remove("%").toInt());

        if(nPourcent==100)
            mCumulSize+=nSize;

        // int nTemp=mCumulSize+nSize;


    }


}

void DataManager::rsyncEnd(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString sReponse=mProcessSynchro->readAllStandardOutput();
    QStringList listRep;
    int nRC=sReponse.count("\n");
    for(int n=0;n<nRC;n++)
    {
        listRep.append(sReponse.section("\n",n,n));
    }

    QStringList delList=listRep.filter("deleting ");

    ui->te_rsync->append(sReponse);
    //qDebug()<<"listRep"<<listRep;
    if(mSimulate&&sReponse.contains("total size"))
    {
        mSizeToCopy=0;
        int nbRC=sReponse.count("\n");
        //qDebug()<<"sReponse"<<sReponse;
        // qDebug()<<"n"<<sReponse.count("\n");

        if(nbRC>3)
        {

            for(int i=1;i<nbRC-3;i++)
            {
                //qDebug()<<sReponse.section("\n",i,i);
                mFilesList.append(sReponse.section("\n",i,i));
            }

            QStringListIterator it(mFilesList);
            while(it.hasNext())
            {
                QString sFile=it.next();
                QFile file(QString("%1/%2").arg(mSourcePath).arg(sFile));
                //  qDebug()<<QString("%1/%2").arg(mSourcePath).arg(sFile);
                // qDebug()<<"file.size()"<<file.size();
                mSizeToCopy=mSizeToCopy+file.size();

            }

            // mSizeToCopy=sReponse.section("total size is",1,1).section("speedup",0,0).remove(" ").remove(",").toInt()/1000;
            mSizeToCopy=mSizeToCopy/1000;
            ui->pb_Total->setRange(0,mSizeToCopy);
            mSyncOK=false;
        }
        else
        {
            mSyncOK=true;
        }


    }


    qDebug()<<"exitCode"<<exitCode<<exitStatus;
    int nRes;
    if(exitCode==0&&mSimulate)
    {
        if(mManualSync)
        {
            if(mSyncOK)
            {
                if(mManualSync)
                    QMessageBox::information(this,"Fichiers synchronisés","Fichiers déjà synchronisés");
                mManualSync=false;
                ui->pb_File->setVisible(false);
                ui->pb_Total->setVisible(false);
                return;
            }
            double dSize=mSizeToCopy;

            dSize=dSize/1000;
            QString sMoToCopy=QString::number(dSize,'f',3);

            if(delList.size()>0)
            {
                QString sFichiers;
                QStringListIterator it(delList);
                while(it.hasNext())
                    sFichiers=it.next()+" "+sFichiers;
                sFichiers.remove("deleting ");
                if(delList.size()==1)

                    nRes=QMessageBox::question(this,"Confirmation de la synchronisation",QString("%1 Mo à copier\n%2 fichier sera supprimé : %3\n"
                                                                                                 "Souhaitez-vous procéder à la synchronisation?").arg(sMoToCopy).arg(delList.size()).arg(sFichiers));
                else
                    nRes=QMessageBox::question(this,"Confirmation de la synchronisation",QString("%1 Mo à copier\n%2 fichiers seront supprimés : %3\n"
                                                                                                 "Souhaitez-vous procéder à la synchronisation?").arg(sMoToCopy).arg(delList.size()).arg(sFichiers));
            }
            else
            {
                if(mManualSync)
                    nRes=QMessageBox::question(this,"Confirmation de la synchronisation",QString("%1 Mo à copier\n"
                                                                                                 "Souhaitez-vous procéder à la synchronisation?").arg(sMoToCopy));
                else
                    nRes=QMessageBox::Yes;
            }
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
            QObject::connect(mProcessSynchro,&QProcess::readyReadStandardOutput,this,&DataManager::rsyncReponse);
            synchro();
        }
        else
        {
            ui->pb_File->setVisible(false);
            ui->pb_Total->setVisible(false);
        }


    }
    else
    {
        mSimulate=true;
        ui->pb_File->setVisible(false);
        ui->pb_Total->setVisible(false);
        mManualSync=false;
        mSynchroIsRunning=false;

        if(exitCode==0)
        {
            mAutoSync->setMessage(QString("Dernière synchronisation terminée à %1 UTC").arg(QDateTime::currentDateTimeUtc().toString("HH:mm:ss")));
        }
        else
        {
            emit erreur(QString("%1 -> %2: %3").arg(QDateTime::currentDateTimeUtc().toString("HH:mm:ss")).arg(getSensorName()).arg(exitStatus));
            mAutoSync->setMessage(QString("Erreur : %1").arg(exitStatus));
        }

        mAutoSync->synchroEnd();

    }



    //
}

void DataManager::treeView_doubleClicked(const QModelIndex &index)
{
    QString sContent=treeModel->filePath(index);
    if(sContent.section("/",-1).contains("."))
    {
        if(!m_DClic)
        {   m_DClic=true;

            QDesktopServices::openUrl(QUrl(sContent));
        }
        else
            m_DClic=false;
    }
}

QString DataManager::getCompletePath(Sensor *sensor)
{
    Sensor::Parameters param(sensor->getParameters());
    QString sCompletePath;
    if(param.type==Sensor::Cruise)
    {
        sCompletePath=param.sDestPath;
    }
    else
    {
        QSettings settings("CruiseManager","Settings");
        QString sMissionPath=QString("%1/%2").arg(settings.value("Mission/Path").toString()).arg(settings.value("Mission/Nom").toString());
        sCompletePath=sMissionPath+param.sDestPath;
    }
    return sCompletePath;
}
