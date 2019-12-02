#include "mainwindow.h"
#include "program.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <qfiledialog.h>
#include <iostream>
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
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processEnd(int, QProcess::ExitStatus)));

    //see if the envirement is windows 64bit er 32bit
    IsWow64();
    if(bIsWow64){
        ui->lstOutput->addItem("Running in 64bit envirement");
    }else{
        ui->lstOutput->addItem("Running in 32bit envirement");
    }

    //variable initialization
    processNumber = 0;

    //calls
    loadPrograms();
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

void MainWindow::IsWow64()
{
    bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
            ui->lstOutput->addItem("error calling fnIsWow64Process\n\r");
        }
    }
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
    QString programPath = "";
    if(bIsWow64){
        programPath = "..\\..\\D-Care";
    }else{
        programPath = "..\\..\\D-Care(x86)";
    }
    QDir path(programPath); // assume it is some path
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
    QString path = "";
    if(bIsWow64){
        path = "programList.txt";
    }else{
        path = "programList(x86).txt";
    }
    QFile openedFile(path);
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
    QString path = "";
    if(bIsWow64){
        path = "programList.txt";
    }else{
        path = "programList(x86).txt";
    }
    QFile newFile(path);
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

//einde van een process opvangen
void MainWindow::processEnd(int, QProcess::ExitStatus status)
{
    if(status == QProcess::ExitStatus::NormalExit){
       ui->lstOutput->addItem(fullUrl + "\n\rended");
    }else{
       ui->lstOutput->addItem(fullUrl + "\n\rFailed to start!!\n\r" + process->errorString());
    }
    int ongoing = 100 / m_programList.count();
    if(processNumber == ui->lstPrograms->count() - 1){
        ui->prbVerloop->setValue(100);
    }else{
        ui->prbVerloop->setValue(ongoing * (processNumber+1));
    }
    processNumber++;
    startInstall();
}

//starten van de instalatie
void MainWindow::startInstall()
{
    ui->btnStart->setDisabled(true);
    ui->lstOutput->addItem("Starting install");
    if(processNumber <= m_programList.count()-1){
        if(m_programList[processNumber].hasScript()){
            fullUrl = m_programList[processNumber].scriptUrl();
        }else{
            fullUrl = m_programList[processNumber].fullUrl();
        }

        if(m_programList[processNumber].hasArguments()){
            arg = m_programList[processNumber].arguments();
        }
        process->start(fullUrl, arg);
    }
    if(processNumber == m_programList.count()){
        processNumber = 0;
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




