#include<QFile>
#include<QDebug>
#include <QDir>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QFileSystemModel>
#include <QSettings>
#include <QMessageBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QCloseEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"fenpreferences.h"
#include "datamanager.h"

#define VERSION "CruiseManager 1.5"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(VERSION);

    mSettings=new QSettings("CruiseManager","Settings");

    treeModel = new QFileSystemModel;
    fenPref=new fenPreferences;
    fenCruise=new fenMission;

    ui->treeView->setModel(treeModel);

    mLayoutDataManager=new QHBoxLayout();
    mLayoutDataManager->setAlignment(Qt::AlignTop);
    mLayoutDataManager->setDirection(QBoxLayout::TopToBottom);
    ui->gb_Details->setLayout(mLayoutDataManager);


    mLayoutSyncAuto=new QVBoxLayout();
    mLayoutSyncAuto->setAlignment(Qt::AlignTop);
    mLayoutSyncAuto->setDirection(QVBoxLayout::TopToBottom);
    ui->gb_SyncAuto->setLayout(mLayoutSyncAuto);




    mLastDataManager=new DataManager(nullptr,new Record);


    init();

    QObject::connect(ui->actionNouvelle_mission,&QAction::triggered,fenCruise,&fenMission::newCruise);
    QObject::connect(ui->actionEditer_Mission,&QAction::triggered,fenCruise,&fenMission::editCruise);
    QObject::connect(ui->btn_OpenRep,&QPushButton::clicked,this,&MainWindow::ouvreRepCourant);
    QObject::connect(ui->actionOuvrirMissions,&QAction::triggered,this,&MainWindow::ouvrirMissions);
    QObject::connect(ui->actionOuvrirConfMission,&QAction::triggered,this,&MainWindow::ouvrirConfMission);
    QObject::connect(ui->treeView,&QTreeView::doubleClicked,this,&MainWindow::treeView_doubleClicked);
    QObject::connect(ui->actionPreferences,&QAction::triggered,fenPref,&fenPreferences::init);
    QObject::connect(ui->actionQuitter,&QAction::triggered,this,&MainWindow::close);
    QObject::connect(fenPref,&fenPreferences::editingFinished,this,&MainWindow::init);
    QObject::connect(fenCruise,&fenMission::editingFinished,this,&MainWindow::init);
    QObject::connect(fenCruise,&fenMission::editingFinished,fenPref,&fenPreferences::endMissionEdit);
    QObject::connect(ui->actionAfficheConfig,&QAction::triggered,this,&MainWindow::afficheConfig);
    QObject::connect(fenCruise,&fenMission::newCruiseSet,fenPref,&fenPreferences::setCurrentCruise);
    QObject::connect(fenCruise,&fenMission::dataListRequest,this,&MainWindow::dataListRequested);
    QObject::connect(ui->listGroupWidget,&QListWidget::itemClicked,this,&MainWindow::groupSelected);
    QObject::connect(ui->listRecordWidget,&QListWidget::itemClicked,this,&MainWindow::recordSelected);
    QObject::connect(ui->btn_RAZLog,&QPushButton::clicked,this,&MainWindow::clickOnRaz);

}

MainWindow::~MainWindow()
{
    delete ui;

}



void MainWindow::init()
{
    mPathToMissions=mSettings->value("Mission/Path","").toString();
    mPathToConfMission=mSettings->value("PathToConfMission","").toString();
    mCurrentCruise=fenCruise->getCurrentCruise();

    if(mPathToMissions.isEmpty()||mPathToConfMission.isEmpty())
    {
        QMessageBox::warning(this,tr("Configuration requise"),tr("Veuillez vérifier la sélection des réperoires Missions et ConfMission"));
        fenPref->init();

    }



    ui->l_mission->setText(QString("Mission %1").arg(mCurrentCruise.sNom));
    ui->treeView->setColumnHidden(1,true);
    ui->treeView->setColumnHidden(2,true);
    ui->treeView->setColumnHidden(3,true);

    ui->listGroupWidget->clear();
    ui->listGroupWidget->addItems(mCurrentCruise.listDataGroups);

    mCurrentRecords.clear();
    mCurrentRecords=fenPref->getRecordsListFromGroups(mCurrentCruise.listDataGroups);

    QListIterator<DataManager*> itM(mDataManagers);
    while(itM.hasNext())
    {
        itM.next()->killSyncAuto();
    }
    mDataManagers.clear();

    QListIterator<Record*> it(mCurrentRecords);
    while(it.hasNext())
    {
        DataManager* data=new DataManager(nullptr,it.next());
        mDataManagers.append(data);
        mLayoutDataManager->addWidget(data);

        mLayoutSyncAuto->addWidget(data->getAutoSyncWidget());

        QObject::connect(data,&DataManager::erreur,ui->te_Logs,&QTextEdit::append);
        data->hide();
    }




    m_DClic=0;

    if(!mCurrentCruise.sNom.isEmpty())
        ui->treeView->setRootIndex(treeModel->setRootPath(QString("%1/%2").arg(mPathToMissions).arg(mCurrentCruise.sNom)));


}

