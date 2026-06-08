#include "containerpage.h"
#include "widgets/collapsibleoutput.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

// Build one tab: action buttons at the top, list fills the rest.
static QWidget *makeTab(QListWidget *list, const QStringList &actions, QObject *receiver,
                        std::function<void(const QString &)> handler)
{
    auto *tab = new QWidget;
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 8, 0, 0);
    layout->setSpacing(8);

    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(12, 0, 12, 0);
    btnRow->setSpacing(8);
    for (const QString &action : actions) {
        auto *btn = new QPushButton(action);
        btn->setCursor(Qt::PointingHandCursor);
        QObject::connect(btn, &QPushButton::clicked, receiver,
                         [handler, action] { handler(action); });
        btnRow->addWidget(btn);
    }
    btnRow->addStretch();
    layout->addLayout(btnRow);
    layout->addWidget(list, 1);

    return tab;
}

// Wire a process so its stdout populates `list` when `listArg` is in the args,
// and all output goes to the shared CollapsibleOutput.
static void connectProcess(QProcess *proc, QListWidget *list, const QString &listArg,
                           CollapsibleOutput *output)
{
    QObject::connect(proc, &QProcess::readyReadStandardOutput, proc, [proc, list, listArg, output] {
        const QByteArray data = proc->readAllStandardOutput();
        if (proc->arguments().contains(listArg)) {
            for (const QByteArray &line : data.split('\n')) {
                const QString text = QString::fromUtf8(line).trimmed();
                if (!text.isEmpty())
                    list->addItem(text);
            }
        }
        output->append(QString::fromUtf8(data));
    });

    QObject::connect(proc, &QProcess::readyReadStandardError, proc, [proc, output] {
        output->append(QString::fromUtf8(proc->readAllStandardError()));
    });

    QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), output,
                     [output](int code, QProcess::ExitStatus) {
                         output->append(code == 0 ? "\n[Done]"
                                                  : QString("\n[Failed — exit code %1]").arg(code));
                     });
}

ContainerPage::ContainerPage(QWidget *parent)
    : QWidget(parent)
    , m_dockerProcess(new QProcess(this))
    , m_distroboxProcess(new QProcess(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(0);

    // ── Title ──────────────────────────────────────────────────
    auto *title = new QLabel("Containers", this);
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(20);

    // ── Card: tab widget ───────────────────────────────────────
    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    m_output = new CollapsibleOutput(this);

    m_tabs = new QTabWidget(card);
    m_tabs->setObjectName("cardTabs");

    m_dockerList = new QListWidget;
    m_distroboxList = new QListWidget;
    m_dockerList->setObjectName("cardList");
    m_distroboxList->setObjectName("cardList");

    connectProcess(m_dockerProcess, m_dockerList, "ps", m_output);
    connectProcess(m_distroboxProcess, m_distroboxList, "list", m_output);

    m_tabs->addTab(makeTab(m_dockerList, {"Start", "Stop", "Remove", "Refresh"}, this,
                           [this](const QString &a) { onDockerAction(a); }),
                   "Docker");

    m_tabs->addTab(makeTab(m_distroboxList, {"Enter", "Stop", "Delete", "Refresh"}, this,
                           [this](const QString &a) { onDistroboxAction(a); }),
                   "Distrobox");

    cardLayout->addWidget(m_tabs);
    root->addWidget(card, 1);
    root->addSpacing(14);

    // ── Collapsible output ─────────────────────────────────────
    root->addWidget(m_output);

    refreshDocker();
    refreshDistrobox();
}

void ContainerPage::refreshDocker()
{
    m_dockerList->clear();
    runDocker({"ps", "-a", "--format", "{{.Names}}\t{{.Status}}\t{{.Image}}"});
}

void ContainerPage::refreshDistrobox()
{
    m_distroboxList->clear();
    runDistrobox({"list"});
}

void ContainerPage::onDockerAction(const QString &action)
{
    if (action == "Refresh") {
        refreshDocker();
        return;
    }
    auto *item = m_dockerList->currentItem();
    if (!item)
        return;
    const QString name = item->text().split('\t').first();
    m_output->expand();
    if (action == "Start")
        runDocker({"start", name});
    else if (action == "Stop")
        runDocker({"stop", name});
    else if (action == "Remove")
        runDocker({"rm", name});
}

void ContainerPage::onDistroboxAction(const QString &action)
{
    if (action == "Refresh") {
        refreshDistrobox();
        return;
    }
    auto *item = m_distroboxList->currentItem();
    if (!item)
        return;
    const QString name = item->text().split('|').first().trimmed();
    m_output->expand();
    if (action == "Enter")
        runDistrobox({"enter", name});
    else if (action == "Stop")
        runDistrobox({"stop", name});
    else if (action == "Delete")
        runDistrobox({"rm", name});
}

void ContainerPage::runDocker(const QStringList &args)
{
    if (m_dockerProcess->state() != QProcess::NotRunning)
        m_dockerProcess->kill();
    m_dockerProcess->setProgram("docker");
    m_dockerProcess->setArguments(args);
    m_dockerProcess->start();
}

void ContainerPage::runDistrobox(const QStringList &args)
{
    if (m_distroboxProcess->state() != QProcess::NotRunning)
        m_distroboxProcess->kill();
    m_distroboxProcess->setProgram("distrobox");
    m_distroboxProcess->setArguments(args);
    m_distroboxProcess->start();
}
