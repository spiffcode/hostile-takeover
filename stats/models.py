import config

from google.appengine.ext import db

# +++

class PlayerModel(db.Model):
    name = db.StringProperty()
    password = db.StringProperty()
    rating = db.IntegerProperty(default = 0)
    game_count = db.IntegerProperty(default = 0)
    games_won = db.IntegerProperty(default = 0)
    avatar_hash = db.StringProperty()
    last_game_key_name = db.StringProperty()
    credits_acquired_total = db.IntegerProperty(default = 0)
    credits_consumed_total = db.IntegerProperty(default = 0)
    munts_killed_total = db.IntegerProperty(default = 0)
    structs_killed_total = db.IntegerProperty(default = 0)
    munts_lost_total = db.IntegerProperty(default = 0)
    structs_lost_total = db.IntegerProperty(default = 0)
    built_counts_total = db.ListProperty(int, default = [0] * config.BUILT_COUNTS_MAX)
    elapsed_seconds_total = db.IntegerProperty(default = 0)
    rating_check_utc = db.IntegerProperty(default = 0)
    last_game_utc = db.IntegerProperty(default = 0)
    blocked = db.BooleanProperty(default = False)
    hidden = db.BooleanProperty(default = False)
    history = db.TextProperty(default = '[]')
    sequence_number = db.IntegerProperty(default = 0)

def playermodel_key_from_key_name(key_name):
    return db.Key.from_path("PlayerModel", key_name)

def playermodel_key(name):
    return db.Key.from_path("PlayerModel", 'k' + name.lower())

# +++

class PlayerUpdateFailed(db.Model):
    name = db.StringProperty()
    password = db.StringProperty()
    avatar_hash = db.StringProperty()
    date = db.DateTimeProperty(auto_now_add=True)
    backtrace = db.TextProperty()

# +++

class AvatarModel(db.Model):
    hash = db.StringProperty()
    content = db.BlobProperty()

def avatarmodel_key(hash):
    return db.Key.from_path("AvatarModel", 'k' + hash.lower())

# +++

class GameStatsModel(db.Model):
    date = db.DateTimeProperty(auto_now_add=True)
    player_key_names = db.StringListProperty()
    mission_title = db.StringProperty()
    start_utc = db.IntegerProperty()
    end_utc = db.IntegerProperty()
    old_ratings = db.TextProperty()
    dids = db.TextProperty()
    json = db.TextProperty()

def gamestatsmodel_key_from_name(key_name):
    return db.Key.from_path("GameStatsModel", key_name)

def gamestatsmodel_key(server_id, server_start, gameid):
    key_name = 'k%02x-%08x-%08x' % (server_id, server_start, gameid)
    return gamestatsmodel_key_from_name(key_name)

# +++

class GameStatsFailed(db.Model):
    json = db.TextProperty()
    date = db.DateTimeProperty(auto_now_add=True)
    backtrace = db.TextProperty()

# +++

class ServerInfoModel(db.Model):
    json = db.TextProperty()
    command = db.TextProperty()
    expires_utc = db.IntegerProperty()

def serverinfomodel_key(name, start_utc):
    key_name = 'k%s-%u' % (name.lower(), start_utc)
    return db.Key.from_path('ServerInfoModel', key_name)

# +++

class AccountModel(db.Model):
    nickname = db.StringProperty()
    access_rights = db.IntegerProperty()

def accountmodel_key(user_id):
    return db.Key.from_path("AccountModel", 'k' + user_id.lower())

# +++

class PlayerActionModel(db.Model):
    player_name = db.StringProperty()
    anonymous = db.BooleanProperty()
    did = db.StringProperty()
    ip_address = db.StringProperty()
    action = db.StringProperty()
    time_utc = db.IntegerProperty()

# +++

class AdminActionModel(db.Model):
    admin_user = db.StringProperty()
    ip_address = db.StringProperty()
    action = db.TextProperty()
    time_utc = db.IntegerProperty()
