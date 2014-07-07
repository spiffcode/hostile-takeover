// NOTES:
// on attempting to call a script-defined JScript function --
// - can't find an instance to call the function methods via
// - function methods expect a 'this' (type unknown), an engine instance, and a parameter array
// - the containing class type is called "JScript 1" and the number increments with each instatiation of the script engine
// - JScript 1 contains another method called "Global Code" which appears to be the, uh, global script code
// - the .ctor for JScript 1 requires an arg, Global Scope
// - easiest thing to do seems to be to create a scaffolding class around user script code that
// puts the global code in a constructor or other well-known method and results in user-defined
// functions being members of this class. Then it can be instanced and its members easily
// enumerated. But what of user-defined classes, etc? P.S. I've verified that members of such
// a wrapped class have the necessary context when called to do JScript-y things like
// calling "print".
// - JScript Engine appears to have an option to keep it from generating the _Startup class
// - JScript has other interesting options -- find some documentation
// - the above approach is jscript-specific. Each plugged-in script language would need its
// own wrapper-generator. Another approach would be to establish a well known name (e.g., Macros)
// and leave it to script writers to provide the class definition.
// UNDONE: rework Compile/Run so Run() leaves the engine running and returns an instance
// of the script.Macros class.

/* Sample scripts:

// 1. Offset all frames

for (frm in AED.Frames) {
	frm.OriginX -= 16;
	frm.OriginY -= 15;
}

// 2. Change a color in all frames

for (frm in AED.Frames) {
	var clrOld = new Array(156, 208, 248)
	var clrNew = new Array(156, 212, 248)
	frm.ReplaceColor(clrOld, clrNew, true)
}

// 3. Change a bunch of colors in all frames

for (frm in AED.Frames) {
	var clrOld = new Array(0,114,232)
	var clrNew = new Array(0,116,232)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,112,232)
	clrNew = new Array(0,116,232)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,96,192)
	clrNew = new Array(0,96,196)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,48,88)
	clrNew = new Array(0,48,92)
	frm.ReplaceColor(clrOld, clrNew, true)
}

*/


using System;
using System.Collections;
using Microsoft.Vsa;
using Microsoft.JScript;
using System.Windows.Forms;
using System.Diagnostics;
using System.Reflection;

namespace AED
{
	/// <summary>
	/// Summary description for Script.
	/// </summary>

	public delegate void CompileErrorEventHandler(object obSender, IVsaError vsaerr);

	/// <summary>
	/// 
	/// </summary>
	public class ScriptEngine : IVsaSite {
		Microsoft.JScript.Vsa.VsaEngine m_vsae = null;
		IVsaCodeItem m_vsaciScript;
		Hashtable m_htEventSources = new Hashtable();
		Hashtable m_htGlobals = new Hashtable();

		public ScriptEngine() {
		}

		public void Init() {
			// UNDONE: this is crap. The whole engine shouldn't have to be recreated and
			// reinitialized for each compile. Unfortunately it's the only way I've found
			// to work around the "Variable 'AED' has not been declared" problem.
			// A better solution needs to be found as this one seems to consume roughly
			// 240K of RAM each compile.

			m_vsae = new Microsoft.JScript.Vsa.VsaEngine();
			m_vsae.RootMoniker = "com.spiffcode://script";
			m_vsae.Site = this;
			m_vsae.InitNew();

			m_vsae.SetOption("print", true);	// Enable the 'print' function for scripters
			m_vsae.SetOption("fast", false);	// Enable legacy mode
			m_vsae.RootNamespace = "script";
			m_vsaciScript = (IVsaCodeItem)m_vsae.Items.CreateItem("scriptspace",
					VsaItemType.Code, VsaItemFlag.None);

			m_vsae.Items.CreateItem("mscorlib.dll", VsaItemType.Reference, VsaItemFlag.None);
			m_vsae.Items.CreateItem("System.Windows.Forms.dll", VsaItemType.Reference, VsaItemFlag.None);
//			m_vsae.Items.CreateItem("System.dll", VsaItemType.Reference, VsaItemFlag.None);

			string[] astrNames = new String[m_htGlobals.Count];
			m_htGlobals.Keys.CopyTo(astrNames, 0);

			for (int i = 0; i < m_htGlobals.Count; i++) {
				IVsaGlobalItem vsagi = (IVsaGlobalItem)m_vsae.Items.CreateItem(astrNames[i],
						VsaItemType.AppGlobal, VsaItemFlag.None);
				// UNDONE: this doesn't seem to be working
				vsagi.ExposeMembers = true;
//				object obInstance = m_htGlobals[astrNames[i]];
//				vsagi.TypeString = obInstance.GetType().FullName;
			}
		}

		public bool Compile(string strScript) {
			if (m_vsae != null) {
				m_vsae.Close();
				m_vsae = null;
			}

			Init();

			m_vsaciScript.SourceText = WrapUserScript(strScript);
			return m_vsae.Compile();
		}

		public bool Run() {
			Debug.Assert(m_vsae != null);

			try {
				m_vsae.Run();

#if false
				Assembly mbly = m_vsae.Assembly;
				object ob = mbly.CreateInstance("script.Foo");
				Type[] atyp = mbly.GetTypes();
				Type typMain = atyp[0];
				MemberInfo[] amthi = typMain.GetMembers();
				foreach (MemberInfo mthi in amthi) {
					MessageBox.Show(mthi.ToString());
				}
				object obT = mbly.CreateInstance(atyp[0].Name);
				MessageBox.Show(obT.ToString());
				obT = null;
				atyp = null;
				mbly = null;
#endif

//			} catch (VsaException exVsa) {
//				return false;
			} finally {
				m_vsae.Close();
				m_vsae = null;
				OnScriptDone(EventArgs.Empty);
			}
			return true;
		}

		private string WrapUserScript(string strUserScript) {
			return strUserScript;
//			return "class Macros {\n" + strUserScript + "\n}";
		}

		public void AddGlobal(string strName, object obInstance) {
			m_htGlobals.Add(strName, obInstance);
		}

		public event EventHandler ScriptDone;

		protected virtual void OnScriptDone(EventArgs evta) {
			if (ScriptDone != null)
				ScriptDone(this, evta);
		}

		public event CompileErrorEventHandler CompileError;

		protected virtual void OnCompileError(IVsaError vsaerr) {
			if (CompileError != null)
				CompileError(this, vsaerr);
			else
				MessageBox.Show("line " + vsaerr.Line + ": " + vsaerr.Description, "Compile Error");
		}

		//
		// IVsaSite implementation
		//

		public void AddEventSource(string strName, object obEventSource) {
			m_htEventSources.Add(strName, obEventSource);
		}

		public void GetCompiledState(out byte[] pe, out byte[] debugInfo) {
			pe = null;
			debugInfo = null;
		}

		public object GetEventSourceInstance(string itemName, string strEventSourceName) {
			return m_htEventSources[strEventSourceName];
		}

		public object GetGlobalInstance(string strName) {
			return m_htGlobals[strName];
		}

		public void Notify(string notify, object info) {
		}

		public bool OnCompilerError(IVsaError vsaerr) {
			OnCompileError(vsaerr);
//			throw new Exception("line " + vsaerr.Line + ": " + vsaerr.Description + "\n" + vsaerr.LineText);
			return true;
		}
	}
}
