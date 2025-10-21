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

#include <QQuickWindow>

using namespace muse::audioplugins;

AudioPluginView::AudioPluginView(QQuickItem* parent)
    : QQuickItem(parent)
{
}

void AudioPluginView::init()
{
    connect(this, &QQuickItem::enabledChanged, this, [this]() {
        if (!isEnabled() && !m_disablingWindow) {
            QWindow* const pluginWindow = this->pluginWindow();
            IF_ASSERT_FAILED(pluginWindow) {
                return;
            }

            m_disablingWindow = new QQuickWindow(pluginWindow);
            m_disablingWindow->setWidth(pluginWindow->width());
            m_disablingWindow->setHeight(pluginWindow->height());
            m_disablingWindow->setColor(Qt::transparent);

            QQmlEngine engine;
            QQmlComponent component(&engine);
            component.setData(
                "import QtQuick 2.15\n"
                "Rectangle { color: 'white'; opacity: 0.25; anchors.fill: parent }",
                QUrl());
            QQuickItem* const rect = qobject_cast<QQuickItem*>(component.create());
            assert(rect);
            if (rect) {
                rect->setParentItem(m_disablingWindow->contentItem());
            }

            m_disablingWindow->show();
        } else if (isEnabled() && m_disablingWindow) {
            m_disablingWindow->hide();
            delete m_disablingWindow;
            m_disablingWindow = nullptr;
        }
    });

    doInit();
}

int AudioPluginView::instanceId() const
{
    return m_instanceId;
}

void AudioPluginView::setInstanceId(int newInstanceId)
{
    if (m_instanceId == newInstanceId) {
        return;
    }
    m_instanceId = newInstanceId;
    emit instanceIdChanged();
}
