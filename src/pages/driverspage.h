#pragma once

#include <QWidget>

class CollapsibleOutput;
class QListWidget;
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
    QPushButton *m_detectBtn;
    QPushButton *m_installBtn;
    CollapsibleOutput *m_output;
    QProcess *m_process;
};
