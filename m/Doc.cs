using System;
using System.IO;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters;
using System.Runtime.Serialization.Formatters.Binary;

namespace m
{
	public enum Command { Cut, Copy, Paste, Delete };

	public interface ICommandTarget {
		void DispatchCommand(Command cmd);
	}
	
	public class DocManager {
		static ArrayList s_alsTemplates = new ArrayList();
		static Form s_frmParent = null;
		static ICommandTarget m_cmdt;

		public static void SetCommandTarget(ICommandTarget cmdt) {
			m_cmdt = cmdt;
		}

		public static ICommandTarget GetCommandTarget() {
			return m_cmdt;
		}

		public static void SetFrameParent(Form frmParent) {
			s_frmParent = frmParent;
		}

		public static Form GetFrameParent() {
			return s_frmParent;
		}

		public static void AddTemplate(DocTemplate doct) {
			s_alsTemplates.Add(doct);
		}

		public static void RemoveTemplate(DocTemplate doct) {
			s_alsTemplates.Remove(doct);
		}

		public static DocTemplate FindDocTemplate(Type type) {
			foreach (DocTemplate doct in s_alsTemplates) {
				if (doct.GetDocumentType() == type)
					return doct;
			}
			return null;
		}

		public static Document NewDocument(Type typeDoc, Object[] aobj) {
			DocTemplate doct = FindDocTemplate(typeDoc);
			return doct.NewDocument(aobj);
		}

		public static Document OpenDocument(Type typeDoc) {
			DocTemplate doct = FindDocTemplate(typeDoc);
			return doct.OpenDocument();
		}

		public static Document OpenDocument(string strFileName) {
			string extFile = Path.GetExtension(strFileName);
			foreach (DocTemplate doct in s_alsTemplates) {
				string extDoc = doct.GetString(DocTemplate.Strings.FilterExt);
				if (extFile == "." + extDoc)
					return doct.OpenDocument(strFileName);
			}
			return null;
		}

		public static bool SaveAllModified(Type type) {
			foreach (DocTemplate doct in s_alsTemplates) {
				if (type == null || doct.GetDocumentType() == type) {
					if (!doct.SaveAllModified())
						return false;
				}
			}
			return true;
		}

		public static bool CloseAllDocuments() {
			foreach (DocTemplate doct in s_alsTemplates) {
				if (!doct.CloseAllDocuments())
					return false;
			}
			return true;
		}

		public static Document GetActiveDocument(Type typeDoc) {
			DocTemplate doct = FindDocTemplate(typeDoc);
			return doct.GetActiveDocument();
		}

		public static void SetActiveDocument(Type typeDoc, Document doc) {
			DocTemplate doct = FindDocTemplate(typeDoc);
			doct.SetActiveDocument(doc);
		}
	}

	public class DocTemplate {
		string[] m_astr;
		protected Type m_typeDoc;
		protected Type m_typeFrame;
		protected Type m_typeView;
		protected ArrayList m_alsDocuments;
		protected SerializationBinder m_serBinder;
		protected Document m_docActive = null;

		public delegate void DocActiveHandler(Document doc);
		public event DocActiveHandler DocActive;
		public delegate void DocAddedHandler(Document doc);
		public event DocAddedHandler DocAdded;
		public delegate void DocRemovedHandler(Document doc);
		public event DocRemovedHandler DocRemoved;

		public DocTemplate(string[] astr, Type typeDoc, Type typeFrame, Type typeView, SerializationBinder serBinder) {
			m_astr = (string[])astr.Clone();
			m_typeDoc = typeDoc;
			m_typeFrame = typeFrame;
			m_typeView = typeView;
			m_alsDocuments = new ArrayList();
			m_serBinder = serBinder;
		}

		public Type GetDocumentType() {
			return m_typeDoc;
		}

		public virtual Document NewDocument(Object[] aobj) {
			// Create a new (empty) document

			Object[] aobjT = { this, null, aobj };
			Document doc = (Document)System.Activator.CreateInstance(m_typeDoc, aobjT);

			// Success?

			if (doc == null)
				return null;

			// Add to doc list

			AddDocument(doc);
			return OpenFinish(doc);
		}

		public virtual Document OpenDocument() {
			string strExt = GetString(DocTemplate.Strings.FilterExt);
			string strFilterName = GetString(DocTemplate.Strings.FilterName);
			string strTitle = GetString(DocTemplate.Strings.WindowTitle);
			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.DefaultExt = strExt;
			frmOpen.Filter = strFilterName + " (*." + strExt + ")|*." + strExt;
			frmOpen.Title = "Open " + strTitle;
			if (frmOpen.ShowDialog() == DialogResult.Cancel)
				return null;
			return OpenDocument(frmOpen.FileName);
		}

