#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "program.h"
#include <QProcess>
#include <QMainWindow>
#include <qdir.h>
#include <qlistwidget.h>
#include <QFile>
#include <QTextStream>
#include <windows.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:

signals:

private slots:
    void on_actionAdd_Program_triggered();
    void loadPrograms();
    void savePrograms();
    void deletePrograms();
    void startInstall();
    void setArguments();
    void openScriptMenu();
    void loadClickedItemSettings();
    void cancelSettings();
    int  programExist(Program);
    void removeUnusedPrograms(QStringList);
    void processEnd(int,QProcess::ExitStatus);
    void IsWow64();

private:
    Ui::MainWindow *ui;
    QDir m_root;
    QListWidget *m_list;
    QList<Program> m_programList;
    QProcess *process = new QProcess(this);
    int processNumber;
    QString fullUrl;
    QStringList arg;
    BOOL bIsWow64;
};

#endif // MAINWINDOW_H
