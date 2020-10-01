#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QDesktopServices>
//#include <QListIterator>

#include "fenmission.h"
#include "ui_fenmission.h"

fenMission::fenMission(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fenMission)
{
    ui->setupUi(this);

    mSettings=new QSettings("CruiseManager","Settings");
    QObject::connect(ui->btn_Valider,&QPushButton::clicked,this,&fenMission::valider);
    QObject::connect(ui->btn_Annuler,&QPushButton::clicked,this,&fenMission::annuler);
    QObject::connect(ui->cb_Admin,&QCheckBox::stateChanged,this,&fenMission::setAdmin);

    mDataLayout=new QVBoxLayout(ui->gb_Datas);
    mDataLayout->setAlignment(Qt::AlignTop);
    mDataLayout->setDirection(QBoxLayout::TopToBottom);

}

fenMission::~fenMission()
{
    delete ui;
}

fenMission::st_Mission fenMission::getCurrentCruise()
{
    st_Mission currentCruise;
    currentCruise.sNom=mSettings->value("Mission/Nom","").toString();
    currentCruise.sZone=mSettings->value("Mission/Zone","").toString();
    currentCruise.sNavire=mSettings->value("Mission/Navire","").toString();
    currentCruise.sChefMission=mSettings->value("Mission/ChefMission","").toString();
    currentCruise.dateDebut=mSettings->value("Mission/DateDebut","").toDate();
    currentCruise.dateFin=mSettings->value("Mission/DateFin","").toDate();
    currentCruise.sOperateurs=mSettings->value("Mission/Operateurs","").toString();
    currentCruise.listDataGroups=mSettings->value("Mission/DataGroups","").toStringList();
    currentCruise.sPath=mSettings->value("Mission/Path","").toString();

    return currentCruise;
}

void fenMission::newCruise()
{
    ui->le_Mission->setText("");
    ui->le_Zone->setText("");
    ui->le_Navire->setText("");
    ui->le_ChefMission->setText("");
    ui->te_Operateurs->clear();
    ui->de_Debut->setDate(QDate::currentDate());
    ui->de_Fin->setDate(QDate::currentDate().addDays(7));

    mNewCruise=true;
    ui->cb_Admin->setChecked(true);
    setAdmin(Qt::Checked);
    emit dataListRequest();
    this->show();
}

void fenMission::editCruise()
{

    st_Mission currentCruise=getCurrentCruise();
    ui->le_Mission->setText(currentCruise.sNom);
    ui->le_Zone->setText(currentCruise.sZone);
    ui->le_Navire->setText(currentCruise.sNavire);
    ui->le_ChefMission->setText(currentCruise.sChefMission);
    ui->te_Operateurs->setText(currentCruise.sOperateurs);
    ui->de_Debut->setDate(currentCruise.dateDebut);
    ui->de_Fin->setDate(currentCruise.dateFin);

    mNewCruise=false;
    ui->cb_Admin->setChecked(false);
    setAdmin(Qt::Unchecked);
    emit dataListRequest();
    this->show();
}

void fenMission::setDataGroups(QStringList groupList)
{
    QListIterator<QCheckBox*>itCB(mListCBData);
    while (itCB.hasNext()) {
        mDataLayout->removeWidget(itCB.next());

    }

    mListCBData.clear();
    mDataGroups=groupList;
    mCurrentCruiseDataGroups.clear();
    QStringListIterator it(mDataGroups);
    QString sGroup;
    int bIsChecked=false;
    while(it.hasNext())
    {
        sGroup=it.next();

        if(sGroup.compare(QString("Tous"))!=0)
        {
            QCheckBox *cb=new QCheckBox(sGroup);
            bIsChecked=mSettings->value(QString("Datas/%1/IsChecked").arg(sGroup),false).toBool();
            cb->setChecked(bIsChecked);

            mListCBData.append(cb);
            mDataLayout->addWidget(cb);
            if(bIsChecked)
                mCurrentCruiseDataGroups.append(sGroup);

        }

    }



}

void fenMission::setRecordList(QList<Sensor *> sensorList)
{
    mListRecords=sensorList;
}

void fenMission::init()
{
    emit dataListRequest();
}



