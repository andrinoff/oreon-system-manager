#include "repopage.h"
#include "widgets/collapsibleoutput.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

RepoPage::RepoPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(0);

    // ── Title ──────────────────────────────────────────────────
    auto *title = new QLabel("Repositories", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(20);

    // ── Card: repo list ────────────────────────────────────────
    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    m_repoList = new QListWidget(card);
    m_repoList->setObjectName("cardList");
    cardLayout->addWidget(m_repoList);

    root->addWidget(card, 1);
    root->addSpacing(10);

    // ── Action buttons ─────────────────────────────────────────
    auto *actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_toggleBtn = new QPushButton("Enable / Disable", this);
    m_refreshBtn = new QPushButton("Refresh", this);
    m_toggleBtn->setEnabled(false);

    actionRow->addWidget(m_toggleBtn);
    actionRow->addStretch();
    actionRow->addWidget(m_refreshBtn);
    root->addLayout(actionRow);
    root->addSpacing(14);

    // ── Collapsible output ─────────────────────────────────────
    m_output = new CollapsibleOutput(this);
    root->addWidget(m_output);

    // ── Connections ────────────────────────────────────────────
    connect(m_refreshBtn, &QPushButton::clicked, this, &RepoPage::loadRepos);
    connect(m_toggleBtn, &QPushButton::clicked, this, &RepoPage::onToggleRepo);
    connect(m_repoList, &QListWidget::itemSelectionChanged, this,
            [this] { m_toggleBtn->setEnabled(!m_repoList->selectedItems().isEmpty()); });

    connect(m_process, &QProcess::readyReadStandardOutput, this, &RepoPage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &RepoPage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });

    loadRepos();
}

void RepoPage::loadRepos()
{
    m_repoList->clear();
    m_output->clear();
    m_process->setProgram("dnf");
    m_process->setArguments({"repolist", "all", "--quiet"});
    m_process->start();
}

void RepoPage::onToggleRepo()
{
    auto *item = m_repoList->currentItem();
    if (!item)
        return;
    const QString repoId = item->text().split(' ').first();
    const bool enabled = item->text().contains("enabled");
    const QString action = enabled ? "--disablerepo" : "--enablerepo";
    m_output->expand();
    runDnfConfig({action, repoId});
}

void RepoPage::onProcessOutput()
{
    const QByteArray stdOut = m_process->readAllStandardOutput();
    const QByteArray stdErr = m_process->readAllStandardError();

    if (!stdOut.isEmpty()) {
        for (const QByteArray &line : stdOut.split('\n')) {
            const QString text = QString::fromUtf8(line).trimmed();
            if (!text.isEmpty())
                m_repoList->addItem(text);
        }
        m_output->append(QString::fromUtf8(stdOut));
    }
    if (!stdErr.isEmpty())
        m_output->append(QString::fromUtf8(stdErr));
}

void RepoPage::onProcessFinished(int exitCode)
{
    m_output->append(exitCode == 0 ? "\n[Done]"
                                   : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void RepoPage::runDnfConfig(const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();
    m_process->setProgram("pkexec");
    m_process->setArguments(QStringList {"dnf", "config-manager"} + args);
    m_process->start();
}
