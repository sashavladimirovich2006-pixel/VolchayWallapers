#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

namespace volchay {

class Logger : public QObject {
    Q_OBJECT
public:
    enum Level {
        Debug = 0,
        Info  = 1,
        Warn  = 2,
        Error = 3,
        Fatal = 4
    };
    Q_ENUM(Level)

    static Logger& instance();

    void init(const QString& appDataDir);
    void shutdown();

    void log(Level lvl, const QString& module, const QString& message);

    Q_INVOKABLE void d(const QString& module, const QString& m) { log(Debug, module, m); }
    Q_INVOKABLE void i(const QString& module, const QString& m) { log(Info,  module, m); }
    Q_INVOKABLE void w(const QString& module, const QString& m) { log(Warn,  module, m); }
    Q_INVOKABLE void e(const QString& module, const QString& m) { log(Error, module, m); }

    QString currentLogFile() const { return m_filePath; }

private:
    Logger() = default;
    ~Logger() override;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void rotateIfNeeded();
    void purgeOld(const QString& logsDir);
    static const char* levelName(Level lvl);

    QMutex     m_mutex;
    QFile      m_file;
    QTextStream m_stream;
    QString    m_filePath;
    QString    m_logsDir;
    QDate      m_currentDate;
    bool       m_initialized = false;
};

// Qt message handler hook
void installQtMessageHandler();

} // namespace volchay
