using System;
using System.Globalization;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Text.RegularExpressions;
using SpiffLib;

namespace m
{
	struct MagicEntry {
		public int index;
		public byte[] aiTheater;
		public short ctx, cty;
		public string strName;
		public string strComment;
	}
	
	public enum Theater { Desert, Temperate, Winter };

	public class MixSuck {
		public static MixMap LoadMap(Stream stm, MixTemplate[] amixt) {
			MixReader mixr = new MixReader("general.mix");
			MixMap map = new MixMap(stm, amixt);
			mixr.Dispose();
			return map;
		}

		public static MixTemplate[] LoadTemplates(Theater theater) {
			ArrayList alMagicTable = GetTemplateList();
			MixReader mixr = new MixReader(s_astrTheaterFileNames[(int)theater]);

			// Load palette
			Stream stm = mixr.GetFileStream(s_idePalette[(int)theater] - 1);
			Color[] aclrPalette = LoadPalette(stm);
			stm.Close();
			
			// Load templates
			ArrayList alsTemplates = new ArrayList();
			foreach (MagicEntry me in alMagicTable) {
				if (me.aiTheater[(int)theater] == 0xff) {
					alsTemplates.Add(null);
					continue;
				}
				stm = mixr.GetFileStream(me.aiTheater[(int)theater] - 1);
				alsTemplates.Add(new MixTemplate(stm, me.ctx, me.cty, aclrPalette, me.index));
				stm.Close();
			}
			mixr.Dispose();
			return (MixTemplate[])alsTemplates.ToArray(typeof(MixTemplate));
		}

		public static int ImportTemplates(MixTemplate[] amixt, TemplateDoc tmpd) {
			Size sizTile = tmpd.TileSize;
			ArrayList alsTmpl = new ArrayList();
			int cImages = 0;
			foreach (MixTemplate mixt in amixt) {
				if (mixt == null)
					continue;
				cImages += mixt.ImageCount;
				alsTmpl.Add(ConstructTemplate(tmpd, mixt, sizTile));
			}
			tmpd.AddTemplates((Template[])alsTmpl.ToArray(typeof(Template)));
			return cImages;
		}

		public static string ImportExportMixMap(string strFile, LevelDoc lvld) {
			// Load ini
			Ini ini = new Ini(strFile);

			// Get name of level
			String strName = strFile;
			Ini.Section secBasic = ini["Basic"];
			if (secBasic != null) {
				Ini.Property propName = secBasic["Name"];
				if (propName != null) {
					strName = propName.Value;
				} else {
					Ini.Property propBrief = secBasic["Brief"];
					if (propBrief != null)
						strName = propBrief.Value;
				}
			}

			// Get theater
			Ini.Section secMap = ini["MAP"];
			if (secMap == null) {
				MessageBox.Show("Could not load " + strFile);
				return null;
			}
			Theater theater;
			switch (secMap["Theater"].Value) {
			case "DESERT":
				theater = Theater.Desert;
				break;

			case "TEMPERATE":
				theater = Theater.Temperate;
				break;

			case "WINTER":
				theater = Theater.Winter;
				break;

			default:
				MessageBox.Show("Could not load " + strFile);
				return null;
			}

			// Get bounds & invert
			int xLeft = Int32.Parse(secMap["X"].Value);
			int yTop = Int32.Parse(secMap["Y"].Value);
			int cxWidth = Int32.Parse(secMap["Width"].Value);
			int cyHeight = Int32.Parse(secMap["Height"].Value);
			Rectangle rcBounds = new Rectangle(64 - (xLeft + cxWidth), yTop, cxWidth, cyHeight);

			// We're ready to go
			lvld.Title = strName;
			//fixme
			//lvld.TileCollectionFileName = theater.ToString() + ".tc";
			lvld.Bounds = rcBounds;

			// Load up
			MixTemplate[] amixt = MixSuck.LoadTemplates(theater);
			MixSuck.ImportTemplates(amixt, lvld.GetTemplateDoc());
			Stream stm = (Stream)new FileStream(strFile.ToLower().Replace(".ini", ".bin"), FileMode.Open, FileAccess.Read, FileShare.Read);
			MixMap map = MixSuck.LoadMap(stm, amixt);
			stm.Close();
			map.ImportMap(lvld);
			//fixme
			//lvld.SetBackgroundTemplate(lvld.GetTemplateDoc()FindTemplate(0));

			// Save

			string strFileLvld = strName + "_" + Path.GetFileName(strFile).Replace(".ini", ".ld");
			lvld.SaveAs(strFileLvld);

			// Also save a scaled image for reference
			TemplateDoc tmpd = lvld.GetTemplateDoc();
			Bitmap bm = lvld.GetMapBitmap(tmpd.TileSize, tmpd, true);
			int cx;
			int cy;
			if (bm.Width > bm.Height) {
				cx = 128;
				cy = bm.Height * 128 / bm.Width;
			} else {
				cx = bm.Width * 128 / bm.Height;
				cy = 128;
			}
			bm = (Bitmap)bm.GetThumbnailImage(cx, cy, null, IntPtr.Zero);
			bm.Save(strFileLvld.Replace(".ld", ".png"), ImageFormat.Png);
			bm.Dispose();

			// Save a full 24x24 original map image for reference
			map.SaveMapBitmap("cc_" + strFileLvld.Replace(".ld", ".png"));

			return strFileLvld;
		}

