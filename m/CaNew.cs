using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;

namespace m
{
	/// <summary>
	/// Summary description for CaNew.
	/// </summary>
	public class CaNew : System.Windows.Forms.Form
	{
		Type[] m_atype;
		CaBase m_cab;
		string m_strParse;
		string m_strKind;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Label m_labelWhatType;
		private System.Windows.Forms.Label m_labelText;
		private System.Windows.Forms.RichTextBox m_richTextBox;
		private System.Windows.Forms.ComboBox m_comboBoxType;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.Label m_labelDescription;
		private System.Windows.Forms.GroupBox groupBox1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CaNew(CaBase cab, string strTitle, string strKind, Type[] atype, string[] astrName)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_atype = atype;
			m_cab = null;
			m_strParse = null;
			m_strKind = strKind;

			// To do - might want to pretty print these type names

			Text = strTitle;
			m_labelWhatType.Text = m_labelWhatType.Text.Replace("$1", strKind);
			m_labelText.Text = m_labelText.Text.Replace("$1", strKind);
			Array.Sort(astrName, atype);
			Array.Sort(astrName);
			m_comboBoxType.DataSource = astrName;
			m_cab = cab;
			if (cab != null) {
				m_comboBoxType.SelectedIndex = Array.IndexOf(atype, m_cab.GetType());
			} else {
				m_comboBoxType.SelectedIndex = 0;
			}
			SelectCa(m_comboBoxType.SelectedIndex);
		}

		CaBase GetCab() {
			return m_cab;
		}

