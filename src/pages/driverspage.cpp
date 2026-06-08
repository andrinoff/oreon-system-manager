#include "driverspage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

DriversPage::DriversPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    auto *title = new QLabel("Drivers", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *desc =
        new QLabel("Detect hardware and install the appropriate drivers via DNF or akmod.", this);
    desc->setWordWrap(true);
    desc->setObjectName("pageSubtitle");
    root->addWidget(desc);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    m_driverList = new QListWidget(splitter);

    auto *bottomPane = new QWidget(splitter);
    auto *bottomLayout = new QVBoxLayout(bottomPane);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(6);

    auto *btnRow = new QHBoxLayout;
    m_detectBtn = new QPushButton("Detect Hardware", this);
    m_installBtn = new QPushButton("Install Driver", this);
    m_installBtn->setEnabled(false);
    btnRow->addWidget(m_detectBtn);
    btnRow->addWidget(m_installBtn);
    btnRow->addStretch();
    bottomLayout->addLayout(btnRow);

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setObjectName("terminal");
    bottomLayout->addWidget(m_output);

    splitter->addWidget(m_driverList);
    splitter->addWidget(bottomPane);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    root->addWidget(splitter, 1);

    connect(m_detectBtn, &QPushButton::clicked, this, &DriversPage::detectDrivers);
    connect(m_installBtn, &QPushButton::clicked, this, &DriversPage::onInstallDriver);
    connect(m_driverList, &QListWidget::itemSelectionChanged, this,
            [this] { m_installBtn->setEnabled(!m_driverList->selectedItems().isEmpty()); });

    connect(m_process, &QProcess::readyReadStandardOutput, this, &DriversPage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &DriversPage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });
}

void DriversPage::detectDrivers()
{
    m_driverList->clear();
    m_output->clear();
    m_output->append("Detecting hardware…");

    // Use lspci to enumerate hardware, then suggest likely driver packages
    runCommand("bash", {"-c", "lspci -mm | awk -F'\"' '{print $2, $4}' | sort -u"});
}

void DriversPage::onInstallDriver()
{
    auto *item = m_driverList->currentItem();
    if (!item)
        return;

    const QString pkg = item->data(Qt::UserRole).toString();
    if (pkg.isEmpty())
        return;

    m_output->clear();
    runCommand("pkexec", {"dnf", "install", "-y", pkg});
}

void DriversPage::onProcessOutput()
{
    const QByteArray stdOut = m_process->readAllStandardOutput();
    const QByteArray stdErr = m_process->readAllStandardError();

    if (!stdOut.isEmpty()) {
        for (const QByteArray &line : stdOut.split('\n')) {
            const QString text = QString::fromUtf8(line).trimmed();
            if (!text.isEmpty())
                m_driverList->addItem(text);
        }
        m_output->append(QString::fromUtf8(stdOut));
    }
    if (!stdErr.isEmpty())
        m_output->append(QString::fromUtf8(stdErr));
}

void DriversPage::onProcessFinished(int exitCode)
{
    m_output->append(exitCode == 0 ? "\n[Done]"
                                   : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void DriversPage::runCommand(const QString &program, const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

    m_process->setProgram(program);
    m_process->setArguments(args);
    m_process->start();
}
