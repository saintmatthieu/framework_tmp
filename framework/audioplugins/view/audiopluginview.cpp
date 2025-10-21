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
#include "audiopluginview.h"

#include "log.h"

#include <QWindow>

using namespace muse::audioplugins;

AudioPluginView::AudioPluginView(QQuickItem* parent)
    : QQuickItem(parent)
{
}

void AudioPluginView::init()
{
    connect(this, &QQuickItem::enabledChanged, this, [this]() {
        if (!isEnabled() && !m_disablingWindow) {
            QWindow* pluginWindow = this->pluginWindow();
            IF_ASSERT_FAILED(pluginWindow) {
                return;
            }
            m_disablingWindow = new QWindow(pluginWindow);
            m_disablingWindow->setGeometry(pluginWindow->geometry());
            m_disablingWindow->setCursor(Qt::ForbiddenCursor);
            m_disablingWindow->show();
        } else if (isEnabled() && m_disablingWindow) {
            m_disablingWindow->hide();
            delete m_disablingWindow;
            m_disablingWindow = nullptr;
        }
    });

    doInit();
}
