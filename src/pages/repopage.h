#pragma once

#include <QWidget>

class CollapsibleOutput;
class QListWidget;
class QPushButton;
class QProcess;

class RepoPage : public QWidget
{
    Q_OBJECT

  public:
    explicit RepoPage(QWidget *parent = nullptr);

  private slots:
    void loadRepos();
    void onToggleRepo();
    void onProcessOutput();
    void onProcessFinished(int exitCode);

  private:
    void runDnfConfig(const QStringList &args);

    QListWidget *m_repoList;
    QPushButton *m_toggleBtn;
    QPushButton *m_refreshBtn;
    CollapsibleOutput *m_output;
    QProcess *m_process;
};
