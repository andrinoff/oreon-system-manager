#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

struct ThemeColors {
    QString bg;      // page / content background
    QString sidebar; // sidebar panel background
    QString sidebarBorder;
    QString cardBg; // card surface (sits on top of bg)
    QString cardBorder;
    QString text;
    QString textMuted;
    QString hover;
    QString active;
    QString activeText;
    QString accent;
    QString inputBg;
    QString inputBorder;
    QString buttonGradTop;
    QString buttonGradBot;
    QString buttonHover;
    QString buttonBorder;
};

class ThemeManager : public QObject
{
    Q_OBJECT

  public:
    static ThemeManager &instance();

    QStringList themeNames() const;
    void apply(const QString &name);
    void restore();
    QString current() const
    {
        return m_current;
    }

  signals:
    void themeChanged(const QString &name);

  private:
    ThemeManager();
    static QString buildStyleSheet(const ThemeColors &c);

    QMap<QString, ThemeColors> m_themes;
    QString m_current {"Breeze"};
};
