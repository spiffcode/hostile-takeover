using System;
using SpiffLib;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;

namespace m
{
	public class UnitActionLoader {
		public static CaBase LoadIni(string strValue) {
			CaBase cab = null;
			switch (int.Parse(strValue.Split(',')[0])) {
			case 0: // knGuardUnitAction
				cab = new GuardUnitAction();
				break;

			case 1: // knGuardVicinityUnitAction
				cab = new GuardVicinityUnitAction();
				break;

			case 2: // knGuardAreaUnitAction
				cab = new GuardAreaUnitAction();
				break;

			case 3: // knMoveUnitAction
				cab = new MoveUnitAction();
				break;

			case 4: // knHuntEnemiesUnitAction
				cab = new HuntEnemiesUnitAction();
				break;

			case 5: // knMineUnitAction
				cab = new MineUnitAction();
				break;

			default:
				return null;
			}
			if (cab != null) {
				cab.FromSaveString(strValue);
			}
			return cab;
		}
	}

	public class UnitGroupActionLoader {
		public static CaBase LoadIni(string strValue) {
			CaBase cab = null;
			switch (int.Parse(strValue.Split(',')[0])) {
			case 0: // knWaitUnitGroupAction
				cab = new WaitUnitGroupAction();
				break;

			case 1: // knSetSwitchUnitGroupAction
				cab = new SetSwitchUnitGroupAction();
				break;

			case 2: // knMoveUnitGroupAction
				cab = new MoveUnitGroupAction();
				break;

			case 3: // knAttackUnitGroupAction
				cab = new AttackUnitGroupAction();
				break;

			case 4: // knGuardUnitGroupAction
				cab = new GuardUnitGroupAction();
				break;

			case 5: // knMineUnitGroupAction
				cab = new MineUnitGroupAction();
				break;

			case 6: // knGuardVicinityUnitGroupAction
				cab = new GuardVicinityUnitGroupAction();
				break;
			}
			if (cab != null) {
				cab.FromSaveString(strValue);
			}
			return cab;
		}
	}

	public class TriggerConditionLoader {
		public static CaBase LoadIni(string strValue) {
			CaBase cab = null;
			switch (int.Parse(strValue.Split(',')[0])) {
			case 0: // knMissionLoadedCondition
				cab = new MissionLoadedCondition();
				break;

			case 1: // knCreditsCondition
				cab = new CreditsCondition();
				break;

			case 2: // knAreaContainsUnitsCondition
				cab = new AreaContainsUnitsCondition();
				break;

			case 3: // knGalaxiteCapacityReachedCondition
				cab = new GalaxiteCapacityReachedCondition();
				break;

			case 4: // knMobileHQDeployedCondition
#if false
				cab = new MobileHQDeployedCondition();
#endif
				break;

			case 5: // knPlaceStructureModeCondition
				cab = new PlaceStructureModeCondition();
				break;

			case 6: // knElapsedTimeCondition
				cab = new ElapsedTimeCondition();
				break;

			case 7: // knOwnsUnitsCondition
				cab = new OwnsUnitsCondition();
				break;

			case 8: // knMinerCantFindGalaxiteCondition
				cab = new MinerCantFindGalaxiteCondition();
				break;

			case 9: // knUnitSeesUnitCondition
#if false
				cab = new UnitSeesUnitCondition();
#endif
				break;

			case 10: // knUnitDestroyedCondition
#if false
				cab = new UnitDestroyedCondition();
#endif
				break;

			case 11: // knSwitchCondition
				cab = new SwitchCondition();
				break;

			case 12: // knPeriodicTimerCondition
				cab = new PeriodicTimerCondition();
				break;

			case 13: // knDiscoversSideCondition
				cab = new DiscoversSideCondition();
				break;

			case 14: // knCountdownTimerCondition
				cab = new CountdownTimerCondition();
				break;

			case 15: // knCounterCondition
#if false
				cab = new CounterCondition();
#endif
				break;

			case 16: // knTestPvarCondition
				cab = new TestPvarCondition();
				break;

			case 17: // knHasUpgradesCondition
				cab = new HasUpgradesCondition();
				break;
			}
			if (cab != null) {
				cab.FromSaveString(strValue);
			}
			return cab;
		}
	}

