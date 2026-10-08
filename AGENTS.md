# AGENTS.md — Registro Modifiche e Note di Sviluppo

Questo file contiene l'analisi approfondita della codebase di `rofi-wayland-hud`, la traccia di tutte le problematiche riscontrate e il registro dettagliato di ogni modifica apportata.

---

## 🔍 Analisi Approfondita della Codebase e Problemi Riscontrati

### 1. Bug Critico nel Formattatore Rofi (`RofiFormatter`)
- **Problema:** L'implementazione utilizzava stringhe letterali formattate come `std::cout << "\0prompt\x1fHUD [" ...` e `std::cout << "\0message\x1f" ...`.
- **Causa tecnica:** In C++, le stringhe C letterali che iniziano con il byte nullo `\0` hanno lunghezza nulla per i puntatori a caratteri (`strlen == 0`). Quando passate all'operatore di stream `std::operator<<(std::ostream&, const char*)`, lo stream si ferma immediatamente al primo carattere terminatore `\0`.
- **Effetto:** Rofi non riceveva mai i comandi di controllo del protocollo script (`\0prompt`, `\0message`). Il prompt risultava vuoto e i messaggi di errore venivano stampati a video come normale testo non formattato anziché come banner message di Rofi.
- **Soluzione:** Scrivere esplicitamente il byte nullo (`std::cout << '\0'` o `std::cout.put('\0')`) prima del nome del comando di controllo. Risolto inoltre il warning di compilazione sulla sequenza esadecimale non valida (`\x1f` unito alla lettera successiva) introducendo una costante tipizzata `constexpr char DELIM = '\x1f'`.

### 2. Mancata Rilevazione dei Menu D-Bus per Applicazioni Qt/KDE (`KWinTracker` & D-Bus Discovery)
- **Problema:** L'elenco `candidate_paths` in `kwin_tracker.cpp` conteneva unicamente:
  `{"/MenuBar/1", "/com/canonical/dbusmenu", "/org/ayatana/NotificationItem/MenuBar", "/org/gtk/menus"}`.
- **Causa tecnica:** Le applicazioni Qt/KDE (come ad esempio Dolphin) creano le barre dei menu dinamicamente con indici incrementali `/MenuBar/2`, `/MenuBar/3`, ecc. Dolphin sulla sessione corrente risponde sul path `/MenuBar/2`. Poiché tale percorso non era incluso tra i candidati e la chiamata a `com.canonical.AppMenu.Registrar` falliva/andava in timeout su Wayland (il registrar Canonical è progettato per gli X11 Window ID e non gestisce PID su Wayland puro), il programma non rilevava mai il menu di Dolphin, restituendo l'errore `Impossible to detect D-BUS menu for the active window`.
- **Soluzione:** 
  1. Estesa la scansione a tutti i percorsi `/MenuBar/1` fino a `/MenuBar/8` e percorsi noti.
  2. Implementata introspezione D-Bus dinamica su `/MenuBar` con parsing regex dei nodi XML per scoprire automaticamente qualsiasi indice `/MenuBar/<N>`.
  3. Gestione dei processi figli / renderer (Electron, Chromium) risalendo ricorsivamente al PPID da `/proc/<pid>/status`.

### 3. Race Condition nella Selezione dell'Azione Rofi (`main.cpp`)
- **Problema:** Quando Rofi esegue il secondo step con `ROFI_RETV=1` per azionare l'elemento selezionato dall'utente, il codice invocava nuovamente `tracker.get_active_window_info()`.
- **Causa tecnica:** Su Wayland con layer-shell, al momento della selezione la finestra attiva può cambiare focus o chiudersi, causando il fallimento o l'attivazione sulla finestra sbagliata.
- **Soluzione:** Codificare nel campo `info` di Rofi sia il servizio D-Bus, sia il percorso dell'oggetto, sia l'ID dell'azione (`service|object_path|item_id`). Al rientro con `ROFI_RETV=1`, il dispatch dell'evento avviene in modo deterministico e istantaneo senza dover re-interrogare il compositor.

### 4. Scorciatoie da Tastiera non Supportate (`DBusMenuClient`)
- **Problema:** `DBusMenuClient` richiedeva `"shortcut"` a D-Bus ma non estraeva né formattava il tipo `aas` (array di array di stringhe) restituito da `com.canonical.dbusmenu`.
- **Soluzione:** Aggiunto il parser per `std::vector<std::vector<std::string>>`, la normalizzazione dei tasti modificatori (`Control` -> `Ctrl`, ecc.) e l'inclusione delle scorciatoie sia nel testo mostrato sia nei metadati di ricerca (`\x1fmeta\x1f`).

### 5. Architettura Monolitica vs Strategy Pattern per Multi-Compositor
- **Problema:** Il tracker era strettamente accoppiato a `kdotool` e KDE Plasma.
- **Soluzione:** Introduzione di `WindowTracker` astratto (pattern Strategy), `TrackerFactory` con rilevamento automatico del compositore (`KWinTracker`, `HyprlandTracker`, `SwayTracker`).

### 6. Configurazione Build e Packaging
- **`PKGBUILD`:** URL del progetto errato (`https://github.com/tuonome/rofi-wayland-hud` sostituito con `https://github.com/sayoridev/rofi-wayland-hud`) e auto-conflitto rimosso.
- **`CMakeLists.txt`:** Utilizzo moderno di `target_include_directories` e flag di warning unificati.

---

## 📝 Registro Dettagliato delle Modifiche Eseguite

