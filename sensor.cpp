#include <QFileDialog>
#include<QDebug>

#include "sensor.h"
#include "ui_sensor.h"


Sensor::Sensor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sensor)
{
    ui->setupUi(this);

    QObject::connect(ui->btn_Valider,&QPushButton::clicked,this,&Sensor::clicOnValider);
    QObject::connect(ui->btn_Annuler,&QPushButton::clicked,this,&Sensor::clickOnAnnuler);
    QObject::connect(ui->btn_ParcSource,&QToolButton::clicked,this,&Sensor::clickOnParcSource);
    QObject::connect(ui->btn_ParcDest,&QToolButton::clicked,this,&Sensor::clickOnParcDest);
    QObject::connect(ui->Btn_Modifier,&QToolButton::clicked,this,&Sensor::verrouiller);
    QObject::connect(ui->cb_Type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Sensor::typeChanged);
   // QObject::connect(ui->cb_Activer,&QCheckBox::stateChanged,this,&Sensor::activation);



    init();
}


Sensor::~Sensor()
{
    delete ui;
}

void Sensor::write(QXmlStreamWriter &writer)
{

        QString uri="http://cqt.cruisemanager.fr/sensorlist";
        writer.writeEmptyElement(uri,"sensor");

        writer.writeAttribute("Name",QString("%1").arg(mParameters.sName));
        writer.writeAttribute("SourcePath",QString("%1").arg(mParameters.sSourcePath));
        writer.writeAttribute("DestPath",QString("%1").arg(mParameters.sDestPath));
        writer.writeAttribute("Type",QString("%1").arg(mParameters.type));
        writer.writeAttribute("Ext",QString("%1").arg(mParameters.sExtensions));
       // writer.writeAttribute("Valid",QString("%1").arg(mParameters.bValid));
        writer.writeAttribute("Group",QString("%1").arg(mParameters.sGroup));
        writer.writeAttribute("Desc",QString("%1").arg(mParameters.sDesc));




}

void Sensor::setParameters(Sensor::Parameters param)
{
    mParameters=param;
    init();
}

Sensor::Parameters Sensor::getParameters()
{
    return mParameters;
}

bool Sensor::getEditStatus()
{
    return mEditing;
}

void Sensor::setDeverrouiller(bool bStatus)
{
    ui->Btn_Modifier->setChecked(bStatus);
    init();
}

void Sensor::setGroupList(QStringList slGroups)
{
    mGroups=slGroups;
    ui->cb_Groupes->clear();
    ui->cb_Groupes->addItems(mGroups);
    selGroup();


}

QStringList Sensor::getGroupList()
{
    return  mGroups;
}




void Sensor::init()
{
    verrouiller();

    ui->le_Name->setText(mParameters.sName);
    ui->le_Dest->setText(mParameters.sDestPath);
    ui->le_Source->setText(mParameters.sSourcePath);
    ui->le_ext->setText(mParameters.sExtensions);
    ui->te_Desc->setText(mParameters.sDesc);
   // ui->cb_Activer->setChecked(mParameters.bValid);
    ui->cb_Type->setCurrentIndex(mParameters.type);
    selGroup();

    typeChanged(ui->cb_Type->currentIndex());
  //  this->show();

}

void Sensor::verrouiller()
{
    bool bVerrou=!ui->Btn_Modifier->isChecked();
    //ui->Btn_Modifier->setVisible(!bVerrou);
    ui->le_Name->setReadOnly(bVerrou);
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

    mEditing=!bVerrou;
    emit editingStatusChanged();

}

void Sensor::clicOnValider()
{
    mParameters.sName=ui->le_Name->text();
    mParameters.sDestPath=ui->le_Dest->text();
    mParameters.sSourcePath=ui->le_Source->text();
    mParameters.sExtensions=ui->le_ext->text();
   // mParameters.bValid=ui->cb_Activer->isChecked();
    mParameters.sDesc=ui->te_Desc->toPlainText();
    mParameters.type=static_cast<Sensor::RecordType>(ui->cb_Type->currentIndex());
    mParameters.sGroup=ui->cb_Groupes->currentText();

    emit editingFinished();

    clickOnAnnuler();
}

void Sensor::clickOnAnnuler()
{
    ui->Btn_Modifier->setChecked(false);
    init();
}

void Sensor::clickOnParcSource()
{
    QString sChemin=QFileDialog::getExistingDirectory(this,QString("Sélectionner le répertoire sources des données %1").arg(ui->le_Name->text()),mParameters.sSourcePath,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
        ui->le_Source->setText(sChemin);
}

void Sensor::clickOnParcDest()
{
    QString sChemin=QFileDialog::getExistingDirectory(this,QString("Sélectionner le répertoire destinations des données %1").arg(ui->le_Name->text()),mParameters.sDestPath,QFileDialog::ShowDirsOnly);
    if(!sChemin.isEmpty())
        ui->le_Dest->setText(sChemin);
}

void Sensor::typeChanged(int nType)
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
        if(nType==RecordType::Folder)
        {
            // qDebug()<<"Folder"<<nType;
            ui->le_ext->setVisible(false);
            ui->l_Ext->setVisible(false);
        }
    }
}
/*
void Sensor::activation(int nValid)
{
    if(nValid==0)
        mParameters.bValid=false;
    else
        mParameters.bValid=true;
}*/

void Sensor::selGroup()
{
    ui->cb_Groupes->setCurrentIndex(ui->cb_Groupes->findText(mParameters.sGroup));
}
