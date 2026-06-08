#include "driverspage.h"
#include "widgets/collapsibleoutput.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

DriversPage::DriversPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(0);

    // ── Title + subtitle ───────────────────────────────────────
    auto *title = new QLabel("Drivers", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(4);

    auto *sub = new QLabel("Detect hardware and install the appropriate drivers.", this);
    sub->setObjectName("pageSubtitle");
    sub->setWordWrap(true);
    root->addWidget(sub);
    root->addSpacing(20);

    // ── Card: driver list ──────────────────────────────────────
    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    m_driverList = new QListWidget(card);
    m_driverList->setObjectName("cardList");
    cardLayout->addWidget(m_driverList);

    root->addWidget(card, 1);
    root->addSpacing(10);

    // ── Action buttons ─────────────────────────────────────────
    auto *actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_detectBtn = new QPushButton("Detect Hardware", this);
    m_installBtn = new QPushButton("Install Driver", this);
    m_installBtn->setEnabled(false);

    actionRow->addWidget(m_detectBtn);
    actionRow->addWidget(m_installBtn);
    actionRow->addStretch();
    root->addLayout(actionRow);
    root->addSpacing(14);

    // ── Collapsible output ─────────────────────────────────────
    m_output = new CollapsibleOutput(this);
    root->addWidget(m_output);

    // ── Connections ────────────────────────────────────────────
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
    m_output->expand();
    m_output->append("Detecting hardware…");
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
    m_output->expand();
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
