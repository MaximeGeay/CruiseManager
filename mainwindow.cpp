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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"fenpreferences.h"
#include "datamanager.h"

#define VERSION "CruiseManager 1.0"
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
    mLastDataManager=new DataManager(nullptr,new Sensor);

/*
    mLayoutEquipement=new QVBoxLayout(ui->gb_Equipement);

    mData2040=new DataManager(ui->gb_Equipement,"EM2040",DataManager::Type::DataEtAnnexe);
    mDataKlein=new DataManager(ui->gb_Equipement,"Klein3000",DataManager::Type::DataEtAnnexe);
    mDataSparker=new DataManager(ui->gb_Equipement,"Sparker",DataManager::Type::DataSansAnnexe);
    mDataPeskavel=new DataManager(ui->gb_Equipement,"Peskavel",DataManager::Type::DataSansAnnexe);
    mDataMaregraphe=new DataManager(ui->gb_Equipement,"Maregraphe",DataManager::Type::DataSansAnnexe);
    mDataEK60=new DataManager(ui->gb_Equipement,"EK60",DataManager::Type::DataEtAnnexe);
    mDataME70=new DataManager(ui->gb_Equipement,"ME70",DataManager::Type::DataEtAnnexe);
    mDataRoxAnn = new DataManager(ui->gb_Equipement,"RoxAnn",DataManager::Type::FichiersSimples);
    mDataScout= new DataManager(ui->gb_Equipement,"Scout",DataManager::Type::FichiersSimples);
    mDataHydrins= new DataManager(ui->gb_Equipement,"Hydrins",DataManager::Type::FichiersSimples);
    mDataOsea= new DataManager(ui->gb_Equipement,"mDataOsea",DataManager::Type::FichiersSimples);


    mLayoutEquipement->addWidget(mData2040);
    mLayoutEquipement->addWidget(mDataKlein);
    mLayoutEquipement->addWidget(mDataSparker);
    mLayoutEquipement->addWidget(mDataPeskavel);
    mLayoutEquipement->addWidget(mDataMaregraphe);
    mLayoutEquipement->addWidget(mDataEK60);
    mLayoutEquipement->addWidget(mDataME70);
    mLayoutEquipement->addWidget(mDataRoxAnn);
    mLayoutEquipement->addWidget(mDataScout);
    mLayoutEquipement->addWidget(mDataHydrins);
    mLayoutEquipement->addWidget(mDataOsea);
    mLayoutEquipement->setAlignment(Qt::AlignTop);
*/


    init();

    QObject::connect(ui->actionNouvelle_mission,&QAction::triggered,fenCruise,&fenMission::newCruise);
    QObject::connect(ui->actionEditer_Mission,&QAction::triggered,fenCruise,&fenMission::editCruise);
    QObject::connect(ui->btn_OpenRep,&QPushButton::clicked,this,&MainWindow::ouvreRepCourant);
    QObject::connect(ui->actionOuvrirMissions,&QAction::triggered,this,&MainWindow::ouvrirMissions);
    QObject::connect(ui->actionOuvrirConfMission,&QAction::triggered,this,&MainWindow::ouvrirConfMission);
    QObject::connect(ui->treeView,&QTreeView::doubleClicked,this,&MainWindow::on_treeView_doubleClicked);
    QObject::connect(ui->actionPreferences,&QAction::triggered,fenPref,&fenPreferences::init);
    QObject::connect(ui->actionQuitter,&QAction::triggered,this,&MainWindow::close);
    QObject::connect(fenPref,&fenPreferences::editingFinished,this,&MainWindow::init);
    QObject::connect(fenCruise,&fenMission::editingFinished,this,&MainWindow::init);
    QObject::connect(fenCruise,&fenMission::editingFinished,fenPref,&fenPreferences::endMissionEdit);
    QObject::connect(ui->actionAfficheConfig,&QAction::triggered,this,&MainWindow::afficheConfig);
    QObject::connect(fenCruise,&fenMission::newCruiseSet,fenPref,&fenPreferences::setCurrentCruise);
    QObject::connect(fenCruise,&fenMission::dataListRequest,this,&MainWindow::dataListRequested);
    QObject::connect(ui->listGroupWidget,&QListWidget::itemClicked,this,&MainWindow::groupSelected);
    QObject::connect(ui->listSensorWidget,&QListWidget::itemClicked,this,&MainWindow::sensorSelected);

}

