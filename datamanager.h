#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QWidget>
#include <QList>
#include <QProcess>

#include "sensor.h"

namespace Ui {
class DataManager;
}



class DataManager : public QWidget
{
    Q_OBJECT

public:


    explicit DataManager(QWidget *parent = nullptr, Sensor* sensor=nullptr);
    ~DataManager();

public slots:
    void setSensor(Sensor* sensor);
    bool getSynchroStatus();
    void execSynchro();
    QString getSensorName();

private slots:
    void clickOnAddFiles();
    void clickOnSynchro();
    void synchroAutoStatusChanged(bool bStatus);
    void synchro();
    void rsyncReponse();
    void rsyncEnd(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    void erreur(QString);
    void copyPourcent(int);

private:
    Ui::DataManager *ui;

    Sensor* mSensor;
    bool mSynchroStatus=false;
    QProcess *mProcessSynchro;
    bool mSystemIsWindows=false;
    bool mSimulate=true;
    bool mManualSync=false;
    int mFilesToSync=0;
    int mCumulSize=0;
    int mSizeToCopy=0;



};

#endif // DATAMANAGER_H
