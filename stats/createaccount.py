import os, StringIO, traceback
from hashlib import md5

import config, avatar, player, models

from google.appengine.ext import db
from google.appengine.api import images
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template

kErrNone = 0
kErrPlayerNameInvalid = 1
kErrPasswordInvalid = 2
kErrPasswordMismatch = 3
kErrAccountExists = 4
kErrAvatarInvalid = 5
kErrUnexpectedError = 6
kErrAccountNotFound = 7
kErrIncorrectPassword = 8

error_strings = {
    kErrNone : '',
    kErrPlayerNameInvalid : "Player names must be between 3 and 31 characters, and not contain spaces or the characters &quot;, &amp;, &lt;, or &gt;.",
    kErrPasswordInvalid : "Passwords must be between 5 and 31 characters.",
    kErrPasswordMismatch : "Both password fields must match.",
    kErrAccountExists : "Account with this name already exists.",
    kErrAvatarInvalid : "The avatar image was invalid. Please try a different image.",
    kErrUnexpectedError : "Unexpected error creating player account.",
    kErrAccountNotFound : "Account not found.",
    kErrIncorrectPassword : "Incorrect password."
}

class CreateAccount(webapp.RequestHandler):
    def get(self):
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        self.response.headers['Cache-Control'] = 'no-cache'
        path = os.path.join(os.path.dirname(__file__), 'createaccount.xhtml')
        self.response.out.write(template.render(path, dict(is_update=self.is_update())))

    def post(self):
        # Gather args
        args = {}
        for arg in self.request.arguments():
            args[arg] = self.request.get(arg).strip()

        # Perform action
        error_id = self.do_action(args, **args)

        # Render template
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        self.response.headers['Cache-Control'] = 'no-cache'
        if error_id == kErrNone:
            path = os.path.join(os.path.dirname(__file__), 'createaccountsuccess.xhtml')
        else:
            path = os.path.join(os.path.dirname(__file__), 'createaccount.xhtml')
        args['error_msg'] = error_strings[error_id]
        args['is_update'] = self.is_update()
        self.response.out.write(template.render(path, args))

    def is_update(self):
        return False

    def do_action(self, args, player_name, password, password2, avatar_data):
        if not self.is_valid_player_name(player_name):
            return kErrPlayerNameInvalid
        if not self.is_valid_password(password):
            return kErrPasswordInvalid
        if password != password2:
            return kErrPasswordMismatch
        p = models.PlayerModel.get(models.playermodel_key(player_name))
        if p:
            return kErrAccountExists
        avatar_data_new = avatar.prepare_avatar(avatar_data) if avatar_data else None
        if avatar_data and not avatar_data_new:
            return kErrAvatarInvalid
        p = self.create_player(player_name, password, avatar_data_new)
        if not p:
            return kErrUnexpectedError
        args['player'] = player.Player(p)
        return kErrNone

    def is_valid_player_name(self, player_name):
        if not player_name:
            return False
        for ch in " \"&<>":
            if player_name.find(ch) >= 0:
                return False
        if len(player_name) < 3 or len(player_name) > 31:
            return False
        return True

    def is_valid_password(self, password):
        if not password:
            return False
        if len(password) < 5 or len(password) > 31:
            return False
        return True

    def get_avatar_hash(self, avatar_data):
        if avatar_data:
            return md5(avatar_data).hexdigest().lower()
        return ''

    def create_player(self, player_name, password, avatar_data):
        avatar_hash = self.get_avatar_hash(avatar_data)
        try:
            p = db.run_in_transaction(self.add_player_txn, player_name, password, avatar_hash)
            if p and avatar_data:
                avatar.save_avatar(avatar_hash, avatar_data)
            return p
        except:
            obj = models.PlayerUpdateFailed()
            obj.name = player_name
            obj.password = password
            obj.avatar_hash = avatar_hash
            io = StringIO.StringIO()
            traceback.print_exc(file=io)
            io.seek(0)
            obj.backtrace = io.read()
            io.close()
            obj.put()
            return None

    def add_player_txn(self, player_name, password, avatar_hash):
        # Player shouldn't exist yet
        p = models.PlayerModel.get(models.playermodel_key(player_name))
        if p:
            return None
        player_key = models.playermodel_key(player_name)
        p = models.PlayerModel(key_name=player_key.name())
        p.name = player_name
        p.password = md5(password).hexdigest().lower()
        p.avatar_hash = avatar_hash
        p.put()
        return p

