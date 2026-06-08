#pragma once

#include <QWidget>

class QListWidget;
class QTextEdit;
class QPushButton;
class QTabWidget;
class QProcess;

class ContainerPage : public QWidget
{
    Q_OBJECT

public:
    explicit ContainerPage(QWidget *parent = nullptr);

private slots:
    void refreshDocker();
    void refreshDistrobox();
    void onDockerAction(const QString &action);
    void onDistroboxAction(const QString &action);
    void onProcessOutput();
    void onProcessFinished(int exitCode);

private:
    void runCommand(const QString &program, const QStringList &args);

    QTabWidget  *m_tabs;
    QListWidget *m_dockerList;
    QListWidget *m_distroboxList;
    QTextEdit   *m_output;
    QProcess    *m_process;
};