	public class TriggerActionLoader {
		public static CaBase LoadIni(string strValue) {
			CaBase cab = null;
			switch (int.Parse(strValue.Split(',')[0])) {

			case 0: // knSetResourcesTriggerAction
#if false
				cab = new SetResourcesTriggerAction();
#endif
				break;

			case 1: // knSetAllowedUnitsTriggerAction
				cab = new SetAllowedUnitsTriggerAction();
				break;

			case 2 : // knEcomTriggerAction
				cab = new EcomTriggerAction();
				break;

			case 3: // knSetObjectiveTriggerAction
				cab = new SetObjectiveTriggerAction();
				break;

			case 4: // knSetNextMissionTriggerAction
				cab = new SetNextMissionTriggerAction();
				break;

			case 5: // knEndMissionTriggerAction
				cab = new EndMissionTriggerAction();
				break;

			case 6: // knWaitTriggerAction
				cab = new WaitTriggerAction();
				break;

			case 7 : // knSetSwitchTriggerAction
				cab = new SetSwitchTriggerAction();
				break;

			case 8: // knSetPlayerControlsTriggerAction
#if false
				cab = new SetPlayerControlsTriggerAction();
#endif
				break;

			case 9: // knPreserveTriggerTriggerAction
				cab = new PreserveTriggerTriggerAction();
				break;

			case 10: // knCenterViewTriggerAction
				cab = new CenterViewTriggerAction();
				break;

			case 11: // knPanViewTriggerAction
#if false
				cab = new PanViewTriggerAction();
#endif
				break;

			case 12: // knDefogAreaTriggerAction
				cab = new DefogAreaTriggerAction();
				break;

			case 13: // knMoveUnitTriggerAction
#if false
				cab = new MoveUnitTriggerAction();
#endif
				break;

			case 14: // knTargetUnitTriggerAction
#if false
				cab = new TargetUnitTriggerAction();
#endif
				break;

			// knCreateUnitGroupTriggerAction
			// knCreateUnitAtAreaTriggerAction
			case 15: 
				// This can be either action. If the referenced trigger has a
				// name prefix of __cuaa, then it is CreateUnitAtAreaTriggerAction.
				cab = new CreateUnitGroupTriggerAction();
				cab.FromSaveString(strValue);
				if (((CaTypeUnitGroup)cab.GetTypes()[0]).ToString().StartsWith("__cuaa")) {
					cab = new CreateUnitAtAreaTriggerAction();
				} else {
					cab = new CreateUnitGroupTriggerAction();
				}
				break;

			case 16: // knHuntTriggerAction
				cab = new HuntTriggerAction();
				break;

			case 17: // knCreateRandomUnitGroupTriggerAction
				cab = new CreateRandomUnitGroupTriggerAction();
				break;

			case 18: // knAlliesTriggerAction
				cab = new AlliesTriggerAction();
				break;

			case 19: // knStartCountdownTimerTriggerAction
				cab = new StartCountdownTimerTriggerAction();
				break;

			case 20: // knModifyCountdownTimerTriggerAction
				cab = new ModifyCountdownTimerTriggerAction();
				break;

			case 21: // knRepairTriggerAction
				cab = new RepairTriggerAction();
				break;

			case 22: // knEnableReplicatorTriggerAction
				cab = new EnableReplicatorTriggerAction();
				break;

			case 23: // knModifyCreditsTriggerAction
				cab = new ModifyCreditsTriggerAction();
				break;

			case 24: // knModifyCounterTriggerAction
				cab = new ModifyCounterTriggerAction();
				break;

			case 25: // knMoveUnitsInAreaTriggerAction
				cab = new MoveUnitsInAreaTriggerAction();
				break;

			case 26: // knSetFormalObjectiveTextTriggerAction
				cab = new SetFormalObjectiveTextTriggerAction();
				break;

			case 27: // knSetFormalObjectiveStatusTriggerAction
				cab = new SetFormalObjectiveStatusTriggerAction();
				break;

			case 28: // knShowObjectivesTriggerAction
				cab = new ShowObjectivesTriggerAction();
				break;

			case 29: // knSetFormalObjectiveInfoTriggerAction
				cab = new SetFormalObjectiveInfoTriggerAction();
				break;

			case 30: // knCutSceneTriggerAction
				cab = new CutSceneTriggerAction();
				break;

			case 31: // knJumpToMissionTriggerAction
				cab = new JumpToMissionTriggerAction();
				break;

			case 32: // knModifyPvarTriggerAction
				cab = new ModifyPvarTriggerAction();
				break;

			case 33: // knSetPvarTextTriggerAction
				cab = new SetPvarTextTriggerAction();
				break;

			case 34: // knShowAlertTriggerAction
				cab = new ShowAlertTriggerAction();
				break;

			case 35: // knSetAllowedUpgradesTriggerAction
				cab = new SetAllowedUpgradesTriggerAction();
				break;

			case 36: // knSetUpgradesTriggerAction
				cab = new SetUpgradesTriggerAction();
				break;
			}
			if (cab != null) {
				cab.FromSaveString(strValue);
			}
			return cab;
		}
	}

	[Serializable]
	abstract public class CaBase {
		// NOTE: This is legacy. We don't want it anymore but it has been serialized
		// into some levels and the effort required to up-convert them is greater 
		// than the value of doing so.
		protected string m_str;

		protected CaType[] m_acat;
		protected bool m_fActive;

		public CaBase() {
			m_str = null;
			m_acat = null;
			m_fActive = true;
		}

		abstract public string GetString();

		public CaType[] GetTypes() {
			return m_acat;
		}

		public virtual CaBase Clone() {
			CaBase cab = (CaBase)MemberwiseClone();
			cab.m_acat = new CaType[m_acat.Length];
			for (int n = 0; n < m_acat.Length; n++)
				cab.m_acat[n] = m_acat[n].Clone();
			return cab;
		}

		public override string ToString() {
			string str = GetString();
			for (int j = 0; j < m_acat.Length; j++)
				str = str.Replace("$" + (j + 1), m_acat[j].ToString());
			return str;
		}

		public virtual string ToSaveString() {
			string str = "";
			for (int n = 0; n < m_acat.Length; n++) {
				str += m_acat[n].ToSaveString();
				if (n < m_acat.Length - 1)
					str += ",";
			}
			string strType = GetType().ToString();
			int ichDot = strType.IndexOf(".");
			strType = strType.Substring(ichDot + 1, strType.Length - ichDot - 1);
			return "kn" + strType + "," + str;
		}

		public virtual string FromSaveString(string strArgs) {
			Regex re = new Regex(@"^\d+,(?<end>.*)$");
			Match m = re.Match(strArgs);
			string strT = m.Groups["end"].Value;
			for (int i = 0; i < m_acat.Length; i++) {
				CaType cat = m_acat[i];
				strT = cat.FromSaveString(strT, i == (m_acat.Length - 1));
				if (strT.Trim().Length != 0) {
					re = new Regex(@"^\s*,(?<end>.*)$");
					m = re.Match(strT);
					strT = m.Groups["end"].Value;
				}
			}
			return strT;
		}

