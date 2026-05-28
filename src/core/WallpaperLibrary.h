#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QFileInfo>
#include <QDateTime>

namespace volchay {

class Settings;

struct WallpaperItem {
    QString filePath;
    QString name;
    qint64  size = 0;
    QDateTime modified;
    QString  thumbnailPath; // optional, may be empty
};

class WallpaperLibrary : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1,
        NameRole,
        SizeRole,
        ModifiedRole,
        ThumbnailRole
    };

    explicit WallpaperLibrary(Settings* settings, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addFile(const QString& filePath);
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE QString filePathAt(int row) const;

    static bool isSupported(const QString& filePath);

signals:
    void countChanged();
    void errorOccurred(const QString& message);

private:
    Settings* m_settings = nullptr;
    QVector<WallpaperItem> m_items;
};

} // namespace volchay
