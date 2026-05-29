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

### 2026-05-29 — Видео не показывалось: hwdec, layered, FPS, fullscreen-pause, battery-mute
- Симптом после фикса WorkerW: окно хоста корректно прицеплено к слою обоев, но в логе сразу после `loadfile` шёл `Mpv end-file (reason=2 error=…)` и кадры не появлялись. Причина — комбинация двух факторов.
  - `hwdec=auto-safe` на окне с `WS_EX_LAYERED` (через `SetLayeredWindowAttributes`) не инициализирует аппаратный декодер на Windows 11 24H2: DXVA2/D3D11VA отказываются работать с layered surface, mpv корректно завершает воспроизведение с `end-file`.
  - Уровень логов mpv был `all=v`, поэтому полезные ошибки тонули в потоке verbose-сообщений и не уходили в наш Logger как ERROR.
- Правка `src/core/MpvObject.cpp`:
  - `hwdec=no` (раньше `auto-safe`). Программный декодер устойчиво работает на всех конфигурациях Windows; для живых обоев это разумный компромисс — нагрузка на CPU небольшая, зато гарантированно играет.
  - `msg-level=all=error`, в `onMpvWakeup` подписываемся на `mpv_request_log_messages(..., "error")` и поднимаем их в наш Logger.
  - `MPV_EVENT_END_FILE` теперь разбирается: `reason` + `error` декодируются в человекочитаемую строку и испускаются сигналом `mpvError(QString)`. В QML это ловится `onMpvError` и показывается в тосте — больше «молча не играет» не будет.
- Правка `src/core/WallpaperEngine.cpp` / `.h`: убран `WS_EX_LAYERED` + `SetLayeredWindowAttributes` при `attach()`. Layered-окна нужны были для альфа-композиции с обоями Windows, но именно они конфликтовали с DXVA. Теперь:
  - в `attach()` сохраняем оригинальные `GWL_STYLE` / `GWL_EXSTYLE` в `m_previousStyle` / `m_previousExStyle` (поля внутри `#ifdef Q_OS_WIN`);
  - в `detach()` восстанавливаем стили точь-в-точь — окно уходит обратно в обычное состояние без следов нашего вмешательства.
- Новое свойство `MpvObject::fpsLimit` (`Q_PROPERTY(int)`). Сеттер вызывает `set_property("display-fps-override", fps)` + `set_property("video-sync", "display-resample")`. По умолчанию 60. В `Main.qml` биндится к `Settings.fpsLimit`, так что ползунок в настройках теперь реально режет FPS — это снимает нагрузку с GPU на ноутбуках.
- Новый класс `PowerWatcher` (`src/core/PowerWatcher.{h,cpp}`):
  - `Q_PROPERTY(bool fullscreenActive)` — Win32 `SHQueryUserNotificationState`. Состояния `QUNS_BUSY`, `QUNS_RUNNING_D3D_FULL_SCREEN`, `QUNS_PRESENTATION_MODE` считаются «полным экраном».
  - `Q_PROPERTY(bool onBattery)` — Win32 `GetSystemPowerStatus`. `ACLineStatus == 0` → батарея, `255` (unknown) — оставляем предыдущее значение, чтобы не дёргать подписчиков на ноутбуках без датчика.
  - Опрос раз в секунду через `QTimer`, сигналы испускаются только на смене состояния (edge), чтобы не создавать шума в QML-биндингах.
  - Под не-Windows платформами класс компилируется в no-op — обоев на Linux/macOS пока нет, но код проекта остаётся переносимым.
- `src/main.cpp`: создаётся `PowerWatcher power;` рядом с остальными core-синглтонами и экспонируется в QML как контекстное свойство `Power`.
- `src/qml/Main.qml`:
  - биндинг `mute: Settings.muteOnBattery && Settings.volume === 0` был косметической ошибкой (mute, когда громкость и так нулевая — это no-op). Заменён на честный `mute: Settings.muteOnBattery && Power.onBattery`. Теперь опция «mute от батареи» делает то, что обещает.
  - `Connections { target: Power; onFullscreenActiveChanged }` — при включённом `Settings.pauseOnFullscreen` вызывает `wallpaperMpv.pause()` / `.play()` по edge-сигналу. Дополнительный `Connections` на `Settings.pauseOnFullscreenChanged` синхронизирует состояние, если опцию переключили во время уже идущей полноэкранной сессии.
  - На `Component.onCompleted` хост-окна, если `pauseOnFullscreen` уже стоит и сейчас активна полноэкранная сессия — сразу ставим на паузу, не ждём следующего опроса.