		private static Color[] LoadPalette(Stream stm) {
			BinaryReader brdr = new BinaryReader(stm);
			int cColors = (int)stm.Length / 3;
			Color[] aclr = new Color[cColors];
			for (int n = 0; n < cColors; n++)
				aclr[n] = Color.FromArgb(brdr.ReadByte() << 2, brdr.ReadByte() << 2, brdr.ReadByte() << 2);
			brdr.Close();
			return aclr;
		}

		public static Template ConstructTemplate(TemplateDoc tmpd, MixTemplate mixt, Size sizTile) {
			Bitmap bm = new Bitmap(mixt.XTileCount * sizTile.Width, mixt.YTileCount * sizTile.Height);
			Graphics g = Graphics.FromImage(bm);
			g.Clear(Color.FromArgb(255, 0, 255));
			Bitmap bmNew = new Bitmap(sizTile.Width, sizTile.Height, PixelFormat.Format24bppRgb);
			Graphics gNew = Graphics.FromImage(bmNew);
			for (int tx = 0; tx < mixt.XTileCount; tx++) {
				for (int ty = 0; ty < mixt.YTileCount; ty++) {
					Bitmap bmT = mixt.TileBitmaps[tx + ty * mixt.XTileCount];
					if (bmT == null)
						continue;
					gNew.DrawImage(bmT, 0, 0, sizTile.Width, sizTile.Height);
					g.DrawImageUnscaled(bmNew, tx * sizTile.Width, ty * sizTile.Height, sizTile.Width, sizTile.Height);
				}
			}
			gNew.Dispose();
			bmNew.Dispose();
			g.Dispose();
			bm.RotateFlip(RotateFlipType.RotateNoneFlipX);
			return new Template(tmpd, bm, mixt.Index.ToString());
		}

		private static ArrayList GetTemplateList() {
			ArrayList alMagicTable = new ArrayList();
			Regex re = new Regex(@"\s*(\w+)h\W*(\w+)\W*(\w+)\W*(\w+)\W*(\d+)x(\d+)]\W*(\w+)\s*\W*(.*)");
			foreach (string strToMatch in s_astrMagicTable) {
				Match mat = re.Match(strToMatch);
				MagicEntry me = new MagicEntry();
				me.index = Int32.Parse(mat.Groups[1].Value, NumberStyles.HexNumber);
				me.aiTheater = new byte[3];
				string strT = mat.Groups[2].Value;
				me.aiTheater[0] = strT == "x" ? (byte)0xff : byte.Parse(strT);
				strT = mat.Groups[3].Value;
				me.aiTheater[1] = strT == "x" ? (byte)0xff : byte.Parse(strT);
				strT = mat.Groups[4].Value;
				me.aiTheater[2] = strT == "x" ? (byte)0xff : byte.Parse(strT);
				me.ctx = short.Parse(mat.Groups[5].Value);
				me.cty = short.Parse(mat.Groups[6].Value);
				me.strName = mat.Groups[7].Value;
				me.strComment = mat.Groups[8].Value;
				alMagicTable.Add(me);
			}
			return alMagicTable;
		}

		// Theater file names
		static string[] s_astrTheaterFileNames = {
				"desert.mix", "temperat.mix", "winter.mix"
		};

		// From cncmap1f.txt:
		// In each MIX there's also a palette, the entries are:
		// DESERT.MIX   entry n. 26
		// TEMPERAT.MIX entry n. 62
		// WINTER.MIX   entry n. 62
		static int[] s_idePalette = { 26, 62, 62 };

