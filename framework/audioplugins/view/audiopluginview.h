/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#pragma once

#include <QQuickItem>

namespace muse::audioplugins {
class AudioPluginView : public QQuickItem
{
    Q_OBJECT

public:
    explicit AudioPluginView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void init();

private:
    virtual void doInit() {}
    virtual QWindow* pluginWindow() = 0;
    QWindow* m_disablingWindow = nullptr;
};
}
