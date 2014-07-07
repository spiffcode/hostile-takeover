import os
import config
import basehandler

from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

# This page isn't dynamic
MAX_AGE_SECONDS = 60 * 60 * 24

class Search(basehandler.BaseHandler):
    def get(self):
        # Get player name, if any
        player_name = self.request.get('p')

        # Render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(player_name),
            'selected_tab': config.TAB_SEARCH,
            'player_name': player_name,
            'games_url': config.GAMES_URL,
        }

        self.set_caching_headers(MAX_AGE_SECONDS)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'search.xhtml')
        self.response.out.write(template.render(path, template_values))
