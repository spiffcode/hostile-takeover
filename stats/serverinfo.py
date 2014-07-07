import os
import models
import config
import time
from hashlib import md5
import playerdetail
import json

from google.appengine.runtime.apiproxy_errors import CapabilityDisabledError
from google.appengine.ext import webapp
from google.appengine.api import memcache
from google.appengine.api import urlfetch

"""cached json format:
{
    "updated_utc": <long>,
    "expires_utc": <long>,
    "infos": [
        {
            "sort_key", <int>,
            "name": "<servername>",
            "location": "<location>",
            "address": "<host:port>",
            "protocol": "<version>",
            "status": "status",
            "expires_utc": <long>,
            "start_utc": <long>,
            "player_count": <int>,
            "type", "beta|production",
        }
    ]
}
"""

SERVERINFO_KEY = 'serverinfo_info'

class ServerInfo(webapp.RequestHandler):
    def get(self):
        # Get did, check for banned dids
        ip = self.request.remote_addr
        did = self.request.get('d')
        if did and self.is_banned_did(did):
            self.save_action(did, ip, True)
            return

        # Check for banned ips
        if ip and self.is_banned_ip(ip):
            self.save_action(did, ip, True)
            return

        # Not banned
        self.save_action(did, ip, False)

        # Attempt to retrieve serverinfo from cache
        j = memcache.get(SERVERINFO_KEY)
        if not j:
            j = self.gen_json()

        # Return response
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write(j)

    def post(self):
        # Update server info
        j = self.request.body[32:]
        info = self.validate_request()
        if not info:
            if config.is_debug():
                self.response.set_status(400, 'not valid')
            return

        # Update the model.
        key = models.serverinfomodel_key(info['name'], info['start_utc'])
        obj = models.ServerInfoModel.get(key)
        if not obj:
            obj = models.ServerInfoModel(key_name=key.name())

        # Save the command
        command = obj.command

        obj.json = j
        obj.command = ''
        obj.expires_utc = info['expires_utc']
        obj.put()

        # Update the cache now rather than just invalidating, so the latency
        # isn't on a client request
        self.gen_json()

        # Return command if there is one
        if command:
            # Use this mime type for simplicity, and so GAE doesn't strip
            # out Content-Length
            self.response.headers['Content-Type'] = 'binary/octet-stream'
            self.response.headers['Content-Length'] = len(command)
            self.response.out.write(command)

    def validate_request(self):
        # Validate hash
        hash = self.request.body[:32]
        j = self.request.body[32:]
        m = md5(j + config.SERVERINFO_SECRET)
        if m.hexdigest() != hash:
            return None

        # Make sure json parses
        try:
            j = json.loads(j)
        except:
            return None

        # Make sure it hasn't expired
        if j['expires_utc'] <= long(time.time()):
            return None
        return j

    def gen_json(self):
        info = ServerInfo.get_serverinfo()
        j = json.dumps(info)
        memcache.set(SERVERINFO_KEY, j, info['expires_utc'])
        return j

    def save_action(self, did, ip, banned):
        try:
            p = self.request.get('p')
            anon = False if p else True
            d = dict(action='serverinfo', banned=banned)
            playerdetail.save(p, anon, did, ip, d)
        except:
            pass

    @staticmethod
    def get_serverinfo():
        # Separate keepers and ones to delete
        now_utc = long(time.time())
        keep = []
        delete = []
        for obj in models.ServerInfoModel.all():
            if obj.expires_utc < now_utc:
                delete.append(obj)
            else:
                keep.append(obj)

        # Delete the ones to delete
        # During app engine maintenance, CapabilityDisabledError
        # is thrown for writes. Keep infos through maintenance.
        try:
            for obj in delete:
                obj.delete()
        except CapabilityDisabledError:
            keep.extend(delete)

        # Collect json of keepers and calc expires
        infos = []
        expires_utc = 0
        for obj in keep:
            infos.append(json.loads(obj.json))
            if expires_utc == 0:
                expires_utc = obj.expires_utc
            else:
                if obj.expires_utc != 0 and obj.expires_utc < expires_utc:
                    expires_utc = obj.expires_utc

        # Sort because the client will display in the order received
        infos.sort(lambda x, y: cmp(x['sort_key'], y['sort_key']))

        # Note expires_utc = 0 is the same as memcache forever, otherwise
        # memcache takes it as an unix epoc time. Forever is ok, since updating
        # a server entity will cause the cache to be updated.
        info = {}
        info['updated_utc'] = now_utc
        info['expires_utc'] = expires_utc
        info['infos'] = infos
        return info

    @staticmethod
    def send_command(info, command):
        # Update the model with the command
        key = models.serverinfomodel_key(info['name'], info['start_utc'])
        obj = models.ServerInfoModel.get(key)
        if obj:
            obj.command = command
            obj.put()

    def is_banned_ip(self, ip):
        return False

    def is_banned_did(self, did):
        return False
