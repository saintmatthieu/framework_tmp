/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include <QTimer>

#include "global/modularity/ioc.h"
#include "../ivstinstancesregister.h"

namespace muse::vst {
class RunLoop;
class VstView : public QQuickItem, public Steinberg::IPlugFrame
{
    Q_OBJECT
    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(int headerHeight READ headerHeight WRITE setHeaderHeight NOTIFY headerHeightChanged FINAL)
    Q_PROPERTY(int footerHeight READ footerHeight WRITE setFooterHeight NOTIFY footerHeightChanged FINAL)

    muse::Inject<IVstInstancesRegister> instancesRegister;

    DECLARE_FUNKNOWN_METHODS

public:
    VstView(QQuickItem* parent = nullptr);
    ~VstView();

    int instanceId() const;
    void setInstanceId(int newInstanceId);

    Q_INVOKABLE void init();
    Q_INVOKABLE void deinit();

    // IPlugFrame
    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSize) override;
    // ----------

    QString title() const;

    int headerHeight() const;
    void setHeaderHeight(int newHeaderHeight);
    int footerHeight() const;
    void setFooterHeight(int newFooterHeight);

signals:
    void instanceIdChanged();
    void titleChanged();
    void headerHeightChanged();
    void footerHeightChanged();

private:

    struct ScreenMetrics {
        QSize availableSize;
        double devicePixelRatio = 0.0;
        bool operator==(const ScreenMetrics& other) const
        {
            return availableSize == other.availableSize && devicePixelRatio == other.devicePixelRatio;
        }

        ScreenMetrics& operator=(const ScreenMetrics& other)
        {
            availableSize = other.availableSize;
            devicePixelRatio = other.devicePixelRatio;
            return *this;
        }
    };

    void updateScreenMetrics();
    void updateViewGeometry();

    int m_instanceId = -1;
    IVstPluginInstancePtr m_instance;
    QWindow* m_window = nullptr;
    PluginViewPtr m_view;
    QString m_title;
    RunLoop* m_runLoop = nullptr;

    ScreenMetrics m_screenMetrics;
    QTimer m_screenMetricsTimer;

    int m_headerHeight = 0;
    int m_footerHeight = 0;
};
}