		// From cncmap1f.txt:
		static string[] s_astrMagicTable = {
				"00h | 007 | 011 | 028 | [4x4] | CLEAR1   | Default terrain",
				"01h | 002 | 007 | 007 | [1x1] | W1       | Water (not animated)",
				"02h |  x  | 009 | 009 | [2x2] | W2       | Water",
				"03h |  x  | 087 | 087 | [3x3] | SH1      | Coast WD (1)",
				"04h |  x  | 106 | 105 | [3x3] | SH2      | Coast WD",
				"05h |  x  | 126 | 124 | [1x1] | SH3      | Rock in water",
				"06h |  x  | 143 | 140 | [2x1] | SH4      | Rock in water",
				"07h |  x  | 159 | 157 | [3x3] | SH5      | Coast WD",
				"08h |  x  | 018 | 017 | [3x3] | SH11     | Fjord WD",
				"09h |  x  | 024 | 023 | [3x3] | SH12     | Coast WU",
				"0Ah |  x  | 031 | 031 | [3x3] | SH13     | Coast WU",
				"0Bh |  x  | 037 | 037 | [3x3] | SH14     | Coast WU",
				"0Ch |  x  | 042 | 042 | [3x3] | SH15     | Coast WU",
				"0Dh | 106 | 074 | 074 | [2x2] | S01      | Cliff Left Edge",
				"0Eh | 122 | 093 | 092 | [2x3] | S02      | Cliff Wu-Wd     (2)",
				"0Fh | 138 | 112 | 110 | [2x2] | S03      | Cliff W-E",
				"10h | 154 | 131 | 128 | [2x2] | S04      | Cliff W-E",
				"11h | 170 | 147 | 144 | [2x2] | S05      | Cliff W-E",
				"12h | 185 | 163 | 161 | [2x3] | S06      | Cliff Wd-Eu",
				"13h | 200 | 180 | 179 | [2x2] | S07      | Cliff Right Edge",
				"14h | 212 | 195 | 195 | [2x2] | S08      | Cliff Top Edge",
				"15h | 225 | 208 | 209 | [3x2] | S09      | Cliff N-S",
				"16h | 096 | 064 | 064 | [2x2] | S10      | Cliff N-S",
				"17h | 108 | 078 | 078 | [2x2] | S11      | Cliff N-S",
				"18h | 124 | 097 | 096 | [2x2] | S12      | Cliff N-S",
				"19h | 140 | 117 | 115 | [3x2] | S13      | Cliff N-S",
				"1Ah | 157 | 135 | 132 | [2x2] | S14      | Cliff Bottom Edge",
				"1Bh | 172 | 151 | 149 | [2x2] | S15      | Cliff Left Edge",
				"1Ch | 187 | 167 | 166 | [2x3] | S16      | Cliff Wu-Ed",
				"1Dh | 202 | 184 | 184 | [2x2] | S17      | Cliff W-E",
				"1Eh | 215 | 199 | 200 | [2x2] | S18      | Cliff W-E",
				"1Fh | 228 | 211 | 213 | [2x2] | S19      | Cliff W-E",
				"20h | 098 | 068 | 069 | [2x3] | S20      | Cliff Wu-Ed",
				"21h | 110 | 082 | 082 | [1x2] | S21      | Cliff Right Edge",
				"22h | 126 | 101 | 100 | [2x1] | S22      | Cliff Corner S-E Internal",
				"23h | 142 | 121 | 119 | [3x2] | S23      | Cliff Sl-Nr",
				"24h | 159 | 139 | 136 | [2x2] | S24      | Cliff N-S",
				"25h | 174 | 155 | 153 | [2x2] | S25      | Cliff N-S",
				"26h | 189 | 171 | 170 | [2x2] | S26      | Cliff N-S",
				"27h | 204 | 188 | 188 | [3x2] | S27      | Cliff Nl-Sr",
				"28h | 218 | 202 | 203 | [2x2] | S28      | Cliff Bottom Edge",
				"29h | 230 | 213 | 215 | [2x2] | S29      | Cliff Corner N-E External",
				"2Ah | 101 | 070 | 071 | [2x2] | S30      | Cliff Corner S-E Ext",
				"2Bh | 113 | 084 | 084 | [2x2] | S31      | Cliff Corner W-S Ext",
				"2Ch | 129 | 103 | 102 | [2x2] | S32      | Cliff Corner N-W Ext",
				"2Dh | 145 | 123 | 121 | [2x2] | S33      | Cliff Corner N-E Internal",
				"2Eh | 162 | 141 | 138 | [2x2] | S34      | Cliff Corner S-E Int",
				"2Fh | 177 | 157 | 155 | [2x2] | S35      | Cliff Corner W-S Int",
				"30h | 192 | 173 | 172 | [2x2] | S36      | Cliff Corner W-N Int",
				"31h | 207 | 190 | 190 | [2x2] | S37      | Cliff Junction NW-SE",
				"32h | 221 | 204 | 205 | [2x2] | S38      | Cliff Junction SW-NE",
				"33h |  x  | 027 | 026 | [3x3] | SH32     | Coast Corner N-W Int",
				"34h |  x  | 033 | 033 | [3x3] | SH33     | Coast Corner N-E Int",
				"35h | 017 |  x  |  x  | [4x1] | SH20     | Coast WD",
				"36h | 024 |  x  |  x  | [3x1] | SH21     | Coast WD",
				"37h | 041 |  x  |  x  | [6x2] | SH22     | Coast WD",
				"38h | 049 |  x  |  x  | [2x2] | SH23     | Coast WD",
				"39h | 118 |  x  |  x  | [1x1] | BR1      | Bush",
				"3Ah | 134 |  x  |  x  | [1x1] | BR2      | Bush",
				"3Bh | 150 |  x  |  x  | [1x1] | BR3      | Cactus",
				"3Ch | 166 |  x  |  x  | [1x1] | BR4      | Cactus",
				"3Dh | 181 |  x  |  x  | [1x1] | BR5      | ??? Purple square (bug ?)",
				"3Eh | 196 |  x  |  x  | [2x2] | BR6      | Bushes",
				"3Fh | 210 |  x  |  x  | [2x2] | BR7      | Bushes",
				"40h | 223 |  x  |  x  | [3x2] | BR8      | Bushes",
				"41h | 234 |  x  |  x  | [3x2] | BR9      | Bushes",
				"42h | 016 |  x  |  x  | [2x1] | BR10     | ??? Purple squares (bug ?)",
				"43h | 105 | 073 |  x  | [1x1] | P01      | Bones / Wall    (3)",
				"44h | 121 | 092 |  x  | [1x1] | P02      | Bones / Wall    (3)",
				"45h | 137 | 111 |  x  | [1x1] | P03      | Mud / UFO       (3) (6)",
				"46h | 153 | 130 |  x  | [1x1] | P04      | Rock / UFO      (3) (6)",
				"47h | 169 |  x  |  x  | [2x2] | P05      | Gray Sand",
				"48h | 184 |  x  |  x  | [6x4] | P06      | Gray Sand",
				"49h | 199 | 179 | 178 | [4x2] | P07      | Mud",
				"4Ah |  x  | 194 | 194 | [3x2] | P08      | Mud",
				"4Bh |  x  | 045 | 045 | [3x2] | SH16     | Fjord WU",
				"4Ch | 072 | 047 | 047 | [2x2] | SH17     | Water (anim.)",
				"4Dh | 078 | 049 | 049 | [2x2] | SH18     | Water (anim.)",
				"4Eh | 084 |  x  |  x  | [3x2] | SH19     | Coast WD",
				"4Fh |  x  | 116 | 114 | [3x2] | P13      | Destroyed House",
				"50h |  x  | 134 | 131 | [2x1] | P14      | Walls",
				"51h |  x  |  x  | 148 | [4x2] | P15      | Snow",
				"52h | 001 | 006 | 006 | [1x1] | B1       | Rock",
				"53h | 003 | 008 | 008 | [2x1] | B2       | Rock",
				"54h |  x  | 010 | 010 | [3x1] | B3       | Rock",
				"55h | 004 |  x  |  x  | [1x1] | B4       | ?? Rock (7)",
				"56h | 005 |  x  |  x  | [1x1] | B5       | ?? Rock (7)",
				"57h | 006 |  x  |  x  | [1x1] | B6       | ?? Rock (7)",
				"58h |  x  | 175 | 174 | [3x3] | SH6      | Coast WD",
				"59h |  x  | 191 | 191 | [2x2] | SH7      | Coast Corner W-N External",
				"5Ah |  x  | 205 | 206 | [3x3] | SH8      | Coast Corner S-E Ext",
				"5Bh |  x  | 215 | 217 | [3x3] | SH9      | Coast Corner W-S Ext",
				"5Ch |  x  | 012 | 011 | [2x2] | SH10     | Coast Corner N-E Ext",
				"5Dh | 104 | 072 | 073 | [2x2] | D01      | Road Bottom End",
				"5Eh | 120 | 091 | 091 | [2x2] | D02      | Road Left End",
				"5Fh | 136 | 110 | 109 | [1x2] | D03      | Road Top End",
				"60h | 152 | 129 | 127 | [2x2] | D04      | Road Right End",
				"61h | 168 | 146 | 143 | [3x4] | D05      | Road S-N",
				"62h | 183 | 162 | 160 | [2x3] | D06      | Road S-N",
				"63h | 198 | 178 | 177 | [3x2] | D07      | Road S-N",
				"64h | 211 | 193 | 193 | [3x2] | D08      | Road S-N",
				"65h | 224 | 207 | 208 | [4x3] | D09      | Road W-E",
				"66h | 095 | 063 | 063 | [4x2] | D10      | Road W-E",
				"67h | 107 | 077 | 077 | [2x3] | D11      | Road W-E",
				"68h | 123 | 096 | 095 | [2x2] | D12      | Road W-E",
				"69h | 139 | 115 | 113 | [4x3] | D13      | Road Wu-Ed",
				"6Ah | 156 | 133 | 130 | [3x3] | D14      | Road T N--W+E  (4)",
				"6Bh | 171 | 150 | 147 | [3x3] | D15      | Road Y S--N+E  (4)",
				"6Ch | 186 | 166 | 164 | [3x3] | D16      | Road Y S--N+E",
				"6Dh | 201 | 183 | 182 | [3x2] | D17      | Road T S--W+E",
				"6Eh | 214 | 198 | 198 | [3x3] | D18      | Road T W--N+S",
				"6Fh | 227 | 210 | 211 | [3x3] | D19      | Road + W-N-E-S",
				"70h | 097 | 067 | 067 | [3x3] | D20      | Road Corner N-E",
				"71h | 109 | 081 | 081 | [3x2] | D21      | Road Corner S-E",
				"72h | 125 | 100 | 099 | [3x3] | D22      | Road Corner W-S",
				"73h | 141 | 120 | 118 | [3x3] | D23      | Road Corner W-N",
				"74h | 158 | 138 | 135 | [3x3] | D24      | Road Diagonal NW-SE      (5)",
				"75h | 173 | 154 | 152 | [3x3] | D25      | Road Diag NW-SE",
				"76h | 188 | 170 | 169 | [2x2] | D26      | Road Diag NW-SE (Conn.)  (5)",
				"77h | 203 | 187 | 187 | [2x2] | D27      | Road Diag NW-SE (Conn.)",
				"78h | 217 | 201 | 202 | [2x2] | D28      | Road Corner W-SE (Conn.)",
				"79h | 229 | 212 | 214 | [2x2] | D29      | Road Corner N-SE (Conn.)",
				"7Ah | 100 | 069 | 070 | [2x2] | D30      | Road Y SE--N+W (Conn.)",
				"7Bh | 112 | 083 | 083 | [2x2] | D31      | Road Corner E-NW (Conn.)",
				"7Ch | 128 | 102 | 101 | [2x2] | D32      | Road Corner S-NW (Conn.)",
				"7Dh | 144 | 122 | 120 | [2x2] | D33      | Road Y NW--S+E (Conn.)",
				"7Eh | 161 | 140 | 137 | [3x3] | D34      | Road Diag SW-NE",
				"7Fh | 176 | 156 | 154 | [3x3] | D35      | Road Diag SW-NE",
				"80h | 191 | 172 | 171 | [2x2] | D36      | Road Diag SW-NE (Conn.)",
				"81h | 206 | 189 | 189 | [2x2] | D37      | Road Diag SW-NE (Conn.)",
				"82h | 220 | 203 | 204 | [2x2] | D38      | Road Corner E-SW (Conn.)",
				"83h | 232 | 214 | 216 | [2x2] | D39      | Road Corner N-SW (Conn.)",
				"84h | 103 | 071 | 072 | [2x2] | D40      | Road Y SW--N+E (Conn.)",
				"85h | 115 | 085 | 085 | [2x2] | D41      | Road Corner W-NE (Conn.)",
				"86h | 131 | 104 | 103 | [2x2] | D42      | Road Corner S-NE (Conn.)",
				"87h | 147 | 124 | 122 | [2x2] | D43      | Road Y NE--W+S (Conn.)",
				"88h |  x  | 017 | 016 | [5x4] | RV01     | River W-E",
				"89h |  x  | 023 | 022 | [5x3] | RV02     | River W-E",
				"8Ah |  x  | 030 | 030 | [4x4] | RV03     | River Wu-Ed",
				"8Bh |  x  | 036 | 036 | [4x4] | RV04     | River Wd-Eu",
				"8Ch |  x  | 041 | 041 | [3x3] | RV05     | River N-S",
				"8Dh |  x  | 044 | 044 | [3x2] | RV06     | River N-S",
				"8Eh |  x  | 046 | 046 | [3x2] | RV07     | River N-S",
				"8Fh |  x  | 048 | 048 | [2x2] | RV08     | River Corner S-E",
				"90h |  x  | 052 | 052 | [2x2] | RV09     | River Corner W-S",
				"91h |  x  | 014 | 013 | [2x2] | RV10     | River Corner N-E",
				"92h |  x  | 020 | 019 | [2x2] | RV11     | River Corner W-N",
				"93h |  x  | 026 | 025 | [3x4] | RV12     | River Y N--W+S",
				"94h |  x  | 032 | 032 | [4x4] | RV13     | River Y Eu--W+S",
				"95h | 055 |  x  |  x  | [4x3] | RV14     | River W-E",
				"96h | 060 |  x  |  x  | [4x3] | RV15     | River W-E",
				"97h | 067 |  x  |  x  | [6x4] | RV16     | River Wd-Eu",
				"98h | 073 |  x  |  x  | [6x5] | RV17     | River Wu-Ed",
				"99h | 079 |  x  |  x  | [4x4] | RV18     | River N-S",
				"9Ah | 085 |  x  |  x  | [4x4] | RV19     | River N-S",
				"9Bh | 018 |  x  |  x  | [6x8] | RV20     | River Nr-Sl",
				"9Ch | 025 |  x  |  x  | [5x8] | RV21     | River Nl-Sr",
				"9Dh | 042 |  x  |  x  | [3x3] | RV22     | River Corner E-S",
				"9Eh | 050 |  x  |  x  | [3x3] | RV23     | River Corner W-S",
				"9Fh | 057 |  x  |  x  | [3x3] | RV24     | River Corner N-E",
				"A0h | 062 |  x  |  x  | [3x3] | RV25     | River Corner N-W",
				"A1h | 009 | 002 | 004 | [3x3] | FORD1    | River Crossing (Road W-E)",
				"A2h | 010 | 003 | 005 | [3x3] | FORD2    | River Crossing (Road N-S)",
				"A3h | 047 | 057 | 057 | [3x3] | FALLS1   | Falls W-E",
				"A4h | 048 | 058 | 058 | [3x2] | FALLS2   | Falls N-S",
				"A5h |  x  | 218 | 220 | [4x4] | BRIDGE1  | Bridge SW-NE",
				"A6h |  x  | 059 | 059 | [4x4] | BRIDGE1D | Fallen Bridge SW-NE",
				"A7h |  x  | 219 | 221 | [5x5] | BRIDGE2  | Bridge NW-SE",
				"A8h |  x  | 060 | 060 | [5x5] | BRIDGE2D | Fallen Bridge NW-SE",
				"A9h | 236 |  x  |  x  | [6x5] | BRIDGE3  | Bridge SW-NE",
				"AAh | 092 |  x  |  x  | [6x5] | BRIDGE3D | Fallen Bridge SW-NE",
				"ABh | 237 |  x  |  x  | [6x4] | BRIDGE4  | Bridge NW-SE",
				"ACh | 093 |  x  |  x  | [6x4] | BRIDGE4D | Fallen Bridge NW-SE",
				"ADh | 056 |  x  |  x  | [3x3] | SH24     | Fjord WD",
				"AEh | 061 |  x  |  x  | [3x2] | SH25     | Coast WU",
				"AFh | 068 |  x  |  x  | [3x2] | SH26     | Coast WU",
				"B0h | 074 |  x  |  x  | [4x1] | SH27     | Coast WU",
				"B1h | 080 |  x  |  x  | [3x1] | SH28     | Coast WU",
				"B2h | 086 |  x  |  x  | [6x2] | SH29     | Coast WU",
				"B3h | 019 |  x  |  x  | [2x2] | SH30     | Coast WU",
				"B4h | 027 |  x  |  x  | [3x3] | SH31     | Fjord WU",
				"B5h |  x  |  x  | 165 | [2x2] | P16      | Snow",
				"B6h |  x  |  x  | 183 | [4x2] | P17      | Snow",
				"B7h |  x  |  x  | 199 | [4x3] | P18      | Snow",
				"B8h |  x  |  x  | 212 | [4x3] | P19      | Snow",
				"B9h |  x  |  x  | 068 | [4x3] | P20      | Snow",
				"BAh |  x  | 038 | 038 | [3x3] | SH34     | Coast WR",
				"BBh |  x  | 043 | 043 | [3x3] | SH35     | Coast WL",
				"BCh | 069 |  x  |  x  | [1x1] | SH36     | Coast Corner S-E Int",
				"BDh | 075 |  x  |  x  | [1x1] | SH37     | Coast Corner W-S Int",
				"BEh | 081 |  x  |  x  | [1x1] | SH38     | Coast Corner N-E Int",
				"BFh | 087 |  x  |  x  | [1x1] | SH39     | Coast Corner N-W Int",
				"C0h | 020 |  x  |  x  | [3x3] | SH40     | Coast Corner S-E Int",
				"C1h | 028 |  x  |  x  | [3x3] | SH41     | Coast Corner N-W Int",
				"C2h | 043 |  x  |  x  | [1x2] | SH42     | Coast WL",
				"C3h | 051 |  x  |  x  | [1x3] | SH43     | Coast WL",
				"C4h | 058 |  x  |  x  | [1x3] | SH44     | Coast WR",
				"C5h | 063 |  x  |  x  | [1x2] | SH45     | Coast WR",
				"C6h | 070 |  x  |  x  | [3x3] | SH46     | Coast Corner S-E Int",
				"C7h | 076 |  x  |  x  | [3x3] | SH47     | Coast Corner S-E Int",
				"C8h | 082 |  x  |  x  | [3x3] | SH48     | Coast Corner N-E Int",
				"C9h | 088 |  x  |  x  | [3x3] | SH49     | Coast Corner N-W Int",
				"CAh | 021 |  x  |  x  | [4x3] | SH50     | Coast Corner S-E Ext",
				"CBh | 029 |  x  |  x  | [4x3] | SH51     | Coast Corner W-S Ext",
				"CCh | 044 |  x  |  x  | [4x3] | SH52     | Coast Corner N-E Ext",
				"CDh | 052 |  x  |  x  | [4x3] | SH53     | Coast Corner N-W Ext",
				"CEh | 059 |  x  |  x  | [3x2] | SH54     | Coast WD",
				"CFh | 064 |  x  |  x  | [3x2] | SH55     | Coast WD",
				"D0h | 071 |  x  |  x  | [3x2] | SH56     | Coast WU",
				"D1h | 077 |  x  |  x  | [3x2] | SH57     | Coast WU",
				"D2h | 083 |  x  |  x  | [2x3] | SH58     | Coast WR",
				"D3h | 089 |  x  |  x  | [2x3] | SH59     | Coast WR",
				"D4h | 022 |  x  |  x  | [2x3] | SH60     | Coast WL",
				"D5h | 030 |  x  |  x  | [2x3] | SH61     | Coast WL",
				"D6h | 045 |  x  |  x  | [6x1] | SH62     | Coast WD",
				"D7h | 053 |  x  |  x  | [4x1] | SH63     | Coast WD",
		};
	}

