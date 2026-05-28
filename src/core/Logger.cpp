#include "Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QDebug>
#include <QStandardPaths>
#include <iostream>

namespace volchay {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::~Logger() {
    shutdown();
}

const char* Logger::levelName(Level lvl) {
    switch (lvl) {
        case Debug: return "DEBUG";
        case Info:  return "INFO ";
        case Warn:  return "WARN ";
        case Error: return "ERROR";
        case Fatal: return "FATAL";
    }
    return "?????";
}

void Logger::init(const QString& appDataDir) {
    QMutexLocker lock(&m_mutex);
    if (m_initialized) return;

    m_logsDir = appDataDir + QStringLiteral("/logs");
    QDir().mkpath(m_logsDir);

    m_currentDate = QDate::currentDate();
    m_filePath = QString("%1/volchay-%2.log")
                     .arg(m_logsDir, m_currentDate.toString(Qt::ISODate));

    m_file.setFileName(m_filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "Logger: failed to open log file: "
                  << m_filePath.toStdString() << std::endl;
        return;
    }
    m_stream.setDevice(&m_file);
    m_stream.setEncoding(QStringConverter::Utf8);
    m_initialized = true;

    purgeOld(m_logsDir);

    // Header
    m_stream << "----- session started "
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " pid=" << QCoreApplication::applicationPid()
             << " -----\n";
    m_stream.flush();
}

void Logger::shutdown() {
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return;
    m_stream << "----- session ended "
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " -----\n";
    m_stream.flush();
    m_file.close();
    m_initialized = false;
}

void Logger::rotateIfNeeded() {
    const QDate today = QDate::currentDate();
    if (today == m_currentDate) return;

    m_stream.flush();
    m_file.close();

    m_currentDate = today;
    m_filePath = QString("%1/volchay-%2.log")
                     .arg(m_logsDir, m_currentDate.toString(Qt::ISODate));
    m_file.setFileName(m_filePath);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        m_stream.setEncoding(QStringConverter::Utf8);
    }
    purgeOld(m_logsDir);
}

void Logger::purgeOld(const QString& logsDir) {
    QDir d(logsDir);
    const auto entries = d.entryInfoList(QStringList{"volchay-*.log"},
                                         QDir::Files, QDir::Time);
    const QDateTime cutoff = QDateTime::currentDateTime().addDays(-14);
    for (const auto& fi : entries) {
        if (fi.lastModified() < cutoff) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

void Logger::log(Level lvl, const QString& module, const QString& message) {
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) {
        // Fallback to stderr so we don't lose early messages
        std::cerr << "[" << levelName(lvl) << "] "
                  << "[" << module.toStdString() << "] "
                  << message.toStdString() << std::endl;
        return;
    }
    rotateIfNeeded();

    const QString ts = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    const auto tid = reinterpret_cast<quintptr>(QThread::currentThreadId());

    m_stream << '[' << ts << "] ["
             << levelName(lvl) << "] [t"
             << tid << "] ["
             << module << "] "
             << message << '\n';
    m_stream.flush();

    // Console mirror for dev
    std::cerr << "[" << levelName(lvl) << "] ["
              << module.toStdString() << "] "
              << message.toStdString() << std::endl;
}

// --- Qt handler ---
static void qtMessageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    Logger::Level lvl = Logger::Info;
    switch (type) {
        case QtDebugMsg:    lvl = Logger::Debug; break;
        case QtInfoMsg:     lvl = Logger::Info;  break;
        case QtWarningMsg:  lvl = Logger::Warn;  break;
        case QtCriticalMsg: lvl = Logger::Error; break;
        case QtFatalMsg:    lvl = Logger::Fatal; break;
    }
    const QString module = ctx.category && *ctx.category ? QString::fromLatin1(ctx.category)
                                                         : QStringLiteral("Qt");
    Logger::instance().log(lvl, module, msg);
}

void installQtMessageHandler() {
    qInstallMessageHandler(qtMessageHandler);
}

} // namespace volchay
