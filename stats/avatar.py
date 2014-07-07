import models
import datetime
import basehandler

from google.appengine.ext import webapp
from google.appengine.api import images
from google.appengine.api import urlfetch

MAX_AGE_SECONDS = 60 * 60 * 24 * 7 * 52

class AvatarHandler(basehandler.BaseHandler):
    def get(self):
        hash = self.request.get('h')
        if hash == '':
            self.response.set_status(400, 'need hash parameter')
            return
        obj = models.AvatarModel.get(models.avatarmodel_key(hash))
        if not obj:
            self.response.set_status(400, 'avatar not found')
            return
        size = self.request.get('s')
        if size == '' or size == '64':
            self.set_caching_headers(MAX_AGE_SECONDS, public=True)
            self.response.headers['Content-Type'] = 'image/jpeg'
            self.response.out.write(obj.content)
            return
        self.response.set_status(400, 'size %s not supported' % size)

def prepare_avatar(avatar_data):
    # Want 64x64 avatar. For now letterbox a square at full res and scale.
    try:
        img = images.Image(image_data=avatar_data)
        x_left = 0.0
        y_top = 0.0
        x_right = float(img.width)
        y_bottom = float(img.height)
        if img.width > img.height:
            x_left = float(img.width - img.height) / 2
            x_right -= x_left
        else:
            y_top = float(img.height - img.width) / 2
            y_bottom -= y_top
        img.crop(x_left / img.width, y_top / img.height, x_right / img.width, y_bottom / img.height)
        img.resize(64, 64)
        return img.execute_transforms(output_encoding=images.JPEG)
    except:
        return None

def save_avatar(hash, avatar_data):
    q = models.AvatarModel.all(keys_only=True)
    q.filter("hash = ", hash)
    r = q.fetch(1)
    if len(r) == 0:
        avatar_key = models.avatarmodel_key(hash)
        kwds = dict(hash=hash, content=avatar_data)
        models.AvatarModel.get_or_insert(avatar_key.name(), **kwds)
