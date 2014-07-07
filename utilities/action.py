class Action:
    def __init__(self, txt):
        self.txt = txt

    def __str__(self):
        return '%s: %s' % (self.__class__.__name__, self.__dict__)

class UndoneAction(Action):
    def __str__(self):
        return '%s: UNDONE' % self.__class__.__name__

class SetResourcesAction(UndoneAction):
    pass

class MoveUnitTriggerAction(UndoneAction):
    pass

class SetPlayerControlsTriggerAction(UndoneAction):
    pass

class PanViewTriggerAction(UndoneAction):
    pass

class TargetUnitTriggerAction(UndoneAction):
    pass

class ModifyCounterTriggerAction(UndoneAction):
    pass

class SetAllowedUnitsTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.unitmask = int(args[1])
        Action.__init__(self, txt)

class EcomTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.background = int(args[0])
        self.parsed = int(args[1])
        self.charfrom = int(args[2])
        self.charto = int(args[3])
        self.message = args[4]
        Action.__init__(self, txt)

class SetObjectiveTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.objective = args[1]
        Action.__init__(self, txt)

class SetNextMissionTriggerAction(Action):
    def __init__(self, txt):
        self.level = ''
        if txt != '[none]':
            self.level = txt
        Action.__init__(self, txt)

class EndMissionTriggerAction(Action):
    def __init__(self, txt):
        self.winlose = int(txt)
        Action.__init__(self, txt)

class WaitTriggerAction(Action):
    def __init__(self, txt):
        self.seconds = int(txt)
        Action.__init__(self, txt)

class SetSwitchTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.switch = int(args[0])
        self.on = (int(args[1]) == 1)
        Action.__init__(self, txt)

class PreserveTriggerTriggerAction(Action):
    pass

class CenterViewTriggerAction(Action):
    def __init__(self, txt):
        self.area = int(txt)
        Action.__init__(self, txt)

class DefogAreaTriggerAction(Action):
    def __init__(self, txt):
        self.area = int(txt)
        Action.__init__(self, txt)

class CreateUnitGroupTriggerAction(Action):
    def __init__(self, txt):
        self.unitgroup = int(txt)
        Action.__init__(self, txt)

class HuntTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.unitmask1 = int(args[0])
        self.sidemask1 = int(args[1])
        self.unitmask2 = int(args[2])
        self.sidemask2 = int(args[3])
        Action.__init__(self, txt)

class CreateRandomUnitGroupTriggerAction(Action):
    pass

class AlliesTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemaskA = int(args[0])
        self.sidemaskB = int(args[1])
        Action.__init__(self, txt)

class StartCountdownTimerTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.seconds = int(args[0])
        self.message = args[1]
        Action.__init__(self, txt)

class ModifyCountdownTimerTriggerAction(Action):
    def __init__(self, txt):
        self.action = int(txt)
        Action.__init__(self, txt)

class RepairTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.on = (int(args[1]) == 1)
        Action.__init__(self, txt)

class EnableReplicatorTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.on = (int(args[1]) == 1)
        Action.__init__(self, txt)

class ModifyCreditsTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.action = int(args[0])
        self.sidemask = int(args[1])
        self.amount = int(args[2])
        Action.__init__(self, txt)

    def __str__(self):
        l = ['ModifyNumberNone','ModifyNumberSet','ModifyNumberAdd','ModifyNumberSubtract']
        return "%s: {'action': '%s', 'amount': %d 'sidemask': %d 'txt': '%s'}" % (self.__class__.__name__, l[self.action], self.amount, self.sidemask, self.txt)

class MoveUnitsInAreaTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.unitmask = int(args[1])
        self.areasrc = int(args[2])
        self.areadst = int(args[3])
        Action.__init__(self, txt)

class SetFormalObjectiveTextTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.objective = int(args[0])
        self.message = args[1]
        Action.__init__(self, txt)

class SetFormalObjectiveStatusTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.objective = int(args[0])
        self.message = args[1]
        Action.__init__(self, txt)

class ShowObjectivesTriggerAction(Action):
        pass

class SetFormalObjectiveInfoTriggerAction(Action):
    def __init__(self, txt):
        self.info = txt
        Action.__init__(self, txt)

class CutSceneTriggerAction(Action):
    def __init__(self, txt):
        self.message = txt
        Action.__init__(self, txt)

class JumpToMissionTriggerAction(Action):
    def __init__(self, txt):
        self.level = txt
        Action.__init__(self, txt)

class ModifyPvarTriggerAction(Action):
    def __init__(self, txt):
        Action.__init__(self, txt)

class SetPvarTextTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.name = args[0]
        self.value = args[1]
        Action.__init__(self, txt)

class ShowAlertTriggerAction(Action):
    def __init__(self, txt):
        self.alert = txt
        Action.__init__(self, txt)

class SetAllowedUpgradesTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.upgrademask = int(args[1])
        Action.__init__(self, txt)

class SetUpgradesTriggerAction(Action):
    def __init__(self, txt):
        args = txt.split(',')
        self.sidemask = int(args[0])
        self.upgrademask = int(args[1])
        Action.__init__(self, txt)
