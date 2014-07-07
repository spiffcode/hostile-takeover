import trigger

class Level:
    def __init__(self, ini):
        self.ini = ini
        self.sidetriggers = ([], [], [], [], [])
        self.LoadTriggers()

    def GetTriggers(self, side):
        return self.sidetriggers[side]

    def LoadTriggers(self):
        section = self.ini['Triggers']
        triggermap = {}
        triggerCurrent = None
        for prop in section:
            if prop[0] == 'Count':
                continue
            if prop[0] == 'T':
                triggerCurrent = trigger.Trigger()
                for key in prop[1].split(','):
                    triggermap[key] = triggerCurrent
                continue
            if prop[0] == 'C':
                triggerCurrent.LoadCondition(prop[1])
                continue
            if prop[0] == 'A':
                triggerCurrent.LoadAction(prop[1])
                continue

        # Make lists of triggers for each side, from triggermap
        for side in xrange(len(self.sidetriggers)):
            index = 0
            while True:
                found = False
                for key in triggermap.keys():
                    sideT = int(key.split(':')[0])
                    if sideT != side:
                        continue
                    indexT = int(key.split(':')[1])
                    if indexT != index:
                        continue
                    found = True
                    self.sidetriggers[side].append(triggermap[key])
                if not found:
                    break
                index = index + 1
                    
        
