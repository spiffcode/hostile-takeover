import os
import config
import urllib

from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

GAME_NOT_FOUND = 0
PLAYER_NOT_FOUND = 1
PLAYER_NOT_PLAYED_GAME = 2

class Message(webapp.RequestHandler):
    def get(self):
        # Get player name, if any
        player_name = self.request.get('p')
        # Get message id, if any
        id = int(self.request.get('id'))

        # Render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(player_name),
            'selected_tab': config.TAB_NONE,
            'id': id,
        }

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'message.xhtml')
        self.response.out.write(template.render(path, template_values))

def show(handler, id):
    q = 'id=%d' % id
    viewing_player_name = handler.request.get('p')
    if viewing_player_name:
        q = '%s&p=%s' % (q, urllib.quote_plus(viewing_player_name))
    handler.redirect('%s?%s' % (config.MESSAGE_URL, q))
