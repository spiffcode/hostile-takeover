import os
import config
import basehandler
import models
import accounts
import urllib
import admin

from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

class AdjustScore(basehandler.BaseHandler):
    def get(self):
        account = self.has_access()
        if not account:
            self.redirect(users.create_login_url(self.request.uri))
            return
        type = self.request.get('t').lower()
        u = self.request.get('u').lower()
        player_name = self.request.get('p').lower()
        message = ''

        # if no type specified, there is no query
        rating = 0
        next_type = ''
        if not type:
            # The next request will be a query for a player name's score
            next_type = 'q'

        # Validate the user. Is this a query or set score request?
        p = None
        if type == 'q' or type == 's':
            # This is a query for a user. Make sure the user is valid
            if not u:
                message = 'No player name entered!'
                type = ''
                next_type = 'q'
            else:
                p = models.PlayerModel.get(models.playermodel_key(u))
                if not p:
                    message = 'Could not find player %s! Please try again:' % u
                    u = ''
                    type = ''
                    next_type = 'q'

        # Step through the states
        rating = 0
        reason = ''
        if type == 'q':
            # The next request will be for changing a score
            next_type = 's'
            rating = p.rating

        if type == 's':
            # Is the new rating valid?
            rating = self.request.get('s')
            reason = self.request.get('r')
            success = True
            try:
                rating = int(rating)
            except:
                success = False
                
            if not success or rating >= 3000 or rating < 0:
                message = '"%s" is an invalid score. Please try again:' % rating
                rating = p.rating
                type = 'q'
                next_type = 's'

            elif not reason:
                message = 'Must enter a reason. Please try again:'
                rating = p.rating
                type = 'q'
                next_type = 's'

            else:
                # Record this action
                d = dict(action='adjust_score', player_name=u, old_rating=p.rating, new_rating=rating, reason=reason)
                admin.save_action(account.name, self.request.remote_addr, d)

                p.rating = rating
                p.put()
                message = 'Successfully set the score of player %s to %s, reason: %s.' % (u, rating, reason)
                type = ''
                next_type = 'q'

        template_values = {
            'tabs': config.get_tabs(player_name, account),
            'selected_tab': config.TAB_NONE,
            'form_url': config.ADJUSTSCORE_URL,
            'message': message,
            'player_name': player_name,
            'u': u,
            'rating': rating,
            'type': type,
            'reason': reason,
            'next_type': next_type
        }

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'adjustscore.xhtml')
        self.response.out.write(template.render(path, template_values))

    def has_access(self):
        # Requires an authenticated user with proper access rights
        account = accounts.account()
        if account and account.ADJUST_SCORE_ACCESS_RIGHT:
            return account
        return None
