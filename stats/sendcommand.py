import os
import models
import config
import time
from hashlib import md5
import json

import serverinfo

from google.appengine.ext import webapp

"""
{
    "info": {
        "name": "<name>",
        "start_utc": <long>
    },
    "command": {
        "command": "<command name>",
        "<arg0 name>": "<arg0 value>",
        "<arg1 name>": "<arg1 value>"
        // etc
    }
}
"""

class SendCommand(webapp.RequestHandler):
    def post(self):
        hash = self.request.body[:32]
        j = self.request.body[32:]
        m = md5(json + config.SENDCOMMAND_SECRET)
        if m.hexdigest() == hash:
            c = json.loads(j)
            serverinfo.ServerInfo.send_command(c['info'],
                    json.dumps(c['command']))
            if config.is_debug():
                self.response.headers['Content-Type'] = 'text/plain'
                self.response.out.write('ok')
        else:
            if config.is_debug():
                self.response.headers['Content-Type'] = 'text/plain'
                self.response.out.write('not ok')
                