		public virtual Document OpenDocument(string strFile) {
			// Already open? If so, return the already open document after incrementing its open count

			Document doc = null;
			string strPath = Path.GetFullPath(strFile);
			foreach (Document docT in m_alsDocuments) {
				string strPathT = docT.GetPath();
				if (strPathT == null)
					continue;
				if (strPathT.ToLower() == strPath.ToLower()) {
					return OpenFinish(docT);
				}
			}

			// Not already open - open it

			Stream stm = null;
			try {
				Hashtable ht = new Hashtable();
				ht.Add("DocTemplate", this);
				ht.Add("Filename", strFile);
				IFormatter fmtr = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.File, ht));
				if (m_serBinder != null)
					fmtr.Binder = m_serBinder;
				stm = new FileStream(strFile, FileMode.Open, FileAccess.Read, FileShare.Read);
				doc = (Document)fmtr.Deserialize(stm);
				stm.Close();

			} catch (SerializationException ex) {
				string strErr = String.Format("Failed to load {0}\n{1}", strFile, ex.ToString());
				MessageBox.Show(strErr, "Deserialization Error");

				if (stm != null)
					stm.Close();
				doc = null;

			} catch (System.Reflection.TargetInvocationException ex) {
				string strErr = String.Format("Failed to load {0}\n{1}", strFile, ex.ToString());
				MessageBox.Show(strErr, "Deserialization TargetInvocation Error");

				if (stm != null)
					stm.Close();
				doc = null;
			}

			// Success?

			if (doc == null)
				return null;

			// Add to doc list