void fenMission::valider()
{
    if (ui->le_Mission->text().isEmpty())
    {
        QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le nom de la mission ne peut pas être vide"));
        return;
    }

    QString sPathToMissions=mSettings->value("Mission/Path","").toString();
    QString sPathToConfMission=mSettings->value("PathToConfMission","").toString();
    QDir dirRoot(sPathToMissions);
    if(dirRoot.exists(ui->le_Mission->text())&&mNewCruise)
    {
        QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("La mission %1 existe déjà.\r\nVeuillez choisir un nom différent").arg(ui->le_Mission->text()));
        return;
    }

    st_Mission currentCruise;
    currentCruise.sNom=ui->le_Mission->text();
    currentCruise.sZone=ui->le_Zone->text();
    currentCruise.sNavire=ui->le_Navire->text();
    currentCruise.sOperateurs=ui->te_Operateurs->toPlainText();
    currentCruise.sChefMission=ui->le_ChefMission->text();
    currentCruise.dateDebut=ui->de_Debut->date();
    currentCruise.dateFin=ui->de_Fin->date();

    mCurrentCruiseDataGroups.clear();
    QListIterator<QCheckBox*> itC(mListCBData);
    while(itC.hasNext())
    {
        QCheckBox* cb=itC.next();
        if(cb->isChecked())
            mCurrentCruiseDataGroups.append(cb->text());

        mSettings->setValue(QString("Datas/%1/IsChecked").arg(cb->text()),cb->isChecked());
    }


    if(mNewCruise)
    {

        creeSensorsPaths();


        /**************************************************************
         * ****A MODIFIER PARAMETRABLE*********************************
         * ***********************************************************/
        QFile *fFile=new QFile(QString("%1/Fiche_recap_exemple.odt").arg(sPathToConfMission));
        if(fFile->exists())
        {

            if(!fFile->copy(QString("%1/%2/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Fiche_recap_%1.odt").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Fiche_recap_%1.odt n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Fiche_recap_exemple.odt n'a pas été trouvé"));

        fFile=new QFile(QString("%1/Cahier_quart_exemple.odt").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/DIVERS/RAPPORT").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Cahier_quart_%1.odt").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Cahier_quart_%1.odt n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Cahier_quart_exemple.odt n'a pas été trouvé"));

        fFile=new QFile(QString("%1/CR_MISSION_DEC.doc").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/DIVERS/RAPPORT").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Rapport_%1.doc").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Rapport_%1.doc n'a pas pu être créé").arg(currentCruise.sNom));
        }else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier CR_MISSION_DEC.doc n'a pas été trouvé"));

        fFile=new QFile(QString("%1/Profils_exemple.ods").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/DIVERS/RAPPORT").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Profils_%1.ods").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Profils_%1.ods n'a pas pu être créé").arg(currentCruise.sNom));
        }else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Profils_%1.ods n'a pas été trouvé"));


        fFile=new QFile(QString("%1/Sippican_exemple.ods").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/CELERITE").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/CELERITE/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Sippican_%1.ods").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_%1.xls n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_exemple.ods n'a pas été trouvé"));


        fFile=new QFile(QString("%1/Tirants_eau_exemple.ods").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/DIVERS/TIRANT_EAU").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/DIVERS/TIRANT_EAU/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Tirants_Eau_%1.ods").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Tirants_Eau_%1.ods n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Tirants_eau_exemple.ods n'a pas été trouvé"));
    }

    else
    {
        st_Mission missionPrec=getCurrentCruise();;

        if(currentCruise.sNom!=missionPrec.sNom)
        {
            int nOk=QMessageBox::question(this,tr("Changement du nom de la mission en cours"),QString("Etes-vous sûr de vouloir remplacer le nom de la mission %1 par %2?").arg(missionPrec.sNom).arg(currentCruise.sNom));
            if(nOk==QMessageBox::Yes)
            {
              //  QDir dirTemp(QString("%1/%2").arg(sPathToMissions).arg(missionPrec.sNom));
                QDir dirTemp;
                if(!dirTemp.rename(QString("%1/%2").arg(sPathToMissions).arg(missionPrec.sNom),QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom)))
                {
                       QMessageBox::warning(this,QString("Erreur lors du changement de nom de la campagne"),
                                            QString("Impossible de renommer %1 en %2").arg(missionPrec.sNom).arg(currentCruise.sNom));
                       //qDebug()<<QString("%1/%2").arg(sPathToMissions).arg(missionPrec.sNom)<<QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom);

                       return;
                }
                emit newCruiseSet(currentCruise.sNom);


                QFile fileTemp;
                fileTemp.rename(QString("%1/%2/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Fiche_recap_%1.odt").arg(missionPrec.sNom)),
                                QString("%1/%2/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Fiche_recap_%1.odt").arg(currentCruise.sNom)));
                fileTemp.rename(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Cahier_quart_%1.odt").arg(missionPrec.sNom)),
                                QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Cahier_quart_%1.odt").arg(currentCruise.sNom)));
                fileTemp.rename(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Rapport_%1.doc").arg(missionPrec.sNom)),
                                QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Rapport_%1.doc").arg(currentCruise.sNom)));
                fileTemp.rename(QString("%1/%2/CELERITE/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Sippican_%1.xls").arg(missionPrec.sNom)),
                                QString("%1/%2/CELERITE/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Sippican_%1.xls").arg(currentCruise.sNom)));
                fileTemp.rename(QString("%1/%2/DIVERS/TIRANT_EAU/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Tirant_Eau_%1.docx").arg(missionPrec.sNom)),
                                QString("%1/%2/DIVERS/TIRANT_EAU/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Tirant_Eau_%1.docx").arg(currentCruise.sNom)));

            }
            else
            {
                ui->le_Mission->setText(missionPrec.sNom);
                currentCruise.sNom=ui->le_Mission->text();
            }

        }

        creeSensorsPaths();


    }


    mSettings->setValue("Mission/Nom",currentCruise.sNom);
    mSettings->setValue("Mission/Zone",currentCruise.sZone);
    mSettings->setValue("Mission/Navire",currentCruise.sNavire);
    mSettings->setValue("Mission/ChefMission",currentCruise.sChefMission);
    mSettings->setValue("Mission/DateDebut",currentCruise.dateDebut);
    mSettings->setValue("Mission/DateFin",currentCruise.dateFin);
    mSettings->setValue("Mission/Operateurs",currentCruise.sOperateurs);
    mSettings->setValue("Mission/DataGroups",mCurrentCruiseDataGroups);
    mSettings->setValue("Mission/Path",dirRoot.path());




    emit editingFinished();


    this->close();
}

