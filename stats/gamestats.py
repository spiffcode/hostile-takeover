import models
import player
import config
import playerstat
import wrap
import ratingjob
import serverinfo
import playerdetail
import json
# import logging

import time
import traceback
import StringIO
from google.appengine.ext import db

"""
AddGameStat json:
{
    "server_id": <integer>,
    "server_start": <unsigned integer>,
    "gameid": <integer>,
    "packid_id": <unsigned integer>,
    "packid_hash": "<string>",
    "title": "<string>"
    "filename": "<string>"
    "game_speed": <integer>,
    "min_players": <integer>,
    "max_players": <integer>,
    "start_utc": <unsigned integer>,
    "end_utc": <unsigned integer>,
    "player_stats": [
        {
            "name": "<string>",
            "pid": <integer>,
            "winstats": {
                "side_mask": <integer>,
                "side_mask_allies": <integer>,
                "credits_acquired": <integer>,
                "credits_consumed": <integer>,
                "enemy_munts_killed": <integer>,
                "enemy_structs_killed": <integer>,
                "munts_lost": <integer>,
                "structs_lost": <integer>,
                "unit_counts": [ <kcutMax integers> ],
                "built_counts": [ <kcutMax integers> ],
                "ff": <integer>
            }
        }
    ]
}
"""

# 0: Security Guard
# 1: Rocket Trooper
# 2: Corporate Raider
# 3: SR-98 Eagle
# 4: T-29 Broadsword
# 5: M-18 Hydra
# 6: T-33 Liberator
# 7: G-4 Bullpup
# 8: H-7 Dominion
# 9: Power Generator
# 10: Galaxite Processor
# 11: Galaxite Storage Warehouse
# 12: Human Resource Center
# 13: Vehicle Transport Station
# 14: Surveillance Center
# 15: Research & Development Center
# 16: Headquarters
# 17: Gatling Tower
# 18: Rocket Tower
# 19: Andy
# 20: A-3 Cyclops
# 21: Replicator
# 22: Fox

GAME_RESULT_SCOREABLE = 0
GAME_RESULT_TOO_SHORT = 1
GAME_RESULT_NEED_REGISTERED_OPPONENTS = 2
GAME_RESULT_NO_WINNERS = 3

PLAYER_RESULT_NONE = 0
PLAYER_RESULT_LOSE = 1
PLAYER_RESULT_WIN = 2

kfwsReceivedStats = 0x0001
kfwsWinner = 0x0002
kfwsLoser = 0x0004
kfwsWinChallenger = 0x0008
kfwsComputer = 0x0010
kfwsHuman = 0x0020
kfwsAnonymous = 0x0040
kfwsLocked = 0x0080
kfwsRemovedAtGameStart = 0x0100
kfwsMask = 0x1ff

COMPUTER_DEFAULT_RATING = 1200
ANONYMOUS_DEFAULT_RATING = 1300
PLAYER_DEFAULT_RATING = 1500

DURATION_SECS_MINIMUM = 60 * 3

