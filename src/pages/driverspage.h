#pragma once

#include <QWidget>

class QListWidget;
class QTextEdit;
class QPushButton;
class QProcess;

class DriversPage : public QWidget
{
    Q_OBJECT

public:
    explicit DriversPage(QWidget *parent = nullptr);

private slots:
    void detectDrivers();
    void onInstallDriver();
    void onProcessOutput();
    void onProcessFinished(int exitCode);

private:
    void runCommand(const QString &program, const QStringList &args);

    QListWidget *m_driverList;
    QPushButton *m_installBtn;
    QPushButton *m_detectBtn;
    QTextEdit   *m_output;
    QProcess    *m_process;
};
