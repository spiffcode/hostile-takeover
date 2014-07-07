from wrap import DictWrap
import gamestats
import config
import serverinfo

mobile_unit_indexes = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 19, 20, 22 ]
struct_unit_indexes = [ 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21 ]

class PlayerStat(DictWrap):
    def __init__(self, d, win_result):
        super(PlayerStat, self).__init__(d)
        self.win_result = win_result
        self.is_anonymous = (self.winstats.ff & gamestats.kfwsAnonymous) != 0
        self.is_computer = (self.winstats.ff & gamestats.kfwsComputer) != 0
        self.is_user = (not self.is_anonymous and not self.is_computer)

    def get_side(self):
        for side in xrange(config.SIDE_COUNT_MAX):
            if self.winstats.side_mask & (1 << side):
                return side
        return 0

    def get_side_color(self):
        # 0: gray, 1: blue, 2: red, 3: yellow, 4: cyan
        # These colors match the ones used in the game
        colors = [ '#d8d8d8', '#0074e8', '#e82000', '#e8e400', '#68fcfc' ]
        return colors[self.side]

    def get_munts_built(self):
        return sum([self.winstats.built_counts[i] for i in mobile_unit_indexes])

    def get_structs_built(self):
        return sum([self.winstats.built_counts[i] for i in struct_unit_indexes])

    def get_units_remaining(self):
        return sum(self.winstats.unit_counts)
