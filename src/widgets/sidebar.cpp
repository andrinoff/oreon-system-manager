#include "sidebar.h"

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

static constexpr int kSidebarWidth = 200;

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
    , m_group(new QButtonGroup(this))
{
    setFixedWidth(kSidebarWidth);
    setObjectName("sidebar");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *title = new QLabel("Oreon", this);
    title->setObjectName("sidebarTitle");
    title->setAlignment(Qt::AlignCenter);
    title->setFixedHeight(60);
    layout->addWidget(title);

    addNavButton(layout, "Packages",   "package",   0);
    addNavButton(layout, "Repos",      "repo",      1);
    addNavButton(layout, "Containers", "container", 2);
    addNavButton(layout, "Drivers",    "drivers",   3);

    layout->addStretch();

    m_group->button(0)->setChecked(true);

    connect(m_group, &QButtonGroup::idClicked, this, &Sidebar::pageRequested);

    setStyleSheet(R"(
        #sidebar {
            background-color: #1e1e2e;
            border-right: 1px solid #313244;
        }
        #sidebarTitle {
            color: #cdd6f4;
            font-size: 16px;
            font-weight: bold;
            border-bottom: 1px solid #313244;
        }
        QPushButton {
            background-color: transparent;
            color: #cdd6f4;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            text-align: left;
            font-size: 14px;
            margin: 2px 8px;
        }
        QPushButton:hover {
            background-color: #313244;
        }
        QPushButton:checked {
            background-color: #45475a;
            color: #89b4fa;
            font-weight: bold;
        }
    )");
}

void Sidebar::addNavButton(QVBoxLayout *layout, const QString &label,
                           const QString & /*icon*/, int index)
{
    auto *btn = new QPushButton(label, this);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    m_group->addButton(btn, index);
    layout->addWidget(btn);
}
