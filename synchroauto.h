#ifndef SYNCHROAUTO_H
#define SYNCHROAUTO_H

#include <QWidget>
#include "sensor.h"

namespace Ui {
class SynchroAuto;
}

class QTimer;

class SynchroAuto : public QWidget
{
    Q_OBJECT

public:
    explicit SynchroAuto(QWidget *parent = nullptr);
    ~SynchroAuto();

signals:
    void runSynchro();
public slots:
    void setSensor(Sensor* sensor);
    bool isRunning();
    void synchroEnd();
    void percent(int nPourcent);
    void setMessage(QString sMessage);

private slots:
    void status();
    void clickOnStart();
    void timeout();

private:
    Ui::SynchroAuto *ui;

    Sensor* mSensor;
    bool mTimerIsRunning=false;
    bool mSyncIsRunning=false;
    QTimer* mTimerSync;
    int mDureeTotale=0; //en secondes
    int mTempsEcoule=0;

};

#endif // SYNCHROAUTO_H
