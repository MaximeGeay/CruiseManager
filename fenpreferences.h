#ifndef FENPREFERENCES_H
#define FENPREFERENCES_H

#include <QWidget>
#include <QList>
#include "record.h"

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
    QList<Record*> getRecordsList();
    void setCurrentCruise(QString sCruise);
    void endMissionEdit();
    QList<Record*> getRecordsListFromGroups(QStringList groupList);

private slots:
    void selectCruisesPath();
    void selectConfMissionPath();
    void valider();
    void annuler();
    void addNewRecord();
    void addRecord(Record *record);
    void removeRecord();
    void showMessage(QString sMsg);
    void editingStatusControl();
    bool listRecordSelected(QListWidgetItem *item);
    void majListItem();
    void recordEditionFinished();
    void addGroup();
    void editGroup();
    void rmGroup();
    void majGroupListWidget();
    void appendGroup(QString sGroup);
    void listGroupSelected(QListWidgetItem *item);
    void clickOnSaveRecordFile();
    void clickOnOpenRecordFile();


signals:
    void editingFinished();

private:
    Ui::fenPreferences *ui;
    QSettings *mSettings;
    QString mPathToMissions;
    QString mCurrentCruise;
    QString mPathToConfMission;
    QString mRecordFileName;
    QVBoxLayout *mRecordLayout;
    QList<Record*> mRecordList;
    Record* mCurrentRecord;
    void saveRecordList();
    bool mbNewRecord=false;
    QStringList mGroupList;
    bool mNewCruise=false;


};

#endif // FENPREFERENCES_H
