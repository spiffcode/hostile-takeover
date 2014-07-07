#include "game/ht.h"

namespace wi {

Transport::Transport() {
    m_ptcb = NULL;
    m_plcb = NULL;
    m_prcb = NULL;
    m_pgcb = NULL;
}

ITransportCallback *Transport::SetCallback(ITransportCallback *ptcb) {
    ITransportCallback *old = m_ptcb;
    m_ptcb = ptcb;
    return old;
}

ITransportCallback *Transport::GetCallback() {
    return m_ptcb;
}

IGameCallback *Transport::SetGameCallback(IGameCallback *pgcb) {
    IGameCallback *old = m_pgcb;
    m_pgcb = pgcb;
    return old;
}

IGameCallback *Transport::GetGameCallback() {
    return m_pgcb;
}

} // namespace wi