	public class MixMap {
		struct Cell {
			public uint iTemplate;
			public uint iTile;
		}

		Cell[,] m_acell;
		MixTemplate[] m_amixt;

		public MixMap(Stream stm, MixTemplate[] amixt) {
			m_amixt = amixt;
			BinaryReader brdr = new BinaryReader(stm);
			m_acell = new Cell[64,64];
			for (int iCell = 0; iCell < 64 * 64; iCell++) {
				m_acell[iCell / 64, iCell % 64].iTemplate = (uint)brdr.ReadByte();
				m_acell[iCell / 64, iCell % 64].iTile = (uint)brdr.ReadByte();
			}
			brdr.Close();
		}

		public void SaveMapBitmap(String strFile) {
			Bitmap bm = new Bitmap(64 * 24, 64 * 24, PixelFormat.Format16bppRgb565);
			using (Graphics g = Graphics.FromImage(bm)) {
				int ctx = m_amixt[0].XTileCount;
				int cty = m_amixt[0].YTileCount;
				for (int ty = 0; ty < 64; ty += cty) {
					for (int tx = 0; tx < 64; tx += ctx) {
						for (int tyT = 0; tyT < cty; tyT++) {
							for (int txT = 0; txT < ctx; txT++) {
								if (tx + txT >= 64 || ty + tyT >= 64)
									continue;
								g.DrawImageUnscaled(m_amixt[0].TileBitmaps[tyT * ctx + txT], (tx + txT) * 24, (ty + tyT) * 24);
							}
						}
					}
				}
				TemplatePos[] atempl = GetTemplatePositions();
				foreach (TemplatePos tpos in atempl) {
					int x = tpos.m_txOrigin * 24;
					int y = tpos.m_tyOrigin * 24;
					for (int ty = 0; ty < tpos.m_mixt.YTileCount; ty++) {
						for (int tx = 0; tx < tpos.m_mixt.XTileCount; tx++) {
							if (!tpos.m_afMapped[ty, tx])
								continue;
							g.DrawImageUnscaled(tpos.m_mixt.TileBitmaps[ty * tpos.m_mixt.XTileCount + tx], x + tx * 24, y + ty * 24);
						}
					}
				}
			}
			bm.Save(strFile, ImageFormat.Png);
		}

