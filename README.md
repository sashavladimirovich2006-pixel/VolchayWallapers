# Volchay Wallpapers

Полноценное настольное приложение для **живых обоев из видео** на Windows.
Написано на **C++17 + Qt6 + QML**, использует **libmpv** для воспроизведения и встраивается в рабочий стол через **WorkerW** (окно живых обоев живёт между обоями Windows и иконками рабочего стола, как полагается).

Проект задуман как серьёзный, длительно поддерживаемый продукт. Любые изменения должны соответствовать правилам в разделе [«Правила разработки»](#правила-разработки) ниже.

---

## Возможности

- Загрузка и применение видео (`.mp4`, `.webm`, `.mkv`, `.mov`, `.avi`) как живых обоев на рабочий стол.
- Локальная библиотека: добавление, удаление, превью, поиск.
- Управление воспроизведением: громкость, FPS-лимит, режим масштабирования (Fill / Fit / Stretch / Center), пауза при полноэкранных приложениях, пауза от батареи.
- Многомониторная поддержка (выбор целевого монитора).
- 5 встроенных тем: **Белоснежная**, **Blackout (полностью чёрная)**, **Тёмная**, **Розовая**, **Чёрно-розовая** — с единым **янтарно-оранжевым** акцентом (`#FFA000` / `#FFB300`).
- Современный QML-интерфейс (Qt Quick Controls 2 + Qt Quick Effects), плавные анимации, SVG-иконки.
- Подробный лог-файл с уровнями (DEBUG / INFO / WARN / ERROR / FATAL) — облегчает диагностику.
- Автозапуск с Windows (опционально), сворачивание в системный трей.

---

## Правила разработки

> Эти правила обязательны для любого, кто вносит изменения в проект (включая ИИ-ассистентов).

1. **Документировать каждый шаг разработки в `README.md` — до мельчайших деталей.**
   Любое изменение архитектуры, добавление файла, новая зависимость, новая страница QML, изменение темы — всё фиксируется в разделе [«Журнал разработки»](#журнал-разработки) с датой, мотивацией, что именно сделано, какие файлы затронуты и почему так, а не иначе. Никаких «по мелочи не записывал».

2. **Не упоминать в `README.md` ссылки на репозитории** (ни этот, ни сторонние) и **не упоминать другие программы похожего назначения**. README остаётся самодостаточным и нейтральным.

3. **После завершения любой задачи всегда предлагать минимум одно улучшение** по теме проекта — либо собственную идею, либо адаптацию практики из аналогичной по духу программы (без её упоминания в README).

4. **Лог-файл обязателен** и должен покрывать запуск, инициализацию подсистем, ошибки mpv, операции с WorkerW, изменения настроек и сбои.

5. **Использовать самые передовые и популярные библиотеки** (Qt6, libmpv, CMake ≥ 3.21) и **SVG-иконки** в интерфейсе — никаких растровых иконок там, где можно SVG.

6. **Дизайн — современный, уютный, удобный**, на Qt6 QML, с максимальным использованием его возможностей (Effects, Shapes, плавные переходы). Янтарно-оранжевый акцент во всех темах.

---

## Технологический стек

| Слой | Технология |
|---|---|
| Язык | C++17 |
| GUI | Qt 6.7+ (Quick, QuickControls2, Svg, Multimedia, Widgets для трея) |
| Видео | libmpv (рендер через `QQuickFramebufferObject` + mpv render API, OpenGL) |
| Сборка | CMake ≥ 3.21, MSVC 2022 |
| Деплой | windeployqt + Inno Setup (installer) |
| CI | GitHub Actions (Windows, MSVC) |
| Логи | Кастомный `Logger` + `qInstallMessageHandler` |
| Настройки | `QSettings` (INI в `%APPDATA%/Volchay/VolchayWallpapers`) |

---

## Структура проекта

```
VolchayWallapers/
├── CMakeLists.txt              # Корневая сборка
├── README.md                   # Этот файл — единый источник правды по проекту
├── LICENSE
├── .gitignore
├── .github/workflows/
│   └── build-windows.yml       # CI: сборка .exe + installer
├── installer/
│   └── volchay.iss             # Скрипт Inno Setup
├── src/
│   ├── main.cpp                # Точка входа, инициализация Logger, QGuiApplication, регистрация типов
│   ├── core/
│   │   ├── Logger.h/.cpp                 # Логгер с уровнями и файлом
│   │   ├── Settings.h/.cpp               # Обёртка над QSettings, экспонируется в QML
│   │   ├── ThemeManager.h/.cpp           # 5 тем + янтарный акцент, доступ из QML
│   │   ├── WallpaperLibrary.h/.cpp       # Модель библиотеки видео, миниатюры
│   │   ├── MpvObject.h/.cpp              # QQuickFramebufferObject поверх libmpv
│   │   ├── MpvRenderer.h/.cpp            # mpv_render_context, OpenGL FBO
│   │   ├── WallpaperEngine.h/.cpp        # WorkerW: создание окна, применение обоев
│   │   └── SystemTray.h/.cpp             # QSystemTrayIcon
│   ├── qml/
│   │   ├── Main.qml                      # Корневое окно, навигация
│   │   ├── pages/
│   │   │   ├── HomePage.qml              # Главный экран: загрузить и применить
│   │   │   ├── LibraryPage.qml           # Библиотека обоев
│   │   │   ├── SettingsPage.qml          # Настройки, темы, монитор
│   │   │   └── AboutPage.qml             # О программе, версия
│   │   ├── components/
│   │   │   ├── SideBar.qml               # Боковая навигация со SVG-иконками
│   │   │   ├── WallpaperCard.qml         # Карточка обоев в библиотеке
│   │   │   ├── AccentButton.qml          # Янтарная акцентная кнопка
│   │   │   ├── IconButton.qml            # Кнопка с SVG-иконкой
│   │   │   ├── ThemePicker.qml           # Выбор из 5 тем
│   │   │   └── PreviewArea.qml           # Превью видео перед применением
│   │   ├── dialogs/
│   │   │   ├── FilePickerDialog.qml      # Выбор видео
│   │   │   └── ConfirmDialog.qml         # Подтверждение действий
│   │   └── Theme/
│   │       ├── qmldir
│   │       ├── Palette.qml               # Текущая палитра (read-only)
│   │       └── Themes.qml                # Каталог 5 тем
│   └── resources/
│       ├── qml.qrc                       # Подключение QML
│       ├── icons.qrc                     # SVG-иконки
│       └── icons/                        # home.svg, library.svg, settings.svg, about.svg, play.svg, pause.svg, apply.svg, delete.svg, folder.svg, theme.svg, monitor.svg
├── assets/                     # Логотип, app.ico
└── tests/                      # Юнит-тесты (Qt Test)
```

---

## Сборка локально

### Требования
- Windows 10/11 x64
- Visual Studio 2022 (Desktop development with C++)
- Qt 6.7+ для MSVC (модули: Quick, QuickControls2, Multimedia, Svg, Widgets)
- libmpv (dev): заголовки `mpv/client.h`, `mpv/render_gl.h` и `mpv.lib` + `libmpv-2.dll`
- CMake ≥ 3.21, Ninja (опционально)

### Шаги
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/msvc2022_64" ^
      -DMPV_ROOT="C:/libs/libmpv"
cmake --build build --config Release
```

После сборки запустите `windeployqt --qmldir src/qml build/Release/VolchayWallpapers.exe` для копирования зависимостей Qt.

---

## Применение обоев — как это работает (Windows)

1. Найти окно `Progman` (рабочий стол).
2. Отправить `0x052C` через `SendMessageTimeout` — это заставит Explorer создать дочернее окно `WorkerW` за иконками.
3. Перечислить топ-уровневые окна `EnumWindows` и найти `WorkerW`, у которого есть дочернее `SHELLDLL_DefView`. Нужное `WorkerW` — это **следующее** окно того же уровня (которое НЕ содержит `SHELLDLL_DefView`).
4. Сделать `SetParent(ourQuickWindow, foundWorkerW)`, растянуть на размер виртуального рабочего стола (или выбранного монитора).
5. Внутри окна крутится `MpvObject` (наследник `QQuickFramebufferObject`), который через mpv render API рендерит кадры видео в OpenGL FBO Qt Quick.
6. При выходе или смене обоев — `SetParent` обратно в `nullptr`, окно скрывается, mpv останавливается.

---

## Логирование

- Файл: `%LOCALAPPDATA%/Volchay/VolchayWallpapers/logs/volchay-YYYY-MM-DD.log`.
- Ротация: новый файл на каждый день; старше 14 дней удаляются.
- Уровни: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`.
- Каждая запись: `[ISO-timestamp] [LEVEL] [thread] [module] message`.
- Дублирование в `qDebug/qWarning/qCritical` через `qInstallMessageHandler`.
- mpv events (`log-message`) маршрутизируются в наш логгер с уровнем mpv.

---

## Темы

Все темы используют единый янтарно-оранжевый акцент `#FFA000` (основной) и `#FFB300` (светлый).

| Тема | Фон | Поверхность | Текст | Заметка |
|---|---|---|---|---|
| Белоснежная | `#FFFFFF` | `#F6F6F8` | `#1A1A1A` | Максимально светлая |
| Blackout | `#000000` | `#0A0A0A` | `#F2F2F2` | OLED-friendly |
| Тёмная | `#1B1B1F` | `#26262B` | `#ECECEC` | Стандартный тёмный режим |
| Розовая | `#FFF1F4` | `#FFE3EA` | `#3A0A1A` | Светло-розовая |
| Чёрно-розовая | `#150009` | `#22000F` | `#FFD7E1` | Тёмная с розовыми поверхностями |

Тема переключается мгновенно через `ThemeManager`, доступный в QML как `Theme`.

---

## Журнал разработки

> Согласно правилу №1 — здесь фиксируется каждый шаг.

### 2026-05-28 — Инициализация проекта
- Создан корневой каталог, инициализирован git (branch `main`).
- Утверждён стек: C++17 + Qt6 + QML + libmpv, сборка CMake + MSVC 2022, CI GitHub Actions.
- Утверждена структура каталогов (см. выше).
- Утверждены 5 тем и янтарно-оранжевый акцент.
- Создан этот `README.md` с правилами разработки.

### 2026-05-28 — Каркас, ядро, QML, CI
- Реализован Logger (per-day ротация, 14-дневное удержание, `qInstallMessageHandler`).
- Реализован Settings поверх `QSettings` (`INI`, `%APPDATA%/Volchay/VolchayWallpapers/VolchayWallpapers.ini`).
- Реализован ThemeManager с пятью темами и янтарно-оранжевым акцентом.
- Реализована WallpaperLibrary как `QAbstractListModel` (роли filePath/name/size/modified/thumbnail).
- Реализован MpvObject на `QQuickFramebufferObject` + mpv render API; при отсутствии libmpv во время сборки превращается в безопасную заглушку (флаг `VOLCHAY_HAVE_MPV`).
- Реализован WallpaperEngine: Progman + 0x052C + EnumWindows → WorkerW, `SetParent`, многомониторная геометрия.
- Реализован SystemTray.
- QML: `Main.qml` с боковой панелью, четырьмя страницами, плавными анимациями, glow-эффектом, тостами; SVG-only иконки (`home/library/settings/about/play/pause/apply/delete/folder/theme/monitor/logo`).
- Добавлен `.github/workflows/build-windows.yml`: MSVC + aqtinstall + libmpv SDK + windeployqt + Inno Setup installer + публикация артефактов.
- Добавлен `installer/volchay.iss` (Inno Setup): задачи «ярлык на рабочем столе» и «автозапуск».

### 2026-05-29 — CI: итерации по установке Qt
- Несколько прогонов падают на шаге `Install Qt`. Перебрали:
  Qt 6.7.2 + msvc2019 → нет архивов; Qt 6.8.0 + msvc2022 на windows-latest (новый vs2026) — `aqt install-qt` завершается ошибкой; pin `windows-2022` + Qt 6.9.1 — тоже падает.
- Аннотации показывают только финальный `aqt install-qt failed`. Полный текст ошибки виден на странице `Actions` репозитория (раздел Install Qt). Следующий шаг — открыть Actions в браузере, скопировать конкретное сообщение (типично: 404 на онлайн-репозитории Qt либо сеть/прокси на runner), затем подобрать рабочую комбинацию: либо более старая стабильная версия (6.6.3), либо `windows-2022` + явный mirror через `--base https://qt-mirror.dannhauer.de/` для aqt.

### 2026-05-29 — Включение/выключение обоев, выбор монитора, реальный автозапуск
- В `Settings` добавлено состояние `wallpaperEnabled` (`state/wallpaperEnabled`) с сигналом `wallpaperEnabledChanged`. Это единственный «выключатель» обоев: всё остальное (Loader в `Main.qml`, кнопки на главной и в библиотеке, пункт трея) подписывается на него. INI хранит значение между запусками, поэтому при автозапуске обои сразу применяются, если были включены.
- `Settings::setAutoStart` теперь действительно прописывает/удаляет ярлык в `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run` под именем `VolchayWallpapers` (через `QSettings::NativeFormat` на Windows). Путь к exe берётся из `QCoreApplication::applicationFilePath()` и нормализуется `QDir::toNativeSeparators`, в кавычках — корректно работает с пробелами в `Program Files`.
- В `WallpaperEngine` появилось Q_PROPERTY `monitors` (`QStringList`) — человекочитаемый список вида `"1. 1920x1080 — \\.\DISPLAY1"` с пунктом «Все мониторы (виртуальный рабочий стол)» в позиции 0. Список переиспускается по `screenAdded`/`screenRemoved` от `QGuiApplication`, поэтому ComboBox в настройках обновляется при подключении/отключении мониторов без перезапуска.
- `Main.qml`: Loader `wallpaperHostLoader` стал декларативным — `active: Settings.wallpaperEnabled && Settings.currentWallpaper.length > 0`. Внутри хост-окна теперь живой `MpvObject` (импорт `Volchay.Mpv 1.0` поднят в `Main.qml`), а не бывшая пустая `previewComponent`. На `Component.onCompleted` окно прикрепляется к WorkerW; если attach не удался — `Settings.wallpaperEnabled = false`, чтобы Loader сам разобрал окно и состояние осталось согласованным. На `Component.onDestruction` вызывается `Engine.detach()`.
- `HomePage` и `LibraryPage` при «Применить» используют паттерн off→on (`wallpaperEnabled = false; wallpaperEnabled = true`), чтобы форсировать пересоздание hostWindow с новым файлом. Кнопка «Остановить» теперь только сбрасывает `wallpaperEnabled` — `Engine.detach()` отрабатывает в `Component.onDestruction`.
- `SettingsPage`: добавлен ComboBox «Монитор» с моделью `Engine.monitors`. Сопоставление: index 0 ↔ `targetMonitor = -1` (все мониторы), index N ↔ `targetMonitor = N-1`. Изменение монитора уже было прокинуто в `WallpaperEngine::resyncGeometry` через `Settings::targetMonitorChanged`.
- `SystemTray` получил третий пункт «Включить / выключить обои» и сигнал `toggleWallpaperRequested`. Обработчик в `main.cpp` инвертирует `wallpaperEnabled`, но если текущий файл пуст — пишет в лог Warn и ничего не делает (нечего показывать).
- Затронутые файлы: `src/core/Settings.{h,cpp}`, `src/core/WallpaperEngine.{h,cpp}`, `src/main.cpp`, `src/qml/Main.qml`, `src/qml/pages/HomePage.qml`, `src/qml/pages/LibraryPage.qml`, `src/qml/pages/SettingsPage.qml`. `SystemTray.{h,cpp}` уже содержали сигнал — изменений не потребовалось.

### 2026-05-29 — CI: убран `-m qtsvg` из aqt install-qt
- Прогон `Build (Windows, MSVC, Qt6)` падал на шаге `Install Qt via aqtinstall`:
  `ERROR : The packages ['qtsvg'] were not found while parsing XML of package information!`
  — и на основном зеркале, и на fallback `download.qt.io`.
- Причина: `qtsvg` в Qt 6 — это часть базовой установки (модуль внутри `qtbase` pos-инсталляции), а не отдельный аддон. Его нельзя запрашивать через `-m qtsvg`, иначе aqt не находит такую запись в `Updates.xml` и валит шаг целиком, при этом базовый Qt тоже не ставится.
- Правка: из обоих вызовов `python -m aqt install-qt … -m qtsvg …` убран флаг `-m qtsvg`. Сам модуль `Qt6::Svg` (нужный по `find_package(Qt6 COMPONENTS … Svg …)`) поставится с базовым Qt автоматически.
- Затронутые файлы: `.github/workflows/build-windows.yml`.

### 2026-05-29 — CI: оставлена только портативная сборка
- Скачанная installer-версия не запускалась у пользователя; до выяснения причины убираем installer-ветку из CI, чтобы не плодить артефакт «который не работает».
- Из `build-windows.yml` удалены шаги `Install Inno Setup`, `Build installer`, `Upload installer`. В `Create GitHub Release` остаётся только `stage/**`.
- Затронутые файлы: `.github/workflows/build-windows.yml`. Скрипт `installer/volchay.iss` оставлен в репозитории — пригодится, когда вернём installer-сценарий.

### 2026-05-29 — CI: ужесточение Stage artifacts (диагностика «не запускается»)
- Чтобы поймать, чего не хватает в портативной сборке, шаг `Stage artifacts` теперь:
  - падает, если `libmpv-2.dll` / `mpv-2.dll` не нашлись под `MPV_ROOT` (раньше молча пропускал — отсюда и «программа не запускается»);
  - падает, если `windeployqt` вернул ненулевой код;
  - убран флаг `--no-opengl-sw`: теперь Qt кладёт `opengl32sw.dll` для машин без подходящего GPU-драйвера. Стоимость — несколько мегабайт; цена за это — приложение запускается на тестовых ВМ и старых GPU.
  - в конец шага добавлен дамп содержимого `stage/` с размерами файлов — по этому листингу видно, что именно попало в портабельную папку (Qt6Core.dll, Qt6Quick.dll, libmpv-2.dll, plugins/platforms/qwindows.dll, qml/QtQuick/Effects/…).
- Затронутые файлы: `.github/workflows/build-windows.yml`.

### 2026-05-29 — Багфикс: дубль присваивания свойств в WallpaperCard.qml
- Программа собиралась, но падала на старте до показа окна. В логе (`%LOCALAPPDATA%\Volchay\VolchayWallpapers\logs\volchay-2026-05-29.log`) повторяющаяся серия:
  - `qrc:/src/qml/components/WallpaperCard.qml:21:5: Property value set multiple times`
  - `qrc:/src/qml/components/WallpaperCard.qml:22:5: Property value set multiple times`
  - `Type WallpaperCard unavailable` → `Type LibraryPage unavailable` → `FATAL Failed to load Main.qml`.
- Причина: в `WallpaperCard.qml` `border.color` и `border.width` задавались дважды — сначала статически (`Theme.border` / `1`), потом сразу же условно через `selected`. Qt 6 запрещает множественное присваивание одного и того же свойства в декларативном блоке.
- Правка: убраны статические инициализации `border.color`/`border.width` сверху; оставлено только условное (`selected ? Theme.accent : Theme.border` / `selected ? 2 : 1`) с `Behavior on border.color`. Поведение для невыделенной карточки не меняется.
- Затронутые файлы: `src/qml/components/WallpaperCard.qml`.
- Кстати, лог был не «отсутствует» — он лежит в `%LOCALAPPDATA%\Volchay\VolchayWallpapers\logs\`, а не рядом с exe. Это `QStandardPaths::AppLocalDataLocation` (см. `main.cpp:42`) — стандартное место для приложений Windows.

### 2026-05-29 — Чёрный экран на рабочем столе: WorkerW + libmpv stub
- Симптом: после «Применить» весь рабочий стол становится чёрным, иконки исчезают, видео не показывается.
- Лог раскрыл сразу две причины:
  - `[Mpv] Built without libmpv (stub mode)` — сборка в CI собралась без libmpv, плеер в режиме заглушки.
  - `[Engine] WorkerW not found, falling back to Progman` — наше окно прицепилось не к слою обоев (`WorkerW` без `SHELLDLL_DefView`), а к `Progman`. Progman перерисовывается **поверх** иконок и обоев. Это и есть «чёрный экран».
- Правка алгоритма поиска WorkerW (`src/core/WallpaperEngine.cpp`):
  - перебираем все top-level окна класса `WorkerW`, выбираем то, у которого **нет** дочернего `SHELLDLL_DefView` (это слой обоев). Раньше код искал WorkerW «как соседа DefView» через `FindWindowEx(nullptr, top, L"WorkerW", ...)` — на Windows 11 24H2 такой порядок не работает.
  - убран Progman-fallback. Лучше отказать (тостом «Не удалось найти WorkerW»), чем поверх иконок нарисовать чёрный экран. `Settings.wallpaperEnabled` сбрасывается на `false` (логика уже была в `Main.qml` — `if (!Engine.attach(...)) Settings.wallpaperEnabled = false`).
  - в лог пишется диагностика: сколько `WorkerW` нашли всего, сколько с `DefView`, какой выбрали. Это поможет, если на других сборках Windows алгоритм снова поведёт себя иначе.
- Правка CI (`.github/workflows/build-windows.yml`): mpv-dev SDK от shinchiro содержит `libmpv-2.dll` и `mpv.def`, но **не** `.lib` для MSVC. Поэтому `find_library` молча уходил в `else`, и `VOLCHAY_HAVE_MPV` становился `OFF`. После распаковки SDK теперь генерируется `mpv.lib` командой `lib.exe /def:mpv.def /out:lib/mpv.lib /machine:X64`. Шаг падает, если `lib.exe` отдал ненулевой код или `mpv.lib` не появился (так что в портабль не попадёт нерабочий бинарь).
- Правка CMake (`CMakeLists.txt`): если `MPV_ROOT` задан, но libmpv не нашли — теперь `FATAL_ERROR` вместо `WARNING`. Без `MPV_ROOT` поведение мягкое (для UI-разработки без mpv SDK).
- Затронутые файлы: `src/core/WallpaperEngine.cpp`, `.github/workflows/build-windows.yml`, `CMakeLists.txt`.

### 2026-05-29 — CI: генерация mpv.lib из dumpbin (нет .def в архиве)
- Прошлый CI-шаг ожидал `.def` в распакованном mpv-dev и падал: `mpv-dev: .def file not found under D:\a\...\mpv-dev`. shinchiro в свежих архивах кладёт `libmpv-2.dll` + MinGW-ный `libmpv.dll.a`, но **не** кладёт `.def` и **не** кладёт MSVC-совместимый `.lib`. Использовать `.dll.a` с MSVC нельзя — формат другой.
- Правка: шаг сначала ищет любой `*.def` (для совместимости с возможными будущими билдами), и если не нашёл — генерирует `.def` сам через `dumpbin /exports libmpv-2.dll`, парсит таблицу экспортов и пишет `LIBRARY libmpv-2 / EXPORTS …`. Затем как и раньше — `lib.exe /def:mpv.def /out:lib/mpv.lib /machine:X64`.
- Шаг падает, если: нет `libmpv-2.dll` под `MPV_ROOT`, `dumpbin` отдал ненулевой код, в выходе `dumpbin` нет таблицы экспортов, `lib.exe` упал, `mpv.lib` не появился. То есть в портабль снова не попадёт stub-сборка.
- Затронутые файлы: `.github/workflows/build-windows.yml`.

### Предложение по улучшению
Реализовать паузу обоев при полноэкранных приложениях (`Settings.pauseOnFullscreen` уже есть в UI, но не подключена к движку). На Windows достаточно периодически опрашивать `SHQueryUserNotificationState` из `shellapi.h` (или ловить `QUNS_RUNNING_D3D_FULL_SCREEN` / `QUNS_PRESENTATION_MODE`) с интервалом 1–2 с. Когда состояние «полный экран» — звать `MpvObject::pause()`, иначе `play()`. Это снимет нагрузку на GPU во время игр и видео и закроет реально востребованный сценарий, под который в настройках уже стоит переключатель.

---

## Лицензия

MIT — см. `LICENSE`.