- `HomePage.qml` / `LibraryPage.qml`: убран паттерн off→on (`wallpaperEnabled = false; ... = true`) на «Применить». Сеттер `MpvObject::source` сам делает `loadfile … replace` — пересоздавать хост-окно не нужно, оно только моргает.
- `CMakeLists.txt`: добавлены `PowerWatcher.{h,cpp}` в `VOLCHAY_SOURCES`; в `target_link_libraries` для Windows добавлен `shell32` (требуется `SHQueryUserNotificationState`).
- Затронутые файлы: `src/core/MpvObject.{h,cpp}`, `src/core/WallpaperEngine.{h,cpp}`, `src/core/PowerWatcher.{h,cpp}` (новые), `src/main.cpp`, `src/qml/Main.qml`, `src/qml/pages/HomePage.qml`, `src/qml/pages/LibraryPage.qml`, `CMakeLists.txt`.

### 2026-05-29 — Live-превью карточек в библиотеке
- Реализовано улучшение из предыдущего предложения. Все карточки в библиотеке выглядели одинаково — пользователь не понимал, какое именно видео внутри, и кликал «Применить» наугад. Теперь при наведении курсора на карточку (>220 мс — фильтр пролёта) поверх статичной иконки `play.svg` запускается миниатюрный экземпляр `MpvObject` с тем же файлом. Уход курсора — мгновенно деактивирует Loader, `MpvObject` уничтожается. В любой момент жив максимум один превью-инстанс.
- `WallpaperCard.qml`:
  - добавлен `import Volchay.Mpv 1.0`.
  - `HoverHandler` на thumb-области ловит `hoveredChanged`. На входе запускается `Timer { interval: 220 }`, по таймауту — `previewLoader.active = true`. На выходе — `warmupTimer.stop()` + `previewLoader.active = false`. Эта пауза в 220 мс — компромисс: пользователь, ведущий курсор к нужной карточке, успевает увидеть превью почти сразу, а просто пролетающий курсор не создаёт mpv-инстансы каждые 50 мс.
  - `Loader { active: false; sourceComponent: ... MpvObject { fpsLimit: 24, mute: true, volume: 0, scaleMode: "fill" } }` — превью без звука, 24 FPS (для миниатюры этого с запасом, нагрузка ниже, чем у основного обойного плеера). `MpvObject` уже выставляет `loop-file=inf` и `audio=no` глобально (см. `initializeMpv`), так что эти настройки не дублируются.
  - `Behavior on opacity` (NumberAnimation 180 мс) на `Loader` и на статичной иконке `play.svg` — превью плавно проявляется, иконка плавно гасится (не остаётся тёмным пятном под полупрозрачным первым кадром).
  - Для уже **применённых** обоев (`root.selected`) превью не запускается — оно уже играет на рабочем столе, дублировать в карточке смысла нет, плюс это экономит ресурсы.
- Возможные накладные расходы: инициализация mpv-контекста занимает ~200–500 мс (software decode, без аппаратного декодера). За эти миллисекунды видна статичная иконка — это нормальная индикация «грузим». Если пользователь уйдёт раньше — `Loader` уничтожит ещё не успевший показать кадр инстанс.
- Затронутые файлы: `src/qml/components/WallpaperCard.qml`.

### 2026-05-30 — Багфикс: «no video or audio streams selected» + развёрнутая диагностика mpv
- **Симптом:** пользователь добавил видео, нажал «Применить», в тосте выскочило `mpv: no video or audio streams selected`, и обои не показались.
- **Причина (root cause):** в `MpvObject::initializeMpv` стояла опция `audio=no` — это эквивалент `--aid=no`, то есть «аудио-дорожка не выбирается ВООБЩЕ». Внутри mpv в `playloop.c` есть проверка: если после автовыбора ни видео-цепочка (`vo_chain`), ни аудио-цепочка (`ao_chain`) не поднялись — испускается `MPV_ERROR_NOTHING_TO_PLAY` с сообщением `no video or audio streams selected`. Достаточно, чтобы по любой причине не выбралась видео-дорожка (странный контейнер, специфический кодек, неожиданный порядок треков), и при `audio=no` mpv считает, что играть нечего, и валит файл с ошибкой. Это контринтуитивно: казалось, «выключим звук, оставим только видео», а получилось «если видео не сразу взял — играть нечего».
- **Правка `src/core/MpvObject.cpp`:**
  - `audio=no` → `mute=yes`. Аудио-трек теперь нормально выбирается, но проигрывается молча (mpv-property `mute`). Это смягчает попадание в «nothing to play», потому что почти у всех видеофайлов есть хотя бы один трек, который mpv может «выбрать». В QML mute-проперти всё так же управляется (`Settings.muteOnBattery && Power.onBattery`), так что обои по-прежнему молчат по умолчанию.
  - `msg-level` поднят с `all=error` до `all=warn,player=v,ao=v,vo=v,vd=v,demux=v`. Этого хватает, чтобы в лог-файле осели: выбор демуксера, выбор декодера видео, причина отказа открыть трек — всё, чего нам сейчас не хватало, чтобы понять «почему не играет».
  - `mpv_request_log_messages` теперь подписывается на уровень `v` (раньше `error`). Лог-сообщения mpv маршрутизируются в наш `Logger` с уровнем, соответствующим уровню mpv (Debug/Info/Warn/Error), и в тост поднимаются только ERROR.
