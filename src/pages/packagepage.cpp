#include "packagepage.h"
#include "widgets/collapsibleoutput.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

PackagePage::PackagePage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(0);

    // ── Title ──────────────────────────────────────────────────
    auto *title = new QLabel("Packages", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(16);

    // ── Search row ─────────────────────────────────────────────
    auto *searchRow = new QHBoxLayout;
    searchRow->setSpacing(10);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Search packages…");

    auto *searchBtn = new QPushButton("Search", this);

    searchRow->addWidget(m_searchBar, 1);
    searchRow->addWidget(searchBtn);
    root->addLayout(searchRow);
    root->addSpacing(12);

    // ── Card: package list ─────────────────────────────────────
    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    m_packageList = new QListWidget(card);
    m_packageList->setObjectName("cardList");
    cardLayout->addWidget(m_packageList);

    root->addWidget(card, 1);
    root->addSpacing(10);

    // ── Action buttons ─────────────────────────────────────────
    auto *actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);

    m_installBtn = new QPushButton("Install", this);
    m_removeBtn = new QPushButton("Remove", this);
    m_installBtn->setEnabled(false);
    m_removeBtn->setEnabled(false);

    actionRow->addWidget(m_installBtn);
    actionRow->addWidget(m_removeBtn);
    actionRow->addStretch();
    root->addLayout(actionRow);
    root->addSpacing(14);

    // ── Collapsible output ─────────────────────────────────────
    m_output = new CollapsibleOutput(this);
    root->addWidget(m_output);

    // ── Connections ────────────────────────────────────────────
    connect(searchBtn, &QPushButton::clicked, this, &PackagePage::onSearch);
    connect(m_searchBar, &QLineEdit::returnPressed, this, &PackagePage::onSearch);
    connect(m_installBtn, &QPushButton::clicked, this, &PackagePage::onInstall);
    connect(m_removeBtn, &QPushButton::clicked, this, &PackagePage::onRemove);

    connect(m_packageList, &QListWidget::itemSelectionChanged, this, [this] {
        bool sel = !m_packageList->selectedItems().isEmpty();
        m_installBtn->setEnabled(sel);
        m_removeBtn->setEnabled(sel);
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PackagePage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &PackagePage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });
}

void PackagePage::onSearch()
{
    const QString query = m_searchBar->text().trimmed();
    if (query.isEmpty())
        return;
    m_packageList->clear();
    m_output->clear();
    runDnf({"search", "--quiet", query});
}

void PackagePage::onInstall()
{
    auto *item = m_packageList->currentItem();
    if (!item)
        return;
    const QString pkg = item->text().split(' ').first().split('.').first();
    m_output->clear();
    m_output->expand();
    runDnf({"install", "-y", pkg});
}

void PackagePage::onRemove()
{
    auto *item = m_packageList->currentItem();
    if (!item)
        return;
    const QString pkg = item->text().split(' ').first().split('.').first();
    m_output->clear();
    m_output->expand();
    runDnf({"remove", "-y", pkg});
}

void PackagePage::onProcessOutput()
{
    const QByteArray stdOut = m_process->readAllStandardOutput();
    const QByteArray stdErr = m_process->readAllStandardError();

    if (!stdOut.isEmpty()) {
        if (m_process->arguments().contains("search")) {
            for (const QByteArray &line : stdOut.split('\n')) {
                const QString text = QString::fromUtf8(line).trimmed();
                if (!text.isEmpty() && !text.startsWith('='))
                    m_packageList->addItem(text);
            }
        }
        m_output->append(QString::fromUtf8(stdOut));
    }
    if (!stdErr.isEmpty())
        m_output->append(QString::fromUtf8(stdErr));
}

void PackagePage::onProcessFinished(int exitCode)
{
    m_output->append(exitCode == 0 ? "\n[Done]"
                                   : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void PackagePage::runDnf(const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

    if (args.first() == "search" || args.first() == "list") {
        m_process->setProgram("dnf");
        m_process->setArguments(args);
    } else {
        m_process->setProgram("pkexec");
        m_process->setArguments(QStringList {"dnf"} + args);
    }
    m_process->start();
}
