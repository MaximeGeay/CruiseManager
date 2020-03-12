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
    ui->cb_2040->setChecked(false);
    ui->cb_EK60->setChecked(false);
    ui->cb_ME70->setChecked(false);
    ui->cb_Osea->setChecked(false);
    ui->cb_Klein->setChecked(false);
    ui->cb_Scout->setChecked(false);
    ui->cb_Roxann->setChecked(false);
    ui->cb_Hydrins->setChecked(false);
    ui->cb_Sparker->setChecked(false);
    ui->cb_Peskavel->setChecked(false);
    ui->cb_Maregraphe->setChecked(false);

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
    /*QStringList listEquip=currentCruise.listDataGroups;

    ui->cb_2040->setChecked(listEquip.contains(ui->cb_2040->text()));
    ui->cb_EK60->setChecked(listEquip.contains(ui->cb_EK60->text()));
    ui->cb_ME70->setChecked(listEquip.contains(ui->cb_ME70->text()));
    ui->cb_Osea->setChecked(listEquip.contains(ui->cb_Osea->text()));
    ui->cb_Klein->setChecked(listEquip.contains(ui->cb_Klein->text()));
    ui->cb_Scout->setChecked(listEquip.contains(ui->cb_Scout->text()));
    ui->cb_Roxann->setChecked(listEquip.contains(ui->cb_Roxann->text()));
    ui->cb_Hydrins->setChecked(listEquip.contains(ui->cb_Hydrins->text()));
    ui->cb_Sparker->setChecked(listEquip.contains(ui->cb_Sparker->text()));
    ui->cb_Peskavel->setChecked(listEquip.contains(ui->cb_Peskavel->text()));
    ui->cb_Maregraphe->setChecked(listEquip.contains(ui->cb_Maregraphe->text()));*/
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


/*
void fenMission::valider()
{

    if (ui->le_Mission->text().isEmpty())
    {
        QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le nom de la mission ne peut pas être vide"));
        return;
    }

    QString sPathToMissions=mSettings->value("PathToMissions","").toString();
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


    if(mNewCruise)
    {
        emit newCruiseSet(currentCruise.sNom);

        dirRoot.mkdir(currentCruise.sNom);
        dirRoot.setPath(QString("%1/%2").arg(dirRoot.path()).arg(currentCruise.sNom));
        dirRoot.mkpath(QString("%1/ARCHIV_NETCDF/DONNEES").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/ARCHIV_NETCDF/ANNEXES").arg(dirRoot.path()));
        dirRoot.mkdir("ARCHIV_NMEA");
        dirRoot.mkdir("CELERITE");
        dirRoot.mkpath(QString("%1/CASINO/CSV").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/CASINO/BASE_LOCALE").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/DIVERS/TIRANT_EAU").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/DIVERS/RAPPORT").arg(dirRoot.path()));
        dirRoot.mkdir("REJEU");


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
            if(!fFile->copy(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Cahier_quart_%1.odt").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Cahier_quart_%1.odt n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Cahier_quart_exemple.odt n'a pas été trouvé"));

        fFile=new QFile(QString("%1/CR_MISSION_DEC.doc").arg(sPathToConfMission));
        if(fFile->exists())
        {
            if(!fFile->copy(QString("%1/%2/DIVERS/RAPPORT/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Rapport_%1.doc").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Rapport_%1.doc n'a pas pu être créé").arg(currentCruise.sNom));
        }else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier CR_MISSION_DEC.doc n'a pas été trouvé"));

        fFile=new QFile(QString("%1/Sippican_exemple.xls").arg(sPathToConfMission));
        if(fFile->exists())
        {
            if(!fFile->copy(QString("%1/%2/CELERITE/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Sippican_%1.xls").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_%1.xls n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_exemple.xls n'a pas été trouvé"));


        fFile=new QFile(QString("%1/Fiche_recap_TIRANT_EAU.docx").arg(sPathToConfMission));
        if(fFile->exists())
        {
            if(!fFile->copy(QString("%1/%2/DIVERS/TIRANT_EAU/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Tirant_Eau_%1.docx").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Tirant_Eau_%1.xlsx n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Fiche_recap_TIRANT_EAU.docx n'a pas été trouvé"));

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
                       qDebug()<<QString("%1/%2").arg(sPathToMissions).arg(missionPrec.sNom)<<QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom);

                       return;
                }
                emit newCruiseSet(currentCruise.sNom);

                dirRoot.setPath(QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom));

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
        else
        {
            dirRoot.setPath(QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom));
        }
    }


    if(ui->cb_2040->isChecked())
    {
        currentCruise.listEquipements.append(ui->cb_2040->text());
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/EM2040/DONNEES").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/EM2040/ANNEXES").arg(dirRoot.path()));
    }
    else
    {
        removeDir(QString("EM2040"),currentCruise.sNom);
    }

    if(ui->cb_EK60->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/EK60/DONNEES").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/EK60/ANNEXES").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_EK60->text());
    }
    else
    {
        removeDir(QString("EK60"),currentCruise.sNom);
    }


    if(ui->cb_ME70->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/ME70/DONNEES").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/ME70/ANNEXES").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_ME70->text());
    }
    else
    {
        removeDir(QString("ME70"),currentCruise.sNom);
    }

    if(ui->cb_Osea->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/OSEA").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Osea->text());
    }
    else
    {
        removeDir(QString("OSEA"),currentCruise.sNom);
    }

    if(ui->cb_Klein->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/KLEIN3000/DONNEES").arg(dirRoot.path()));
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/KLEIN3000/ANNEXES").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Klein->text());
    }
    else
    {
        removeDir(QString("KLEIN3000"),currentCruise.sNom);
    }
    if(ui->cb_Scout->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/SCOUT").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Scout->text());
    }
    else
    {
        removeDir(QString("SCOUT"),currentCruise.sNom);
    }
    if(ui->cb_Roxann->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/ROXANN").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Roxann->text());
    }
    else
    {
        removeDir(QString("ROXANN"),currentCruise.sNom);
    }
    if(ui->cb_Hydrins->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/HYDRINS").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Hydrins->text());
    }
    else
    {
        removeDir(QString("HYDRINS"),currentCruise.sNom);
    }
    if(ui->cb_Sparker->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/SPARKER").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Sparker->text());
    }
    else
    {
        removeDir(QString("SPARKER"),currentCruise.sNom);
    }
    if(ui->cb_Peskavel->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/PESKAVEL").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Peskavel->text());
    }
    else
    {
        removeDir(QString("PESKAVEL"),currentCruise.sNom);
    }
    if(ui->cb_Maregraphe->isChecked())
    {
        dirRoot.mkpath(QString("%1/EQUIPEMENTS/MAREGRAPHE").arg(dirRoot.path()));
        currentCruise.listEquipements.append(ui->cb_Maregraphe->text());
    }
    else
    {
        removeDir(QString("MAREGRAPHE"),currentCruise.sNom);
    }




    mSettings->setValue("Mission/Nom",currentCruise.sNom);
    mSettings->setValue("Mission/Zone",currentCruise.sZone);
    mSettings->setValue("Mission/Navire",currentCruise.sNavire);
    mSettings->setValue("Mission/ChefMission",currentCruise.sChefMission);
    mSettings->setValue("Mission/DateDebut",currentCruise.dateDebut);
    mSettings->setValue("Mission/DateFin",currentCruise.dateFin);
    mSettings->setValue("Mission/Operateurs",currentCruise.sOperateurs);
    mSettings->setValue("Mission/Equipements",currentCruise.listEquipements);
    mSettings->setValue("Mission/Path",dirRoot.path());


    QListIterator<QCheckBox*> it(mListCBData);
    while(it.hasNext())
    {
        QCheckBox* cb=it.next();
        mSettings->setValue(QString("Datas/%1/IsChecked").arg(cb->text()),cb->isChecked());
    }


    emit editingFinished();


    this->close();


}*/

