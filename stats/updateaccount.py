import os, StringIO, traceback
from hashlib import md5

import createaccount, config, avatar, player, models

from google.appengine.ext import db
from google.appengine.api import images
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template

class UpdateAccount(createaccount.CreateAccount):
    def do_action(self, args, player_name, password, password2, avatar_data):
        if not self.is_valid_player_name(player_name):
            return createaccount.kErrPlayerNameInvalid
        p = models.PlayerModel.get(models.playermodel_key(player_name))
        if not p:
            return createaccount.kErrAccountNotFound
        if p.password != md5(password).hexdigest().lower():
            return createaccount.kErrIncorrectPassword
        if password2 and not self.is_valid_password(password2):
            return createaccount.kErrPasswordInvalid

        # If no avatar was offered, don't change the existing one.
        avatar_data_new = avatar.prepare_avatar(avatar_data) if avatar_data else None
        if avatar_data and not avatar_data_new:
            return createaccount.kErrAvatarInvalid

        # Perform transaction
        avatar_hash = self.get_avatar_hash(avatar_data_new)
        db.run_in_transaction(self.update_player_txn, p, password2, avatar_hash)
        if avatar_data_new:
            avatar.save_avatar(avatar_hash, avatar_data_new)

        # Set up args for the template
        args['player'] = player.Player(p)
        args['password'] = password2 if password2 else '<unchanged>'
        return createaccount.kErrNone

    def update_player_txn(self, p, password, avatar_hash):
        if password:
            p.password = md5(password).hexdigest().lower()
        if avatar_hash:
            p.avatar_hash = avatar_hash
        p.put()

    def is_update(self):
        return True