		public bool IsValid() {
			if (m_acat == null)
				return false;

			foreach (CaType cat in m_acat) {
				if (!cat.IsInitialized())
					return false;
			}

			return true;
		}

		public bool Active {
			get {
				return m_fActive;
			}
			set {
				m_fActive = value;
			}
		}
	}

	//
	// Conditions
	//

	[Serializable]
	[DisplayName("Mission Loaded")]
	[Description("This condition is set immediately after the mission is loaded.")]
	public class MissionLoadedCondition : CaBase {
		override public string GetString() {
			return "Mission loaded";
		}
		public MissionLoadedCondition() {
			m_acat = new CaType[0];
		}
	}

	[Serializable]
	[DisplayName("Credits")]
	[Description("Use this condition to respond to Credits going above or below a value of interest. " +
			"WARNING: it is not a good idea to use the \"Exactly\" comparison against any value other than 0 " +
			"because Credits typically change at increments greater than one.")]
	public class CreditsCondition : CaBase {
		override public string GetString() {
			return "$1 has $2 Credits";
		}
		public CreditsCondition() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeQualifiedNumber() };
		}
	}

	[Serializable]
	[DisplayName("Area Contains Units")]
	[Description("All units of all sides and all types specified that are " +
			"inside the area are totaled and that total is what is compared against the value.")]
	public class AreaContainsUnitsCondition : CaBase {
		override public string GetString() {
			return "Area '$1' contains $2 $3 owned by $4";
		}
		public AreaContainsUnitsCondition() {
			m_acat = new CaType[] { new CaTypeArea(), new CaTypeQualifiedNumber(), new CaTypeUnitTypes(), new CaTypeSide() };
		}
	}

	[Serializable]
	[DisplayName("Place Structure Mode Active")]
	[Description("This condition is met when, after ordering a new structure, " +
			"the game enters the \"place structure\" mode. It is intended to " +
			"be used only by the tutorial.")]
	public class PlaceStructureModeCondition : CaBase {
		override public string GetString() {
			return "Place structure mode is active";
		}
		public PlaceStructureModeCondition() {
			m_acat = new CaType[0];
		}
	}

#if false
	[Serializable]
	public class DeathsCondition : CaBase {
		override public string GetString() {
			return "$1 has suffered $2 deaths of $3";
		}
		public DeathsCondition() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeQualifiedNumber(), new CaTypeUnitTypes() };
		}
	}

	[Serializable]
	[DisplayName("Mobile Headquarters deployable")]
	public class MobileHQDeployableCondition : CaBase {
		override public string GetString() {
			return "MobileHQ is deployable";
		}
		public MobileHQDeployableCondition() {
			m_acat = new CaType[0];
		}
	}

	[Serializable]
	[DisplayName("Mobile Headquarters deployed")]
	public class MobileHQDeployedCondition : CaBase {
		override public string GetString() {
			return "MobileHQ is deployed";
		}
		public MobileHQDeployedCondition() {
			m_acat = new CaType[0];
		}
	}
#endif

	[Serializable]
	[DisplayName("Countdown Timer")]
	[Description("The countdown timer is compared against the \"qualified quantity\" " +
		 "to determine if condition is satisfied.")]
	public class CountdownTimerCondition : CaBase {
		override public string GetString() {
			return "Countdown timer is $1";
		}
		public CountdownTimerCondition() {
			m_acat = new CaType[] { new CaTypeQualifiedNumber() };
		}
	}

	[Serializable]
	[DisplayName("Owns Units")]
	[Description("All units of all sides and types specified are totaled " +
			"and that total is what is compared against the \"quantity\" value.")]
	public class OwnsUnitsCondition : CaBase {
		override public string GetString() {
			return "$1 owns $2 $3";
		}
		public OwnsUnitsCondition() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeQualifiedNumber(), new CaTypeUnitTypes() };
		}
	}

	[Serializable]
	[DisplayName("GalaxMiner Can't Find Galaxite")]
	[Description("This condition is met when the GalaxMiner searches the " +
			"unfogged area of the map and can't see any Galaxite. It is " +
			"intended to be used only by the tutorial.")]
	public class MinerCantFindGalaxiteCondition : CaBase {
		override public string GetString() {
			return "GalaxMiner owned by $1 can't find galaxite";
		}
		public MinerCantFindGalaxiteCondition() {
			m_acat = new CaType[] { new CaTypeSide() };
		}
	}

	[Serializable]
	[DisplayName("Galaxite Capacity Reached")]
	[Description("This condition is met when all the side's Processors and Warehouses are " +
			"full. It is intended to be used only by the tutorial.")]
	public class GalaxiteCapacityReachedCondition : CaBase {
		override public string GetString() {
			return "$1's Galaxite capacity is reached";
		}
		public GalaxiteCapacityReachedCondition() {
			m_acat = new CaType[] { new CaTypeSide() };
		}
	}

	[Serializable]
	[DisplayName("Elapsed Time")]
	[Description("Time starts at 0 when the mission begins. If you want to set up " +
			"an action that recurs every NN seconds do it with an \"Always\" condition " +
			"and a \"Wait NN\" action.")]
	public class ElapsedTimeCondition : CaBase {
		override public string GetString() {
			return "Elapsed mission time is $1 seconds";
		}
		public ElapsedTimeCondition() {
			m_acat = new CaType[] { new CaTypeQualifiedNumber() };
		}
	}

	[Serializable]
	[DisplayName("Has Upgrades")]
	[Description("Use this to determine if the specified side possesses the specified upgrades.")]
	public class HasUpgradesCondition : CaBase {
		override public string GetString() {
			return "$1 has $2 upgrade(s)";
		}
		public HasUpgradesCondition() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUpgradeTypes() };
		}
	}

