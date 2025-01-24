/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef QT_QML_DEBUG
#include <QQmlDebuggingEnabler>
#endif

#include <QTextCodec>

#include <csignal>

#include "app.h"

#include "log.h"

#include "accessibility/accessibilitymodule.h"

#include "actions/actionsmodule.h"

#ifdef MU_BUILD_EXTENSIONS_MODULE
#include "extensions/extensionsmodule.h"
#endif

#ifdef MU_BUILD_LANGUAGES_MODULE
#include "languages/languagesmodule.h"
#endif

#ifdef MU_BUILD_MIDI_MODULE
#include "midi/midimodule.h"
#endif

#include "multiinstances/multiinstancesmodule.h"

#ifdef MU_BUILD_NETWORK_MODULE
#include "network/networkmodule.h"
#endif

#ifdef MU_BUILD_SHORTCUTS_MODULE
#include "shortcuts/shortcutsmodule.h"
#endif

#include "ui/uimodule.h"
#include "uicomponents/uicomponentsmodule.h"
#include "context/contextmodule.h"

#include "workspace/workspacemodule.h"

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>
#endif

#ifndef MU_BUILD_CRASHPAD_CLIENT
static void crashCallback(int signum)
{
    const char* signame = "UNKNOWN SIGNAME";
    const char* sigdescript = "";
    switch (signum) {
    case SIGILL:
        signame = "SIGILL";
        sigdescript = "Illegal Instruction";
        break;
    case SIGSEGV:
        signame = "SIGSEGV";
        sigdescript =  "Invalid memory reference";
        break;
    }
    LOGE() << "Oops! Application crashed with signal: [" << signum << "] " << signame << "-" << sigdescript;
    exit(EXIT_FAILURE);
}

#endif

int main(int argc, char** argv)
{
#ifndef MUE_BUILD_CRASHPAD_CLIENT
    signal(SIGSEGV, crashCallback);
    signal(SIGILL, crashCallback);
    signal(SIGFPE, crashCallback);
#endif

    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
    // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    au::app::App app;

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (an should be) unimportant.
    //app.addModule(new mu::diagnostics::DiagnosticsModule());

    // framework
    app.addModule(new muse::accessibility::AccessibilityModule());


#ifdef MU_BUILD_EXTENSIONS_MODULE
    app.addModule(new mu::extensions::ExtensionsModule());
#endif

#ifdef MU_BUILD_LANGUAGES_MODULE
    app.addModule(new mu::languages::LanguagesModule());
#endif

#ifdef MU_BUILD_MIDI_MODULE
    app.addModule(new mu::midi::MidiModule());
#endif

    app.addModule(new muse::mi::MultiInstancesModule());

#ifdef MU_BUILD_NETWORK_MODULE
    app.addModule(new mu::network::NetworkModule());
#endif

#ifdef MU_BUILD_SHORTCUTS_MODULE
    app.addModule(new mu::shortcuts::ShortcutsModule());
#endif

    app.addModule(new muse::workspace::WorkspaceModule());
    app.addModule(new muse::actions::ActionsModule());
    app.addModule(new muse::ui::UiModule());
    app.addModule(new muse::uicomponents::UiComponentsModule());
    app.addModule(new au::context::ContextModule());

#if (defined (_MSCVER) || defined (_MSC_VER))
    // On MSVC under Windows, we need to manually retrieve the command-line arguments and convert them from UTF-16 to UTF-8.
    // This prevents data loss if there are any characters that wouldn't fit in the local ANSI code page.
    int argcUTF16 = 0;
    LPWSTR* argvUTF16 = CommandLineToArgvW(GetCommandLineW(), &argcUTF16);

    std::vector<QByteArray> argvUTF8Q;
    std::for_each(argvUTF16, argvUTF16 + argcUTF16, [&argvUTF8Q](const auto& arg) {
        argvUTF8Q.emplace_back(QString::fromUtf16(reinterpret_cast<const char16_t*>(arg), -1).toUtf8());
    });

    LocalFree(argvUTF16);

    std::vector<char*> argvUTF8;
    for (auto& arg : argvUTF8Q) {
        argvUTF8.push_back(arg.data());
    }

    // Don't use the arguments passed to main(), because they're in the local ANSI code page.
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    int argcFinal = argcUTF16;
    char** argvFinal = argvUTF8.data();
#else

    int argcFinal = argc;
    char** argvFinal = argv;

#endif

    int code = app.run(argcFinal, argvFinal);
    LOGI() << "Goodbye!! code: " << code;
    return code;
}
