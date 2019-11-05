#include "mainwindow.h"
#include "program.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <qfiledialog.h>
#include <iostream>
#include <QProcess>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //layout
    ui->setupUi(this);
    ui->prbVerloop->setValue(0);
    ui->lstPrograms->setContextMenuPolicy(Qt::CustomContextMenu);

    //connects
    connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(startInstall()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(setArguments()));
    connect(ui->btnScriptUrl, SIGNAL(clicked()), this, SLOT(openScriptMenu()));
    connect(ui->lstPrograms, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(loadClickedItemSettings()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(cancelSettings()));

    //calls
    loadPrograms();
}

//deconstructor + opslagen van de programma's
MainWindow::~MainWindow()
{
    savePrograms();
    delete ui;
}

//menu pragramma toevoegen
void MainWindow::on_actionAdd_Program_triggered()
{
    Program newProgram;
    QDir path("..\\..\\D-Care"); // assume it is some path
    path.setNameFilters(QStringList()<<"*.exe"<<"*.bat");
    QStringList programs = path.entryList();
    for(int i = 0; i < programs.count(); i++){
        newProgram.setFullUrl("..\\..\\D-Care\\" + programs[i]);
        if(programExist(newProgram) == -1){
            m_programList.append(newProgram);
            ui->lstPrograms->addItem(newProgram.name());
        }
    }
    removeUnusedPrograms(programs);
}

//programma's inladen
void MainWindow::loadPrograms()
{
    Program loadedProgram;
    QFile openedFile("programList.txt");
    if(openedFile.open(QIODevice::ReadOnly)){
        QTextStream in(&openedFile);
        while(!in.atEnd()){
            loadedProgram.setFullUrl(in.readLine());
            loadedProgram.setArguments(in.readLine().split(" "));
            loadedProgram.setScriptUrl(in.readLine());
            m_programList.append(loadedProgram);
            ui->lstPrograms->addItem(loadedProgram.name());
        }
    }
    openedFile.close();
    on_actionAdd_Program_triggered();
}

//programma's opslaan
void MainWindow::savePrograms()
{
    QFile newFile("programList.txt");
    if(!newFile.open(QIODevice::WriteOnly)){
        qFatal("could not open file");
    }
    QTextStream out(&newFile);
    for(int i=0; i < m_programList.count(); ++i){
        out << m_programList[i].fullUrl() << endl << m_programList[i].arguments().join(" ") << endl << m_programList[i].scriptUrl() << endl;
    }
    newFile.close();
}

//programa uit de lijst verwijderen
void MainWindow::deletePrograms()
{
    foreach (QListWidgetItem *item, ui->lstPrograms->selectedItems()){
        m_programList.removeAt(ui->lstPrograms->row(item));
        ui->lneArguments->clear();
        ui->lneScriptUrl->clear();
        delete item;
    }
}

//starten van de instalatie
void MainWindow::startInstall()
{
    QString fullUrl;
    QStringList arg;

    int ongoing = 100 / m_programList.count();
    for(int i=0; i < m_programList.count(); ++i){
        if(i == ui->lstPrograms->count() - 1){
            ui->prbVerloop->setValue(100);
        }else{
            ui->prbVerloop->setValue(ongoing * (i+1));
        }
        if(m_programList[i].hasScript()){
            fullUrl = m_programList[i].scriptUrl();
        }else{
            fullUrl = m_programList[i].fullUrl();
        }

        if(m_programList[i].hasArguments()){
            arg = m_programList[i].arguments();
        }

        QProcess *process = new QProcess(this);
        process->start(fullUrl, arg);
        process->waitForFinished(-1);
        QProcess::ProcessError error = process->error();
        if(error == QProcess::ProcessError::FailedToStart){
            ui->lstOutput->addItem(fullUrl + "\n\rFailed to start!!\n\r" + process->errorString());
        }else if(error == QProcess::ProcessError::Crashed){
            ui->lstOutput->addItem(fullUrl + "\n\rCrached!!\n\r" + process->errorString());
        }else if(error == QProcess::ProcessError::Timedout){
            ui->lstOutput->addItem(fullUrl + "\n\rTimedout!!\n\r" + process->errorString());
        }else if(error == QProcess::ProcessError::ReadError){
            ui->lstOutput->addItem(fullUrl + "\n\rRead error!!\n\r" + process->errorString());
        }else if(error == QProcess::ProcessError::WriteError){
            ui->lstOutput->addItem(fullUrl + "\n\rWrite error!!\n\r" + process->errorString());
        }
        else{
            ui->lstOutput->addItem(fullUrl + "\n\rended");
        }
        process->close();
    }
}

void MainWindow::setArguments()
{
    if(ui->lstPrograms->currentRow() != -1){
        m_programList[ui->lstPrograms->currentRow()].setArguments(ui->lneArguments->text().split(" "));
        m_programList[ui->lstPrograms->currentRow()].setScriptUrl(ui->lneScriptUrl->text());
        savePrograms();
    }else{
        QMessageBox allert;
        allert.setIcon(QMessageBox::Information);
        allert.setText("Please select a program first.");
        allert.exec();
    }
}

void MainWindow::openScriptMenu()
{
    ui->lneScriptUrl->setText(QFileDialog::getOpenFileName(this, tr("Shose program"),m_root.absolutePath(),tr("Executables (*.*)")));
}

void MainWindow::loadClickedItemSettings()
{
    ui->lneArguments->setText(m_programList[ui->lstPrograms->currentRow()].arguments().join(" "));
    ui->lneScriptUrl->setText(m_programList[ui->lstPrograms->currentRow()].scriptUrl());
}

void MainWindow::cancelSettings()
{
    if(ui->lstPrograms->currentRow() != -1){
        ui->lneArguments->setText(m_programList[ui->lstPrograms->currentRow()].arguments().join(" "));
        ui->lneScriptUrl->setText(m_programList[ui->lstPrograms->currentRow()].scriptUrl());
    }
}

//kijken of het programma bestaat, als het programma bestaat return -1 anders index
int MainWindow::programExist(Program program)
{
    int index = -1;
    int count = 0;
    while(count < m_programList.count() && index == -1){
        if(m_programList[count].name() == program.name()){
            index = count;
        }
        count++;
    }
    return index;
}

void MainWindow::removeUnusedPrograms(QStringList programs)
{
    bool exist = false;
    for(int i = 0; i < m_programList.count(); i++){
        exist = false;
        for(int j = 0; j < programs.count(); j++){
            if(m_programList[i].name() == programs[j]){
               exist = true;
            }
        }
        if(!exist){
            ui->lstPrograms->takeItem(i);
            m_programList.removeAt(i);
            if(i != 0){
                i--;
            }
        }
    }
}


