#include "thememanager.h"

#include <QApplication>
#include <QSettings>

ThemeManager &ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager()
{
    // ── Breeze (Light) ─────────────────────────────────────────────────────
    // Off-white page bg + pure-white cards — Apple Settings aesthetic
    ThemeColors breeze;
    breeze.bg = "#F4F4F6"; // light-gray page
    breeze.sidebar = "#FFFFFF";
    breeze.sidebarBorder = "#DDDDE3";
    breeze.cardBg = "#FFFFFF";
    breeze.cardBorder = "#E3E3E8";
    breeze.text = "#1C1C1E";
    breeze.textMuted = "#86868B";
    breeze.hover = "#EBEBF0";
    breeze.active = "#3DAEE9";
    breeze.activeText = "#FFFFFF";
    breeze.accent = "#3DAEE9";
    breeze.inputBg = "#FFFFFF";
    breeze.inputBorder = "#D2D2D7";
    breeze.buttonGradTop = "#FFFFFF";
    breeze.buttonGradBot = "#F4F4F6";
    breeze.buttonHover = "#EBEBF0";
    breeze.buttonBorder = "#C8C8CE";
    m_themes["Breeze"] = breeze;

    // ── Breeze Dark ────────────────────────────────────────────────────────
    ThemeColors breezeDark;
    breezeDark.bg = "#1C1C1E"; // dark base
    breezeDark.sidebar = "#232325";
    breezeDark.sidebarBorder = "#38383A";
    breezeDark.cardBg = "#2C2C2E";
    breezeDark.cardBorder = "#3A3A3C";
    breezeDark.text = "#F5F5F7";
    breezeDark.textMuted = "#8E8E93";
    breezeDark.hover = "#3A3A3C";
    breezeDark.active = "#3DAEE9";
    breezeDark.activeText = "#FFFFFF";
    breezeDark.accent = "#3DAEE9";
    breezeDark.inputBg = "#1C1C1E";
    breezeDark.inputBorder = "#48484A";
    breezeDark.buttonGradTop = "#3A3A3C";
    breezeDark.buttonGradBot = "#323234";
    breezeDark.buttonHover = "#48484A";
    breezeDark.buttonBorder = "#545458";
    m_themes["Breeze Dark"] = breezeDark;

    // ── Catppuccin Mocha ───────────────────────────────────────────────────
    ThemeColors mocha;
    mocha.bg = "#1e1e2e";
    mocha.sidebar = "#181825";
    mocha.sidebarBorder = "#313244";
    mocha.cardBg = "#313244";
    mocha.cardBorder = "#45475a";
    mocha.text = "#cdd6f4";
    mocha.textMuted = "#a6adc8";
    mocha.hover = "#45475a";
    mocha.active = "#89b4fa";
    mocha.activeText = "#1e1e2e";
    mocha.accent = "#89b4fa";
    mocha.inputBg = "#313244";
    mocha.inputBorder = "#45475a";
    mocha.buttonGradTop = "#45475a";
    mocha.buttonGradBot = "#45475a";
    mocha.buttonHover = "#585b70";
    mocha.buttonBorder = "#585b70";
    m_themes["Catppuccin Mocha"] = mocha;

    // ── Nord ───────────────────────────────────────────────────────────────
    ThemeColors nord;
    nord.bg = "#2e3440";
    nord.sidebar = "#242933";
    nord.sidebarBorder = "#3b4252";
    nord.cardBg = "#3b4252";
    nord.cardBorder = "#434c5e";
    nord.text = "#eceff4";
    nord.textMuted = "#9099aa";
    nord.hover = "#434c5e";
    nord.active = "#81a1c1";
    nord.activeText = "#eceff4";
    nord.accent = "#88c0d0";
    nord.inputBg = "#2e3440";
    nord.inputBorder = "#434c5e";
    nord.buttonGradTop = "#434c5e";
    nord.buttonGradBot = "#3f4859";
    nord.buttonHover = "#4c566a";
    nord.buttonBorder = "#4c566a";
    m_themes["Nord"] = nord;
}

QStringList ThemeManager::themeNames() const
{
    return m_themes.keys();
}

void ThemeManager::apply(const QString &name)
{
    if (!m_themes.contains(name))
        return;

    m_current = name;
    qApp->setStyleSheet(buildStyleSheet(m_themes.value(name)));

    QSettings settings;
    settings.setValue("theme", name);

    emit themeChanged(name);
}

void ThemeManager::restore()
{
    QSettings settings;
    const QString saved = settings.value("theme", "Breeze").toString();
    apply(m_themes.contains(saved) ? saved : "Breeze");
}

