#pragma once

#include <QtWidgets/QMainWindow>
#include <QMessageBox>
#include <qtimer.h>
#include <vector>
#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <Windows.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <QMetaObject>
#include "ui_threadsManager.h"
#include "Logic.h"

struct GlobalMin
{
    int Element;
    int Index;
};

struct ThreadData
{
    std::vector<int>* arr;
    int start;
    int end;
    int minElement;
    int minIndex;
};

const int Size = 847483646;

class threadsManager : public QMainWindow
{
    Q_OBJECT

public:
    threadsManager(QWidget *parent = nullptr);
    ~threadsManager();

    void terminateAllThreads();

    void on_reloadButton_clicked();

    void insertProcessToTable(const HANDLE thread);

    void updateTableUI();

    static DWORD WINAPI updateTableThread(LPVOID param);

    void updateTable();

    void launchThreads(int method, int numOfThreads);

    static DWORD WINAPI launchThreadsWrapper(LPVOID param);

    void on_runButton_clicked();

    void on_suspendButton_clicked();

    void on_resumeButton_clicked();

    void on_priorityComboBox_currentIndexChanged(int index);

    void on_killButton_clicked();

private:
    Ui::threadsManagerClass ui;
    QTimer* timer;
    QTimer* countdownTimer;
};
