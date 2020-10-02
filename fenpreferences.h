#ifndef FENPREFERENCES_H
#define FENPREFERENCES_H

#include <QWidget>
#include <QList>
#include "sensor.h"

namespace Ui {
class fenPreferences;
}
class QSettings;
class QVBoxLayout;
class QListWidgetItem;
class fenPreferences : public QWidget
{
    Q_OBJECT

public:
    explicit fenPreferences(QWidget *parent = nullptr);
    ~fenPreferences();

public slots:
    void init();
    QStringList getDataGroups();
    QList<Sensor*> getSensorsList();
    void setCurrentCruise(QString sCruise);
    void endMissionEdit();
    QList<Sensor*> getSensorsListFromGroups(QStringList groupList);

private slots:
    void selectCruisesPath();
    void selectConfMissionPath();
    void valider();
    void annuler();
    void addNewSensor();
    void addSensor(Sensor *sensor);
    void removeSensor();
    void showMessage(QString sMsg);
    void editingStatusControl();
    bool listSensorSelected(QListWidgetItem *item);
    void majListItem();
    void sensorEditionFinished();
    void addGroup();
    void editGroup();
    void rmGroup();
    void majGroupListWidget();
    void appendGroup(QString sGroup);
    void listGroupSelected(QListWidgetItem *item);


signals:
    void editingFinished();

private:
    Ui::fenPreferences *ui;
    QSettings *mSettings;
    QString mPathToMissions;
    QString mCurrentCruise;
    QString mPathToConfMission;
    QVBoxLayout *mSensorLayout;
    QList<Sensor*> mSensorList;
    Sensor* mCurrentSensor;
    void saveSensorList();
    bool mbNewSensor=false;
    QStringList mGroupList;
    bool mNewCruise=false;


};

#endif // FENPREFERENCES_H
