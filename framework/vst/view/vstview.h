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
    Q_PROPERTY(int sidePadding READ sidePadding WRITE setsidePadding NOTIFY sidePaddingChanged FINAL)
    Q_PROPERTY(int topPadding READ topPadding WRITE setTopPadding NOTIFY topPaddingChanged FINAL)
    Q_PROPERTY(int bottomPadding READ bottomPadding WRITE setBottomPadding NOTIFY bottomPaddingChanged FINAL)

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

    int sidePadding() const;
    void setsidePadding(int);
    int topPadding() const;
    void setTopPadding(int);
    int bottomPadding() const;
    void setBottomPadding(int);

signals:
    void instanceIdChanged();
    void titleChanged();
    void sidePaddingChanged();
    void topPaddingChanged();
    void bottomPaddingChanged();

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
    QSize vstSize(Steinberg::IPlugView&) const;
    void setVstSize(Steinberg::IPlugView&, int width, int height);

    int m_instanceId = -1;
    IVstPluginInstancePtr m_instance;
    QWindow* m_vstWindow = nullptr;
    PluginViewPtr m_view;
    QString m_title;
    RunLoop* m_runLoop = nullptr;

    ScreenMetrics m_screenMetrics;
    QTimer m_screenMetricsTimer;

    int m_sidePadding = 0;
    int m_topPadding = 0;
    int m_bottomPadding = 0;
};
}
