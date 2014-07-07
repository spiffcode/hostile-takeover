import config

from google.appengine.ext import webapp

class SyncError(webapp.RequestHandler):
    def post(self):
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write('not implemented')
