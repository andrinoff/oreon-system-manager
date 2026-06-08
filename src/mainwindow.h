#pragma once

#include <QMainWindow>

class QStackedWidget;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private:
    Sidebar *m_sidebar;
    QStackedWidget *m_stack;

    void setupUi();
};
