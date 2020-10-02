#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "fenmission.h"
#include "sensor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSettings;
class QFileSystemModel;
class fenPreferences;
class DataManager;
class QVBoxLayout;
class QHBoxLayout;
class QListWidgetItem;
class QScrollArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:

    void init();
    void ouvreRepCourant();
    void treeView_doubleClicked(const QModelIndex &index);
    void afficheConfig();
    void dataListRequested();
    void ouvrirMissions();
    void ouvrirConfMission();
    void groupSelected(QListWidgetItem *item);
    void sensorSelected(QListWidgetItem *item);
    void closeEvent(QCloseEvent* event);
    void clickOnRaz();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *treeModel;
    QString mPathToMissions;
    QString mPathToConfMission;
    QSettings *mSettings;
    bool m_DClic;
    fenPreferences *fenPref;
    fenMission *fenCruise;
    fenMission::st_Mission mCurrentCruise;
    QStringList mDataGroups;
    QList<Sensor*>mCurrentSensors;
    QList<DataManager*>mDataManagers;
     QHBoxLayout *mLayoutDataManager;
     QVBoxLayout *mLayoutSyncAuto;
     DataManager *mLastDataManager;




};
#endif // MAINWINDOW_H
