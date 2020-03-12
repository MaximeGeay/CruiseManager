#ifndef SENSOR_H
#define SENSOR_H

#include <QWidget>
#include "xmlserializable.h"

namespace Ui {
class Sensor;
}

class Sensor : public QWidget
{
    Q_OBJECT

public:

    enum RecordType{
            Files=0,
            Folder=1
        };

        struct Parameters{
          QString sSourcePath,sDestPath,sName,sExtensions="*.*",sGroup="Tous",sDesc;

          //bool bValid;
          RecordType type;
        };


    explicit Sensor(QWidget *parent = nullptr);

    ~Sensor();

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
   // void activation(int nValid);
    void selGroup();

signals:
    void editingStatusChanged();
    void editingFinished();


private:
    Ui::Sensor *ui;
    Parameters mParameters;
    bool mEditing=false;
    QStringList mGroups;
};

#endif // SENSOR_H
