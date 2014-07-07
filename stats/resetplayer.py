import os
import config
import player
import models
import basehandler

from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

class ResetPlayer(basehandler.BaseHandler):
    def get(self):
        if not users.is_current_user_admin():
            self.redirect(users.create_logout_url(config.ADMIN_URL))
            return

        template_values = {
            'tabs': config.get_tabs(''),
            'selected_tab': config.TAB_NONE,
        }

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'resetplayer.xhtml')
        self.response.out.write(template.render(path, template_values))

    def post(self):
        if not users.is_current_user_admin():
            self.redirect(users.create_logout_url(config.ADMIN_URL))
            return

        self.response.headers['Content-Type'] = 'text/plain'
        player_name = self.request.get('u')
        if not player_name:
            self.response.out.write('no player name entered')
            return

        def txn():
            p = models.PlayerModel.get(models.playermodel_key(player_name))
            if not p:
                return False
            player.reset_stats(p)
            p.put()
            return True

        if not db.run_in_transaction(txn):
            self.response.out.write('cannot find player in database')
        else:
            self.response.out.write('%s\'s stats reset.' % player_name)