class GameStats(wrap.DictWrap):
    def __init__(self, json, viewing_player_name='', load_players=False):
        super(GameStats, self).__init__(json.loads(json))
        self.json = json
        self.viewing_player_name = viewing_player_name

        # Prune out non-contributing players
        self.prune_zombie_players()

        # Calc results
        self.init_win_results()

        # Initialize PlayerStats objects
        self.init_player_stats()

        # Create pid->player_stat map
        self.pid_player_stat_map = {}
        for player_stat in self.player_stats:
            self.pid_player_stat_map[player_stat.pid] = player_stat
                
        # Create key->pid map
        self.key_pid_map = {}
        for player_stat in self.player_stats:
            if player_stat.is_user:
                key = models.playermodel_key(player_stat.name)
                self.key_pid_map[key] = player_stat.pid

        # If asked, load and push Player objects into PlayerStat objects
        if load_players:
            self.load_player_objects()

        # Remember teams
        self.teams = self.get_teams()

    def init_player_stats(self):
        # Replace player_stats with PlayerStats objects and sort by side
        player_stats = []
        for player_stat in self.player_stats:
            player_stat = playerstat.PlayerStat(player_stat,
                    self.pid_win_result_map[player_stat.pid])
            player_stats.append(player_stat)
        player_stats.sort(lambda x, y: cmp(x.side, y.side))
        self.player_stats = player_stats

    def load_player_objects(self):
        # Get all the player models at once
        keys = self.key_pid_map.keys()
        objs = models.PlayerModel.get(keys)

        # Append player objects to player_stat objects
        for i in xrange(len(keys)):
            pid = self.key_pid_map[keys[i]]
            player_stat = self.pid_player_stat_map[pid]
            player_stat.player = player.Player(objs[i],
                    self.viewing_player_name)

    def prune_zombie_players(self):
        # Remove players that had no contribution. This removes computer
        # players that were allocated but never used.
        i = 0
        while i < len(self.player_stats):
            if sum(self.player_stats[i].winstats.built_counts) == 0:
                self.player_stats.pop(i)
                continue
            if self.player_stats[i].winstats.ff & kfwsRemovedAtGameStart:
                self.player_stats.pop(i)
                continue
            i += 1

    def get_nonanonymous_human_count(self):
        humans = 0
        for team in self.teams:
            for player_stat in team:
                if (player_stat.winstats.ff & kfwsComputer):
                    continue
                if (player_stat.winstats.ff & kfwsAnonymous):
                    continue
                if (player_stat.winstats.ff & kfwsHuman):
                    humans += 1
        return humans

    def get_game_result(self):
        # Games must run for a minimum time to be scored
        elapsed_secs = self.end_utc - self.start_utc
        if elapsed_secs < DURATION_SECS_MINIMUM:
            return GAME_RESULT_TOO_SHORT;

        # There must be at least one winner
        winner_tids = self.get_winner_tids()
        if len(winner_tids) == 0:
            return GAME_RESULT_NO_WINNERS

        winners_registered = False
        for tid in winner_tids:
            for player_stat in self.teams[tid]:
                if player_stat.winstats.ff & (kfwsComputer | kfwsAnonymous):
                    continue
                winners_registered = True

        losers_registered = False
        for tid in xrange(len(self.teams)):
            if winner_tids.__contains__(tid):
                continue
            for player_stat in self.teams[tid]:
                if player_stat.winstats.ff & (kfwsComputer | kfwsAnonymous):
                    continue
                losers_registered = True

        # For a registered player to win, there must be a registered loser too,
        # and vice versa.
        if not winners_registered or not losers_registered:
            return GAME_RESULT_NEED_REGISTERED_OPPONENTS

        return GAME_RESULT_SCOREABLE
        
    def init_win_results(self):
        self.pid_win_result_map = {}
        winner_tids = self.get_winner_tids()
        for player_stat in self.player_stats:
            tid = self.find_tid(player_stat.pid)
            if winner_tids.__contains__(tid):
                self.pid_win_result_map[player_stat.pid] = PLAYER_RESULT_WIN
            else:
                self.pid_win_result_map[player_stat.pid] = PLAYER_RESULT_LOSE

    def get_winner_tids(self):
        # Players on the same team as winners are winners too
        winner_tids = []
        for tid in xrange(len(self.teams)):
            for player_stat in self.teams[tid]:
                if player_stat.winstats.ff & kfwsWinner:
                    winner_tids.append(tid)
                    break
        return winner_tids

    def get_winner_count(self):
        count = 0
        for value in self.pid_win_result_map.values():
            if value == PLAYER_RESULT_WIN:
                count += 1
        return count

    def get_first_winner_color(self):
        for pid in self.pid_win_result_map.keys():
            if self.pid_win_result_map[pid] == PLAYER_RESULT_WIN:
                return self.pid_player_stat_map[pid].get_side_color()
        return 'black'

    def find_tid(self, pid):
        for tid in xrange(len(self.teams)):
            for player_stat in self.teams[tid]:
                if player_stat.pid == pid:
                    return tid
        return -1

    def get_duration_minutes(self):
        return '%.2f' % ((self.end_utc - self.start_utc) / 60.0)

    def get_duration_seconds(self):
        return self.end_utc - self.start_utc

    def get_game_speed_multiplier(self):
        n = 0
        if self.game_speed != 0:
            n = (8 * 100) / self.game_speed
        return '%d.%dx' % (n / 100, n % 100)

    def get_teams(self):
        # Return a list of player_stat tuples, each tuple representing a team.
        # Most of the time these tuples will have one player in them each,
        # but if there are allies, there may be more. Only check for ally
        # equality, to prune out enemies being on the same team.

        # Calc unique masks.
        unique_masks = []
        for player_stat in self.player_stats:
            ally_mask = player_stat.winstats.side_mask_allies
            if not unique_masks.__contains__(ally_mask):
                unique_masks.append(ally_mask)
            
        # Calc team tuples (of player_stats)
        teams = []
        for mask in unique_masks:
            team = []
            for player_stat in self.player_stats:
                ally_mask = player_stat.winstats.side_mask_allies
                if ally_mask == mask:
                    team.append(player_stat)
            teams.append(tuple(team))
        return teams

    def get_has_multiplayer_teams(self):
        return len(self.teams) != len(self.player_stats)

    def get_game_key(self):
        return models.gamestatsmodel_key(self.server_id, self.server_start,
                self.gameid)

    def get_game_key_name(self):
        return self.get_game_key().name()

    def get_detail_url(self):
        url = '%s?g=%s' % (config.GAMEDETAIL_URL, self.get_game_key_name())
        if self.viewing_player_name:
            url = '%s&p=%s' % (url, self.viewing_player_name)
        return url

    def get_team_ratings(self, win_shares):
        # Players with usernames have default ratings. Anonymous users
        # have a lower rating, so users prefer to play real users.
        # Computer players have an even lower rating.
        # Team rating is a weighted total based on kill contribution
        r = {}
        for tid in xrange(len(self.teams)):
            team_rating = 0
            for player_stat in self.teams[tid]:
                # Win share for this player. If not present,
                # this player had no kill contribution
                win_share = win_shares[player_stat.pid]

                # Add up weighted rating
                if player_stat.is_computer:
                    team_rating += COMPUTER_DEFAULT_RATING * win_share
                    continue
                if player_stat.is_anonymous:
                    team_rating += ANONYMOUS_DEFAULT_RATING * win_share
                    continue
                rating = player_stat.player.rating
                if rating == 0:
                    team_rating += PLAYER_DEFAULT_RATING * win_share
                else:
                    team_rating += rating * win_share
            r[tid] = team_rating
            # logging.info('+++ team: %d rating: %d' % (tid, team_rating))
        return r

    def calc_win_share(self, team, pid, winner_team):
        # A player's win share on a team is the % of units killed by that
        # player. 
        stat_map = {}
        total_killed = 0
        for player_stat in team:
            total_killed += player_stat.winstats.enemy_munts_killed
            total_killed += player_stat.winstats.enemy_structs_killed
            stat_map[player_stat.pid] = player_stat
        pid_killed = 0
        pid_killed += stat_map[pid].winstats.enemy_munts_killed
        pid_killed += stat_map[pid].winstats.enemy_structs_killed
        if total_killed != 0:
            return float(pid_killed) / float(total_killed)

        return 1.0 / len(team)

    def get_ratings(self):
        # Get winner team ids
        winner_tids = self.get_winner_tids()

        # Calc win shares for each pid
        win_shares = {}
        for tid in xrange(len(self.teams)):
            for player_stat in self.teams[tid]:
                win_shares[player_stat.pid] = self.calc_win_share(self.teams[tid], player_stat.pid, winner_tids.__contains__(tid))

        # logging
        for pid in win_shares.keys():
            name = self.pid_player_stat_map[pid].name
            # logging.info('+++ win_share %s: %f' % (name, win_shares[pid]))

        # Get team ratings
        r = self.get_team_ratings(win_shares)

        # Estimate win percentages [0..1] for each team, which indicates a
        # percentage likelihood of a win. Estimate across all teams (this is
        # a bit different than standard ELO which is 2 player only). See:
        # http://en.wikipedia.org/wiki/Elo_rating_system#Mathematical_details
        q = {}
        qt = 0.0
        for tid in r.keys():
            q[tid] = 10**(float(r[tid])/400)
            qt += q[tid]
        e = {}
        for tid in r.keys():
            e[tid] = q[tid] / qt

        # Calc actual win percentages [0..1] for each team.
        s = {}
        for tid in r.keys():
            if winner_tids.__contains__(tid):
                s[tid] = 1.0
            else:
                s[tid] = 0.0

        # Calc per-team actual minus expected rating deltas [0..1]
        d = {}
        for tid in r.keys():
            d[tid] = s[tid] - e[tid]
            # logging.info('+++ actual - expected tid: %d = %f' % (tid, d[tid]))

        # Now calculate per-player rating percentages based on percentage
        # contribution to kills for that team.
        ratings = []
        for key in self.key_pid_map.keys():
            pid = self.key_pid_map[key]
            tid = self.find_tid(pid)
            rating = (key, d[tid] * win_shares[pid], winner_tids.__contains__(tid))
            ratings.append(rating)
        return ratings

    def lookup_dids(self):
        dids = {}
        for player_stat in self.player_stats:
            if 'did' in player_stat.__dict__ and len(player_stat.did) != 0:
                dids[player_stat.name] = player_stat.did
                continue
            dids[player_stat.name] = ''
        return dids

    def save_actions(self, dids):
        # For every legit player, add a player action
        for player_stat in self.player_stats:
            if player_stat.winstats.ff & kfwsComputer:
                continue
            player_name = player_stat.name
            anonymous = (player_stat.winstats.ff & kfwsAnonymous) != 0
            did = dids[player_name] if player_name in dids else ''
            ip = player_stat.ip if 'ip' in player_stat.__dict__ else ''
            action = dict(action='game', key=self.get_game_key_name())
            utc = self.end_utc
            playerdetail.save(player_name, anonymous, did, ip, action, utc)

    def save(self, update_player_stats=True, lookup_dids=True, save_actions=True):
        try:
            # Lookup dids if asked
            dids = self.lookup_dids() if lookup_dids else {}
            
            # If not saved, it's already in the db.
            if not db.run_in_transaction(self.add_txn, dids):
                return False
            if update_player_stats:
                self.update_player_stats()
            if save_actions:
                self.save_actions(dids)
            return True
        except:
            # Save game in a special place for later analysis
            # This should be rare but it'll help for chasing down bugs.
            obj = models.GameStatsFailed()
            obj.json = self.json
            io = StringIO.StringIO()
            traceback.print_exc(file=io)
            io.seek(0)
            obj.backtrace = io.read()
            io.close()
            obj.put()
            raise

    def update_player_stats(self):
        if self.get_game_result() != GAME_RESULT_SCOREABLE:
            return
        for player_key, rating_percent, winner in self.get_ratings():
            db.run_in_transaction(self.update_player_txn, player_key,
                    rating_percent, winner)

    def update_player_txn(self, player_key, rating_percent, winner):
        p = models.PlayerModel.get(player_key)

        # Update rating
        rating = p.rating
        if rating == 0:
            rating = PLAYER_DEFAULT_RATING
        k = 32
        if rating >= 2000:
            k = 20
        if rating >= 2400:
            k = 10
        p.rating = int(round(rating + k * rating_percent))

        # Winners get "match point"
        if winner:
            p.rating += 1

        # Update misc totals
        pid = self.key_pid_map[player_key]
        player_stat = self.pid_player_stat_map[pid]
        p.credits_acquired_total += player_stat.winstats.credits_acquired
        p.credits_consumed_total += player_stat.winstats.credits_consumed
        p.munts_killed_total += player_stat.winstats.enemy_munts_killed
        p.structs_killed_total += player_stat.winstats.enemy_structs_killed
        p.munts_lost_total += player_stat.winstats.munts_lost
        p.structs_lost_total += player_stat.winstats.structs_lost

        # Update built unit counts. Adding units to this will be a pain.
        for index in xrange(len(player_stat.winstats.built_counts)):
            count = int(player_stat.winstats.built_counts[index])
            p.built_counts_total[index] += count

        # Update game count, last game, and elapsed time
        p.game_count += 1
        if winner:
            p.games_won += 1

        p.last_game_key_name = self.get_game_key_name()
        p.elapsed_seconds_total += self.get_duration_seconds()
        if self.get_game_result() == GAME_RESULT_SCOREABLE:
            p.last_game_utc = self.end_utc
            p.rating_check_utc = ratingjob.next_rating_check_utc(self.end_utc)
        p.put()

    def add_txn(self, dids):
        game_key = self.get_game_key()
        if models.GameStatsModel.get(game_key):
            return False

        # Collect key names in game
        key_names = [key.name() for key in self.key_pid_map.keys()]

        # Collect old ratings of these players, for debugging purposes
        old_ratings = {}
        for key in self.key_pid_map.keys():
            pid = self.key_pid_map[key]
            player_stat = self.pid_player_stat_map[pid]
            old_ratings[key.name()] = player_stat.player.rating

        obj = models.GameStatsModel(key_name=game_key.name(),
                player_key_names = key_names,
                mission_title = self.title,
                start_utc = self.start_utc,
                end_utc = self.end_utc,
                old_ratings = json.dumps(old_ratings),
                dids = json.dumps(dids),
                json = self.json)
        obj.put()
        return True

def load_from_key_name(key_name, viewing_player_name, load_players=False):
    key = models.gamestatsmodel_key_from_name(key_name)
    obj = models.GameStatsModel.get(key)
    if obj:
        return GameStats(obj.json, viewing_player_name,
                load_players=load_players), obj
    return None, None
