import logging
import models
from google.appengine.api import users

rights = dict(
    BLOCK_PLAYER_ACCESS_RIGHT = 0x00000001,
    RESET_PLAYER_ACCESS_RIGHT = 0x00000002,
    SEND_CHAT_ACCESS_RIGHT = 0x00000004,
    DRAIN_ACCESS_RIGHT = 0x00000008,
    ADJUST_SCORE_ACCESS_RIGHT = 0x00000010,
    SEE_PLAYER_INFO_ACCESS_RIGHT = 0x00000020,
    ADD_ACCOUNT_ACCESS_RIGHT = 0x00000040,
    REMOVE_ACCOUNT_ACCESS_RIGHT = 0x00000080,
    ADMIN_LINK_ACCESS_RIGHT = 0x00000100,
    ADMIN_LOG_ACCESS_RIGHT = 0x00000200,
    HIDE_PLAYER_ACCESS_RIGHT = 0x00000400,
)

BASE_ADMIN_ACCESS_RIGHTS = \
    rights['SEE_PLAYER_INFO_ACCESS_RIGHT'] | \
    rights['BLOCK_PLAYER_ACCESS_RIGHT'] | \
    rights['ADMIN_LINK_ACCESS_RIGHT'] | \
    rights['ADJUST_SCORE_ACCESS_RIGHT'] | \
    rights['ADMIN_LOG_ACCESS_RIGHT'] | \
    rights['HIDE_PLAYER_ACCESS_RIGHT']

BASE_ADMIN_ACCESS_RIGHTS_PLUS = \
    BASE_ADMIN_ACCESS_RIGHTS | \
    rights['SEND_CHAT_ACCESS_RIGHT'] | \
    rights['DRAIN_ACCESS_RIGHT'] 
    
class Account(object):
    def __init__(self, account):
        self.__dict__['account'] = account

    def __getattr__(self, name):
        if name == 'name':
            return self.account.nickname
        try:
            return (self.account.access_rights & rights[name]) != 0
        except:
            raise AttributeError

    def __setattr__(self, name, value):
        try:
            self.account.access_rights &= ~rights[name]
            if value:
                self.account.access_rights |= rights[name]
        except:
            raise AttributeError

    def save(self):
        self.account.put()

def load(user):
    logging.info('nickname=' + user.nickname())
    key = models.accountmodel_key(user.nickname())
    if users.is_current_user_admin():
        account = models.AccountModel(key_name=key.name())
        account.nickname = user.nickname()
        account.access_rights = -1
    else:
        account = models.AccountModel.get(key)
    if account:
        return Account(account)
    return None

def account():
    user = None
    try:
        user = users.get_current_user()
    except:
        user = None
    if not user:
        return None
    return load(user)
