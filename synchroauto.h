#ifndef SYNCHROAUTO_H
#define SYNCHROAUTO_H

#include <QWidget>
#include "record.h"

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
    void setRecord(Record* record);
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

    Record* mRecord;
    bool mTimerIsRunning=false;
    bool mSyncIsRunning=false;
    QTimer* mTimerSync;
    int mDureeTotale=0; //en secondes
    int mTempsEcoule=0;

};

#endif // SYNCHROAUTO_H
