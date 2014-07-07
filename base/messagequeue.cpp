#include "base/messagequeue.h"
#include "base/tick.h"

namespace base {

MessageQueue::MessageQueue(SocketServer *ss) : ss_(ss), stop_(false),
        pmsg_(NULL), pmsgDelayed_(NULL)
{
    if (ss_ == NULL) {
        ss_ = SocketServer::Create();
    }
}

MessageQueue::~MessageQueue()
{
    Clear();
    ClearDispose();
    delete ss_;
}

void MessageQueue::Stop()
{
    stop_ = true;
    ss_->WakeUp();
}

bool MessageQueue::IsStopping()
{
    return stop_;
}

bool MessageQueue::Get(Message *pmsg, long64 ctWait)
{
    long64 ctTotal = ctWait;
    long64 ctElapsed = 0;
    long64 ctDelayNext = kctForever;
    long64 tStart = GetTickCount();
    long64 tCurrent = tStart;
    while (true) {
        {
            CritScope cs(&crit_);

            // Check for timed messages (delayed messages, and timers)
            if (pmsgDelayed_ != NULL) {
                ctDelayNext = pmsgDelayed_->tTrigger - tCurrent;
                if (ctDelayNext <= 0) {
                    // If it's a delayed message, move it onto the message
                    // queue, and we're good to go.
                    if (!(pmsgDelayed_->ff & kfMsgTimer)) {
                        Message *pmsgT = pmsgDelayed_;
                        pmsgDelayed_ = pmsgDelayed_->pmsgNext;
                        pmsgT->pmsgNext = pmsg_;
                        pmsg_ = pmsgT;
                    } else {
                        // It is a timer, which means it goes off on time
                        // at a constant rate. Remove it from the timer list,
                        // since it will need to be re-inserted because of the
                        // fixed sort.
                        Message *pmsgT = pmsgDelayed_;
                        pmsgDelayed_ = pmsgDelayed_->pmsgNext;
                        
                        // Make a copy and put it at the top of the regular
                        // message queue, so it gets processed.
                        Message *pmsgNew = new Message;
                        if (pmsgNew != NULL) {
                            *pmsgNew = *pmsgT;
                            pmsgNew->pmsgNext = pmsg_;
                            pmsg_ = pmsgNew;
                        }

                        // Adjust the trigger to track a constant rate, so
                        // that processing time doesn't add to latency.
                        pmsgT->tTrigger += pmsgT->ctRate;

                        // Add latency if the trigger is already behind,
                        // and insert it back into the delayed message list.
                        if ((tCurrent - pmsgT->tTrigger) >= pmsgT->ctRate) {
                            pmsgT->tTrigger = tCurrent;
                        }
                        InsertDelayedMessage(pmsgT);
                    }
                }
            }

            // Check for regular messages
            while (pmsg_ != NULL) {
                Message *pmsgT = pmsg_;
                pmsg_ = pmsg_->pmsgNext;
                if (pmsgT->id == kidmDispose && pmsgT->handler == NULL) {
                    delete pmsgT->data;
                    delete pmsgT;
                    continue;
                }
                *pmsg = *pmsgT;
                delete pmsgT;
                return true;
            }
        }

        // If asked, stop before waiting

        if (stop_) {
            break;
        }

        // Calculate wait remaining

        long64 ctNext;
        if (ctWait == kctForever) {
            ctNext = ctDelayNext;
        } else {
            ctNext = ctTotal - ctElapsed;
            if (ctNext < 0) {
                ctNext = 0;
            }
            if (ctDelayNext != kctForever && ctDelayNext < ctNext) {
                ctNext = ctDelayNext;
            }
        }

        // Wait and multiplex

        ss_->Wait(ctNext, true);

        // If the requested time has expired, return

        tCurrent = GetTickCount();
        ctElapsed = tCurrent - tStart;
        if (ctWait != kctForever) {
            if (ctElapsed >= ctWait) {
                return false;
            }
        }
    }
    return false;
}

void MessageQueue::Post(Message *pmsg, int idCoalesce)
{
    if (stop_)
        return;

    // Copy message before putting in queue
    Message *pmsgNew = new Message;
    if (pmsgNew == NULL) {
        return;
    }
    *pmsgNew = *pmsg;

    // Search for end or message to coalesce
    CritScope cs(&crit_);
    Message **ppmsgT = &pmsg_;
    Message *pmsgCoalesce = NULL;
    while (*ppmsgT != NULL) {
        // Find the last posted message that matches pmsg->id. Allow
        // idCoalesce after, but no others.
        
        if (idCoalesce != -1) {
            if ((*ppmsgT)->id == pmsg->id) {
                pmsgCoalesce = *ppmsgT;
            } else {
                if ((*ppmsgT)->id != idCoalesce) {
                    pmsgCoalesce = NULL;
                }
            }
        }

        ppmsgT = &((*ppmsgT)->pmsgNext);
    }
   
    // If coalesce message found, mark it as a message to be coalesced. The
    // game will decide what to do with it.
    
    if (pmsgCoalesce != NULL) {
        pmsgCoalesce->ff |= kfMsgCoalesce;
    }
    
    // Put new message at the end
    pmsgNew->pmsgNext = NULL;
    *ppmsgT = pmsgNew;

    ss_->WakeUp();
}

void MessageQueue::PostDelayed(Message *pmsg, long ct, long ctBoost) {
    if (stop_) {
        return;
    }

    // Copy message and set timestamp before putting in queue
    Message *pmsgNew = new Message;
    if (pmsgNew == NULL) {
        return;
    }
    *pmsgNew = *pmsg;
    pmsgNew->ctRate = ct;
    pmsgNew->tTrigger = GetTickCount() + ct + ctBoost;
    InsertDelayedMessage(pmsgNew);

    ss_->WakeUp();
}

void MessageQueue::BoostTimer(int id, MessageHandler *handler, long ctBoost) {
    // Find the timer, remove it, adjust trigger, then re-insert it
    CritScope cs(&crit_);
    Message **ppmsg = &pmsgDelayed_;
    while (*ppmsg != NULL) {
        Message *pmsgT = *ppmsg;
        if (pmsgT->id == id && pmsgT->handler == handler) {
            *ppmsg = pmsgT->pmsgNext;
            pmsgT->tTrigger += ctBoost;
            InsertDelayedMessage(pmsgT);
            return;
        }
        ppmsg = &pmsgT->pmsgNext;
    }
}

void MessageQueue::InsertDelayedMessage(Message *pmsg) {
    // Perform insertion sort on timestamp
    CritScope cs(&crit_);
    Message **ppmsg = &pmsgDelayed_;
    for (; (*ppmsg) != NULL; ppmsg = &(*ppmsg)->pmsgNext) {
        if (pmsg->tTrigger < (*ppmsg)->tTrigger) {
            pmsg->pmsgNext = *ppmsg;
            *ppmsg = pmsg;
            break;
        }
    }
    if ((*ppmsg) == NULL) {
        *ppmsg = pmsg;
        pmsg->pmsgNext = NULL;
    }
}

void MessageQueue::Clear(MessageHandler *handler, int id) {
    CritScope cs(&crit_);
    ClearChain(&pmsg_, handler, id);
    ClearChain(&pmsgDelayed_, handler, id);
}

void MessageQueue::ClearDispose(MessageHandler *handler) {
    // Restart loop if a data is deleted, since that can change the list
    bool restart = false;
    do {
        Message **ppmsg = &pmsg_;
        for (; (*ppmsg) != NULL; ppmsg = &(*ppmsg)->pmsgNext) {
            Message *pmsg = *ppmsg;
            if (pmsg->id != kidmDispose || pmsg->handler != NULL) {
                continue;
            }
            DisposeData<void *> *dispose = (DisposeData<void *> *)pmsg->data;
            if (handler == NULL || dispose->data_ == (void *)handler) {
                *ppmsg = pmsg->pmsgNext;
                delete pmsg->data;
                delete pmsg;
                restart = true;
                break;
            }
        }
    } while (restart);
}

void MessageQueue::ClearChain(Message **ppmsg, MessageHandler *handler,
        int id) {
    Message **ppmsgStart = ppmsg;
    while (true) {
        ppmsg = ppmsgStart;
        bool restart = false;
        while (!restart && (*ppmsg) != NULL) {
            Message *pmsg = *ppmsg;
            bool remove = false;
            if (handler == NULL) {
                if (id == kidmNone) {
                    remove = true;
                } else if (pmsg->id == id) {
                    remove = true;
                }
            } else if (pmsg->handler == handler) {
                if (id == -1) {
                    remove = true;
                } else if (pmsg->id == id) {
                    remove = true;
                }
            }
            if (remove) {
                // Unlink the message first, and restart the scan if deleting
                // data, since that can modify the message list.
                *ppmsg = pmsg->pmsgNext;
                if (pmsg->data != NULL) {
                    delete pmsg->data;
                    restart = true;
                }
                delete pmsg;
                continue;
            }
            ppmsg = &(*ppmsg)->pmsgNext;
        }
        if (!restart) {
            break;
        }
    }
}

void MessageQueue::Dispatch(Message *pmsg) {
    if (pmsg->handler != NULL) {
        pmsg->handler->OnMessage(pmsg);
    } else {
#ifdef LOGGING
        if (pmsg->data != NULL) {
            LOG() << "message " << pmsg->id << " NULL handler non-NULL data";
        }
#endif
    }
}

bool MessageQueue::FindDispose(void *pv) {
    for (Message *pmsg = pmsg_; pmsg != NULL; pmsg = pmsg->pmsgNext) {
        if (pmsg->id != kidmDispose) {
            continue;
        }
        DisposeData<void *> *dispose = (DisposeData<void *> *)pmsg->data;
        if (dispose->data_ == pv) {
            return true;
        }
    }
    return false;
}

} // namespace base
