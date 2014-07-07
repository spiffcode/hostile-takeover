#ifndef __CREATEROOMFORM_H__
#define __CREATEROOMFORM_H__

#include "game/ht.h"
#include <string>

namespace wi {

const int knCreateRoomAskNone = 0;
const int knCreateRoomAskRoomname = 1;
const int knCreateRoomAskPassword = 2;

class CreateRoomForm : public ShellForm {
public:
    CreateRoomForm();
    ~CreateRoomForm();

    static bool DoForm(char *roomname, int cbRoomname, char *password,
            int cbPassword);
    void GetRoomname(char *roomname, int cb);
    void GetPassword(char *password, int cb);

    // Form callbacks
	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow,
            Sfx sfxHide = ksfxGuiFormHide);
    virtual void OnControlSelected(word idc);
    virtual bool OnFilterEvent(Event *pevt);

private:
    void HideShowPasswordFields();

    int ask_;
    std::string roomname_;
    std::string password_;
};

} // namespace wi
#endif // __CREATEROOMFORM_H__
