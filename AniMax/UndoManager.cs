using System;
using System.Collections;
using System.Diagnostics;

namespace SpiffCode {
	public delegate void UndoDelegate(object[] aobArgs);

	public class UndoManager {
		private static Stack s_stk = new Stack();
		private static int s_cElements;
		private static bool s_fGrouping = false;
		
		public static void AddUndo(UndoDelegate dgtUndo, object[] aobArgs) {
			if (s_fGrouping)
				s_cElements++;

			s_stk.Push(aobArgs);
			s_stk.Push(dgtUndo);
		}

		public static void Flush() {
			s_stk.Clear();
		}

		public static void Undo() {
			if (s_stk.Count == 0)
				return;

			UndoDelegate dgtUndo;
			object[] aobArgs;

			object ob = s_stk.Pop();
			if (ob is int) {
				int cElements = (int)ob;
				for (int i = 0; i < cElements; i++) {
					dgtUndo = (UndoDelegate)s_stk.Pop();
					aobArgs = (object[])s_stk.Pop();
					dgtUndo(aobArgs);
				}
				return;
			}

			dgtUndo = (UndoDelegate)ob;
			aobArgs = (object[])s_stk.Pop();
			dgtUndo(aobArgs);
		}

		public static void Redo() {
			// UNDONE:
		}

		public static bool AnyUndos() {
			return s_stk.Count != 0;
		}

		// WARNING: can't do nested groups

		public static void BeginGroup() {
			Debug.Assert(!s_fGrouping);
			s_fGrouping = true;
			s_cElements = 0;
		}

		public static void EndGroup() {
			s_fGrouping = false;
			s_stk.Push(s_cElements);
		}
	}
}
