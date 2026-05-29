#include "WallpaperLibrary.h"
#include "Settings.h"
#include "Logger.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>

namespace volchay {

static const QStringList kExtensions = {
    "mp4", "webm", "mkv", "mov", "avi", "m4v", "wmv"
};

WallpaperLibrary::WallpaperLibrary(Settings* settings, QObject* parent)
    : QAbstractListModel(parent), m_settings(settings)
{
    if (m_settings) {
        connect(m_settings, &Settings::libraryChanged,
                this, &WallpaperLibrary::refresh);
    }
    refresh();
}

bool WallpaperLibrary::isSupported(const QString& filePath) {
    const QString ext = QFileInfo(filePath).suffix().toLower();
    return kExtensions.contains(ext);
}

int WallpaperLibrary::rowCount(const QModelIndex&) const {
    return m_items.size();
}

QHash<int, QByteArray> WallpaperLibrary::roleNames() const {
    return {
        { FilePathRole,  "filePath" },
        { NameRole,      "name" },
        { SizeRole,      "size" },
        { ModifiedRole,  "modified" },
        { ThumbnailRole, "thumbnail" }
    };
}

QVariant WallpaperLibrary::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};
    const auto& it = m_items.at(index.row());
    switch (role) {
        case FilePathRole:  return it.filePath;
        case NameRole:      return it.name;
        case SizeRole:      return it.size;
        case ModifiedRole:  return it.modified;
        case ThumbnailRole: return it.thumbnailPath;
    }
    return {};
}

void WallpaperLibrary::refresh() {
    beginResetModel();
    m_items.clear();
    QSet<QString> seen;
    if (m_settings) {
        const auto paths = m_settings->libraryPaths();
        for (const auto& p : paths) {
            if (seen.contains(p)) continue;
            seen.insert(p);
            QFileInfo fi(p);
            if (!fi.exists() || !fi.isFile()) continue;
            if (!isSupported(p)) continue;
            WallpaperItem it;
            it.filePath = p;
            it.name     = fi.completeBaseName();
            it.size     = fi.size();
            it.modified = fi.lastModified();
            m_items.push_back(it);
        }
    }
    endResetModel();
    emit countChanged();
    Logger::instance().log(Logger::Info, "Library",
        QStringLiteral("refresh -> %1 items").arg(m_items.size()));
}

bool WallpaperLibrary::addFile(const QString& filePath) {
    Logger::instance().log(Logger::Info, "Library",
        QStringLiteral("addFile: %1").arg(filePath));
    if (filePath.isEmpty()) {
        Logger::instance().log(Logger::Warn, "Library", "addFile: empty path");
        return false;
    }
    if (!QFile::exists(filePath)) {
        const QString msg = QStringLiteral("Файл не найден: ") + filePath;
        Logger::instance().log(Logger::Warn, "Library", msg);
        emit errorOccurred(msg);
        return false;
    }
    if (!isSupported(filePath)) {
        const QString msg = QStringLiteral("Неподдерживаемый формат: ") + filePath;
        Logger::instance().log(Logger::Warn, "Library", msg);
        emit errorOccurred(msg);
        return false;
    }
    const qint64 size = QFileInfo(filePath).size();
    Logger::instance().log(Logger::Info, "Library",
        QStringLiteral("addFile: accepted, size=%1 bytes").arg(size));
    if (m_settings) {
        m_settings->addLibraryPath(filePath);
    }
    return true;
}

void WallpaperLibrary::removeAt(int row) {
    if (row < 0 || row >= m_items.size()) return;
    const QString p = m_items.at(row).filePath;
    if (m_settings) m_settings->removeLibraryPath(p);
}

QString WallpaperLibrary::filePathAt(int row) const {
    if (row < 0 || row >= m_items.size()) return {};
    return m_items.at(row).filePath;
}

} // namespace volchay
