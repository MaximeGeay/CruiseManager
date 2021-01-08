#include <QFileDialog>
#include<QDebug>
#include <QSettings>

#include "record.h"
#include "ui_record.h"


Record::Record(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Record)
{
    ui->setupUi(this);

    QObject::connect(ui->btn_Valider,&QPushButton::clicked,this,&Record::clicOnValider);
    QObject::connect(ui->btn_Annuler,&QPushButton::clicked,this,&Record::clickOnAnnuler);
    QObject::connect(ui->btn_ParcSource,&QToolButton::clicked,this,&Record::clickOnParcSource);
    QObject::connect(ui->btn_ParcDest,&QToolButton::clicked,this,&Record::clickOnParcDest);
    QObject::connect(ui->Btn_Modifier,&QToolButton::clicked,this,&Record::verrouiller);
    QObject::connect(ui->cb_Type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Record::typeChanged);



    //init();
}

Record::~Record()
{
    delete ui;
}

void Record::write(QXmlStreamWriter &writer)
{

        QString uri="http://cqt.cruisemanager.fr/recordlist";
        writer.writeEmptyElement(uri,"record");

        writer.writeAttribute("Name",QString("%1").arg(mParameters.sName));
        writer.writeAttribute("SourcePath",QString("%1").arg(mParameters.sSourcePath));
        writer.writeAttribute("DestPath",QString("%1").arg(mParameters.sDestPath));
        writer.writeAttribute("Type",QString("%1").arg(mParameters.type));
        writer.writeAttribute("Sync",QString("%1").arg(mParameters.sync));
        writer.writeAttribute("Ext",QString("%1").arg(mParameters.sExtensions));
       // writer.writeAttribute("Valid",QString("%1").arg(mParameters.bValid));
        writer.writeAttribute("Group",QString("%1").arg(mParameters.sGroup));
        writer.writeAttribute("Desc",QString("%1").arg(mParameters.sDesc));
        writer.writeAttribute("SyncAuto",QString("%1").arg(mParameters.bSyncAuto));
        writer.writeAttribute("Recurrence",QString("%1").arg(mParameters.nRecurrence));




}

void Record::setParameters(Record::Parameters param)
{
    mParameters=param;
    init();
}

Record::Parameters Record::getParameters()
{
    return mParameters;
}

bool Record::getEditStatus()
{
    return mEditing;
}

void Record::setDeverrouiller(bool bStatus)
{
    ui->Btn_Modifier->setChecked(bStatus);
    init();
}

void Record::setGroupList(QStringList slGroups)
{
    mGroups=slGroups;
    ui->cb_Groupes->clear();
    ui->cb_Groupes->addItems(mGroups);
    selGroup();


}

QStringList Record::getGroupList()
{
    return  mGroups;
}




void Record::init()
{
    verrouiller();

    ui->le_Name->setText(mParameters.sName);
    ui->le_Dest->setText(mParameters.sDestPath);
    ui->le_Source->setText(mParameters.sSourcePath);
    ui->le_ext->setText(mParameters.sExtensions);
    ui->te_Desc->setText(mParameters.sDesc);
    ui->cb_Auto->setChecked(mParameters.bSyncAuto);
    ui->sp_timer->setValue(mParameters.nRecurrence);
    //ui->sp_timer->setEnabled(ui->cb_Auto->isChecked());

   // ui->cb_Activer->setChecked(mParameters.bValid);
    ui->cb_Type->setCurrentIndex(mParameters.type);
    selGroup();
   //qDebug()<<"param"<<mParameters.sName<<mParameters.sync;
    typeChanged(ui->cb_Type->currentIndex());
    syncChanged(mParameters.sync);

    //  this->show();

}