		public static CaBase DoModal(CaBase cab, string strTitle, string strKind) {
			string strEndsWith = strKind;
			System.Reflection.Assembly ass = typeof(CaBase).Module.Assembly;
			Type[] atype = ass.GetTypes();
			ArrayList alsType = new ArrayList();
			ArrayList alsName = new ArrayList();
			foreach (Type type in atype) {
				string strName = type.ToString();
				if (strName.EndsWith(strEndsWith)) {
					alsType.Add(type);
					string strDisplayName = Helper.GetDisplayName(type);
					if (strDisplayName == null) {
						int ichDot = strName.IndexOf('.');
						strDisplayName = strName.Substring(ichDot + 1, strName.Length - (ichDot + 1 + strEndsWith.Length));
					}
					alsName.Add(strDisplayName);
				}
			}
			CaNew frmCaNew = new CaNew(cab, strTitle, strKind, (Type[])alsType.ToArray(typeof(Type)), (string[])alsName.ToArray(typeof(string)) );
			frmCaNew.ShowDialog();
			if (frmCaNew.DialogResult == DialogResult.Cancel)
				return null;
			return frmCaNew.GetCab();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.m_labelWhatType = new System.Windows.Forms.Label();
			this.m_comboBoxType = new System.Windows.Forms.ComboBox();
			this.m_labelText = new System.Windows.Forms.Label();
			this.button1 = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.m_richTextBox = new System.Windows.Forms.RichTextBox();
			this.m_labelDescription = new System.Windows.Forms.Label();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// m_labelWhatType
			// 
			this.m_labelWhatType.Location = new System.Drawing.Point(8, 16);
			this.m_labelWhatType.Name = "m_labelWhatType";
			this.m_labelWhatType.Size = new System.Drawing.Size(496, 16);
			this.m_labelWhatType.TabIndex = 0;
			this.m_labelWhatType.Text = "&What type of $1 do you wish to create?";
			// 
			// m_comboBoxType
			// 
			this.m_comboBoxType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.m_comboBoxType.Location = new System.Drawing.Point(8, 40);
			this.m_comboBoxType.MaxDropDownItems = 16;
			this.m_comboBoxType.Name = "m_comboBoxType";
			this.m_comboBoxType.Size = new System.Drawing.Size(496, 21);
			this.m_comboBoxType.TabIndex = 1;
			this.m_comboBoxType.SelectedIndexChanged += new System.EventHandler(this.m_comboBoxType_SelectedIndexChanged);
			// 
			// m_labelText
			// 
			this.m_labelText.Location = new System.Drawing.Point(8, 80);
			this.m_labelText.Name = "m_labelText";
			this.m_labelText.Size = new System.Drawing.Size(496, 16);
			this.m_labelText.TabIndex = 2;
			this.m_labelText.Text = "&$1 Text (click on underlined words to set values):";
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(168, 328);
			this.button1.Name = "button1";
			this.button1.TabIndex = 4;
			this.button1.Text = "Ok";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// buttonCancel
			// 
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point(288, 328);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.TabIndex = 4;
			this.buttonCancel.Text = "Cancel";
			this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
			// 
			// m_richTextBox
			// 
			this.m_richTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.m_richTextBox.Location = new System.Drawing.Point(8, 104);
			this.m_richTextBox.Name = "m_richTextBox";
			this.m_richTextBox.ReadOnly = true;
			this.m_richTextBox.Size = new System.Drawing.Size(496, 120);
			this.m_richTextBox.TabIndex = 5;
			this.m_richTextBox.Text = "richTextBox1";
			this.m_richTextBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.m_richTextBox_MouseDown);
			// 
			// m_labelDescription
			// 
			this.m_labelDescription.Dock = System.Windows.Forms.DockStyle.Fill;
			this.m_labelDescription.ForeColor = System.Drawing.Color.Indigo;
			this.m_labelDescription.Location = new System.Drawing.Point(3, 16);
			this.m_labelDescription.Name = "m_labelDescription";
			this.m_labelDescription.Size = new System.Drawing.Size(490, 61);
			this.m_labelDescription.TabIndex = 7;
			this.m_labelDescription.Text = "label2";
			this.m_labelDescription.UseMnemonic = false;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.m_labelDescription});
			this.groupBox1.Location = new System.Drawing.Point(8, 240);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(496, 80);
			this.groupBox1.TabIndex = 8;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Description";
			// 
			// CaNew
			// 
			this.AcceptButton = this.button1;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size(514, 360);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.groupBox1,
																		  this.m_richTextBox,
																		  this.button1,
																		  this.m_labelText,
																		  this.m_comboBoxType,
																		  this.m_labelWhatType,
																		  this.buttonCancel});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "CaNew";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "CaNew";
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		void SelectCa(int n) {
			if (m_cab == null || m_cab.GetType() != m_atype[n])
				m_cab = (CaBase)System.Activator.CreateInstance(m_atype[n]);

			m_labelDescription.Text = Helper.GetDescription(m_cab.GetType());

			m_richTextBox.Clear();

			// String replacement, order independent

			string str = m_cab.GetString();
			CaType[] acat = m_cab.GetTypes();
			for (int j = 0; j < acat.Length; j++)
				str = str.Replace("$" + (j + 1), "~" + j + acat[j].ToString() + "~" + j);

			// Save this away for hittesting purposes

			m_strParse = (string)str.Clone();

			// && delimited pieces are links

			while (str.Length != 0) {
				int ichT = str.IndexOf("~");
				if (ichT == -1) {
					m_richTextBox.AppendText(str);
					break;
				}
				if (ichT != 0)
					m_richTextBox.AppendText(str.Substring(0, ichT));
				str = str.Remove(0, ichT + 2);

				// Now add the underlined text

				int ichStart = m_richTextBox.TextLength;
				int cchLink = str.IndexOf("~");
				Debug.Assert(cchLink != -1);
				m_richTextBox.AppendText(str.Substring(0, cchLink));
				str = str.Remove(0, cchLink + 2);
				m_richTextBox.Select(ichStart, cchLink);
				Color clr = m_richTextBox.SelectionColor;
				Font fnt = m_richTextBox.SelectionFont;
				m_richTextBox.SelectionColor = Color.Blue;
				m_richTextBox.SelectionFont = new Font(fnt.FontFamily, fnt.Size, /* FontStyle.Bold | */ FontStyle.Underline);
				m_richTextBox.Select(m_richTextBox.TextLength, 0);
				m_richTextBox.SelectionFont = fnt;
				m_richTextBox.SelectionColor = clr;
			}
		}

		private void m_comboBoxType_SelectedIndexChanged(object sender, System.EventArgs e) {
			SelectCa(m_comboBoxType.SelectedIndex);
		}

		int GetCatIndexFromCharIndex(int ich) {
			int ichTranslated = 0;
			int icat = -1;
			for (int ichT = 0; ichT < m_strParse.Length; ichT++) {
				if (m_strParse[ichT] == '~') {
					if (icat == -1) {
						icat = m_strParse[ichT + 1] - '0';
					} else {
						icat = -1;
					}
					ichT++;
					continue;
				}
				if (ich == ichTranslated)
					return icat;
				ichTranslated++;
			}
			return -1;
		}

		private void m_richTextBox_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			int ich = m_richTextBox.GetCharIndexFromPosition(new Point(e.X, e.Y));
			if (ich == -1)
				return;
			int icat = GetCatIndexFromCharIndex(ich);
			if (icat == -1)
				return;
			CaType[] acat = m_cab.GetTypes();
			if (acat[icat].EditProperties())
				SelectCa(m_comboBoxType.SelectedIndex);
		}

		private void button1_Click(object sender, System.EventArgs e) {
			if (m_cab == null || !m_cab.IsValid()) {
				MessageBox.Show(this, "Invalid " + m_strKind);
				return;
			}
			DialogResult = DialogResult.OK;
		}

		private void buttonCancel_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.Cancel;
		}
	}
}
