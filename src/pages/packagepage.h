#pragma once

#include <QWidget>

class CollapsibleOutput;
class QLineEdit;
class QListWidget;
class QPushButton;
class QProcess;

class PackagePage : public QWidget
{
    Q_OBJECT

  public:
    explicit PackagePage(QWidget *parent = nullptr);

  private slots:
    void onSearch();
    void onInstall();
    void onRemove();
    void onProcessOutput();
    void onProcessFinished(int exitCode);

  private:
    void runDnf(const QStringList &args);

    QLineEdit *m_searchBar;
    QListWidget *m_packageList;
    QPushButton *m_installBtn;
    QPushButton *m_removeBtn;
    CollapsibleOutput *m_output;
    QProcess *m_process;
};
