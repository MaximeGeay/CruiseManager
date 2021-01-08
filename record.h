#ifndef RECORD_H
#define RECORD_H

#include <QWidget>
#include "xmlserializable.h"

namespace Ui {
class Record;
}

class Record : public QWidget
{
    Q_OBJECT

public:

    enum RecordType{
            Files=0,
            Folder=1,
            Cruise=2
        };
    enum SyncType{
            Incremental=0,
            Mirror=1
    };

        struct Parameters{
          QString sSourcePath,sDestPath,sName,sExtensions="*.*",sGroup="Tous",sDesc;
          bool bSyncAuto=false;
          int nRecurrence=30;
          RecordType type=Files;
          SyncType sync=Incremental;
        };


    explicit Record(QWidget *parent = nullptr);

    ~Record();

    virtual void write(QXmlStreamWriter &writer);

public slots:
    void setParameters(Parameters param);
    Parameters getParameters();
    bool getEditStatus();
    void setDeverrouiller(bool bStatus);
    void setGroupList(QStringList slGroups);
    QStringList getGroupList();


private slots:
    void init();
    void verrouiller();
    void clicOnValider();
    void clickOnAnnuler();
    void clickOnParcSource();
    void clickOnParcDest();
    void typeChanged(int nType);
    void syncChanged(int nSync);
   // void activation(int nValid);
    void selGroup();

signals:
    void editingStatusChanged();
    void editingFinished();


private:
    Ui::Record *ui;
    Parameters mParameters;
    bool mEditing=false;
    QStringList mGroups;
};

#endif // RECORD_H
