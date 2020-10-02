#include <QTimer>
#include <QDebug>
#include "synchroauto.h"
#include "ui_synchroauto.h"


SynchroAuto::SynchroAuto(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SynchroAuto)
{
    ui->setupUi(this);

    mTimerSync=new QTimer;
    mTimerSync->setInterval(1000);
    QObject::connect(ui->btn_startstop,&QPushButton::clicked,this,&SynchroAuto::clickOnStart);
    QObject::connect(mTimerSync,&QTimer::timeout,this,&SynchroAuto::timeout);
}

SynchroAuto::~SynchroAuto()
{
    mTimerSync->stop();
    delete mTimerSync;

    delete ui;
}

void SynchroAuto::setSensor(Sensor *sensor)
{
    mSensor=sensor;
    Sensor::Parameters param=mSensor->getParameters();
    ui->l_Name->setText(param.sName);
    ui->sp_timer->setValue(param.nRecurrence);

    status();
}




bool SynchroAuto::isRunning()
{
    return mSyncIsRunning;
}

void SynchroAuto::synchroEnd()
{
    mSyncIsRunning=false;
    if(mTimerIsRunning)
    {
        mTempsEcoule=0;
        mTimerSync->start();

    }
    status();
}

void SynchroAuto::percent(int nPourcent)
{

    ui->progressBar->setValue(nPourcent);
}

void SynchroAuto::setMessage(QString sMessage)
{
   ui->l_message->setText(sMessage);
   ui->stackedWidget->setCurrentIndex(1);


}



void SynchroAuto::status()
{
    ui->sp_timer->setEnabled(!mTimerIsRunning);
    ui->progressBar->setVisible(mSyncIsRunning);

    if(!mTimerIsRunning)
    {
        ui->l_etat->setText("Synchronisation arrêtée");
        ui->btn_startstop->setText("Start");
    }
    else
    {
        ui->btn_startstop->setText("Stop");
        if(mSyncIsRunning)
        {
            ui->l_etat->setText("Synchronisation en cours");
            ui->stackedWidget->setCurrentIndex(0);
        }
        else
        {
            ui->l_etat->setText(QString("Prochaine synchronisation dans : %1 s").arg(mDureeTotale-mTempsEcoule));
        }

    }




}

void SynchroAuto::clickOnStart()
{
    if(ui->btn_startstop->isChecked())
    {

        mDureeTotale=ui->sp_timer->value()*60;
        mTempsEcoule=0;
        mTimerSync->start();
        mTimerIsRunning=true;
    }

    else
    {
        mTimerSync->stop();
        mTimerIsRunning=false;
    }
    status();
}

void SynchroAuto::timeout()
{
    mTempsEcoule++;
    if(mTempsEcoule>=mDureeTotale)
    {
        mTimerSync->stop();
        mSyncIsRunning=true;
        emit runSynchro();

    }

    status();

}