MainWindow::~MainWindow()
{
    delete ui;

}



void MainWindow::init()
{
    mPathToMissions=mSettings->value("PathToMissions","").toString();
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

    mCurrentSensors.clear();
    mCurrentSensors=fenPref->getSensorsListFromGroups(mCurrentCruise.listDataGroups);

    mDataManagers.clear();
    QListIterator<Sensor*> it(mCurrentSensors);
    while(it.hasNext())
    {
        DataManager* data=new DataManager(nullptr,it.next());
        mDataManagers.append(data);
        mLayoutDataManager->addWidget(data);
        data->hide();
    }




    m_DClic=0;

    if(!mCurrentCruise.sNom.isEmpty())
        ui->treeView->setRootIndex(treeModel->setRootPath(QString("%1/%2").arg(mPathToMissions).arg(mCurrentCruise.sNom)));

/*
   mData2040->setVisible(mCurrentCruise.listEquipements.contains("EM2040"));
   mDataEK60->setVisible(mCurrentCruise.listEquipements.contains("EK60"));
   mDataME70->setVisible(mCurrentCruise.listEquipements.contains("ME70"));
   mDataKlein->setVisible(mCurrentCruise.listEquipements.contains("Klein3000"));
   mDataRoxAnn->setVisible(mCurrentCruise.listEquipements.contains("RoxAnn"));
   mDataSparker->setVisible(mCurrentCruise.listEquipements.contains("Sparker"));
   mDataPeskavel->setVisible(mCurrentCruise.listEquipements.contains("Peskavel"));
   mDataMaregraphe->setVisible(mCurrentCruise.listEquipements.contains("Maregraphe"));
   mDataScout->setVisible(mCurrentCruise.listEquipements.contains("Scout"));
   mDataHydrins->setVisible(mCurrentCruise.listEquipements.contains("Hydrins"));
   mDataOsea->setVisible(mCurrentCruise.listEquipements.contains("Osea"));*/

    /*
   if(mCurrentCruise.listEquipements.contains("EM2040"))
   {
       mData2040->setSynchroPath(QString("%1/%2").arg(mCurrentCruise.sPath).arg(QString("EQUIPEMENTS/EM2040/DONNEES")));
       mData2040->setAnnexePath(QString("%1/%2").arg(mCurrentCruise.sPath).arg(QString("EQUIPEMENTS/EM2040/ANNEXES")));
       QStringList filter;
       filter<<"*.all"<<"*.wcd";
       mData2040->setFilter(filter);
   }
   if(mCurrentCruise.listEquipements.contains("EK60"))
   {
       mDataEK60->setSynchroPath(QString("%1/%2").arg(mCurrentCruise.sPath).arg(QString("EQUIPEMENTS/EK60/DONNEES")));
       mDataEK60->setAnnexePath(QString("%1/%2").arg(mCurrentCruise.sPath).arg(QString("EQUIPEMENTS/EK60/ANNEXES")));

   }


*/

}

void MainWindow::ouvreRepCourant()
{
    QDesktopServices::openUrl(QUrl(QString("%1/%2").arg(mPathToMissions).arg(mCurrentCruise.sNom)));
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
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
    fenCruise->setRecordList(fenPref->getSensorsList());
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
    ui->listSensorWidget->clear();
    mLastDataManager->hide();
    QListIterator<Sensor*>it(mCurrentSensors);
    while (it.hasNext()) {
        Sensor* sensor=it.next();
        Sensor::Parameters param=sensor->getParameters();

        if(item->text()==param.sGroup)
        {
            ui->listSensorWidget->addItem(param.sName);
        }

    }


    /*
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
*/
}

void MainWindow::sensorSelected(QListWidgetItem *item)
{
    mLastDataManager->hide();
    QListIterator<DataManager*>it(mDataManagers);
    while (it.hasNext()) {
        DataManager* data=it.next();
        if(data->getSensorName().compare(item->text())==0)
        {
            mLastDataManager=data;
            mLastDataManager->show();


        }

    }


}


