#include "containerpage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

static QWidget *makeContainerTab(QListWidget *list,
                                  const QStringList &actions,
                                  QObject *receiver,
                                  std::function<void(const QString &)> handler)
{
    auto *tab    = new QWidget;
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 8, 0, 0);
    layout->setSpacing(6);

    auto *btnRow = new QHBoxLayout;
    for (const QString &action : actions) {
        auto *btn = new QPushButton(action);
        btn->setCursor(Qt::PointingHandCursor);
        QObject::connect(btn, &QPushButton::clicked, receiver, [handler, action] {
            handler(action);
        });
        btnRow->addWidget(btn);
    }
    btnRow->addStretch();
    layout->addLayout(btnRow);
    layout->addWidget(list, 1);
    return tab;
}

ContainerPage::ContainerPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    auto *title = new QLabel("Container Management", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    m_tabs         = new QTabWidget(splitter);
    m_dockerList   = new QListWidget;
    m_distroboxList = new QListWidget;

    m_tabs->addTab(
        makeContainerTab(m_dockerList,
                         {"Start", "Stop", "Remove", "Refresh"},
                         this,
                         [this](const QString &a) { onDockerAction(a); }),
        "Docker");

    m_tabs->addTab(
        makeContainerTab(m_distroboxList,
                         {"Enter", "Stop", "Delete", "Refresh"},
                         this,
                         [this](const QString &a) { onDistroboxAction(a); }),
        "Distrobox");

    m_output = new QTextEdit(splitter);
    m_output->setReadOnly(true);
    m_output->setObjectName("terminal");
    m_output->setPlaceholderText("Command output appears here…");

    splitter->addWidget(m_tabs);
    splitter->addWidget(m_output);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    root->addWidget(splitter, 1);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &ContainerPage::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,  this, &ContainerPage::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });

    refreshDocker();
    refreshDistrobox();
}

void ContainerPage::refreshDocker()
{
    m_dockerList->clear();
    runCommand("docker", {"ps", "-a", "--format", "{{.Names}}\t{{.Status}}\t{{.Image}}"});
}

void ContainerPage::refreshDistrobox()
{
    m_distroboxList->clear();
    runCommand("distrobox", {"list"});
}

void ContainerPage::onDockerAction(const QString &action)
{
    if (action == "Refresh") { refreshDocker(); return; }

    auto *item = m_dockerList->currentItem();
    if (!item) return;

    const QString name = item->text().split('\t').first();
    if (action == "Start")  runCommand("docker", {"start",  name});
    else if (action == "Stop")   runCommand("docker", {"stop",   name});
    else if (action == "Remove") runCommand("docker", {"rm",     name});
}

void ContainerPage::onDistroboxAction(const QString &action)
{
    if (action == "Refresh") { refreshDistrobox(); return; }

    auto *item = m_distroboxList->currentItem();
    if (!item) return;

    const QString name = item->text().split('|').first().trimmed();
    if (action == "Enter")  runCommand("distrobox", {"enter", name});
    else if (action == "Stop")   runCommand("distrobox", {"stop",  name});
    else if (action == "Delete") runCommand("distrobox", {"rm",    name});
}

void ContainerPage::onProcessOutput()
{
    const QByteArray stdOut = m_process->readAllStandardOutput();
    const QByteArray stdErr = m_process->readAllStandardError();

    if (!stdOut.isEmpty()) {
        for (const QByteArray &line : stdOut.split('\n')) {
            const QString text = QString::fromUtf8(line).trimmed();
            if (text.isEmpty()) continue;
            if (m_process->program() == "docker")
                m_dockerList->addItem(text);
            else if (m_process->program() == "distrobox" &&
                     m_process->arguments().contains("list"))
                m_distroboxList->addItem(text);
        }
        m_output->append(QString::fromUtf8(stdOut));
    }
    if (!stdErr.isEmpty())
        m_output->append(QString::fromUtf8(stdErr));
}

void ContainerPage::onProcessFinished(int exitCode)
{
    m_output->append(exitCode == 0
        ? "\n[Done]"
        : QString("\n[Failed — exit code %1]").arg(exitCode));
}

void ContainerPage::runCommand(const QString &program, const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

    m_process->setProgram(program);
    m_process->setArguments(args);
    m_process->start();
}