- **Новые обработчики mpv-событий в `onMpvWakeup`:**
  - `MPV_EVENT_START_FILE` — пишет в лог `start-file: <path>` ровно с тем путём, который mpv получил. Полезно, чтобы убедиться: путь не URL-энкоженный, без потерь Юникода, без обрезанного префикса.
  - `MPV_EVENT_FILE_LOADED` — самое важное. mpv успешно распарсил контейнер и выбрал дорожки. Тут мы запрашиваем свойства `file-format`, `video-codec`, `audio-codec`, `video-format`, `audio-format`, `width`, `height` и пишем компактную строку: `file-loaded: container=mp4 video=h264 (yuv420p) 1920x1080 audio=aac (stereo)`. Дальше — полный `track-list` (JSON-строка от mpv) на уровне Debug. В следующий раз, когда что-то «не играет», в логе будет точно: что mpv увидел, что взял, что не взял.
- **Дружелюбные сообщения в тосте:** для трёх самых частых opaque-ошибок (`no video or audio streams selected`, `Failed to recognize file format`, `Failed to open …`) выводится понятный пользователю текст на русском, а не сырой английский от mpv. Сырое сообщение всё равно ложится в лог.
- **Логи в `WallpaperLibrary::addFile`:** на входе пишется путь; при пустом пути / не найден / неподдерживаемый формат — отдельная Warn-запись с причиной (раньше сообщение уходило только в тост, в логе следа не оставалось); при успехе — Info с размером файла. Теперь связка «пользователь нажал → файл попал в библиотеку → mpv открыл» однозначно прослеживается по лог-файлу даже без отладчика.
- **Лог в `Settings::setCurrentWallpaper`:** Info `currentWallpaper -> <path>`. До этого момента смена обоев в логе не отражалась.
- **Затронутые файлы:** `src/core/MpvObject.cpp`, `src/core/WallpaperLibrary.cpp`, `src/core/Settings.cpp`. README — этот журнал.

### 2026-05-30 — Багфикс: «Error opening/initializing the selected video_out (--vo) device»
- **Симптом:** после правки `audio=no → mute=yes` файл стал открываться (даже звук пошёл), но видеоряд не появлялся, в тосте — `mpv: Error opening/initializing the selected video_out (--vo) device.` И в логе тот же текст с уровня mpv:vo.
- **Причина:** `vo=libmpv` — это специальный video output, который рендерит в OpenGL FBO через **render API**. Он требует, чтобы `mpv_render_context_create` был вызван **до** того, как mpv попытается инициализировать vo при первом `loadfile`. У нас render-контекст создаётся лениво в `MpvRenderer::ensureContext`, а та вызывается только на первый `render()` — то есть когда Qt Quick впервые рисует сцену. Биндинг `source: Settings.currentWallpaper` в QML, наоборот, стреляет немедленно при создании `Window` хоста — раньше первого фрейма. mpv получает `loadfile`, аудио-цепочка поднимается успешно (звук пошёл), видео-цепочка просит vo подняться, у vo нет render-контекста, vo возвращает ошибку. После этого render-контекст наконец создаётся (по первому `render()`), но mpv уже отказал текущему файлу и больше не пытается.
- **Правка `src/core/MpvObject.{h,cpp}`:**
  - Поля `bool m_renderReady = false;` и `QString m_pendingSource;` в `MpvObject`.
  - `Q_INVOKABLE void onRenderReady()` — слот для уведомления из render-треда. Идемпотентен (повторные вызовы игнорируются).
  - `setSource`: если `m_renderReady == false`, путь сохраняется в `m_pendingSource` и `loadfile` НЕ отправляется в mpv. Сигнал `sourceChanged` всё равно эмитится — QML-биндинги (`mute`, `volume`, `fpsLimit`) продолжают работать.
  - В `MpvRenderer::ensureContext` после успешного `mpv_render_context_create` (и установки update-callback) — `QMetaObject::invokeMethod(m_obj, "onRenderReady", Qt::QueuedConnection)`. Очередь доставляет вызов на GUI-тред, где живёт `MpvObject`.
  - `onRenderReady`: ставит `m_renderReady = true`, и если в `m_pendingSource` что-то есть — отправляет `loadfile` сейчас. К этому моменту render-контекст уже живой, vo=libmpv инициализируется без ошибки, видео начинает играть.