		class TemplatePos {
			public MixTemplate m_mixt;
			public int m_txOrigin;
			public int m_tyOrigin;
			public bool[,] m_afMapped;

			public TemplatePos(MixTemplate mixt, int txOrigin, int tyOrigin, bool[,] afMapped) {
				m_mixt = mixt;
				m_txOrigin = txOrigin;
				m_tyOrigin = tyOrigin;
				m_afMapped = afMapped;
			}
		}

		public void ImportMap(LevelDoc lvld) {
			TemplatePos[] atpos = GetTemplatePositions();
			ArrayList alsTiles = new ArrayList();
			foreach (TemplatePos tpos in atpos) {
				int txOrigin = 64 - tpos.m_mixt.XTileCount - tpos.m_txOrigin;
				int tyOrigin = tpos.m_tyOrigin;
				bool[,] afDraw = new bool[tpos.m_mixt.YTileCount, tpos.m_mixt.XTileCount];
				for (int ty = 0; ty < tpos.m_mixt.YTileCount; ty++) {
					for (int tx = 0; tx < tpos.m_mixt.XTileCount; tx++) {
						afDraw[ty, tx] = tpos.m_afMapped[ty, tpos.m_mixt.XTileCount - tx - 1];
					}
				}
				Tile tile = new Tile(tpos.m_mixt.Index.ToString(), txOrigin, tyOrigin, afDraw, null);
				alsTiles.Add(tile);
			}
			lvld.AddMapItems((IMapItem[])alsTiles.ToArray(typeof(IMapItem)));
		}