### 1. File Creati:
- **`include/rofi_hud/window_tracker.hpp`**: Interfaccia base astratta per il tracking delle finestre e definizione della struct `ActiveWindowInfo` (con campi `service_name`, `object_path`, `app_title`, `app_class`, `pid`).
- **`include/rofi_hud/dbus_discovery.hpp`** & **`src/dbus_discovery.cpp`**: Modulo centralizzato e performante per:
  - Risoluzione del bus D-Bus proprietario da PID (`resolve_owner_bus_name`).
  - Scansione intelligente dei percorsi menu (`/MenuBar/1` .. `/MenuBar/8`, `/com/canonical/dbusmenu`, ecc.).
  - Introspezione dinamica di `/MenuBar` per catturare qualsiasi sotto-nodo Qt/KDE.
  - Risoluzione PPID per Chromium/Electron.
  - Timeout di sicurezza su tutte le chiamate D-Bus (50ms) per garantire zero latenza.
- **`include/rofi_hud/hyprland_tracker.hpp`** & **`src/hyprland_tracker.cpp`**: Tracker per Hyprland basato su `hyprctl activewindow -j`.
- **`include/rofi_hud/sway_tracker.hpp`** & **`src/sway_tracker.cpp`**: Tracker per Sway/wlroots basato su `swaymsg -t get_tree`.
- **`include/rofi_hud/tracker_factory.hpp`** & **`src/tracker_factory.cpp`**: Factory automatica con prioritizzazione e auto-detection del compositore Wayland attivo (`HYPRLAND_INSTANCE_SIGNATURE`, `SWAYSOCK`, `XDG_CURRENT_DESKTOP`, fallback su KWin).

### 2. File Aggiornati e Bug Fixati:
- **`include/rofi_hud/kwin_tracker.hpp`**: Aggiornato per ereditare da `WindowTracker`.
- **`src/kwin_tracker.cpp`**: Aggiornato per recuperare `pid`, `title` e `class` da `kdotool getactivewindow getwindowpid %1 getwindowname %1 getwindowclassname %1` in una singola chiamata senza fork multipli, con fallback su `DBusDiscovery::get_process_name`.
- **`include/rofi_hud/dbus_menu_client.hpp`** & **`src/dbus_menu_client.cpp`**:
  - Aggiunto parsing completo di scorciatoie da tastiera D-Bus (`aas` -> `Ctrl+Shift+...`).
  - Timeout di sicurezza (1.5s su `GetLayout`, 1.0s su `Event`) per evitare blocchi dell'interfaccia.
  - Pulizia label preservando sequenze escape `&&` e `__`.
  - Filtro degli elementi di tipo `separator` e non visibili.
- **`include/rofi_hud/rofi_formatter.hpp`** & **`src/rofi_formatter.cpp`**:
  - Correzione del bug critico del byte nullo `\0` in `\0prompt` e `\0message`.
  - Rimozione del warning del compilatore per escape hex sequence fuori range tramite `constexpr char DELIM = '\x1f'`.
  - Integrazione delle scorciatoie da tastiera nel testo visualizzato e nei metadati `meta` per ricerca rapida.
  - Codifica deterministica `service|object_path|item_id` in `info` per eliminare qualsiasi race condition di focus al click.
- **`src/main.cpp`**:
  - Integrazione di `TrackerFactory::create_tracker()`.
  - Parsing del payload `service|object_path|item_id` con dispatch immediato dell'azione selezionata.
- **`CMakeLists.txt`**:
  - Aggiunti tutti i nuovi sorgenti C++ al target `rofi_hud_core`.
  - Configurato `target_include_directories` moderno e flag `-Wall -Wextra -Wpedantic` sia su libreria sia sull'eseguibile.
- **`AGENTS.md`**:
  - Documentazione completa di analisi, architettura, modifiche e bugfix.

- **`PKGBUILD`**:
  - Corretto URL del repository (`https://github.com/sayoridev/rofi-wayland-hud`).
  - Rimossa auto-dipendenza / self-conflict `conflicts=('rofi-wayland-hud')`.
  - Aggiunti `optdepends` per i diversi compositori (`kdotool`, `hyprland`, `sway`).
- **`TODO.md`**:
  - Marcati come completati i task della Fase 1 (Modular Architecture & Multi-Compositor Support), Fase 2 (Keyboard Shortcuts, Breadcrumbs) e Fase 3 (Timeout Protection, Focus Dispatch deterministico, workaround Qt/GTK/Electron).
- **`README.md`**:
  - Aggiornate le feature con evidenza del supporto multi-compositor Wayland (KDE Plasma 6, Hyprland, Sway), ricerca per shortcut e backend performante C++20.

---

## 🛠️ Verifica Compilazione e Test
- Compilazione eseguita con CMake:
  - Standard C++20, flag `-Wall -Wextra -Wpedantic`.
  - Nessun errore, nessun warning residuo.
  - Generazione dell'eseguibile `rofi_wayland_hud` e della libreria `librofi_hud_core.a`.
- Test funzionali:
  - Output formattato correttamente secondo il protocollo script Rofi (byte nullo `\0` seguito da prompt o message).
  - Rilevamento automatico delle scorciatoie da tastiera e supporto deterministico al clic.

---

## 🚀 Prossimo Passo: Commit e Push su GitHub
1. Eseguire `git add` e commit di tutti i file modificati.
2. Eseguire `git push origin main` autenticandosi tramite SSH con la passphrase fornita.
