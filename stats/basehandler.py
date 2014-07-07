import datetime

from google.appengine.ext import webapp

class BaseHandler(webapp.RequestHandler):
    def set_caching_headers(self, seconds_valid, public=False):
        utcnow = datetime.datetime.utcnow()
        expires_date = utcnow + datetime.timedelta(seconds=seconds_valid)
        expires_str = expires_date.strftime('%a, %d %b %Y %H:%M:%S GMT')
        self.response.headers['Expires'] = expires_str
        if public:
            cachectl_str = 'public, max-age=%d' % seconds_valid
        else:
            cachectl_str = 'max-age=%d' % seconds_valid
        self.response.headers['Cache-Control'] = cachectl_str
