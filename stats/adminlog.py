import os
import urllib
import config
import models
import basehandler
import accounts
import time
import re
import json

from google.appengine.api import users
from google.appengine.ext.webapp import template

MAX_QUERY_COUNT = 2000

class AdminLog(basehandler.BaseHandler):
    def get(self):
        # Requires an authenticated user with proper access rights
        account = accounts.account()
        if not account or not account.ADMIN_LOG_ACCESS_RIGHT:
            self.redirect(users.create_logout_url(self.request.uri))

        # Get args
        self.get_args()

        # If json request, perform that response
        if self.request.get('j'):
            self.response.headers['Content-Type'] = 'text/plain'
            data = dict(tables=self.collect_tables())
            self.response.out.write(json.dumps(data))
            return

        # Calc all-url
        if self.player_name:
            all_url = '%s?p=%s&c=%d' % (config.ADMINLOG_URL, self.player_name, self.count)
        else:
            all_url = '%s?c=%d' % (config.ADMINLOG_URL, self.count)

        # Calc count-url
        if self.player_name:
            count_url = '%s?p=%s' % (config.ADMINLOG_URL, self.player_name)
        else:
            count_url = '%s?x=1' % config.ADMINLOG_URL

        # Otherwise render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(self.player_name, account),
            'selected_tab': config.TAB_NONE,
            'all_url': all_url,
            'count_url': count_url,
            'tables': self.collect_tables(),
            'player_name': self.player_name,
            'count': self.count,
            'show_counts': [ 50, 100, 250, 500, 1000 ],
        }

        self.set_caching_headers(60)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'adminlog.xhtml')
        self.response.out.write(template.render(path, template_values))

    def get_args(self):
        self.player_name = self.request.get('p').lower()
        self.count = int(self.request.get('c', '50'))
        if self.count > MAX_QUERY_COUNT:
            self.count = MAX_QUERY_COUNT

    def get_results(self):
        q = models.AdminActionModel.all()
        q.order('-time_utc')
        return q.fetch(self.count)

    def collect_tables(self):
        # Perform the query
        results = self.get_results()

        # Gather tables
        tables = []

        log_table = {}
        log_table['title'] = 'Admin Log'
        log_table['columns'] = ['Admin', 'Time', 'Action', 'Reason', 'Ip']
        log_table['rows'] = []
        for o in results:
            row = []
            row.append([(o.admin_user, '')])
            row.append([(o.time_utc, 'time_utc')])
            if o.action:
                a = json.loads(o.action)
                row.append([self.get_action(a)])
                row.append([self.get_reason(a)])
            else:
                row.append([('', '')])
                row.append([('', '')])
            row.append([(o.ip_address, '')])
            log_table['rows'].append(row)
        tables.append(log_table)

        return tables

    def get_action(self, a):
        if a['action'] == 'adjust_score':
            s = 'adjust score of %s, old: %s, new: %s' % \
                    (a['player_name'], a['old_rating'], a['new_rating'])
            return (s, '')
        if a['action'] == 'hide_player':
            s = 'hide player %s from leaderboard' % a['player_name']
            return (s, '')
        if a['action'] == 'show_player':
            s = 'show player %s on leaderboard' % a['player_name']
            return (s, '')
        if a['action'] == 'block_player':
            s = 'block player %s from logging in' % a['player_name']
            return (s, '')
        if a['action'] == 'unblock_player':
            s = 'unblock player %s from logging in' % a['player_name']
            return (s, '')
        return ('empty', '')

    def get_reason(self, a):
        if 'reason' in a:
            return (a['reason'], '')
        return ('empty', '')
