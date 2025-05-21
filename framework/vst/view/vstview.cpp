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
#include "vstview.h"

#include <QQuickWindow>

#ifdef Q_OS_LINUX
#define USE_LINUX_RUNLOOP
#endif

#ifdef USE_LINUX_RUNLOOP
#include "../internal/platform/linux/runloop.h"
#endif

#include "log.h"

using namespace muse::vst;

::Steinberg::uint32 PLUGIN_API VstView::addRef()
{
    return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

::Steinberg::uint32 PLUGIN_API VstView::release()
{
    if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0) {
        return 0;
    }
    return __funknownRefCount;
}

Steinberg::tresult VstView::queryInterface(const ::Steinberg::TUID _iid, void** obj)
{
    QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::IPlugFrame);
    QUERY_INTERFACE(_iid, obj, Steinberg::IPlugFrame::iid, Steinberg::IPlugFrame);
    //As VST3 documentation states, IPlugFrame also has to provide
    //reference to the Steinberg::Linux::IRunLoop implementation.
#ifdef USE_LINUX_RUNLOOP
    if (m_runLoop && Steinberg::FUnknownPrivate::iidEqual(_iid, Steinberg::Linux::IRunLoop::iid)) {
        m_runLoop->addRef();
        *obj = static_cast<Steinberg::Linux::IRunLoop*>(m_runLoop);
        return ::Steinberg::kResultOk;
    }
#endif
    *obj = nullptr;
    return ::Steinberg::kNoInterface;
}

static FIDString currentPlatformUiType()
{
#ifdef Q_OS_MAC
    return Steinberg::kPlatformTypeNSView;
#elif defined(Q_OS_IOS)
    return Steinberg::kPlatformTypeUIView;
#elif defined(Q_OS_WIN)
    return Steinberg::kPlatformTypeHWND;
#else
    return Steinberg::kPlatformTypeX11EmbedWindowID;
#endif
}

VstView::VstView(QQuickItem* parent)
    : QQuickItem(parent)
{
    FUNKNOWN_CTOR; // IPlugFrame

#ifdef USE_LINUX_RUNLOOP
    m_runLoop = new RunLoop();
#endif
}

VstView::~VstView()
{
    FUNKNOWN_DTOR; // IPlugFrame

    deinit();
}

namespace {
constexpr bool operator==(const Steinberg::ViewRect& lhs, const Steinberg::ViewRect& rhs)
{
    return lhs.left == rhs.left && lhs.top == rhs.top && lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}
}

void VstView::init()
{
    m_instance = instancesRegister()->instanceById(m_instanceId);
    IF_ASSERT_FAILED(m_instance) {
        return;
    }

    m_title = QString::fromStdString(m_instance->name());
    emit titleChanged();

    m_view = m_instance->createView();
    if (!m_view) {
        return;
    }

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    updateScreenMetrics();

    m_view->setFrame(this);

    m_vstWindow = new QWindow(window());

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(m_vstWindow->winId()), currentPlatformUiType());
    if (attached != Steinberg::kResultOk) {
        LOGE() << "Unable to attach vst plugin view to window"
               << ", instance name: " << m_instance->name();
        return;
    }

    // Do not rely on `QWindow::screenChanged` signal, which often does not get emitted though it should.
    // Proactively check for screen resolution changes instead.
    // Note: optmization attempts only to call `updateViewGeometry` if screen metrics or vst view size changed
    // seem to miss out on relevant changes for some plugins.
    connect(&m_screenMetricsTimer, &QTimer::timeout, this, [this]() {
        updateScreenMetrics();
        updateViewGeometry();
    });
    m_screenMetricsTimer.start(std::chrono::milliseconds { 100 });

    updateViewGeometry();

    m_vstWindow->show();
}

