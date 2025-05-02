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

#ifdef Q_OS_WIN
#include <windows.h>

namespace {
void* getUser32Function(const char* functionName)
{
    HMODULE module = GetModuleHandleA("user32.dll");

    if (module != nullptr) {
        return (void*)GetProcAddress(module, functionName);
    }

    assert(false);
    return nullptr;
}

using GetDPIForWindowFunc                      = UINT(WINAPI*) (HWND);
double getScaleFactorForWindow(HWND h)
{
    // NB. Using a local function here because we need to call this method from the plug-in wrappers
    // which don't load the DPI-awareness functions on startup
    static auto localGetDPIForWindow = (GetDPIForWindowFunc)getUser32Function("GetDpiForWindow");

    if (localGetDPIForWindow != nullptr) {
        return (double)localGetDPIForWindow(h) / USER_DEFAULT_SCREEN_DPI;
    }

    return 1.0;
}
}
#endif

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

    m_view->setFrame(this);

    m_window = new QWindow(window());

    updateScreenMetrics();

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(m_window->winId()), currentPlatformUiType());
    if (attached != Steinberg::kResultOk) {
        LOGE() << "Unable to attach vst plugin view to window"
               << ", instance name: " << m_instance->name();
        return;
    }

    // connect(mainWindow()->qWindow(), &QWindow::widthChanged, this, [this](int width) {
    //     updateScreenMetrics();
    //     updateViewGeometry();
    // });

    // connect(window(), &QWindow::widthChanged, this, [this](int) {
    //     updateScreenMetrics();
    //     updateViewGeometry();
    // });

    updateViewGeometry();

    m_window->show();

#ifdef Q_OS_WIN
    // Every second, log the devicePixelRatio and availableSize of the screen
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        Steinberg::ViewRect size;
        m_view->getSize(&size);
        const QScreen* screen = window()->screen();
        const auto ppd = screen->devicePixelRatio();
        const auto winWidthDt = window()->size().width();
        const auto viewWidthDt = size.getWidth() / ppd;
        const auto estimatedScale = (float)getScaleFactorForWindow(reinterpret_cast<HWND>(m_window->winId()));
        LOGI() << "ppd:" << ppd << ", estimatedScale:" << estimatedScale << ", winWidthDt:" << winWidthDt << ", viewWidthDt:" << viewWidthDt;
        if (estimatedScale != ppd) {
            LOGI() << "Matt: fixing inconsistent scale factor";
            updateScreenMetrics();
            updateViewGeometry();
        }
    });
    timer->start(3000);
#endif
}

void VstView::deinit()
{
    if (m_view) {
        m_view->setFrame(nullptr);
        m_view->removed();
        m_view = nullptr;

        m_window->hide();
        delete m_window;
        m_window = nullptr;
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

Steinberg::tresult VstView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSizePx)
{
    IF_ASSERT_FAILED(m_window) {
        return Steinberg::kResultFalse;
    }

    view->checkSizeConstraint(requiredSizePx);

    const int newWidthPx = requiredSizePx->getWidth();
    const int newHeightPx = requiredSizePx->getHeight();

    // pixels per dots
    const auto ppd = m_screenMetrics.devicePixelRatio;

//! NOTE: newSize already includes the UI scaling on Windows, so we have to remove it before setting the fixed size.
//! Otherwise, the user will get an extremely large window and won't be able to resize it
#ifndef Q_OS_MAC
    int newWidthDt = newWidthPx / ppd;
    int newHeightDt = newHeightPx / ppd;
#endif

    newWidthDt = std::min(newWidthDt, m_screenMetrics.availableSize.width());
    newHeightDt = std::min(newHeightDt, m_screenMetrics.availableSize.height());

    setImplicitHeight(newHeightDt);
    setImplicitWidth(newWidthDt);

    m_window->setGeometry(this->x(), this->y(), this->implicitWidth(), this->implicitHeight());
    Steinberg::ViewRect vstSizePx;
    vstSizePx.right = m_window->width() * ppd;
    vstSizePx.bottom = m_window->height() * ppd;
    view->onSize(&vstSizePx);

    return Steinberg::kResultTrue;
}

void VstView::updateScreenMetrics()
{
    QScreen* screen = window()->screen();
    m_screenMetrics.availableSize = screen->availableSize();
    m_screenMetrics.devicePixelRatio = screen->devicePixelRatio();
    // m_screenMetrics.devicePixelRatio = getScaleFactorForWindow(reinterpret_cast<HWND>(window()->winId()));
}

void VstView::updateViewGeometry()
{
    IF_ASSERT_FAILED(m_view) {
        return;
    }

    Steinberg::ViewRect size;
    m_view->getSize(&size);

    #ifdef Q_OS_MAC
    const auto doResize = m_view->checkSizeConstraint(&size) == Steinberg::kResultTrue;
    #else
    constexpr auto doResize = true;
    #endif
    if (doResize) {
        resizeView(m_view, &size);
    }
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
