from hashlib import md5
import basehandler
import serverinfo
import config

from google.appengine.api import users
from google.appengine.ext.webapp import template

class Command(basehandler.BaseHandler):
    def have_access(self):
        return users.is_current_user_admin()

    def get(self):
        if not self.have_access():
            self.redirect(users.create_logout_url(config.ADMIN_URL))
            return
        self.finish_get(self.get_template_values())

    def post(self):
        if not users.is_current_user_admin():
            self.redirect(users.create_logout_url(config.ADMIN_URL))
            return
        infos = self.get_serverinfos()
        selected = []
        for info in infos:
            if self.request.arguments().__contains__(info['hash']):
                selected.append(info)
        self.finish_post(selected, self.get_template_values())

    def get_template_values(self):
        template_values = {
            'tabs': config.get_tabs(''),
            'selected_tab': config.TAB_NONE,
            'serverinfos': self.get_serverinfos(),
            'refresh_url': self.request.uri,
            'admin_url': config.ADMIN_URL,
            'logout_url': users.create_logout_url(config.ADMIN_URL),
        }
        return template_values

    def get_serverinfos(self):
        infos = []
        for info in serverinfo.ServerInfo.get_serverinfo()['infos']:
            server = {}
            server['hash'] = md5('%s-%s' % (info['name'], info['start_utc'])).hexdigest()
            server['name'] = info['name']
            server['start_utc'] = info['start_utc']
            server['props'] = ['%s: %s' % (key, info[key]) for key in info.keys()]
            infos.append(server)
        return infos