- **Почему именно слот, а не таймер/задержка:** таймером пришлось бы гадать, через сколько мс контекст готов; на быстрой машине это лишняя задержка, на медленной — недостаточная. Слот срабатывает ровно в момент готовности и ровно один раз.
- **Дополнительные логи** (в самом нужном месте, где всё ломалось без объяснения): `setSource: render not ready, deferring loadfile: <path>`, `Render context ready`, `Replaying deferred loadfile: <path>`. По журналу видно: путь дошёл → отложен → контекст поднялся → файл перезапущен.
- **Затронутые файлы:** `src/core/MpvObject.h`, `src/core/MpvObject.cpp`.

### 2026-05-30 — CI: windeployqt падал на чтении sqldrivers
- **Симптом:** шаг `Stage artifacts` падает с `windeployqt failed with exit code -1073740791`. Расшифровка кода: `0xC0000409` — `STATUS_STACK_BUFFER_OVERRUN`, fast-fail от Windows /GS, когда нарушен стековый канарейка. Это краш самого windeployqt, а не наш ассерт.
- **Где именно:** добавил в шаг `--verbose 2`, и журнал показал: windeployqt доходит до перечисления плагинов Qt и крашится внутри `readPeExecutable` после успешного чтения `plugins\sqldrivers\qsqlodbc.dll` (release). Следующего файла (`qsqlodbcd.dll` debug или `qsqlpsql.dll`) уже нет в логе — процесс умирает на нём.
- **Почему windeployqt туда вообще лезет:** он по дизайну сканирует ВСЕ DLL во всех plugin-директориях Qt-инсталляции (чтобы знать, какие из них к какому модулю относятся). От того, линкуем мы `Qt6Sql` или нет, это не зависит. Нашего бинарника, который вообще не использует SQL, эта работа не касается, но windeployqt всё равно её делает — и валится посередине.
- **Правка `.github/workflows/build-windows.yml`:**
  - Перед запуском windeployqt из `$env:Qt6_DIR\plugins\` сносятся директории, которые наш бинарник заведомо не использует: `sqldrivers` (нет Qt6Sql), `networkinformation`, `printsupport`, `designer`, `qmlls`, `qmllint`, `help`, `generic`. Параллельно это уменьшает портабельный артефакт на пару мегабайт.
  - В команду windeployqt добавлен `--no-virtualkeyboard` (тоже не нужен) — на всякий случай, чтобы и эту директорию не сканировал.
  - `--qmldir` уже переведён на абсолютный путь через `Resolve-Path` (см. предыдущий запис журнала про qmlimportscanner), эта правка остаётся.
- **Почему через удаление директорий, а не через флаг:** у windeployqt 6.9 нет селективного флага вроде `--no-sql` или `--skip-plugin-types sqldrivers`. Есть только `--no-plugins` (выключает все плагины — тогда придётся вручную копировать платформенный, SVG-иконки и т.д.) или ничего. Удаление неиспользуемых директорий — наименее инвазивное и точечное решение: оставляем то, что нужно, выкидываем то, что нет.
- **Затронутые файлы:** `.github/workflows/build-windows.yml`. Кода приложения это не касается — баг чисто инструментальный.

### Предложение по улучшению
Кешировать **первый кадр** каждого видео библиотеки как PNG-миниатюру (`%LOCALAPPDATA%/Volchay/VolchayWallpapers/thumbs/<sha1(filePath)>.png`) и показывать её до запуска live-превью. Сейчас при наведении на карточку видна статичная иконка `play.svg` 200–500 мс, пока mpv инициализируется; с кешированной миниатюрой пользователь сразу увидит «о чём видео». Реализация: одноразовый проход по библиотеке через `mpv --frames=1 --vo=image-jpeg --o=...` (или через тот же `QQuickFramebufferObject` с `seek 25%` → `screenshot-raw`), сохранение в каталог thumbs, добавление роли `thumbnail` в `WallpaperLibrary` (она уже задумывалась — см. структуру в README, но не реализована). `WallpaperCard` показывает миниатюру как фон thumb, а live-превью поверх — плавный переход от статики к движению.

---

## Лицензия

MIT — см. `LICENSE`.