		TemplatePos[] GetTemplatePositions() {
			bool[,] afCellTaken = new bool[64,64];
			ArrayList alsTemplPos = new ArrayList();
			for (int ty = 0; ty < 64; ty++) {
				for (int tx = 0; tx < 64; tx++) {
					if (afCellTaken[ty, tx])
						continue;
					if (m_acell[ty, tx].iTemplate == 255)
						continue;

					// Find out what part of this template matches
					MixTemplate mixt = m_amixt[m_acell[ty, tx].iTemplate];

					// scg03ea has an invalid iTemplate outside the map bounds
					if (mixt == null)
						continue;

					int ctxTmpl = mixt.XTileCount;
					int ctyTmpl = mixt.YTileCount;
					bool[,]afMapped = new bool[ctyTmpl, ctxTmpl];
					int txTmpl = (int)m_acell[ty, tx].iTile % ctxTmpl;
					int tyTmpl = (int)m_acell[ty, tx].iTile / ctxTmpl;
					int txOrigin = tx - txTmpl;
					int tyOrigin = ty - tyTmpl;

					for (tyTmpl = 0; tyTmpl < ctyTmpl; tyTmpl++) {
						for (txTmpl = 0; txTmpl < ctxTmpl; txTmpl++) {
							int txT = txOrigin + txTmpl;
							int tyT = tyOrigin + tyTmpl;
							if (txT < 0 || tyT < 0 || txT >= 64 || tyT >= 64)
								continue;
							if (afCellTaken[tyT, txT])
								continue;
							Cell cell = m_acell[tyT, txT];
							if (cell.iTemplate == 255)
								continue;
							if (cell.iTemplate != m_acell[ty, tx].iTemplate)
								continue;
							if (cell.iTile != tyTmpl * ctxTmpl + txTmpl)
								continue;
							afMapped[tyTmpl, txTmpl] = true;
							afCellTaken[tyT, txT] = true;
						}
					}
					alsTemplPos.Add(new TemplatePos(mixt, txOrigin, tyOrigin, afMapped));
				}
			}
			return (TemplatePos[])alsTemplPos.ToArray(typeof(TemplatePos));
		}
	}

