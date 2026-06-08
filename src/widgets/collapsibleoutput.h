#pragma once

#include <QWidget>

class QPushButton;
class QTextEdit;

class CollapsibleOutput : public QWidget
{
    Q_OBJECT

  public:
    explicit CollapsibleOutput(QWidget *parent = nullptr);

    void append(const QString &text);
    void clear();
    void expand();

  private:
    QPushButton *m_toggle;
    QTextEdit *m_output;
    bool m_expanded {false};

    void onToggle();
    void syncLabel();
};
