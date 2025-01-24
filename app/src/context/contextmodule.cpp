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
#include "contextmodule.h"
#include "ui/iuicontextresolver.h"
#include "shortcuts/ishortcutsregister.h"

using namespace muse;

namespace au::context {
namespace {
class MockUiContextResolver : public ui::IUiContextResolver
{
    INTERFACE_ID(IUiContextResolver)

public:

    ui::UiContext currentUiContext() const override
    {
        static ui::UiContext ctx{ "Any" };
        return ctx;
    }

    async::Notification currentUiContextChanged() const override
    {
        static async::Notification n;
        return n;
    }

    bool match(const ui::UiContext& currentCtx, const ui::UiContext& actCtx) const override
    {
        return true;
    }

    bool matchWithCurrent(const ui::UiContext& ctx) const override
    {
        return true;
    }

    bool isShortcutContextAllowed(const std::string& scContext) const override
    {
        return true;
    }
};
}

class MockShortcutsRegister : public shortcuts::IShortcutsRegister
{
    INTERFACE_ID(IShortcutsRegister)
public:

    const shortcuts::ShortcutList& shortcuts() const override
    {
        static shortcuts::ShortcutList list;
        return list;
    }

    Ret setShortcuts(const shortcuts::ShortcutList& shortcuts) override
    {
        return make_ok();
    }

    void resetShortcuts() override {}
    async::Notification shortcutsChanged() const override
    {
        static async::Notification n;
        return n;
    }

    Ret setAdditionalShortcuts(const std::string& context, const shortcuts::ShortcutList& shortcuts) override
    {
        return make_ok();
    }

    const shortcuts::Shortcut& shortcut(const std::string& actionCode) const override
    {
        static shortcuts::Shortcut s;
        return s;
    }

    const shortcuts::Shortcut& defaultShortcut(const std::string& actionCode) const override
    {
        static shortcuts::Shortcut s;
        return s;
    }

    bool isRegistered(const std::string& sequence) const override
    {
        return false;
    }

    shortcuts::ShortcutList shortcutsForSequence(const std::string& sequence) const override
    {
        static shortcuts::ShortcutList list;
        return list;
    }

    Ret importFromFile(const io::path_t& filePath) override
    {
        return make_ok();
    }

    Ret exportToFile(const io::path_t& filePath) const override
    {
        return make_ok();
    }

    bool active() override
    {
        return false;
    }

    void setActive(bool active) override {}
    async::Notification activeChanged() const override
    {
        static async::Notification n;
        return n;
    }

    // for autobot tests
    void reload(bool onlyDef = false) override {}
};

std::string ContextModule::moduleName() const
{
    return "context";
}

void ContextModule::registerExports()
{
    ioc()->registerExport<muse::ui::IUiContextResolver>(moduleName(), new MockUiContextResolver());
    ioc()->registerExport<shortcuts::IShortcutsRegister>(moduleName(), new MockShortcutsRegister());
}
}
