#include "mainwindow.h"
#include "widgets/sidebar.h"
#include "pages/packagepage.h"
#include "pages/repopage.h"
#include "pages/containerpage.h"
#include "pages/driverspage.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Oreon System Manager");
    setMinimumSize(1000, 650);
    resize(1200, 750);
    setupUi();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(new PackagePage(this));   // index 0
    m_stack->addWidget(new RepoPage(this));      // index 1
    m_stack->addWidget(new ContainerPage(this)); // index 2
    m_stack->addWidget(new DriversPage(this));   // index 3

    m_sidebar = new Sidebar(this);
    connect(m_sidebar, &Sidebar::pageRequested, m_stack, &QStackedWidget::setCurrentIndex);

    layout->addWidget(m_sidebar);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);
}