			AddDocument(doc);
			return OpenFinish(doc);
		}


		Document OpenFinish(Document doc) {
			// This doc is being opened

			doc.IncrementOpenCount();

			// Doc is not modified

			doc.SetModified(false);

			// Make it active

			SetActiveDocument(doc);

			// Create a frame window of the desired type. It might already know what
			// view type to create, but parameterize it just in case

			if (m_typeFrame != null) {
				Object[] aobjT = { DocManager.GetFrameParent(), doc, m_typeView };
				System.Activator.CreateInstance(m_typeFrame, aobjT);
			}

			// The doc is ready to go

			doc.InitDone();
			return doc;
		}

		public virtual void AddDocument(Document doc) {
			m_alsDocuments.Add(doc);
			if (DocAdded != null)
				DocAdded(doc);
		}

		public virtual void RemoveDocument(Document doc) {
			m_alsDocuments.Remove(doc);
			if (doc == m_docActive) {
				if (m_alsDocuments.Count != 0) {
					SetActiveDocument((Document)m_alsDocuments[0]);
				} else {
					SetActiveDocument(null);
				}
			}
			if (DocRemoved != null)
				DocRemoved(doc);
		}

		public virtual Document[] GetDocuments() {
			return (Document[])m_alsDocuments.ToArray(typeof(Document));
		}

		public enum Strings { WindowTitle, NewFileName, FilterName, FilterExt };

		public virtual string GetString(Strings enmStr) {
			return m_astr[(int)enmStr];
		}

		public virtual bool SaveAllModified() {
			foreach (Document doc in m_alsDocuments) {
				if (doc.IsModified()) {
					if (!doc.Save())
						return false;
				}
			}
			return true;
		}

		public virtual bool CloseAllDocuments() {
			while (m_alsDocuments.Count != 0) {
				Document doc = (Document)m_alsDocuments[0];
				if (!doc.Close())
					return false;
			}
			return true;
		}

		public virtual Document GetActiveDocument() {
			return m_docActive;
		}

		public virtual void SetActiveDocument(Document docActive) {
			if (Globals.PropertyGrid != null) {
				if (Globals.PropertyGrid.SelectedObject != null) {
					if (Globals.PropertyGrid.SelectedObject.GetType() == m_typeDoc) {
						Globals.PropertyGrid.SelectedObject = (object)docActive;
					} else {
						Globals.PropertyGrid.SelectedObject = null;
					}
				}
			}
			if (m_docActive == docActive)
				return;
			m_docActive = docActive;
			if (DocActive != null)
				DocActive(docActive);
		}
	}

	public class Document {
		protected DocTemplate m_doct;
		protected string m_strDir;
		protected string m_strFileName;
		protected bool m_fModified;
		protected bool m_fIniting;
		protected int m_cOpen;

		public delegate void ModifiedChangedHandler(Document doc, bool fModified);
		public event ModifiedChangedHandler ModifiedChanged;
		public delegate void PathChangedHandler(Document doc);
		public event PathChangedHandler PathChanged;
		public delegate void OpenCountChangedHandler(Document doc);
		public event OpenCountChangedHandler OpenCountChanged;

		public Document(DocTemplate doct, string strFile) {
			m_doct = doct;
			SetPath(strFile);
			m_fModified = false;
			m_fIniting = true;
			m_cOpen = 0;
		}

		public DocTemplate GetDocTemplate() {
			return m_doct;
		}

		public virtual void InitDone() {
			m_fIniting = false;
		}

		public bool IsModified() {
			return m_fModified;
		}

		public void SetModified(bool fModified) {
			if (m_fIniting)
				return;
			if (fModified == m_fModified)
				return;
			m_fModified = fModified;
			if (ModifiedChanged != null)
				ModifiedChanged(this, fModified);
		}

		public virtual string GetName() {
			return m_doct.GetString(DocTemplate.Strings.NewFileName);
		}

		public virtual string GetPath() {
			if (m_strFileName == null)
				return null;
			return m_strDir + Path.DirectorySeparatorChar + m_strFileName;
		}

		public virtual void SetPath(string strFile) {
			string strDir;
			string strFileName;
			if (strFile == null) {
				strDir = null;
				strFileName = null;
			} else {
				strDir = Path.GetDirectoryName(Path.GetFullPath(strFile));
				if (strDir == "")
					strDir = ".";
				strFileName = Path.GetFileName(strFile);
			}
			if (strDir != m_strDir || strFileName != m_strFileName) {
				m_strDir = strDir;
				m_strFileName = strFileName;
				if (PathChanged != null)
					PathChanged(this);
			}
		}

		public virtual int GetOpenCount() {
			return m_cOpen;
		}

		public virtual void DecrementOpenCount() {
			m_cOpen--;
			if (OpenCountChanged != null)
				OpenCountChanged(this);
		}

		public virtual void IncrementOpenCount() {
			m_cOpen++;
			if (OpenCountChanged != null)
				OpenCountChanged(this);
		}

		public virtual bool Save() {
			if (m_strFileName == null)
				return SaveAs(null);
			return SaveHelper(m_strDir + Path.DirectorySeparatorChar + m_strFileName);
		}

		bool SaveHelper(string strFile) {
			Stream stm = null;
			try {
				IFormatter fmtr = new BinaryFormatter();
				stm = new FileStream(strFile, FileMode.Create, FileAccess.Write, FileShare.None);
				fmtr.Serialize(stm, this);
				stm.Close();
			} catch (Exception ex) {
				if (stm != null)
					stm.Close();
				MessageBox.Show("Error saving..." + ex.ToString());
				return false;
			}

			SetPath(strFile);
			SetModified(false);
			return true;
		}

		public virtual bool SaveAs(string strFile) {
			// Have a filename? use it.

			if (strFile != null) {
				if (SaveHelper(strFile)) {
					SetPath(strFile);
					return true;
				}
				return false;
			}

			// Open dialog

			if (m_strFileName != null)
				strFile = m_strDir + Path.DirectorySeparatorChar + m_strFileName;

			SaveFileDialog frmSave = new SaveFileDialog();

			string strExt = m_doct.GetString(DocTemplate.Strings.FilterExt);
			string strFilterName = m_doct.GetString(DocTemplate.Strings.FilterName);
			string strTitle = m_doct.GetString(DocTemplate.Strings.WindowTitle);
			string strFileNew = m_doct.GetString(DocTemplate.Strings.NewFileName);

			frmSave.DefaultExt = strExt;
			frmSave.Filter = strFilterName + " (*." + strExt + ")|*." + strExt;
			frmSave.Title = "Save " + GetName() + " As...";
			frmSave.FileName = strFile != null ? strFile : strFileNew;
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return false;

			return SaveHelper(frmSave.FileName);
		}

		public bool Close() {
			// Don't close if the open count is not zero yet

			if (GetOpenCount() > 1) {
				DecrementOpenCount();
				return true;
			}

			// Ask if modified

			if (m_fModified) {
				string strName;
				string strPath = GetPath();
				if (strPath == null) {
					strName = GetName();
				} else {
					strName = Path.GetFileName(strPath);
				}
				switch (MessageBox.Show("Save changes to " + strName + "?", Application.ProductName, MessageBoxButtons.YesNoCancel)) {
				case DialogResult.Cancel:
					return false;

				case DialogResult.No:
					break;

				case DialogResult.Yes:
					if (!Save())
						return false;
					break;
				}
			}
			m_doct.RemoveDocument(this);
			return true;
		}
	}
}
