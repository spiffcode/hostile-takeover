import condition
import action
import cStringIO

class Trigger:
    def __init__(self):
        self.conditions = []
        self.actions = []

    def LoadCondition(self, txt):
        c = {
            0 : lambda t: condition.MissionLoadedCondition(t),
            1 : lambda t: condition.CreditsCondition(t),
            2 : lambda t: condition.AreaContainsUnitsCondition(t),
            3 : lambda t: condition.GalaxiteCapacityReachedCondition(t),
            4 : lambda t: condition.MobileHQDeployedCondition(t),
            5 : lambda t: condition.PlaceStructureModeCondition(t),
            6 : lambda t: condition.ElapsedTimeCondition(t),
            7 : lambda t: condition.OwnsUnitsCondition(t),
            8 : lambda t: condition.MinerCantFindGalaxiteCondition(t),
            9 : lambda t: condition.UnitSeesUnitCondition(t),
            10 : lambda t: condition.UnitDestroyedCondition(t),
            11 : lambda t: condition.SwitchCondition(t),
            12 : lambda t: condition.PeriodicTimerCondition(t),
            13 : lambda t: condition.DiscoversSideCondition(t),
            14 : lambda t: condition.CountdownTimerCondition(t),
            15 : lambda t: condition.CounterCondition(t),
            16 : lambda t: condition.TestPvarCondition(t),
            17 : lambda t: condition.HasUpgradesCondition(t)
        }[int(txt.split(',')[0])](','.join(txt.split(',')[1:]))
        self.conditions.append(c)

    def LoadAction(self, txt):
        a = {
            0 : lambda t: action.SetResourcesTriggerAction(t),
            1 : lambda t: action.SetAllowedUnitsTriggerAction(t),
            2 : lambda t: action.EcomTriggerAction(t),
            3 : lambda t: action.SetObjectiveTriggerAction(t),
            4 : lambda t: action.SetNextMissionTriggerAction(t),
            5 : lambda t: action.EndMissionTriggerAction(t),
            6 : lambda t: action.WaitTriggerAction(t),
            7 : lambda t: action.SetSwitchTriggerAction(t),
            8 : lambda t: action.SetPlayerControlsTriggerAction(t),
            9 : lambda t: action.PreserveTriggerTriggerAction(t),
            10 : lambda t: action.CenterViewTriggerAction(t),
            11 : lambda t: action.PanViewTriggerAction(t),
            12 : lambda t: action.DefogAreaTriggerAction(t),
            13 : lambda t: action.MoveUnitTriggerAction(t),
            14 : lambda t: action.TargetUnitTriggerAction(t),
            15 : lambda t: action.CreateUnitGroupTriggerAction(t),
            16 : lambda t: action.HuntTriggerAction(t),
            17 : lambda t: action.CreateRandomUnitGroupTriggerAction(t),
            18 : lambda t: action.AlliesTriggerAction(t),
            19 : lambda t: action.StartCountdownTimerTriggerAction(t),
            20 : lambda t: action.ModifyCountdownTimerTriggerAction(t),
            21 : lambda t: action.RepairTriggerAction(t),
            22 : lambda t: action.EnableReplicatorTriggerAction(t),
            23 : lambda t: action.ModifyCreditsTriggerAction(t),
            24 : lambda t: action.ModifyCounterTriggerAction(t),
            25 : lambda t: action.MoveUnitsInAreaTriggerAction(t),
            26 : lambda t: action.SetFormalObjectiveTextTriggerAction(t),
            27 : lambda t: action.SetFormalObjectiveStatusTriggerAction(t),
            28 : lambda t: action.ShowObjectivesTriggerAction(t),
            29 : lambda t: action.SetFormalObjectiveInfoTriggerAction(t),
            30 : lambda t: action.CutSceneTriggerAction(t),
            31 : lambda t: action.JumpToMissionTriggerAction(t),
            32 : lambda t: action.ModifyPvarTriggerAction(t),
            33 : lambda t: action.SetPvarTextTriggerAction(t),
            34 : lambda t: action.ShowAlertTriggerAction(t),
            35 : lambda t: action.SetAllowedUpgradesTriggerAction(t),
            36 : lambda t: action.SetUpgradesTriggerAction(t)
        }[int(txt.split(',')[0])](','.join(txt.split(',')[1:]))
        self.actions.append(a)

    def __str__(self):
        io = cStringIO.StringIO()
        for condition in self.conditions:
            io.write('%s\n' % condition)
        for action in self.actions:
            io.write('%s\n' % action)
        io.write('\n')
        return io.getvalue()

