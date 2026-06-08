#include "repopage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

RepoPage::RepoPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    auto *title = new QLabel("Repository Management", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    m_repoList = new QListWidget(splitter);

    auto *bottomPane  = new QWidget(splitter);
    auto *bottomLayout = new QVBoxLayout(bottomPane);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(6);

    auto *btnRow = new QHBoxLayout;
    m_toggleBtn  = new QPushButton("Enable / Disable", this);
    m_refreshBtn = new QPushButton("Refresh", this);
    m_toggleBtn->setEnabled(false);
    btnRow->addWidget(m_toggleBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_refreshBtn);
    bottomLayout->addLayout(btnRow);

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setObjectName("terminal");
    bottomLayout->addWidget(m_output);

    splitter->addWidget(m_repoList);
    splitter->addWidget(bottomPane);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    root->addWidget(splitter, 1);

    connect(m_refreshBtn,  &QPushButton::clicked, this, &RepoPage::loadRepos);
    connect(m_toggleBtn,   &QPushButton::clicked, this, &RepoPage::onToggleRepo);
    connect(m_repoList, &QListWidget::itemSelectionChanged, this, [this] {
        m_toggleBtn->setEnabled(!m_repoList->selectedItems().isEmpty());
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, &RepoPage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,  this, &RepoPage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });

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
    if (!item) return;

    // First token in the line is the repo-id
    const QString repoId = item->text().split(' ').first();
    const bool enabled   = item->text().contains("enabled");
    const QString action = enabled ? "--disablerepo" : "--enablerepo";

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
    m_output->append(exitCode == 0
        ? "\n[Done]"
        : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void RepoPage::runDnfConfig(const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

    m_process->setProgram("pkexec");
    m_process->setArguments(QStringList{"dnf", "config-manager"} + args);
    m_process->start();
}
