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
#ifndef MUSE_GLOBAL_PROGRESS_H
#define MUSE_GLOBAL_PROGRESS_H

#include "async/channel.h"
#include "async/notification.h"

#include "types/retval.h"
#include "types/val.h"

namespace muse {
using ProgressResult = RetVal<Val>;

class Progress
{
public:

    // start
    void start()
    {
        m_isCanceled = false;
        m_isStarted = true;
        m_prevCurrent = -1;
        m_prevTotal = -1;
        m_started.notify();
    }

    async::Notification& started() { return m_started; }
    bool isStarted() const { return m_isStarted; }

    void setMaxNumIncrements(int64_t maxNumIncrements) { m_maxNumIncrements = maxNumIncrements; }

    // progress
    bool progress(int64_t current, int64_t total, const std::string& msg = {})
    {
        if (m_maxNumIncrements > 0 && total > m_maxNumIncrements) {
            current *= static_cast<double>(m_maxNumIncrements) / total;
            total = m_maxNumIncrements;
        }

        if (current == m_prevCurrent && total == m_prevTotal) {
            return false;
        }

        m_prevCurrent = current;
        m_prevTotal = total;
        m_progressChanged.send(current, total, msg);

        return true;
    }

    async::Channel<int64_t /*current*/, int64_t /*total*/, std::string /*title*/>& progressChanged()
    {
        return m_progressChanged;
    }

    void finish(const ProgressResult& res)
    {
        m_isStarted = false;
        m_prevCurrent = -1;
        m_prevTotal = -1;
        m_finished.send(res);
    }

    async::Channel<ProgressResult>& finished() { return m_finished; }

    void cancel()
    {
        m_isCanceled = true;
        m_canceled.notify();
        finish(make_ret(Ret::Code::Cancel));
    }

    async::Notification& canceled() { return m_canceled; }
    bool isCanceled() const { return m_isCanceled; }

private:
    async::Notification m_started;
    async::Channel<ProgressResult> m_finished;
    async::Notification m_canceled;
    bool m_isStarted = false;
    bool m_isCanceled = false;
    int64_t m_prevCurrent = -1;
    int64_t m_prevTotal = -1;
    int64_t m_maxNumIncrements = 0;

    async::Channel<int64_t /*current*/, int64_t /*total*/, std::string /*title*/> m_progressChanged;
};

using ProgressPtr = std::shared_ptr<Progress>;
}

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(muse::Progress*)
#endif

#endif // MUSE_GLOBAL_PROGRESS_H