	public class MixTemplate {
		public int XTileCount;
		public int YTileCount;
		public int ImageCount;
		public int TileCount;
		public int Index;
		public Bitmap[] TileBitmaps;
		private byte[] m_mpiTileiTile;

		public MixTemplate(Stream stm, int ctx, int cty, Color[] aclrPalette, int index) {
			XTileCount = ctx;
			YTileCount = cty;
			Index = index;

			BinaryReader brdr = new BinaryReader(stm);

			// From cncmap1f.txt:
			// Header = record
			// Width  : word;  {Width of images, always 24 (18h)}
			// Heigth : word;  {Heigth of images, always 24}
			// NumTil : word;  {Number of Tiles (may differ from num. of Images}
			// Zero1  : word;      {Seems to be always 0000h}
			// Size   : longint;   {size of file}
			// ImgStart : longint; {Offset of first image}
			// Zero2  : longint;   {Seems to be always 00000000h}
			// ID1    : word;      {Always FFFFh}
			// ID2    : word;      {Always 1A0Dh (or 0D1Ah I can't remeber}
			// Index2 : longint;   {Offset of Index2}
			// Index1 : longint;   {Offset of Index1} {I will explain these later}
			// end;

			// Skip width and height word[2]
			brdr.ReadUInt16();
			brdr.ReadUInt16();

			// Count of tiles
			TileCount = brdr.ReadInt16();

			// Zero1
			brdr.ReadUInt16();

			// Size of file
			brdr.ReadInt32();

			// ImgStart
			int ibImageStart = brdr.ReadInt32();

			// Zero2
			brdr.ReadInt32();

			// ID1 & ID2
			brdr.ReadUInt16();
			brdr.ReadUInt16();

			// Index2 & Index1
			int ibIndex2 = brdr.ReadInt32();
			int ibIndex1 = brdr.ReadInt32();
			brdr.BaseStream.Seek(ibIndex1, SeekOrigin.Begin);
			m_mpiTileiTile = brdr.ReadBytes(TileCount);

			// Derive useful information
			ImageCount = (ibIndex1 - ibImageStart) / (24 * 24);

			// Load images
			TileBitmaps = new Bitmap[TileCount];
			for (int n = 0; n < TileCount; n++) {
				int iTile = m_mpiTileiTile[n];
				if (iTile == 0xff)
					continue;
				brdr.BaseStream.Seek(ibImageStart + iTile * 24 * 24, SeekOrigin.Begin);
				byte[] abImage = brdr.ReadBytes(24 * 24);
				TileBitmaps[n] = LoadTileImage(abImage, aclrPalette);
			}

			brdr.Close();
		}

