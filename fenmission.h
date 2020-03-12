#ifndef FENMISSION_H
#define FENMISSION_H

#include <QWidget>
#include <QDate>

#include <sensor.h>

namespace Ui {
class fenMission;
}
class QSettings;
class QVBoxLayout;
class QCheckBox;
class fenMission : public QWidget
{
    Q_OBJECT

public:

    struct st_Mission
    {
        QString sNom,sNavire,sChefMission,sZone,sOperateurs,sPath;
        QStringList listDataGroups;
        QDate dateDebut,dateFin;
    };

    explicit fenMission(QWidget *parent = nullptr);
    ~fenMission();

    st_Mission getCurrentCruise();

public slots:
    void newCruise();
    void editCruise();
    void setDataGroups(QStringList groupList);
    void setRecordList(QList<Sensor*> sensorList);



private slots:
    void init();
    void valider();
    //void validerTest();
    void annuler();
    void setAdmin(int nAdm);

signals:
    void editingFinished();
    void dataListRequest();
    void newCruiseSet(QString sNewCruise);

private:
    Ui::fenMission *ui;

    QSettings *mSettings;

    bool mNewCruise=false;
    bool removeDir(QString sDir, QString sMission);
    QStringList mDataGroups;
    QStringList mCurrentCruiseDataGroups;
    QList<Sensor*> mListRecords;
    QVBoxLayout *mDataLayout;
    QList<QCheckBox*> mListCBData;

    void creeSensorsPaths();


};

#endif // FENMISSION_H
