#include "collapsibleoutput.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

CollapsibleOutput::CollapsibleOutput(QWidget *parent)
    : QWidget(parent)
    , m_toggle(new QPushButton(this))
    , m_output(new QTextEdit(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_toggle->setObjectName("outputToggle");
    m_toggle->setCursor(Qt::PointingHandCursor);

    m_output->setObjectName("terminal");
    m_output->setReadOnly(true);
    m_output->setPlaceholderText("Command output will appear here…");
    m_output->setMinimumHeight(130);
    m_output->setMaximumHeight(220);
    m_output->setVisible(false);

    syncLabel();

    layout->addWidget(m_toggle);
    layout->addWidget(m_output);

    connect(m_toggle, &QPushButton::clicked, this, &CollapsibleOutput::onToggle);
}

void CollapsibleOutput::append(const QString &text)
{
    m_output->append(text);
}

void CollapsibleOutput::clear()
{
    m_output->clear();
}

void CollapsibleOutput::expand()
{
    if (!m_expanded) {
        m_expanded = true;
        m_output->setVisible(true);
        syncLabel();
    }
}

void CollapsibleOutput::onToggle()
{
    m_expanded = !m_expanded;
    m_output->setVisible(m_expanded);
    syncLabel();
}

void CollapsibleOutput::syncLabel()
{
    m_toggle->setText(m_expanded ? "▾   Output" : "▸   Output");
}
