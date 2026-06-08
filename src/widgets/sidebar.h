#pragma once

#include <QWidget>

class QButtonGroup;
class QVBoxLayout;

class Sidebar : public QWidget
{
    Q_OBJECT

  public:
    explicit Sidebar(QWidget *parent = nullptr);

  signals:
    void pageRequested(int index);

  private:
    void addNavButton(QVBoxLayout *layout, const QString &label, const QString &icon, int index);

    QButtonGroup *m_group;
};
