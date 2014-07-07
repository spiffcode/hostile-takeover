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

class PlayerDetail(basehandler.BaseHandler):
    def get(self):
        # Requires an authenticated user with proper access rights
        account = accounts.account()
        if not account or not account.SEE_PLAYER_INFO_ACCESS_RIGHT:
            self.redirect(users.create_logout_url(self.request.uri))

        # Get args
        self.get_args()

        # Query for results
        keys = []
        if self.user_name:
            keys.append('player: %s' % self.user_name)
        if self.did:
            keys.append('did: %s' % self.did)
        if self.ip_address:
            keys.append('ip: %s' % self.ip_address)
        if len(keys) != 0:
            query_string = '%s' % ', '.join(keys)
        else:
            query_string = 'all'

        # If json request, perform that response
        if self.request.get('j'):
            self.response.headers['Content-Type'] = 'text/plain'
            data = dict(query_string=query_string,tables=self.collect_tables())
            self.response.out.write(json.dumps(data))
            return

        # Calc all-url
        if self.player_name:
            all_url = '%s?p=%s&c=%d' % (config.PLAYERDETAIL_URL, self.player_name, self.count)
        else:
            all_url = '%s?c=%d' % (config.PLAYERDETAIL_URL, self.count)

        # Calc count-url
        if self.player_name:
            if self.q:
                count_url = '%s?p=%s&q=%s' % (config.PLAYERDETAIL_URL, self.player_name, self.q)
            else:
                count_url = '%s?p=%s' % (config.PLAYERDETAIL_URL, self.player_name)
        else:
            if self.q:
                count_url = '%s?q=%s' % (config.PLAYERDETAIL_URL, self.q)
            else:
                count_url = '%s?x=1' % config.PLAYERDETAIL_URL

        # Otherwise render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(self.player_name, account),
            'selected_tab': config.TAB_NONE,
            'query_string': query_string,
            'show_all': query_string != 'all',
            'all_url': all_url,
            'count_url': count_url,
            'tables': self.collect_tables(),
            'player_name': self.player_name,
            'count': self.count,
            'show_counts': [ 50, 100, 250, 500, 1000 ],
        }

        self.set_caching_headers(60)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'playerdetail.xhtml')
        self.response.out.write(template.render(path, template_values))

    def get_args(self):
        self.q = self.request.get('q').lower()
        self.player_name = self.request.get('p').lower()
        self.count = int(self.request.get('c', '50'))
        if self.count > MAX_QUERY_COUNT:
            self.count = MAX_QUERY_COUNT

        self.user_name = None
        self.ip_address = None
        self.did = None

        # Is there a query string? If so override other args
        if not self.q:
            return

        # Is an ip address?
        if re.search(r'^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$', self.q):
            self.ip_address = self.q
            return

        # Is a did?
        if len(self.q) == 32 and re.search(r'^[a-f0-9]+$', self.q):
            self.did = self.q
            return

        # Otherwise treat as a user name
        self.user_name = self.q

    def get_results(self):
        q = models.PlayerActionModel.all()
        if self.user_name:
            q.filter('player_name', self.user_name)
        if self.did:
            q.filter('did', self.did)
        if self.ip_address:
            q.filter('ip_address', self.ip_address)
        q.order('-time_utc')
        return q.fetch(self.count)

    def collect_tables(self):
        # Perform the query
        results = self.get_results()

        # Sort by field values
        player_names = {}
        dids = {}
        ips = {}
        for r in results:
            if self.user_name:
                if r.player_name and not r.anonymous:
                    if r.player_name in player_names:
                        player_names[r.player_name].append(r)
                    else:
                        player_names[r.player_name] = [r]

            if self.did:
                if r.did:
                    if r.did in dids:
                        dids[r.did].append(r)
                    else:
                        dids[r.did] = [r]

            if self.ip_address:
                if r.ip_address:
                    if r.ip_address in ips:
                        ips[r.ip_address].append(r)
                    else:
                        ips[r.ip_address] = [r]

        # Gather tables
        tables = []

        if self.user_name:
            player_table = {}
            player_table['title'] = 'Player Usage'
            player_table['columns'] = ['Player', 'Last', 'Dids', 'Ips']
            player_table['rows'] = []
            for player_name in sorted(player_names.keys()):
                r = player_names[player_name]
                row = []
                row.append([(player_name, self.get_url(player_name))])
                row.append([(r[0].time_utc, 'time_utc')])
                row.append(self.get_row_dids(r))
                row.append(self.get_row_ips(r))
                player_table['rows'].append(row)
            tables.append(player_table)

        if self.did:
            did_table = {}
            did_table['title'] = 'Did Usage'
            did_table['columns'] = ['Did', 'Last', 'Players', 'Ips']
            did_table['rows'] = []
            for did in sorted(dids.keys()):
                r = dids[did]
                row = []
                row.append([(did, self.get_url(did))])
                row.append([(r[0].time_utc, 'time_utc')])
                row.append(self.get_row_players(r))
                row.append(self.get_row_ips(r))
                did_table['rows'].append(row)
            tables.append(did_table)

        if self.ip_address:
            ip_table = {}
            ip_table['title'] = 'Ip Usage'
            ip_table['columns'] = ['Ip', 'Last', 'Players', 'Dids']
            ip_table['rows'] = []
            for ip in sorted(ips.keys()):
                r = ips[ip]
                row = []
                row.append([(ip, self.get_url(ip))])
                row.append([(r[0].time_utc, 'time_utc')])
                row.append(self.get_row_players(r))
                row.append(self.get_row_dids(r))
                ip_table['rows'].append(row)
            tables.append(ip_table)

        history_table = {}
        history_table['title'] = 'History Table'
        history_table['columns'] = ['Time', 'Player', 'Did', 'Ip', 'Action']
        history_table['rows'] = []
        for r in results:
            row = []
            row.append([(r.time_utc, 'time_utc')])
            row.append([(r.player_name, self.get_url(r.player_name))])
            row.append([(r.did, self.get_url(r.did))])
            row.append([(r.ip_address, self.get_url(r.ip_address))])
            row.append([self.get_action_field(r)])
            history_table['rows'].append(row)
        tables.append(history_table)

        return tables

    def get_row_players(self, actions):
        players = []
        for r in actions:
            if not r.player_name or r.anonymous:
                continue
            players.append(r.player_name)
        results = []
        for player in list(set(players)):
            results.append((player, self.get_url(player)))
        return results

    def get_row_dids(self, actions):
        dids = []
        for r in actions:
            if not r.did:
                continue
            dids.append(r.did)
        results = []
        for did in list(set(dids)):
            results.append((did, self.get_url(did)))
        return results

    def get_row_ips(self, actions):
        ips = []
        for r in actions:
            if not r.ip_address:
                continue
            ips.append(r.ip_address)
        results = []
        for ip in list(set(ips)):
            results.append((ip, self.get_url(ip)))
        return results

    def get_url(self, q):
        if not q:
            return ''
        if self.player_name:
            return '%s?p=%s&q=%s&c=%d' % (config.PLAYERDETAIL_URL,
                    self.player_name, urllib.quote_plus(q),
                    self.count)
        else:
            return '%s?q=%s&c=%d' % (config.PLAYERDETAIL_URL,
                    urllib.quote_plus(q), self.count)

    def get_action_field(self, action):
        url = ''
        a = json.loads(action.action)
        if a['action'] == 'game':
            if self.player_name:
                url = '%s?p=%s&g=%s' % (config.GAMEDETAIL_URL,
                        self.player_name, a['key'])
            else:
                url = '%s?g=%s' % (config.GAMEDETAIL_URL, a['key'])
        return a['action'], url

def save(player_name, anonymous, did, ip, action, utc=int(time.time())):
    try:
        a = models.PlayerActionModel()
        a.player_name = player_name.lower()
        a.anonymous = anonymous
        a.did = did
        a.ip_address = ip
        a.action = json.dumps(action)
        a.time_utc = utc
        a.put()
    except:
        pass