void VstView::deinit()
{
    if (m_view) {
        m_view->setFrame(nullptr);
        m_view->removed();
        m_view = nullptr;

        m_vstWindow->hide();
        delete m_vstWindow;
        m_vstWindow = nullptr;
    }

#ifdef USE_LINUX_RUNLOOP
    if (m_runLoop) {
        m_runLoop->stop();
        delete m_runLoop;
    }
#endif

    if (m_instance) {
        m_instance->refreshConfig();
        m_instance = nullptr;
    }
}

Steinberg::tresult VstView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSize)
{
    IF_ASSERT_FAILED(m_vstWindow) {
        return Steinberg::kResultFalse;
    }

    view->checkSizeConstraint(requiredSize);

    int newWidth = requiredSize->getWidth();
    int newHeight = requiredSize->getHeight();

//! NOTE: newSize already includes the UI scaling on Windows, so we have to remove it before setting the fixed size.
//! Otherwise, the user will get an extremely large window and won't be able to resize it
#ifndef Q_OS_MAC
    newWidth = newWidth / m_screenMetrics.devicePixelRatio;
    newHeight = newHeight / m_screenMetrics.devicePixelRatio;
#endif

    const int titleBarHeight = window()->frameGeometry().height() - window()->geometry().height();

    newWidth = std::min(newWidth, m_screenMetrics.availableSize.width() - 2 * m_sidePadding);
    newHeight = std::min(newHeight, m_screenMetrics.availableSize.height() - titleBarHeight - m_topPadding - m_bottomPadding);

    setImplicitHeight(newHeight);
    setImplicitWidth(newWidth);

    m_vstWindow->setGeometry(m_sidePadding, m_topPadding, this->implicitWidth(), this->implicitHeight());

    setVstSize(*view, newWidth, newHeight);

    return Steinberg::kResultTrue;
}

void VstView::setVstSize(Steinberg::IPlugView& view, int width, int height)
{
    const auto dpi = m_screenMetrics.devicePixelRatio;
    Steinberg::ViewRect vstSize;
    vstSize.right = width * dpi;
    vstSize.bottom = height * dpi;
    view.onSize(&vstSize);
}

QSize VstView::vstSize(Steinberg::IPlugView&) const
{
    Steinberg::ViewRect size;
    m_view->getSize(&size);
    const auto dpi = m_screenMetrics.devicePixelRatio;
    return QSize(size.getWidth() / dpi, size.getHeight() / dpi);
}

void VstView::updateScreenMetrics()
{
    const QScreen* const screen = window()->screen();
    m_screenMetrics.availableSize = screen->availableSize();
#ifdef Q_OS_MAC
    constexpr auto devicePixelRatio = 1.0;
#else
    const auto devicePixelRatio = screen->devicePixelRatio();
#endif
    m_screenMetrics.devicePixelRatio = devicePixelRatio;
}

void VstView::updateViewGeometry()
{
    IF_ASSERT_FAILED(m_view) {
        return;
    }

    Steinberg::ViewRect size;
    m_view->getSize(&size);

    resizeView(m_view, &size);
}

int VstView::instanceId() const
{
    return m_instanceId;
}

void VstView::setInstanceId(int newInstanceId)
{
    if (m_instanceId == newInstanceId) {
        return;
    }
    m_instanceId = newInstanceId;
    emit instanceIdChanged();
}

QString VstView::title() const
{
    return m_title;
}

int VstView::sidePadding() const
{
    return m_sidePadding;
}

void VstView::setsidePadding(int sidePadding)
{
    if (m_sidePadding == sidePadding) {
        return;
    }
    m_sidePadding = sidePadding;
    emit sidePaddingChanged();
}

int VstView::topPadding() const
{
    return m_topPadding;
}

void VstView::setTopPadding(int topPadding)
{
    if (m_topPadding == topPadding) {
        return;
    }
    m_topPadding = topPadding;
    emit topPaddingChanged();
}

int VstView::bottomPadding() const
{
    return m_bottomPadding;
}

void VstView::setBottomPadding(int bottomPadding)
{
    if (m_bottomPadding == bottomPadding) {
        return;
    }
    m_bottomPadding = bottomPadding;
    emit bottomPaddingChanged();
}