void Record::verrouiller()
{
    bool bVerrou=!ui->Btn_Modifier->isChecked();
    //ui->Btn_Modifier->setVisible(!bVerrou);
    ui->le_Name->setReadOnly(bVerrou);
    ui->rd_Increment->setEnabled(!bVerrou);
    ui->rd_Mirror->setEnabled(!bVerrou);
    ui->le_Source->setReadOnly(bVerrou);
    ui->btn_ParcSource->setEnabled(!bVerrou);
    ui->btn_ParcDest->setEnabled(!bVerrou);
    ui->le_Dest->setReadOnly(bVerrou);
    ui->cb_Type->setEnabled(!bVerrou);
    ui->le_ext->setReadOnly(bVerrou);
    ui->cb_Groupes->setEnabled(!bVerrou);
    ui->btn_Annuler->setVisible(!bVerrou);
    ui->btn_Valider->setVisible(!bVerrou);
    ui->Btn_Modifier->setVisible(bVerrou);
    ui->te_Desc->setReadOnly(bVerrou);
    ui->cb_Auto->setEnabled(!bVerrou);
    ui->sp_timer->setEnabled(!bVerrou);

    mEditing=!bVerrou;
    emit editingStatusChanged();

}

void Record::clicOnValider()
{
    mParameters.sName=ui->le_Name->text();
    mParameters.sDestPath=ui->le_Dest->text();
    qDebug()<<mParameters.sDestPath;
    mParameters.sSourcePath=ui->le_Source->text();
    mParameters.sExtensions=ui->le_ext->text();
   // mParameters.bValid=ui->cb_Activer->isChecked();
    mParameters.sDesc=ui->te_Desc->toPlainText();
    mParameters.type=static_cast<Record::RecordType>(ui->cb_Type->currentIndex());
    mParameters.sGroup=ui->cb_Groupes->currentText();
    if(ui->rd_Mirror->isChecked())
        mParameters.sync=SyncType::Mirror;
    else
        mParameters.sync=SyncType::Incremental;
    //qDebug()<<"sync"<<mParameters.sync;


    mParameters.bSyncAuto=ui->cb_Auto->isChecked();
    mParameters.nRecurrence=ui->sp_timer->value();
    emit editingFinished();

    clickOnAnnuler();
}

void Record::clickOnAnnuler()
{
    ui->Btn_Modifier->setChecked(false);
    init();
}

void Record::clickOnParcSource()
{
    QString sChemin=QFileDialog::getExistingDirectory(this,QString("Sélectionner le répertoire sources des données %1").arg(ui->le_Name->text()),mParameters.sSourcePath,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
        ui->le_Source->setText(sChemin);
}

void Record::clickOnParcDest()
{
    QSettings settings("CruiseManager","Settings");

    QString sMissionPath=QString("%1/%2").arg(settings.value("Mission/Path").toString()).arg(settings.value("Mission/Nom").toString());
    QString sChemin=QFileDialog::getExistingDirectory(this,QString("Sélectionner le répertoire destinations des données %1").arg(ui->le_Name->text()),sMissionPath,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
    {
        ui->le_Dest->setText(sChemin.section(sMissionPath,-1));

    }
}

void Record::typeChanged(int nType)
{
    //qDebug()<<"nType"<<nType;
    if(nType==RecordType::Files)
    {
      //   qDebug()<<"File"<<nType;
        ui->le_ext->setVisible(true);
        ui->l_Ext->setVisible(true);
    }
    else
    {
        if(nType==RecordType::Folder || nType==RecordType::Cruise)
        {
            // qDebug()<<"Folder"<<nType;
            ui->le_ext->setVisible(false);
            ui->l_Ext->setVisible(false);
        }
    }
}

void Record::syncChanged(int nSync)
{

    if(nSync==SyncType::Incremental)
    {
        ui->rd_Increment->setChecked(true);
        ui->rd_Mirror->setChecked(false);
    }
    if(nSync==SyncType::Mirror)
    {
        ui->rd_Increment->setChecked(false);
        ui->rd_Mirror->setChecked(true);
    }

}

void Record::selGroup()
{
    ui->cb_Groupes->setCurrentIndex(ui->cb_Groupes->findText(mParameters.sGroup));
}
