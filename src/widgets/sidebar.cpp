#include "sidebar.h"
#include "theme/thememanager.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

static constexpr int kSidebarWidth = 240;

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
    , m_group(new QButtonGroup(this))
    , m_themeBtn(new QPushButton(this))
{
    setFixedWidth(kSidebarWidth);
    setObjectName("sidebar");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Header ─────────────────────────────────────────────────
    auto *header = new QWidget(this);
    header->setObjectName("sidebarHeader");
    header->setFixedHeight(72);

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(18, 0, 18, 0);
    headerLayout->setSpacing(10);
    headerLayout->setAlignment(Qt::AlignVCenter);

    QPixmap logo;
    if (logo.load(":/assets/logo.png") || logo.load(":/assets/logo.svg")) {
        auto *logoLabel = new QLabel(this);
        logoLabel->setPixmap(logo.scaled(34, 34, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setObjectName("sidebarTitle");
        headerLayout->addWidget(logoLabel);
    }

    auto *appName = new QLabel("Oreon", this);
    appName->setObjectName("sidebarTitle");
    headerLayout->addWidget(appName);
    headerLayout->addStretch();

    layout->addWidget(header);

    // ── Nav buttons ─────────────────────────────────────────────
    layout->addSpacing(8);
    addNavButton(layout, "Packages", "package", 0);
    addNavButton(layout, "Repos", "repo", 1);
    addNavButton(layout, "Containers", "container", 2);
    addNavButton(layout, "Drivers", "drivers", 3);

    layout->addStretch();

    // ── Light / dark toggle ─────────────────────────────────────
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("sidebarSeparator");
    layout->addWidget(sep);

    auto *toggleArea = new QWidget(this);
    toggleArea->setObjectName("sidebar");
    auto *toggleLayout = new QHBoxLayout(toggleArea);
    toggleLayout->setContentsMargins(12, 10, 12, 14);
    toggleLayout->setSpacing(4);

    const bool isDark = ThemeManager::instance().current() == "Breeze Dark";
    m_themeBtn->setObjectName("themeToggle");
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setText(isDark ? "☀" : "☾");
    m_themeBtn->setToolTip(isDark ? "Switch to Light" : "Switch to Dark");

    toggleLayout->addStretch();
    toggleLayout->addWidget(m_themeBtn);
    toggleLayout->addStretch();

    layout->addWidget(toggleArea);

    // ── Connections ─────────────────────────────────────────────
    m_group->button(0)->setChecked(true);

    connect(m_group, &QButtonGroup::idClicked, this, &Sidebar::pageRequested);

    connect(m_themeBtn, &QPushButton::clicked, this, []() {
        auto &tm = ThemeManager::instance();
        tm.apply(tm.current() == "Breeze Dark" ? "Breeze" : "Breeze Dark");
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this](const QString &name) {
                const bool dark = (name == "Breeze Dark");
                m_themeBtn->setText(dark ? "☀" : "☾");
                m_themeBtn->setToolTip(dark ? "Switch to Light" : "Switch to Dark");
            });
}

void Sidebar::addNavButton(QVBoxLayout *layout, const QString &label, const QString & /*icon*/,
                           int index)
{
    auto *btn = new QPushButton(label, this);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    m_group->addButton(btn, index);
    layout->addWidget(btn);
}