#if false
	[Serializable]
	public class UnitSeesUnitCondition : CaBase {
		override public string GetString() {
			return "$1 owned by $2 sees $3 owned by $4";
		}
		public UnitSeesUnitCondition() {
			m_acat = new CaType[] { new CaTypeUnit(), new CaTypeSide(), new CaTypeUnit(), new CaTypeSide() };
		}
	}

	[Serializable]
	public class UnitDestroyedCondition : CaBase {
		override public string GetString() {
			return "$1 owned by $2 destroyed";
		}
		public UnitDestroyedCondition() {
			m_acat = new CaType[] { new CaTypeUnit(), new CaTypeSide() };
		}
	}
#endif

	[Serializable]
	[DisplayName("Switch On/Off")]
	[Description("Switch names can be anything (including spaces, etc) but ARE case sensitive. That is, setting " +
			"a switch named \"Whatever\" and testing \"whatever\" won't work.")]
	public class SwitchCondition : CaBase {
		override public string GetString() {
			return "Switch '$1' is $2";
		}
		public SwitchCondition() {
			m_acat = new CaType[] { new CaTypeSwitch(), new CaTypeOnOff() };
		}
	}

	[Serializable]
	[DisplayName("Every NN seconds")]
	[Description("This condition is satisfied repeatedly at the specified rate. " +
			"NOTE: the trigger will only fire once unless the " +
			"\"Preserve Trigger\" action is used.")]
	public class PeriodicTimerCondition : CaBase {
		override public string GetString() {
			return "Every $1 seconds";
		}
		public PeriodicTimerCondition() {
			m_acat = new CaType[] { new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Comment")]
	[Description("Use this to comment your conditions. It has no in-game effect.")]
	public class CommentCondition : CaBase {
		override public string GetString() {
			return "Comment: $1";
		}
		public CommentCondition() {
			m_acat = new CaType[] { new CaTypeRichText() };
		}
	}

	[Serializable]
	[DisplayName("Discovers Side")]
	[Description("This condition is met the first time side A 'discovers' side B. " +
			"Discovery means that a unit owned by side B has become visible to " +
			"a unit owned by side A (and implicitly, vice-versa). The virtual " +
			"area [LastDiscovery] is initialized at discovery time and is " +
			"positioned over the unit found.")]
	public class DiscoversSideCondition : CaBase {
		override public string GetString() {
			return "$1 discovers $2";
		}
		public DiscoversSideCondition() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeSide() };
		}
	}

#if false
	[Serializable]
	[DisplayName("Counter")]
	[Description("Use this condition to respond to Counter going above or below a value of interest.")]
	public class CounterCondition : CaBase {
		override public string GetString() {
			return "Counter '$1' is $2";
		}
		public CounterCondition() {
			m_acat = new CaType[] { new CaTypeCounter(), new CaTypeQualifiedNumber() };
		}
	}
#endif

	[Serializable]
	[DisplayName("Test Persistent Variable")]
	[Description("Use this condition to test whether a persistent variable is above, below, or equal to a value interest.")]
	public class TestPvarCondition : CaBase {
		override public string GetString() {
			return "Persistent variable '$1' is $2";
		}
		public TestPvarCondition() {
			m_acat = new CaType[] { new CaTypeText(), new CaTypeQualifiedNumber() };
		}
		public string GetVariableString() {
			CaTypeText cat = (CaTypeText)m_acat[0];
			return cat.Text;
		}
	}

	//
	// Trigger Actions
	//

	[Serializable]
	[DisplayName("Center View")]
	[Description("Centers the player's view over the specified Area. This view change " +
			"won't be apparent until the next display refresh. A display refresh can " +
			"be forced by adding a subsequent \"Wait 0\" action.")]
	public class CenterViewTriggerAction : CaBase {
		override public string GetString() {
			return "Center view over '$1'";
		}
		public CenterViewTriggerAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}

	[Serializable]
	[DisplayName("Set Next Mission")]
	[Description("Sets the mission that will be launched following an \"End Mission\" action. " +
			"In theory this could be used to set up a branching mission structure or provide " +
			"access to secret missions. The mission naming convention is \"mission.lvl\".")]
	public class SetNextMissionTriggerAction : CaBase {
		override public string GetString() {
			return "Set next mission to \"$1\"";
		}
		public SetNextMissionTriggerAction() {
			m_acat = new CaType[] { new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("End Mission")]
	[Description("Ends the mission immediately. If the win/lose parameter is set to \"Win\" " +
			"and a next mission has been specified via \"Set Next Mission\" that mission " +
			"will be launched. Otherwise gameplay ends.")]
	public class EndMissionTriggerAction : CaBase {
		override public string GetString() {
			return "End Mission: $1";
		}
		public EndMissionTriggerAction() {
			m_acat = new CaType[] { new CaTypeWinLose() };
		}
	}

	[Serializable]
	[DisplayName("Set Allowed Units")]
	[Description("Use this to specify which Units the player is allowed to build. " +
			"Only these units will show in the build panels.")]
	public class SetAllowedUnitsTriggerAction : CaBase {
		override public string GetString() {
			return "Set $1 allowed units to $2";
		}
		public SetAllowedUnitsTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUnitTypes() };
		}
	}

	[Serializable]
	[DisplayName("Allies")]
	[Description("Use this to specify the sides a side considers as its allies. " +
			"A side's units do not (can not) target or attack allies' units. " +
			"Players have no control over their allies' units, nor do they share 'sight' " +
			"with them. NOTE: just because side A considers side B its ally doesn't mean side B " +
			"reciprocates. Use an Allies action for each side if that's what you want. " +
			"Also, do not use the virtual sides \"Enemies\" or \"Allies\" for either side.")]
	public class AlliesTriggerAction : CaBase {
		override public string GetString() {
			return "$1 allies with $2";
		}
		public AlliesTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeSide() };
		}
	}

	[Serializable]
	[DisplayName("Ecom")]
	[Description("...")]
	public class EcomTriggerAction : CaBase {
		override public string GetString() {
			return "Ecom ($1 w/ $2) from $3 to $4: \"$5\"";
		}
		public EcomTriggerAction() {
			m_acat = new CaType[] { new CaTypeSmallLarge(), new CaTypeMoreClose(), new CaTypeCharacter(), new CaTypeCharacter(), new CaTypeRichText() };
		}

		// override ToString base to horn our new CaType into older files which don't have it yet
		public override string ToString() {
			if (m_acat.Length < 4) {
				// old file type
				
				CaType[] acat = new CaType[5];
				acat[0] = new CaTypeSmallLarge();
				acat[1] = new CaTypeMoreClose();
				for (int n = 0; n < m_acat.Length; n++)
					acat[n+2] = m_acat[n].Clone();
				m_acat = acat;
			}

			string str = GetString();
			for (int j = 0; j < m_acat.Length; j++)
				str = str.Replace("$" + (j + 1), m_acat[j].ToString());
			return str;
		}

	}

	[Serializable]
	[DisplayName("Set Objective")]
	[Description("Use this to specify a string to be displayed continuously at the upper-left " +
			"of the screen.")]
	public class SetObjectiveTriggerAction : CaBase {
		override public string GetString() {
			return "Set $1's mission objective to \"$2\"";
		}
		public SetObjectiveTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Wait")]
	[Description("Delay a number of seconds before advancing to the next action.")]
	public class WaitTriggerAction : CaBase {
		override public string GetString() {
			return "Wait $1 seconds";
		}
		public WaitTriggerAction() {
			m_acat = new CaType[] { new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Set Switch")]
	[Description("Switch names can be anything (including spaces, etc) but ARE case sensitive. That is, setting " +
			 "a switch named \"Whatever\" and testing \"whatever\" won't work.")]
	public class SetSwitchTriggerAction : CaBase {
		override public string GetString() {
			return "Set switch '$1' $2";
		}
		public SetSwitchTriggerAction() {
			m_acat = new CaType[] { new CaTypeSwitch(), new CaTypeOnOff() };
		}
	}

	[Serializable]
	[DisplayName("Start Countdown Timer")]
	[Description("Use this to initialize the value of a countdown timer and start a countdown." +
		 "Put a %s in the string where you want the time displayed. You must use a \"ModifyCountdownTrigger\" to Show the timer. " +
		 "The timer will stop at 00:00 but you must call \"ModifyCountdownTrigger\" with \"hide\" to make it disappear.")]
	public class StartCountdownTimerTriggerAction : CaBase {
		override public string GetString() {
			return "Start Countdown Timer with $1 seconds and show this string: \"$2\"";
		}
		public StartCountdownTimerTriggerAction() {
			m_acat = new CaType[] { new CaTypeNumber(), new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Modify Countdown Timer")]
	[Description("Use this to stop (or pause), resume, show, or hide the countdown. To resume a countdown " +
		"you must create another modify action with \"resume\". Showing/hiding will not affect the countdown. " +	 
		"Stopping will not automatically hide the timer - you must create another action to hide it")]
	public class ModifyCountdownTimerTriggerAction : CaBase {
		override public string GetString() {
			return "$1 the Countdown Timer";
		}
		public ModifyCountdownTimerTriggerAction() {
			m_acat = new CaType[] { new CaTypeModifyCountdown() };
		}
	}
	
#if false
	[Serializable]
	public class SetPlayerControlsTriggerAction : CaBase {
		override public string GetString() {
			return "Set player controls $1";
		}
		public SetPlayerControlsTriggerAction() {
			m_acat = new CaType[] { new CaTypeOnOff() };
		}
	}
#endif

	[Serializable]
	[DisplayName("Preserve Trigger")]
	[Description("Use this action if you want a trigger to execute again the " +
			"next time its conditions are met. The default behavior is that " +
			"triggers only execute the first time their conditions are met.")]
	public class PreserveTriggerTriggerAction : CaBase {
		override public string GetString() {
			return "Preserve trigger";
		}
		public PreserveTriggerTriggerAction() {
			m_acat = new CaType[] { };
		}
	}

#if false
	[Serializable]
	public class PanViewTriggerAction : CaBase {
		override public string GetString() {
			return "Pan view to $1";
		}
		public PanViewTriggerAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}
#endif

	[Serializable]
	[DisplayName("Defog Area")]
	[Description("Clear the fog in the area specified. Yes, because Areas are " +
			"rectangular the result can look strange. Note, this only affects the " +
			"local/human player.")]
	public class DefogAreaTriggerAction : CaBase {
		override public string GetString() {
			return "Defog area over '$1'";
		}
		public DefogAreaTriggerAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}

#if false
	[Serializable]
	public class MoveUnitTriggerAction : CaBase {
		override public string GetString() {
			return "Move $1 $2 at $3 to $4";
		}
		public MoveUnitTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUnit(), new CaTypeArea(), new CaTypeArea() };
		}
	}

	[Serializable]
	public class TargetUnitTriggerAction : CaBase {
		override public string GetString() {
			return "Set target for $1 $2 at $3 to $4 $5";
		}
		public TargetUnitTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUnit(), new CaTypeArea(), new CaTypeSide(), new CaTypeUnit() };
		}
	}
#endif

	[Serializable]
	[DisplayName("Create Unit Group")]
	[Description("Create a pre-defined Unit Group and send it on its way according to its intrinsic actions.")]
	public class CreateUnitGroupTriggerAction : CaBase {
		override public string GetString() {
			return "Create Unit Group '$1'";
		}
		public CreateUnitGroupTriggerAction() {
			m_acat = new CaType[] { new CaTypeUnitGroup() };
		}
	}

	[Serializable]
	[DisplayName("Comment")]
	[Description("Use this to comment your actions. It has no in-game effect.")]
	public class CommentTriggerAction : CaBase {
		override public string GetString() {
			return "Comment: $1";
		}
		public CommentTriggerAction() {
			m_acat = new CaType[] { new CaTypeRichText() };
		}
	}

	[Serializable]
	[DisplayName("Hunt Units")]
	[Description("Each unit of the first specified side and type will seek " + 
			"out a unit of the second side and type and attack it and continue " +
			"doing so until it is dead or there are no more enemies.")]
	public class HuntTriggerAction : CaBase {
		override public string GetString() {
			return "$1 of $2 hunt $3 of $4";
		}
		public HuntTriggerAction() {
			m_acat = new CaType[] { new CaTypeUnitTypes(), new CaTypeSide(), new CaTypeUnitTypes(), new CaTypeSide() };
		}
	}

	[Serializable]
	[DisplayName("Create Random Unit Group")]
	[Description("Create a random pre-defined Unit Group and send it on its " +
		 "way according to its intrinsic actions. The Unit Group is selected from" +
		 "amongst those with the \"Random\" box checked.")]
	public class CreateRandomUnitGroupTriggerAction : CaBase {
		override public string GetString() {
			return "Create a random Unit Group";
		}
		public CreateRandomUnitGroupTriggerAction() {
			m_acat = new CaType[0];
		}
	}

	[Serializable]
	[DisplayName("Repair")]
	[Description("Enable or disable repairing of a side's structures. While enabled " +
		 "repairing will continue as long as there are credits available.")]
	public class RepairTriggerAction : CaBase {
		override public string GetString() {
			return "Set repairing for $1 $2";
		}
		public RepairTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeOnOff() };
		}
	}

	[Serializable]
	[DisplayName("Replicator")]
	[Description("Enable or disable a side's Replicator(s).")]
	public class EnableReplicatorTriggerAction : CaBase {
		override public string GetString() {
			return "Turn $1's Replicator $2";
		}
		public EnableReplicatorTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeOnOff() };
		}
	}

	[Serializable]
	[DisplayName("Modify Credits")]
	[Description("Use this to set, add, or subtract Credits for specified players.")]
	public class ModifyCreditsTriggerAction : CaBase {
		override public string GetString() {
			return "$2's Credits: $1 $3";
		}
		public ModifyCreditsTriggerAction() {
			m_acat = new CaType[] { new CaTypeModifyNumber(), new CaTypeSide(), new CaTypeNumber() };
		}
	}
	
	[Serializable]
	[DisplayName("Modify Counter")]
	[Description("Use this to modify the value of a Counter.")]
	public class ModifyCounterTriggerAction : CaBase {
		override public string GetString() {
			return "Counter '$1': $2 $3";
		}
		public ModifyCounterTriggerAction() {
			m_acat = new CaType[] { new CaTypeCounter(), new CaTypeModifyNumber(), new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Move Units In Area")]
	[Description("Use this to units from one area to another.")]
	public class MoveUnitsInAreaTriggerAction : CaBase {
		override public string GetString() {
			return "Move $1 $2 from '$3' to '$4'";
		}
		public MoveUnitsInAreaTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUnitTypes(), new CaTypeArea(), new CaTypeArea() };
		}
	}

	[Serializable]
	[DisplayName("Set Formal Objective Text")]
	[Description("Use this to set one of the objectives shown on the Objectives screen.")]
	public class SetFormalObjectiveTextTriggerAction : CaBase {
		override public string GetString() {
			return "Set formal objective $1 text to \"$2\"";
		}
		public SetFormalObjectiveTextTriggerAction() {
			m_acat = new CaType[] { new CaTypeNumber(), new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Set Formal Objective Status")]
	[Description("Use this to set one of the status of an objective shown on the Objectives screen.")]
	public class SetFormalObjectiveStatusTriggerAction : CaBase {
		override public string GetString() {
			return "Set formal objective $1 status to \"$2\"";
		}
		public SetFormalObjectiveStatusTriggerAction() {
			m_acat = new CaType[] { new CaTypeNumber(), new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Set Formal Objective Info")]
	[Description("Use this to set the informational text shown on the Objectives screen.")]
	public class SetFormalObjectiveInfoTriggerAction : CaBase {
		override public string GetString() {
			return "Set formal objective info to \"$1\"";
		}
		public SetFormalObjectiveInfoTriggerAction() {
			m_acat = new CaType[] { new CaTypeRichText() };
		}
	}

	[Serializable]
	[DisplayName("Show Formal Objectives")]
	[Description("Use this to invoke the Objectives screen. Make sure you've set all the appropriate " +
			"formal objective parameters (text, status, info) first.")]
	public class ShowObjectivesTriggerAction : CaBase {
		override public string GetString() {
			return "Show formal objectives";
		}
		public ShowObjectivesTriggerAction() {
			m_acat = new CaType[] {};
		}
	}

	[Serializable]
	[DisplayName("Cut Scene")]
	[Description("Use this to display a cut scene. Supported cut scene tags: <img image.rbm>.")]
	public class CutSceneTriggerAction : CaBase {
		override public string GetString() {
			return "Cut scene \"$1\"";
		}
		public CutSceneTriggerAction() {
			m_acat = new CaType[] { new CaTypeRichText() };
		}
	}

	[Serializable]
	[DisplayName("Jump To Mission")]
	[Description("Jumps immediately to the specified mission. The mission naming convention is \"mission.lvl\".")]
	public class JumpToMissionTriggerAction : CaBase {
		override public string GetString() {
			return "Jump to mission: \"$1\"";
		}
		public JumpToMissionTriggerAction() {
			m_acat = new CaType[] { new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Create Unit At Area")]
	[Description("Builds or spawns a unit/structure at area. 'Built' units follow some game rules (building takes " +
			"time and requires credits). 'Spawned' units appear immediately and are free. " +
			"Structures are built at the upper-left of the area and the space they require must be unocccupied.")]
	public class CreateUnitAtAreaTriggerAction : CaBase {
		override public string GetString() {
			return "$1 $2 at '$3' for $4 with $5 percent health";
		}
		public CreateUnitAtAreaTriggerAction() {
			m_acat = new CaType[] { new CaTypeCreate(), new CaTypeUnitType(), new CaTypeArea(), new CaTypeOneSide(), new CaTypeNumber() };
		}

		public override string FromSaveString(string strArgs) {
			Regex re = new Regex(@"^\d+,");
			Match m = re.Match(strArgs);
			string strT = re.Split(strArgs)[1];
			CaTypeUnitGroup cat = new CaTypeUnitGroup();
			cat.FromSaveString(strT, false);
			UnitGroup ug = cat.UnitGroup;
			((CaTypeCreate)m_acat[0]).Result = (ug.Spawn ? CreateType.Spawn : CreateType.Build);
			((CaTypeUnitType)m_acat[1]).UnitType = ((UnitTypeAndCount)(ug.UnitTypeAndCounts[0])).ut;
			((CaTypeArea)m_acat[2]).Area = ug.SpawnArea;
			((CaTypeOneSide)m_acat[3]).Side = ug.Side;
			((CaTypeNumber)m_acat[4]).Value = ug.Health;
			return "";
		}

		public override string ToSaveString() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));

			// Create a new UnitGroup

			int iug = lvld.UnitGroupManager.Items.Count;
			UnitGroup ug = new UnitGroup("__cuaa" + iug);
			ug.Spawn = ((CaTypeCreate)m_acat[0]).Result == CreateType.Spawn ? true : false;
			ug.SpawnArea = ((CaTypeArea)m_acat[2]).Area;
			ug.UnitTypeAndCounts.Add(new UnitTypeAndCount(((CaTypeUnitType)m_acat[1]).UnitType, 1));
			ug.Side = ((CaTypeOneSide)m_acat[3]).Side;
			ug.Health = ((CaTypeNumber)m_acat[4]).Value;
			ug.LoopForever = false;
			ug.ReplaceDestroyedGroup = false;
			ug.CreateAtLevelLoad = false;
			ug.Aggressiveness = Aggressiveness.Defender;
			ug.RandomGroup = false;
			lvld.UnitGroupManager.AddUnitGroup(ug);

			string strType = GetType().ToString();
			int ichDot = strType.IndexOf(".");
			strType = strType.Substring(ichDot + 1, strType.Length - ichDot - 1);
			return "kn" + strType + "," + iug.ToString();
		}
	}

	[Serializable]
	[DisplayName("Modify Persistent Variable")]
	[Description("Use this to set, add to, or subtract from the value of persistent variable.")]
	public class ModifyPvarTriggerAction : CaBase {
		override public string GetString() {
			return "Modify persistent variable '$1': $2 $3";
		}
		public ModifyPvarTriggerAction() {
			m_acat = new CaType[] { new CaTypeText(), new CaTypeModifyNumber(), new CaTypeNumber() };
		}
	}
	
	[Serializable]
	[DisplayName("Set Persistent Variable Text")]
	[Description("Use this to set a text string to a persistent variable.")]
	public class SetPvarTextTriggerAction : CaBase {
		override public string GetString() {
			return "Set persistent variable '$1' to \"$2\"";
		}
		public SetPvarTextTriggerAction() {
			m_acat = new CaType[] { new CaTypeText(), new CaTypeText() };
		}
	}
	
	[Serializable]
	[DisplayName("Show Alert")]
	[Description("Use this to show an alert string at the lower-left of the display.")]
	public class ShowAlertTriggerAction : CaBase {
		override public string GetString() {
			return "Show alert \"$1\"";
		}
		public ShowAlertTriggerAction() {
			m_acat = new CaType[] { new CaTypeText() };
		}
	}

	[Serializable]
	[DisplayName("Set Allowed Upgrades")]
	[Description("Use this to specify which upgrades the side is allowed to research. " +
		 "Only these Upgrades will show in the R&D panel. " +
		"NOTE: all upgrades are allowed by default if no 'Set Allowed Upgrades' action is specified.")]
	public class SetAllowedUpgradesTriggerAction : CaBase {
		override public string GetString() {
			return "Set $1 allowed upgrades to $2";
		}
		public SetAllowedUpgradesTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUpgradeTypes() };
		}
	}

	[Serializable]
	[DisplayName("Set Upgrades")]
	[Description("Use this to specify which upgrades the side posseses. " +
		 "NOTE: this can only used as part of a Mission Loaded trigger.")]
	public class SetUpgradesTriggerAction : CaBase {
		override public string GetString() {
			return "Set $1 upgrades to $2";
		}
		public SetUpgradesTriggerAction() {
			m_acat = new CaType[] { new CaTypeSide(), new CaTypeUpgradeTypes() };
		}
	}

	//
	// Unit Group Actions
	//

	[Serializable]
	[DisplayName("Set Switch")]
	[Description("Switch names can be anything (including spaces, etc) but ARE case sensitive. That is, setting " +
		 "a switch named \"Whatever\" and testing \"whatever\" won't work.")]
	public class SetSwitchUnitGroupAction : CaBase {
		override public string GetString() {
			return "Set switch '$1' $2";
		}
		public SetSwitchUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeSwitch(), new CaTypeOnOff() };
		}
	}

	[Serializable]
	[DisplayName("Wait")]
	[Description("Delay a number of seconds before continuing on with the next action.")]
	public class WaitUnitGroupAction : CaBase {
		override public string GetString() {
			return "Wait $1 seconds";
		}
		public WaitUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Move")]
	[Description("All members of the group are directed to the specified Area. This action " +
			"is not considered complete and the next action will not execute until all " +
			"the members of the group reach the destination (or as close as they can " +
			"reasonably get).")]
	public class MoveUnitGroupAction : CaBase {
		override public string GetString() {
			return "Move to area '$1'";
		}
		public MoveUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}

	[Serializable]
	[DisplayName("Attack")]
	[Description("Attack the nearest enemy unit matching the specified side(s) and type(s) for the specified interval. " +
			"The entire group will attack the unit together. If the unit is destroyed before the interval " +
			"elapses the next nearest matching enemy will be targeted by the group.")]
	public class AttackUnitGroupAction : CaBase {
		override public string GetString() {
			return "Attack nearest $1 owned by $2 for $3 seconds";
		}
		public AttackUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeUnitTypes(), new CaTypeSide(), new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Comment")]
	[Description("Use this to comment your actions. It has no in-game effect.")]
	public class CommentUnitGroupAction : CaBase {
		override public string GetString() {
			return "Comment: $1";
		}
		public CommentUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeRichText() };
		}
	}

	[Serializable]
	[DisplayName("Guard")]
	[Description("All members of the group are directed guard the area within " +
		 "their sight range for the specified interval.")]
	public class GuardUnitGroupAction : CaBase {
		override public string GetString() {
			return "Guard for $1 seconds";
		}
		public GuardUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("GuardVicinity")]
	[Description("All members of the group are directed guard the area within " +
		 "an expanded sight range for the specified interval.")]
	public class GuardVicinityUnitGroupAction : CaBase {
		override public string GetString() {
			return "Guard vicinity for $1 seconds";
		}
		public GuardVicinityUnitGroupAction() {
			m_acat = new CaType[] { new CaTypeNumber() };
		}
	}

	[Serializable]
	[DisplayName("Mine")]
	[Description("Find the nearest Galaxite, mine it, and bring it to the nearest Processor. " +
			 "Repeat forever or until death. NOTE: This action has no effect on any unit other than the Bullpup.")]
	public class MineUnitGroupAction : CaBase {
		override public string GetString() {
			return "Mine";
		}
		public MineUnitGroupAction() {
			m_acat = new CaType[] { };
		}
	}

	//
	// Unit Actions
	//

	[Serializable]
	[DisplayName("Guard")]
	[Description("Guard the area within the unit's immediate range of sight.")]
	public class GuardUnitAction : CaBase {
		override public string GetString() {
			return "Guard sight range";
		}
		public GuardUnitAction() {
			m_acat = new CaType[0];
		}
	}

	[Serializable]
	[DisplayName("Guard Vicinity")]
	[Description("Guard a largish area surrounding the unit.")]
	public class GuardVicinityUnitAction : CaBase {
		override public string GetString() {
			return "Guard vicinity";
		}
		public GuardVicinityUnitAction() {
			m_acat = new CaType[0];
		}
	}

	[Serializable]
	[DisplayName("Guard Area")]
	[Description("Guard a specific area. Any enemies entering the area willl be attacked. " +
			"If the enemy is destroyed the unit will sit idle until another enemy enters the area.")]
	public class GuardAreaUnitAction : CaBase {
		override public string GetString() {
			return "Guard '$1'";
		}
		public GuardAreaUnitAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}

	[Serializable]
	[DisplayName("Move")]
	[Description("Move to the specified area. Depending on the unit's "	+ 
			"Aggressiveness setting it may attack enemies along the way.")]
	public class MoveUnitAction : CaBase {
		override public string GetString() {
			return "Move to '$1'";
		}
		public MoveUnitAction() {
			m_acat = new CaType[] { new CaTypeArea() };
		}
	}

	[Serializable]
	[DisplayName("Hunt Enemies")]
	[Description("Find an enemy unit/structure of the specified type(s) and attack it. " +
			"If this unit survives the encounter it will continue the hunt. " +
			"The enemy unit is chosen randomly, not based on proximity or any other factors.")]
	public class HuntEnemiesUnitAction : CaBase {
		override public string GetString() {
			return "Hunt $1";
		}
		public HuntEnemiesUnitAction() {
			m_acat = new CaType[] { new CaTypeUnitTypes() };
		}
	}

	[Serializable]
	[DisplayName("Mine")]
	[Description("Find the nearest Galaxite, mine it, and bring it to the nearest Processor. " +
			 "Repeat forever or until death. NOTE: This action has no effect on any unit other than the Bullpup.")]
	public class MineUnitAction : CaBase {
		override public string GetString() {
			return "Mine";
		}
		public MineUnitAction() {
			m_acat = new CaType[] { };
		}
	}
}
