import os
import config
import basehandler
import models
import accounts
import admin

from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

class HidePlayer(basehandler.BaseHandler):
    def get(self):
        account = self.has_access()
        if not account:
            self.redirect(users.create_login_url(self.request.uri))
            return
        player_name = self.request.get('p').lower()

        template_values = {
            'tabs': config.get_tabs(player_name, account),
            'selected_tab': config.TAB_NONE,
        }

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'hideplayer.xhtml')
        self.response.out.write(template.render(path, template_values))

    def post(self):
        account = self.has_access()
        if not account:
            self.redirect(users.create_logout_url(config.ADMIN_URL))
            return

        self.response.headers['Content-Type'] = 'text/plain'
        player_name = self.request.get('u')
        if not player_name:
            self.response.out.write('no player name entered')
            return

        choice = self.request.get('choice')
        if not choice:
            self.response.out.write('no choice selected')
            return

        reason = self.request.get('r')
        if not reason:
            self.response.out.write('no reason specified')
            return
            
        p = models.PlayerModel.get(models.playermodel_key(player_name))
        if not p:
            self.response.out.write('cannot find player in database')
            return
            
        if choice == 'hide':
            p.hidden = True
        elif choice == 'show':
            p.hidden = False
        else:
            self.response.out.write('choice not hide or show')
            return
        p.put()

        # Record this action
        d = dict(action='hide_player' if choice == 'hide' else 'show_player', player_name=player_name, reason=reason)
        admin.save_action(account.name, self.request.remote_addr, d)

        # Return the response
        self.response.out.write('success. %s is now hidden=%s, reason: %s.' % (player_name, choice == 'hide', reason))

    def has_access(self):
        # Requires an authenticated user with proper access rights
        account = accounts.account()
        if account and account.HIDE_PLAYER_ACCESS_RIGHT:
            return account
        return None
