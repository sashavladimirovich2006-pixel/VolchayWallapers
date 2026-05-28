#include "ThemeManager.h"
#include "Settings.h"
#include "Logger.h"

namespace volchay {

// Янтарно-оранжевый акцент — общий для всех тем
static const QColor kAccent      = QColor("#FFA000");
static const QColor kAccentSoft  = QColor("#FFB300");
static const QColor kDanger      = QColor("#E53935");

ThemeManager::ThemeManager(Settings* settings, QObject* parent)
    : QObject(parent), m_settings(settings)
{
    apply(settings ? settings->themeName() : QStringLiteral("dark"));
    if (settings) {
        connect(settings, &Settings::themeNameChanged, this, [this] {
            apply(m_settings->themeName());
        });
    }
}

QStringList ThemeManager::availableThemes() const {
    return { "snow", "blackout", "dark", "pink", "blackpink" };
}

QString ThemeManager::displayName(const QString& key) const {
    if (key == "snow")      return QStringLiteral("Белоснежная");
    if (key == "blackout")  return QStringLiteral("Blackout");
    if (key == "dark")      return QStringLiteral("Тёмная");
    if (key == "pink")      return QStringLiteral("Розовая");
    if (key == "blackpink") return QStringLiteral("Чёрно-розовая");
    return key;
}

ThemeManager::Palette ThemeManager::paletteFor(const QString& key) {
    Palette p;
    p.accent     = kAccent;
    p.accentSoft = kAccentSoft;
    p.danger     = kDanger;

    if (key == "snow") {
        p.background = QColor("#FFFFFF");
        p.surface    = QColor("#F6F6F8");
        p.surfaceAlt = QColor("#ECECF1");
        p.text       = QColor("#1A1A1A");
        p.textMuted  = QColor("#5A5A66");
        p.border     = QColor("#E1E1E8");
        p.dark = false;
    } else if (key == "blackout") {
        p.background = QColor("#000000");
        p.surface    = QColor("#0A0A0A");
        p.surfaceAlt = QColor("#141414");
        p.text       = QColor("#F2F2F2");
        p.textMuted  = QColor("#9A9A9A");
        p.border     = QColor("#1F1F1F");
        p.dark = true;
    } else if (key == "pink") {
        p.background = QColor("#FFF1F4");
        p.surface    = QColor("#FFE3EA");
        p.surfaceAlt = QColor("#FFD3DE");
        p.text       = QColor("#3A0A1A");
        p.textMuted  = QColor("#7A2A40");
        p.border     = QColor("#F2BFCC");
        p.dark = false;
    } else if (key == "blackpink") {
        p.background = QColor("#150009");
        p.surface    = QColor("#22000F");
        p.surfaceAlt = QColor("#2E0015");
        p.text       = QColor("#FFD7E1");
        p.textMuted  = QColor("#C58AA0");
        p.border     = QColor("#3A0018");
        p.dark = true;
    } else {
        // dark (default)
        p.background = QColor("#1B1B1F");
        p.surface    = QColor("#26262B");
        p.surfaceAlt = QColor("#2E2E34");
        p.text       = QColor("#ECECEC");
        p.textMuted  = QColor("#A0A0AA");
        p.border     = QColor("#3A3A42");
        p.dark = true;
    }
    return p;
}

void ThemeManager::apply(const QString& key) {
    m_name = key;
    m_p = paletteFor(key);
    Logger::instance().log(Logger::Info, "Theme", "Applied: " + key);
    emit changed();
}

void ThemeManager::setTheme(const QString& name) {
    if (name == m_name) return;
    if (m_settings) m_settings->setThemeName(name);
    else apply(name);
}

} // namespace volchay