void fenMission::annuler()
{
    this->close();
}

void fenMission::setAdmin(int nAdm)
{
    bool bAdm=false;
    if(nAdm==Qt::Checked)
        bAdm=true;
    ui->le_Zone->setReadOnly(!bAdm);
    ui->le_Navire->setReadOnly(!bAdm);
    ui->le_Mission->setReadOnly(!bAdm);
    ui->le_ChefMission->setReadOnly(!bAdm);
    ui->te_Operateurs->setReadOnly(!bAdm);
    ui->de_Debut->setReadOnly(!bAdm);
    ui->de_Fin->setReadOnly(!bAdm);

}

bool fenMission::removeDir(QString sDir,QString sMission)
{
    QString sPathToMissions=mSettings->value("Mission/Path","").toString();
    QDir dirTemp;
    QDir dirRoot(QString("%1/%2").arg(sPathToMissions).arg(sMission));
    bool bRes=false;

    if(dirTemp.exists(QString("%1/EQUIPEMENTS/%2").arg(dirRoot.path()).arg(sDir)))
    {
        int nRes=QMessageBox::question(this,QString("Suppression du répertoire %1").arg(sDir),QString("Etes-vous sûr de vouloir supprimer le répertoire %1?\r\n"
                                                                                                "Cette action est irréversible.").arg(sDir));
        if(nRes==QMessageBox::Yes)
        {
            dirTemp.setPath(QString("%1/EQUIPEMENTS/%2").arg(dirRoot.path()).arg(sDir));
            bRes=dirTemp.removeRecursively();

        }
    }

    return bRes;
}

void fenMission::creeSensorsPaths()
{
    QString sPathToMissions=mSettings->value("Mission/Path","").toString();
    QString sPathToNewCruise=QString("%1/%2").arg(sPathToMissions).arg(ui->le_Mission->text());

    QString sRelativePath;
    QDir dir;
    QList<Sensor*>tempList;
    foreach(Sensor* sensor,mListRecords)
    {
        Sensor::Parameters param=sensor->getParameters();
        if(param.type==Sensor::Folder || param.type==Sensor::Files)
        {
        QString sCompletePath=sPathToNewCruise+param.sDestPath;

        if(mCurrentCruiseDataGroups.contains(param.sGroup))
        {

        dir.mkpath(sCompletePath);
        }
        else
        {


            if(dir.exists(sCompletePath))
            {
                QMessageBox::warning(this,QString("Répertoires à supprimer"),QString("%1 n'est plus sélectionné, il faudrait supprimer les répertoires correspondants").arg(param.sName));
                QDesktopServices::openUrl(sCompletePath);
            }
        }
    }
    }
    emit newCruiseSet(ui->le_Mission->text());
}

