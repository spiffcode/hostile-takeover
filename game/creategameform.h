#ifndef __CREATEGAMEFORM_H__
#define __CREATEGAMEFORM_H__

#include "game/ht.h"
#include "game/chatter.h"
#include "game/loginhandler.h"
#include "base/sigslot.h"
#include <string>

namespace wi {

class CreateGameForm : public ShellForm, ITransportCallback,
        public base::has_slots<> {
public:
    CreateGameForm(LoginHandler& handler, const PackId *ppackidFind,
            Chatter& chatter, GameParams *prams);
    virtual ~CreateGameForm();
   
    static bool DoForm(LoginHandler& handler, const PackId *ppackidFind,
            Chatter& chatter, GameParams *params); 
    virtual bool DoModal(int *pnResult = NULL);
    virtual bool OnFilterEvent(Event *pevt);
   
private:
    void OnChatBlink(bool on);
    int IndexFromMissionType(MissionType mt);    
    MissionType MissionTypeFromIndex(int i);    
    void SwitchToMissionType(MissionType mt);    
    MissionType InitLists(int iMissionSelect);
    void OnControlSelected(word idc);    
    void OnControlNotify(word idc, int nNotify);    
    void UpdateLabels();

    // TransportCallback
    void OnStatusUpdate(char *pszStatus) { }
    void OnConnectionClose();
    void OnShowMessage(const char *message);

    LoginHandler& handler_;
    Chatter& chatter_;
    MissionType m_mt;
    GameParams *m_prams;
    MissionIdentifier m_miidFind;
    long m_tGameSpeed;
    MissionList *m_pml;
    ListControl *m_aplstc[2];
    std::string message_;
};

} // namespace wi

#endif // __CREATEGAMEFORM_H__
