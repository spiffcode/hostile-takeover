import os
import time
import models
import config
import basehandler
import accounts
import adminlog
import json

from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template

class Admin(basehandler.BaseHandler):
    def get(self):
        account = accounts.account()
        if not account or not account.ADMIN_LINK_ACCESS_RIGHT:
            self.redirect(users.create_login_url(self.request.uri))
            return
        self.player_name = self.request.get('p').lower()

        # Show admin UI
        template_values = {
            'tabs': config.get_tabs(self.player_name, account),
            'selected_tab': config.TAB_NONE,
            'account': account,
            'adminlog_url': self.get_url(config.ADMINLOG_URL),
            'playerdetail_url': self.get_url(config.PLAYERDETAIL_URL),
            'adjust_score_url': self.get_url(config.ADJUSTSCORE_URL),
            'blockplayer_url': self.get_url(config.BLOCKPLAYER_URL),
            'hideplayer_url': self.get_url(config.HIDEPLAYER_URL),
            'resetplayer_url': self.get_url(config.RESETPLAYER_URL),
            'sendchat_url': self.get_url(config.SENDCHAT_URL),
            'drain_url': self.get_url(config.DRAIN_URL),
            'logout_url': users.create_logout_url(self.request.uri),
        }

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'admin.xhtml')
        self.response.out.write(template.render(path, template_values))

    def get_url(self, url):
        if self.player_name:
            return '%s?p=%s' % (url, self.player_name)
        else:
            return url

def save_action(admin_user, ip, action, time_utc=int(time.time())):
    try:
        a = models.AdminActionModel()
        a.admin_user = admin_user.lower()
        a.ip_address = ip
        a.action = json.dumps(action)
        a.time_utc = time_utc
        a.put()
    except:
        pass
