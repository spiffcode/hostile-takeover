class QualifiedNumber:
    def __init__(self, txt):
        args = txt.split(',')
        self.txt = txt
        self.qualifier = int(args[0])
        self.number = int(args[1])

    def __repr__(self):
        d = ['AtLeast', 'AtMost', 'Exactly']
        return '%s: %s %d' % (self.__class__.__name__, d[self.qualifier], self.number)

class Condition:
    def __init__(self, txt):
        self.txt = txt

    def __str__(self):
        return '%s: %s' % (self.__class__.__name__, self.__dict__)

class UndoneCondition(Condition):
    def __str__(self):
        return '%s: UNDONE' % self.__class__.__name__

class MobileHQDeployedCondition(UndoneCondition):
    pass

class UnitDestroyedCondition(UndoneCondition):
    pass

class UnitSeesUnitCondition(UndoneCondition):
    pass

class CounterCondition(UndoneCondition):
    pass

class MissionLoadedCondition(Condition):
    pass

class PlaceStructureModeCondition(Condition):
    pass

class CreditsCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.qnum = QualifiedNumber(','.join(args[1:]))
        Condition.__init__(self, txt)

class AreaContainsUnitsCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.area = int(args[0])
        self.qnum = QualifiedNumber(','.join(args[1:3]))
        self.unitmask = int(args[3])
        self.sidemask = int(args[4])
        Condition.__init__(self, txt)

class GalaxiteCapacityReachedCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        Condition.__init__(self, txt)

class ElapsedTimeCondition(Condition):
    def __init__(self, txt):
        self.qnum = QualifiedNumber(txt)
        Condition.__init__(self, txt)

class OwnsUnitsCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.qnum = QualifiedNumber(','.join(args[1:3]))
        self.unitmask = int(args[3])
        Condition.__init__(self, txt)

class MinerCantFindGalaxiteCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        Condition.__init__(self, txt)

class SwitchCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.switch = int(args[0])
        self.on = (int(args[1]) == 1)
        Condition.__init__(self, txt)

class PeriodicTimerCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.seconds = int(args[0])
        Condition.__init__(self, txt)

class DiscoversSideCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask_a = int(args[0])
        self.sidemask_b = int(args[1])
        Condition.__init__(self, txt)

class CountdownTimerCondition(Condition):
    def __init__(self, txt):
        self.qnum = QualifiedNumber(txt)
        Condition.__init__(self, txt)

class TestPvarCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.name = args[0]
        self.qnum = QualifiedNumber(','.join(args[1:]))
        Condition.__init__(self, txt)

class HasUpgradesCondition(Condition):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.upgrademask = int(args[1])
        Condition.__init__(self, txt)