QString ThemeManager::buildStyleSheet(const ThemeColors &c)
{
    QString ss = R"(
        /* ═══════════════════════════════════════════════════════
           Base
        ═══════════════════════════════════════════════════════ */
        QMainWindow, QDialog {
            background-color: {{bg}};
        }
        QWidget {
            background-color: {{bg}};
            color: {{text}};
        }

        /* ═══════════════════════════════════════════════════════
           Sidebar
        ═══════════════════════════════════════════════════════ */
        #sidebar {
            background-color: {{sidebar}};
            border-right: 1px solid {{sidebarBorder}};
        }
        #sidebarHeader {
            background-color: {{sidebar}};
            border-bottom: 1px solid {{sidebarBorder}};
        }
        #sidebarTitle {
            color: {{accent}};
            font-size: 14pt;
            font-weight: bold;
            background-color: transparent;
            letter-spacing: 1px;
        }

        /* Nav items */
        #sidebar QPushButton {
            background-color: transparent;
            color: {{text}};
            border: none;
            border-radius: 8px;
            padding: 9px 14px;
            text-align: left;
            font-size: 10pt;
            margin: 1px 8px;
            min-height: 22px;
        }
        #sidebar QPushButton:hover {
            background-color: {{hover}};
        }
        #sidebar QPushButton:checked {
            background-color: {{active}};
            color: {{activeText}};
            font-weight: 600;
        }
        #sidebar QPushButton:focus {
            outline: none;
        }

        /* Theme picker at bottom */
        #themeLabel {
            color: {{textMuted}};
            font-size: 8pt;
            font-weight: 700;
            letter-spacing: 1px;
            background-color: transparent;
        }
        #sidebarSeparator {
            background-color: {{sidebarBorder}};
            border: none;
            max-height: 1px;
        }

        /* ═══════════════════════════════════════════════════════
           Page chrome
        ═══════════════════════════════════════════════════════ */
        #pageTitle {
            font-size: 20pt;
            font-weight: 700;
            color: {{text}};
            background-color: transparent;
        }
        #pageSubtitle {
            color: {{textMuted}};
            font-size: 9pt;
            background-color: transparent;
        }

        /* ═══════════════════════════════════════════════════════
           Cards  —  the main visual container on each page
        ═══════════════════════════════════════════════════════ */
        QFrame#card {
            background-color: {{cardBg}};
            border: 1px solid {{cardBorder}};
            border-radius: 12px;
        }

        /* List inside a card — transparent so card bg shows through */
        QListWidget#cardList {
            background-color: transparent;
            border: none;
            outline: none;
            padding: 6px 0;
            font-size: 10pt;
        }
        QListWidget#cardList::item {
            padding: 7px 16px;
            border-radius: 6px;
            min-height: 22px;
        }
        QListWidget#cardList::item:hover {
            background-color: {{hover}};
        }
        QListWidget#cardList::item:selected {
            background-color: {{active}};
            color: {{activeText}};
        }

        /* Hairline separator inside card */
        QFrame#cardSep {
            background-color: {{cardBorder}};
            border: none;
            max-height: 1px;
        }

        /* Tab widget inside a card */
        QTabWidget#cardTabs::pane {
            border: none;
            background: transparent;
        }
        QTabWidget#cardTabs QTabBar::tab {
            background: transparent;
            color: {{textMuted}};
            padding: 8px 20px;
            border: none;
            border-bottom: 2px solid transparent;
            font-size: 10pt;
        }
        QTabWidget#cardTabs QTabBar::tab:selected {
            color: {{accent}};
            border-bottom-color: {{accent}};
            font-weight: 600;
        }
        QTabWidget#cardTabs QTabBar::tab:hover:!selected {
            color: {{text}};
            background-color: {{hover}};
            border-radius: 6px 6px 0 0;
        }

        /* ═══════════════════════════════════════════════════════
           Action buttons (below cards)
        ═══════════════════════════════════════════════════════ */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 {{buttonGradTop}}, stop:1 {{buttonGradBot}});
            color: {{text}};
            border: 1px solid {{buttonBorder}};
            border-radius: 8px;
            padding: 6px 20px;
            font-size: 10pt;
            min-height: 26px;
        }
        QPushButton:hover:enabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 {{buttonGradTop}}, stop:1 {{buttonHover}});
            border-color: {{accent}};
        }
        QPushButton:pressed:enabled {
            background-color: {{buttonHover}};
            padding-top: 7px;
            padding-bottom: 5px;
        }
        QPushButton:disabled {
            color: {{textMuted}};
            border-color: {{sidebarBorder}};
        }
        QPushButton:focus {
            outline: none;
            border-color: {{accent}};
        }

        /* Collapsible output toggle — flat disclosure row */
        QPushButton#outputToggle {
            background: transparent;
            border: none;
            border-radius: 0;
            text-align: left;
            color: {{textMuted}};
            font-size: 10pt;
            padding: 4px 2px;
            min-height: 0;
        }
        QPushButton#outputToggle:hover {
            color: {{text}};
            background: transparent;
            border: none;
        }
        QPushButton#outputToggle:pressed {
            background: transparent;
            padding-top: 4px;
            padding-bottom: 4px;
        }
        QPushButton#outputToggle:focus {
            outline: none;
            border: none;
        }

        /* ═══════════════════════════════════════════════════════
           Inputs
        ═══════════════════════════════════════════════════════ */
        QLineEdit {
            background-color: {{inputBg}};
            color: {{text}};
            border: 1px solid {{inputBorder}};
            border-radius: 8px;
            padding: 6px 11px;
            font-size: 10pt;
            selection-background-color: {{accent}};
            selection-color: #FFFFFF;
        }
        QLineEdit:focus {
            border-color: {{accent}};
        }
        QLineEdit:disabled {
            color: {{textMuted}};
        }

        /* ═══════════════════════════════════════════════════════
           Terminal output
        ═══════════════════════════════════════════════════════ */
        QTextEdit#terminal {
            background-color: {{cardBg}};
            color: {{text}};
            border: 1px solid {{cardBorder}};
            border-radius: 12px;
            font-family: "JetBrains Mono", "Hack", "Fira Code", "Menlo",
                         "Cascadia Code", "Consolas", "Courier New", monospace;
            font-size: 10pt;
            padding: 12px;
            selection-background-color: {{accent}};
            selection-color: #FFFFFF;
        }

        /* ═══════════════════════════════════════════════════════
           ComboBox
        ═══════════════════════════════════════════════════════ */
        QComboBox {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 {{buttonGradTop}}, stop:1 {{buttonGradBot}});
            color: {{text}};
            border: 1px solid {{buttonBorder}};
            border-radius: 8px;
            padding: 4px 10px;
            font-size: 10pt;
            min-height: 24px;
        }
        QComboBox:hover { border-color: {{accent}}; }
        QComboBox:focus { outline: none; border-color: {{accent}}; }
        QComboBox::drop-down { border: none; padding-right: 8px; }
        QComboBox QAbstractItemView {
            background-color: {{cardBg}};
            color: {{text}};
            border: 1px solid {{cardBorder}};
            border-radius: 8px;
            selection-background-color: {{active}};
            selection-color: {{activeText}};
            padding: 4px;
            outline: none;
        }

        /* ═══════════════════════════════════════════════════════
           Splitter
        ═══════════════════════════════════════════════════════ */
        QSplitter::handle { background-color: {{cardBorder}}; }
        QSplitter::handle:horizontal { width: 1px; }
        QSplitter::handle:vertical   { height: 1px; }

        /* ═══════════════════════════════════════════════════════
           Scrollbars  (narrow, minimal)
        ═══════════════════════════════════════════════════════ */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: {{cardBorder}};
            border-radius: 3px;
            min-height: 24px;
        }
        QScrollBar::handle:vertical:hover { background-color: {{inputBorder}}; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0; background: none;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 6px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: {{cardBorder}};
            border-radius: 3px;
            min-width: 24px;
        }
        QScrollBar::handle:horizontal:hover { background-color: {{inputBorder}}; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0; background: none;
        }
    )";

    ss.replace("{{bg}}", c.bg);
    ss.replace("{{sidebar}}", c.sidebar);
    ss.replace("{{sidebarBorder}}", c.sidebarBorder);
    ss.replace("{{cardBg}}", c.cardBg);
    ss.replace("{{cardBorder}}", c.cardBorder);
    ss.replace("{{text}}", c.text);
    ss.replace("{{textMuted}}", c.textMuted);
    ss.replace("{{hover}}", c.hover);
    ss.replace("{{active}}", c.active);
    ss.replace("{{activeText}}", c.activeText);
    ss.replace("{{accent}}", c.accent);
    ss.replace("{{inputBg}}", c.inputBg);
    ss.replace("{{inputBorder}}", c.inputBorder);
    ss.replace("{{buttonGradTop}}", c.buttonGradTop);
    ss.replace("{{buttonGradBot}}", c.buttonGradBot);
    ss.replace("{{buttonHover}}", c.buttonHover);
    ss.replace("{{buttonBorder}}", c.buttonBorder);

    return ss;
}