void MainWindow::ouvreRepCourant()
{
    QDesktopServices::openUrl(QUrl(QString("%1/%2").arg(mPathToMissions).arg(mCurrentCruise.sNom)));
}

void MainWindow::treeView_doubleClicked(const QModelIndex &index)
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

void MainWindow::afficheConfig()
{

    QStringList listKeys=mSettings->allKeys();


    QTextEdit *te=new QTextEdit();
    te->append(QString("Fichier : %1").arg(mSettings->fileName()));
    te->append("");


    QStringListIterator it(listKeys);
    QString oneKey;
    while(it.hasNext())
    {
        oneKey=it.next();
        te->append(QString("[%1] : %2").arg(oneKey).arg(mSettings->value(oneKey).toString()));

    }
    te->setReadOnly(true);


    //te->show();
    QWidget *window = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(te);
    window->setLayout(layout);
    window->setGeometry(200,200,400,600);
    window->show();




}

void MainWindow::dataListRequested()
{
    fenCruise->setDataGroups(fenPref->getDataGroups());
    fenCruise->setRecordList(fenPref->getRecordsList());
}

void MainWindow::ouvrirMissions()
{
    QDesktopServices::openUrl(QUrl(QString("%1").arg(mPathToMissions)));
}

void MainWindow::ouvrirConfMission()
{
    QDesktopServices::openUrl(QUrl(QString("%1").arg(mPathToConfMission)));
}

void MainWindow::groupSelected(QListWidgetItem *item)
{
    ui->listRecordWidget->clear();
    mLastDataManager->hide();
    QListIterator<Record*>it(mCurrentRecords);
    while (it.hasNext()) {
        Record* record=it.next();
        Record::Parameters param=record->getParameters();

        if(item->text()==param.sGroup)
        {
            ui->listRecordWidget->addItem(param.sName);
        }

    }

}

void MainWindow::recordSelected(QListWidgetItem *item)
{
    mLastDataManager->hide();
    QListIterator<DataManager*>it(mDataManagers);
    while (it.hasNext()) {
        DataManager* data=it.next();
        if(data->getRecordName().compare(item->text())==0)
        {
            mLastDataManager=data;
            mLastDataManager->show();


        }

    }


}

void MainWindow::closeEvent(QCloseEvent *event)
{

    QListIterator<DataManager*>it(mDataManagers);
    int nRep=QMessageBox::No;
    QStringList listSynchro;
    while(it.hasNext())
    {
        DataManager* data=it.next();
        if(data->getSynchroStatus())
        {
            listSynchro.append(data->getRecordName());
        }
    }

    if(listSynchro.size()>0)
    {
        QStringListIterator it(listSynchro);
        QString sSynchros;

        while(it.hasNext())
            sSynchros=it.next()+" "+sSynchros;

        nRep=QMessageBox::question(this,QString("Quitter"),QString("La synchronisation de %1 est en cours\n"
                                                                       "Les fichiers en cours de copie risquent d'être corrompus\n"
                                                                        "Souhaitez-vous quitter malgré tout?").arg(sSynchros),QMessageBox::Yes|QMessageBox::No);

    }
    else
    {
        nRep=QMessageBox::question(this,QString("Quitter"),QString("Êtes-vous sûr de vouloir quitter?"),QMessageBox::Yes|QMessageBox::No);
    }

    if(nRep==QMessageBox::Yes)
        event->accept();
    else
        event->ignore();

}

void MainWindow::clickOnRaz()
{
 ui->te_Logs->clear();
}


