#include "packagepage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

PackagePage::PackagePage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    auto *title = new QLabel("Package Management", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);

    // Search row
    auto *searchRow = new QHBoxLayout;
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Search packages…");
    auto *searchBtn = new QPushButton("Search", this);
    searchRow->addWidget(m_searchBar);
    searchRow->addWidget(searchBtn);
    root->addLayout(searchRow);

    // Splitter: package list | output terminal
    auto *splitter = new QSplitter(Qt::Vertical, this);

    m_packageList = new QListWidget(splitter);

    auto *bottomPane = new QWidget(splitter);
    auto *bottomLayout = new QVBoxLayout(bottomPane);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(6);

    auto *btnRow = new QHBoxLayout;
    m_installBtn = new QPushButton("Install", this);
    m_removeBtn  = new QPushButton("Remove",  this);
    m_installBtn->setEnabled(false);
    m_removeBtn->setEnabled(false);
    btnRow->addWidget(m_installBtn);
    btnRow->addWidget(m_removeBtn);
    btnRow->addStretch();
    bottomLayout->addLayout(btnRow);

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setObjectName("terminal");
    m_output->setPlaceholderText("Command output appears here…");
    bottomLayout->addWidget(m_output);

    splitter->addWidget(m_packageList);
    splitter->addWidget(bottomPane);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    root->addWidget(splitter, 1);

    connect(searchBtn,     &QPushButton::clicked,  this, &PackagePage::onSearch);
    connect(m_searchBar,   &QLineEdit::returnPressed, this, &PackagePage::onSearch);
    connect(m_installBtn,  &QPushButton::clicked,  this, &PackagePage::onInstall);
    connect(m_removeBtn,   &QPushButton::clicked,  this, &PackagePage::onRemove);
    connect(m_packageList, &QListWidget::itemSelectionChanged, this, [this] {
        bool sel = !m_packageList->selectedItems().isEmpty();
        m_installBtn->setEnabled(sel);
        m_removeBtn->setEnabled(sel);
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PackagePage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,  this, &PackagePage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });
}

void PackagePage::onSearch()
{
    const QString query = m_searchBar->text().trimmed();
    if (query.isEmpty()) return;

    m_packageList->clear();
    m_output->clear();
    runDnf({"search", "--quiet", query});
}

void PackagePage::onInstall()
{
    auto *item = m_packageList->currentItem();
    if (!item) return;

    // Package name is the first token before whitespace in the list item
    const QString pkg = item->text().split(' ').first().split('.').first();
    m_output->clear();
    runDnf({"install", "-y", pkg});
}

void PackagePage::onRemove()
{
    auto *item = m_packageList->currentItem();
    if (!item) return;

    const QString pkg = item->text().split(' ').first().split('.').first();
    m_output->clear();
    runDnf({"remove", "-y", pkg});
}

void PackagePage::onProcessOutput()
{
    const QByteArray stdOut = m_process->readAllStandardOutput();
    const QByteArray stdErr = m_process->readAllStandardError();

    if (!stdOut.isEmpty()) {
        // Populate list when search results arrive
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
    m_output->append(exitCode == 0
        ? "\n[Done]"
        : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void PackagePage::runDnf(const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

    m_process->setProgram("pkexec");
    // For read-only ops (search/list) skip pkexec to avoid auth prompts
    if (args.first() == "search" || args.first() == "list") {
        m_process->setProgram("dnf");
        m_process->setArguments(args);
    } else {
        m_process->setArguments(QStringList{"dnf"} + args);
    }
    m_process->start();
}
