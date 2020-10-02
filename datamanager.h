#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QWidget>
#include <QList>
#include <QProcess>

#include "sensor.h"
#include "synchroauto.h"

namespace Ui {
class DataManager;
}


class QFileSystemModel;

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
    SynchroAuto* getAutoSyncWidget();
    void killSyncAuto();

private slots:
    void clickOnAddFiles();
    void clickOnSynchro();
    void clickOnAutoSync();


    void synchro();
    void rsyncReponse();
    void rsyncEnd(int exitCode, QProcess::ExitStatus exitStatus);
    void treeView_doubleClicked(const QModelIndex &index);

signals:
    void erreur(QString);
    void copyPourcent(int);


private:
    Ui::DataManager *ui;

    Sensor* mSensor;
    SynchroAuto* mAutoSync;
    bool mSynchroIsRunning=false;
    QProcess *mProcessSynchro;
    bool mSystemIsWindows=false;
    bool mSimulate=true;
    bool mManualSync=false;
    int mFilesToSync=0;
    QStringList mFilesList;
    int mCumulSize=0;
    int mSizeToCopy=0;
    bool mSyncOK=false;
    bool m_DClic=false;
    QString mSourcePath;

    QFileSystemModel *treeModel;
    QString getCompletePath(Sensor *sensor);




};

#endif // DATAMANAGER_H
