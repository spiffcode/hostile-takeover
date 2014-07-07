import wrap
import config
import models

class Player(wrap.ObjWrap):
    def __init__(self, player_obj, viewing_player_name=''):
        super(Player, self).__init__(player_obj)
        self.viewing_player_name = viewing_player_name
        self.units_built_per_game = [self.per_game(x) for x in self.built_counts_total]

    def get_avatar_url(self):
        if self.avatar_hash:
            return '%s?s=64&h=%s' % (config.AVATAR_URL, self.avatar_hash)
        else:
            return config.ANONYMOUS_AVATAR_URL

    def is_viewer_also_player(self):
        key1 = models.playermodel_key(self.name)
        key2 = models.playermodel_key(self.viewing_player_name)
        return key1.name() == key2.name()

    def get_player_formatted_url(self, base_url):
        if self.is_viewer_also_player():
            return '%s?p=%s' % (base_url, self.viewing_player_name)
        url = '%s?u=%s' % (base_url, self.name)
        if self.viewing_player_name:
            url = '%s&p=%s' % (url, self.viewing_player_name)
        return url

    def get_player_stats_url(self):
        return self.get_player_formatted_url(config.STATS_URL)

    def get_games_url(self):
        return self.get_player_formatted_url(config.GAMES_URL)

    def get_units_built_sum_per_game(self):
        return self.per_game(sum(self.built_counts_total))

    def get_munts_lost_per_game(self):
        return self.per_game(self.munts_lost_total)

    def get_structs_lost_per_game(self):
        return self.per_game(self.structs_lost_total)

    def get_munts_killed_per_game(self):
        return self.per_game(self.munts_killed_total)

    def get_structs_killed_per_game(self):
        return self.per_game(self.structs_killed_total)

    def get_credits_acquired_per_game(self):
        return self.per_game(self.credits_acquired_total)

    def get_credits_consumed_per_game(self):
        return self.per_game(self.credits_consumed_total)

    def get_minutes_per_game(self):
        return self.per_game(self.elapsed_seconds_total / 60.0)

    def get_units_built_per_game(self):
        return self.units_built_per_game

    def per_game(self, count):
        if not self.game_count:
            return 0
        value = float(count) / self.game_count
        if value >= 10:
            return self.thousands(str(int(round(value))))
        if value < 1:
            if value == 0:
                return '0'
            return '%.2f' % value
        if value == int(value):
            return str(int(value))
        return '%.1f' % value

    def thousands(self, s, sep=','):  
        if len(s) <= 3:
            return s  
        return self.thousands(s[:-3], sep) + sep + s[-3:]

def reset_stats(p):
    p.rating = 0
    p.game_count = 0
    p.games_won = 0
    p.last_game_key_name = ''
    p.credits_acquired_total = 0
    p.credits_consumed_total = 0
    p.munts_killed_total = 0
    p.structs_killed_total = 0
    p.munts_lost_total = 0
    p.structs_lost_total = 0
    p.built_counts_total = [0] * config.BUILT_COUNTS_MAX
    p.elapsed_seconds_total = 0
    p.rating_check_utc = 0
    p.last_game_utc = 0
    p.blocked = False
    p.hidden = False

def load_from_name(name, viewing_player_name=''):
    p = models.PlayerModel.get(models.playermodel_key(name))
    if p:
        return Player(p, viewing_player_name)
    return None
