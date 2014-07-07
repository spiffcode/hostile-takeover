#include "base/selectserver.h"

#include "base/log.h"
#include "base/socket.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

namespace base {

SelectServer::SelectServer() {
    notifying_ = 0;
    wait_ = false;
    pdispFirst_ = NULL;
    pdispNotifyNext_ = NULL;
    eventer_ = new Eventer(this, &wait_);
}

SelectServer::~SelectServer() {
    delete eventer_;
}

void SelectServer::Add(SelectDispatcher *pdisp) {
    CritScope cs(&crit_);
    pdisp->pdispNext_ = pdispFirst_;
    pdispFirst_ = pdisp;
}

void SelectServer::Remove(SelectDispatcher *pdisp) {
    CritScope cs(&crit_);
    SelectDispatcher **ppdispT = &pdispFirst_;
    while ((*ppdispT) != NULL) {
        if (*ppdispT == pdisp) {
            *ppdispT = pdisp->pdispNext_;
            if (pdispNotifyNext_ == pdisp) {
                pdispNotifyNext_ = pdisp->pdispNext_;
            }
            break;
        }
        ppdispT = &(*ppdispT)->pdispNext_;
    }
}

void SelectServer::WakeUp() {
    eventer_->Signal();
}

bool SelectServer::Wait(long64 ctWaitBig, bool fProcessIO) {
    // Allow recursion when processing input.
    if (notifying_ != 0) {
        LOG() << "Error - re-entering!";
        fProcessIO = false;
    }

    // Calculate timing information
   
    int ctWait = (int)ctWaitBig; 
    struct timeval *ptvWait = NULL; 
    struct timeval tvWait;
    struct timeval tvStop;
    if (ctWait != kctForever) {
        // Calculate wait timeval

        tvWait.tv_sec = ctWait / 100;
        tvWait.tv_usec = (ctWait % 100) * 10000;
        ptvWait = &tvWait;

        // Calculate when to return in a timeval

        gettimeofday(&tvStop, NULL);
        tvStop.tv_sec += tvWait.tv_sec;
        tvStop.tv_usec += tvWait.tv_usec;
        if (tvStop.tv_usec >= 1000000) {
            tvStop.tv_usec -= 1000000;
            tvStop.tv_sec += 1;
        } 
    }

    // Zero all fd_sets. Don't need to do this inside the loop since
    // select() zeros the descriptors not signaled

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    fd_set fdsWrite;
    FD_ZERO(&fdsWrite);

    wait_ = true;

    while (wait_) {
        int fdMax = -1;
        {
            CritScope cr(&crit_);

            SelectDispatcher *pdispT = pdispFirst_;
            for (; pdispT != NULL; pdispT = pdispT->pdispNext_) {
                // Query dispatchers for read and write wait state

                if (!fProcessIO && (pdispT != eventer_->dispatcher())) {
                    continue;
                }

                int fd = pdispT->descriptor();
                if (fd > fdMax) {
                    fdMax = fd;
                }

                dword ff = pdispT->GetEvents();
                if (ff & (Dispatcher::kfRead | Dispatcher::kfRemoteClose)) {
                    FD_SET(fd, &fdsRead);
                }
                if (ff & (Dispatcher::kfWrite | Dispatcher::kfConnect)) {
                    FD_SET(fd, &fdsWrite);
                }
            }
        }

        // Wait then call handlers as appropriate
        // < 0 means error
        // 0 means timeout
        // > 0 means count of descriptors ready

        int n = select(fdMax + 1, &fdsRead, &fdsWrite, NULL, ptvWait);

        // If error, return error

        if (n < 0) {
            LOG() << "select returns < 0, errno: " << errno << ", "
                    << Socket::GetErrorString(errno);
            return false;
        }

        // If timeout, return success

        if (n == 0) {
            return true;
        }

        // Find a dispatcher to notify, then notify outside of the loop
        // and outside of the critical section.

        notifying_++;
        SelectDispatcher *pdispNotify = pdispFirst_;
        do {
            dword ff = 0;
            {
                CritScope cs(&crit_);
                while (pdispNotify != NULL) {
                    if (!fProcessIO && (pdispNotify != eventer_->dispatcher())) {
                        pdispNotify = pdispNotify->pdispNext_;
                        continue;
                    }
                    int fd = pdispNotify->descriptor();
                    if (FD_ISSET(fd, &fdsRead)) {
                        FD_CLR(fd, &fdsRead);
                        if (pdispNotify->GetEvents() & Dispatcher::kfRemoteClose) {
                            ff |= Dispatcher::kfClose;
                        } else {
                            ff |= Dispatcher::kfRead;
                        }
                    }
                    if (FD_ISSET(fd, &fdsWrite)) {
                        FD_CLR(fd, &fdsWrite);
                        if (pdispNotify->GetEvents() & Dispatcher::kfConnect) {
                            ff |= Dispatcher::kfConnect;
                        } else {
                            ff |= Dispatcher::kfWrite;
                        }
                    }
                    if (ff != 0) {
                        break;
                    }
                    pdispNotify = pdispNotify->pdispNext_;
                }
            }

            // Notify outside of the critical section and the loop.

            if (pdispNotify != NULL) {
                // Remember pdispNext so that it can be updated in case
                // of deletion during callback
                if (fProcessIO) {
                    pdispNotifyNext_ = pdispNotify->pdispNext_;
                }
                pdispNotify->OnEvent(ff);
                pdispNotify = pdispNotifyNext_;
                if (fProcessIO) {
                    pdispNotifyNext_ = NULL;
                }
            }
        } while (pdispNotify != NULL);
        notifying_--;

        // Recalc the time remaining to wait. Doing it here means it doesn't
        // get calced twice the first time through the loop

        if (ctWait != kctForever) {
            ptvWait->tv_sec = 0;
            ptvWait->tv_usec = 0;
            struct timeval tvT;
            gettimeofday(&tvT, NULL);
            if (tvStop.tv_sec < tvT.tv_sec) {
                continue;
            }
            if (tvStop.tv_sec == tvT.tv_sec) {
                if (tvStop.tv_usec > tvT.tv_usec) {
                    ptvWait->tv_usec = tvStop.tv_usec - tvT.tv_usec;
                }
                continue;
            }
            ptvWait->tv_sec = tvStop.tv_sec - tvT.tv_sec;
            ptvWait->tv_usec = tvStop.tv_usec - tvT.tv_usec;
            if (ptvWait->tv_usec < 0) {
                ptvWait->tv_usec += 1000000;
                ptvWait->tv_sec -= 1;
            }
        }
    }

    return true;
}

Dispatcher *SelectServer::CreateDispatcher() {
    return new SelectDispatcher(this);
}

SocketServer *SocketServer::Create() {
    return new SelectServer;
}

} // namespace base
