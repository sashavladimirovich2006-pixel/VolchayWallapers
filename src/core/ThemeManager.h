#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QHash>
#include <QStringList>

namespace volchay {

class Settings;

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QStringList availableThemes READ availableThemes CONSTANT)

    Q_PROPERTY(QColor background READ background NOTIFY changed)
    Q_PROPERTY(QColor surface    READ surface    NOTIFY changed)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY changed)
    Q_PROPERTY(QColor text       READ text       NOTIFY changed)
    Q_PROPERTY(QColor textMuted  READ textMuted  NOTIFY changed)
    Q_PROPERTY(QColor border     READ border     NOTIFY changed)
    Q_PROPERTY(QColor accent     READ accent     NOTIFY changed)
    Q_PROPERTY(QColor accentSoft READ accentSoft NOTIFY changed)
    Q_PROPERTY(QColor danger     READ danger     NOTIFY changed)
    Q_PROPERTY(bool   isDark     READ isDark     NOTIFY changed)

public:
    explicit ThemeManager(Settings* settings, QObject* parent = nullptr);

    QString name() const { return m_name; }
    QStringList availableThemes() const;

    QColor background() const { return m_p.background; }
    QColor surface()    const { return m_p.surface; }
    QColor surfaceAlt() const { return m_p.surfaceAlt; }
    QColor text()       const { return m_p.text; }
    QColor textMuted()  const { return m_p.textMuted; }
    QColor border()     const { return m_p.border; }
    QColor accent()     const { return m_p.accent; }
    QColor accentSoft() const { return m_p.accentSoft; }
    QColor danger()     const { return m_p.danger; }
    bool   isDark()     const { return m_p.dark; }

    Q_INVOKABLE void setTheme(const QString& name);
    Q_INVOKABLE QString displayName(const QString& key) const;

signals:
    void changed();

private:
    struct Palette {
        QColor background;
        QColor surface;
        QColor surfaceAlt;
        QColor text;
        QColor textMuted;
        QColor border;
        QColor accent;
        QColor accentSoft;
        QColor danger;
        bool   dark;
    };

    void apply(const QString& key);
    static Palette paletteFor(const QString& key);

    Settings* m_settings = nullptr;
    QString   m_name;
    Palette   m_p;
};

} // namespace volchay