void fenMission::valider()
{
    if (ui->le_Mission->text().isEmpty())
    {
        QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le nom de la mission ne peut pas être vide"));
        return;
    }

    QString sPathToMissions=mSettings->value("PathToMissions","").toString();
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

        fFile=new QFile(QString("%1/Sippican_exemple.xls").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/CELERITE").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/CELERITE/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Sippican_%1.xls").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_%1.xls n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Sippican_exemple.xls n'a pas été trouvé"));


        fFile=new QFile(QString("%1/Fiche_recap_TIRANT_EAU.docx").arg(sPathToConfMission));
        if(fFile->exists())
        {
            dirRoot.mkpath(QString("%1/%2/DIVERS/TIRANT_EAU").arg(sPathToMissions).arg(currentCruise.sNom));
            if(!fFile->copy(QString("%1/%2/DIVERS/TIRANT_EAU/%3").arg(sPathToMissions).arg(currentCruise.sNom).arg(QString("Tirant_Eau_%1.docx").arg(currentCruise.sNom))))
                QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Tirant_Eau_%1.xlsx n'a pas pu être créé").arg(currentCruise.sNom));
        }
        else
            QMessageBox::warning(this,QString("Erreur lors de la création de campagne"),QString("Le fichier Fiche_recap_TIRANT_EAU.docx n'a pas été trouvé"));
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
                       qDebug()<<QString("%1/%2").arg(sPathToMissions).arg(missionPrec.sNom)<<QString("%1/%2").arg(sPathToMissions).arg(currentCruise.sNom);

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
    ui->cb_2040->setEnabled(bAdm);
    ui->cb_EK60->setEnabled(bAdm);
    ui->cb_ME70->setEnabled(bAdm);
    ui->cb_Osea->setEnabled(bAdm);
    ui->cb_Klein->setEnabled(bAdm);
    ui->cb_Scout->setEnabled(bAdm);
    ui->cb_Roxann->setEnabled(bAdm);
    ui->cb_Hydrins->setEnabled(bAdm);
    ui->cb_Sparker->setEnabled(bAdm);
    ui->cb_Peskavel->setEnabled(bAdm);
    ui->cb_Maregraphe->setEnabled(bAdm);
}

bool fenMission::removeDir(QString sDir,QString sMission)
{
    QString sPathToMissions=mSettings->value("PathToMissions","").toString();
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
    QString sPathToMissions=mSettings->value("PathToMissions","").toString();
    QString sPathToNewCruise=QString("%1/%2").arg(sPathToMissions).arg(ui->le_Mission->text());
    QString sRelativePath;
    QDir dir;
    QList<Sensor*>tempList;
    foreach(Sensor* sensor,mListRecords)
    {
        Sensor::Parameters param=sensor->getParameters();

        sRelativePath=param.sDestPath.section(sPathToMissions,-1).section("/",2,-1);
        param.sDestPath=QString("%1/%2").arg(sPathToNewCruise).arg(sRelativePath);

        if(mCurrentCruiseDataGroups.contains(param.sGroup))
        {

        dir.mkpath(param.sDestPath);
        }
        else
        {
            if(dir.exists(param.sDestPath))
            {
                QMessageBox::warning(this,QString("Répertoires à supprimer"),QString("%1 n'est plus sélectionné, il faudrait supprimer les répertoires correspondants").arg(param.sName));
                QDesktopServices::openUrl(param.sDestPath);
            }
        }

    }
    emit newCruiseSet(ui->le_Mission->text());
}

