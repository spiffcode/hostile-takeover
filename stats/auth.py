import models
import config
import time
import base64
import random
from hashlib import md5
import serverinfo
import playerdetail

from google.appengine.ext import webapp
from google.appengine.ext.webapp.util import run_wsgi_app

# 24 hours
#COOKIE_TIMEOUT_SECONDS = 60*60*24

# 1 hour (so accounts can be disabled more easily)
COOKIE_TIMEOUT_SECONDS = 60*60

class AuthUser(webapp.RequestHandler):
    def get(self):
        self.response.headers['Content-Type'] = 'text/plain'
        username = 'anonymous'
        password = ''
        t = None
        try:
            username,password,did = self.get_username_password_did()
            if self.authenticate(username, password, did):
                t = self.generate_token(username, config.AUTH_GOOD_SECRET)
                self.save_action(username, did)
            else:
                t = self.generate_token(username, config.AUTH_BAD_SECRET)
        except:
            t = self.generate_token(username, config.AUTH_BAD_SECRET)
        self.response.out.write(t)

    def get_username_password_did(self):
        d = base64.b64decode(self.request.get('a'))
        username = d[0:d.find('\0')]
        password = d[d.find('\0')+1:]
        did = self.request.get('d')
        return username,password,did

    def generate_token(self, username, secret):
        a = {}
        a['u'] = base64.b64encode(username)
        a['c'] = random.randint(1000, 65535)
        a['t'] = int(time.time()) + COOKIE_TIMEOUT_SECONDS
        s = ''.join(a.__str__().split(' ')).replace("'",'"')
        m = md5(s + secret)
        return base64.b64encode('[%s,"%s"]' % (s, m.hexdigest()))

    def authenticate(self, username, password, did):
        m = models.PlayerModel.get_by_key_name('k' + username.lower())
        if m == None or m.blocked or not did:
            return False
        return m.password.lower() == md5(password).hexdigest().lower()

    def save_action(self, username, did):
        try:
            player_name = username
            anonymous = False
            ip = self.request.remote_addr
            action = dict(action='auth')
            playerdetail.save(player_name, anonymous, did, ip, action)
        except:
            pass
