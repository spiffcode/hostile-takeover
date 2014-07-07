using System;
using System.Collections;
using SpiffLib;

namespace m {
	[Serializable]
	public class Trigger {
		int m_nfSides;
		ArrayList m_alsConditions;
		ArrayList m_alsActions;

		public Trigger() {
			m_nfSides = 0;
			m_alsConditions = new ArrayList();
			m_alsActions = new ArrayList();
		}

		public virtual Trigger Clone() {
			Trigger tgr = new Trigger();
			tgr.Sides = Sides;
			foreach (CaBase cab in Conditions)
				tgr.Conditions.Add(cab.Clone());
			foreach (CaBase cab in Actions)
				tgr.Actions.Add(cab.Clone());
			return tgr;
		}

		public bool IsValid() {
			return GetError() == null;
		}

		public string GetError() {
			// Invalid if 0 sides

			if (m_nfSides == 0)
				return "No Sides selected";

			// Invalid if 0 conditions

			if (m_alsConditions.Count == 0)
				return "No Conditions Entered";

			// All conditions must be valid

			int n = 0;
			foreach (CaBase cab in m_alsConditions) {
				if (!cab.IsValid()) {
					return "Condition " + n + " is invalid";
				}
				n++;
			}

			// Invalid if 0 actions

			if (m_alsActions.Count == 0)
				return "No Actions Entered";

			// All actions must be valid

			n = 0;
			foreach (CaBase cab in m_alsActions) {
				if (!cab.IsValid()) {
					return "Action " + n + " is invalid";
				}
				n++;
			}

			// Looks good

			return null;
		}

		public ArrayList Conditions {
			get {
				return m_alsConditions;
			}
		}

		public ArrayList Actions {
			get {
				return m_alsActions;
			}
		}

		public int Sides {
			get {
				return m_nfSides;
			}
			set {
				m_nfSides = value;
			}
		}

		public void AddIniProperties(Ini.Section sec) {
			// Save conditions & actions

			foreach (CaBase cab in m_alsConditions) {
				if (!(cab is CommentCondition))
					sec.Add(new Ini.Property("C", cab.ToSaveString()));
			}

			foreach (CaBase cab in m_alsActions) {
				if (!(cab is CommentTriggerAction))
					sec.Add(new Ini.Property("A", cab.ToSaveString()));
			}
		}
	}
}