		Bitmap LoadTileImage(byte[] abImage, Color[] aclrPalette) {
			Bitmap bm = new Bitmap(24, 24, PixelFormat.Format24bppRgb);
			for (int n = 0; n < abImage.Length; n++)
				bm.SetPixel(n % 24, n / 24, aclrPalette[abImage[n]]);
			return bm;
		}
	}

	class MixReader {
		FileStream m_fstm;
		BinaryReader m_brdr;
		int m_cFiles;
		int m_cbBodyLength;
		ArrayList m_alDir;

		public MixReader(string strFile) {
			m_fstm = new FileStream(strFile, FileMode.Open, FileAccess.Read);
			m_brdr = new BinaryReader(m_fstm);
			ReadDirectory();
		}

		public void Dispose() {
			m_brdr.Close();
		}

		struct DirectoryEntry {
			public uint ID;
			public int ibStart;
			public int cbLength;
		}

		private bool ReadDirectory() {
			m_brdr.BaseStream.Seek(0, SeekOrigin.Begin);

			m_cFiles = m_brdr.ReadInt16();
			m_cbBodyLength = m_brdr.ReadInt32();

			m_alDir = new ArrayList(m_cFiles);

			for (int i = 0; i < m_cFiles; i++) {
				DirectoryEntry de = new DirectoryEntry();
				de.ID = m_brdr.ReadUInt32();
				de.ibStart = m_brdr.ReadInt32() + m_cFiles * 12 + 6;
				de.cbLength = m_brdr.ReadInt32();
				m_alDir.Add(de);
			}
			return true;
		}

		public Stream GetFileStream(int index) {
			DirectoryEntry de = (DirectoryEntry)m_alDir[index];
			m_brdr.BaseStream.Seek(de.ibStart, SeekOrigin.Begin);
			return (Stream)new MemoryStream(m_brdr.ReadBytes(de.cbLength));
		}
	}
}