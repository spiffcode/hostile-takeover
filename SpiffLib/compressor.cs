using System;
using System.IO;
using System.Collections;
using System.Diagnostics;

namespace SpiffLib {
	enum CodeType { Literal, Match, Run, End };

	struct Code {
		public byte[] ab;
		public int ibSrc;
		public int cbSrc;
		public int cBitsSavings;
		public CodeType cotype;
	};

	public class Compressor {
		byte[] m_abSrc;
		ArrayList[] m_mpbalsIndexes = new ArrayList[256];

		public static byte[] CompressChunk(byte[] abChunk) {
#if false
			return abChunk;
#else
			Compressor comp = new Compressor();
			return comp.Compress(abChunk);
#endif
		}

		public Compressor() {
		}

		void PushIndexes(int ib, int cb) {
			while (cb-- != 0) {
				byte b = m_abSrc[ib];
				if (m_mpbalsIndexes[b] == null)
					m_mpbalsIndexes[b] = new ArrayList();
				m_mpbalsIndexes[b].Add(ib);
				ib++;
			}
		}

		void PopIndexes(int ib, int cb) {
			ib += cb - 1;
			while (cb-- != 0) {
				byte b = m_abSrc[ib];
				ArrayList alsT = m_mpbalsIndexes[b];
				Debug.Assert(ib == (int)alsT[alsT.Count - 1]);
				alsT.RemoveAt(alsT.Count - 1);
				ib--;
			}
		}

		Code[] FindMatchCodes(int ibSrc) {
			ArrayList alsCode = new ArrayList();
			ArrayList alsIndexes = m_mpbalsIndexes[m_abSrc[ibSrc]];
			if (alsIndexes != null) {
				// Find the longest match. 
			
				int ibLongest = -1;
				int cbLongest = 0;

				for (int iIndex = alsIndexes.Count - 1; iIndex >= 0; iIndex--) {
					// The largest index is 16383.

					int ibStart = (int)alsIndexes[iIndex];
					if (ibSrc - ibStart > 16383)
						break;

					// Check length of match.

					Debug.Assert(m_abSrc[ibSrc] == m_abSrc[ibStart]);
					int c = 1;
					int cbT = (ibSrc - ibStart) < (m_abSrc.Length - ibSrc) ? (ibSrc - ibStart) : (m_abSrc.Length - ibSrc);
					for (int ibT = ibStart + 1; ibT < ibStart + cbT; ibT++) {
						if (m_abSrc[ibT] != m_abSrc[ibSrc + (ibT - ibStart)])
							break;
						c++;

						// Max length allowed is 129

						if (c == 129)
							break;
					}

					// The match has to be >= 2 to be useful

					if (c < 2)
						continue;

					// Remember this match if it is longer than what we know about
					// already

					if (c > cbLongest) {
						cbLongest = c;
						ibLongest = ibStart;
					}
				}

				// Add best known matches

				if (ibLongest != -1) {
					Code code;
					int ib = ibSrc - ibLongest;
					int cb = cbLongest;

					if (cb > 8 || ib >= 8192) {
						// 3 bits old count (must be 0 since this is escape code)
						// 7 bits count (0-127 translates to 2-129 count)
						// 14 bits index (1-16383, 0 means end if count is also 0)

						Debug.Assert(ib <= 16384);
						Debug.Assert(cb <= 129);
						code.ab = new byte[3];
						code.ab[0] = (byte)(ib >> 9);
						code.ab[1] = (byte)((ib >> 1) & 0xff);
						code.ab[2] = (byte)((ib << 7) | (cb - 2));
					} else {
						// 3 bits count (1-7 translates to 2-8, 0 reserved for escape code)
						// 13 bits index (1-8191, 0 means end if count is also 0)

						Debug.Assert(ib < 8192);
						Debug.Assert(cb <= 8);
						code.ab = new byte[2];
						code.ab[0] = (byte)(((cb - 1) << 5) | (ib >> 8));
						code.ab[1] = (byte)ib;
					}
					code.cotype = CodeType.Match;
					code.ibSrc = ibSrc;
					code.cbSrc = cb;
					code.cBitsSavings = (code.cbSrc - code.ab.Length) * 8;
					alsCode.Add(code);
				}
			}

			// Add literal to round out the possibilities

			Code codeLiteral;
			codeLiteral.ab = new byte[1];
			codeLiteral.ab[0] = m_abSrc[ibSrc];
			codeLiteral.cBitsSavings = 0;
			codeLiteral.cbSrc = 1;
			codeLiteral.ibSrc = ibSrc;
			codeLiteral.cotype = CodeType.Literal;
			alsCode.Add(codeLiteral);

			// Return them all

			return (Code[])alsCode.ToArray(typeof(Code));
		}

		Code FindBestCode(int ibSrc, int cLookAhead) {
			// First find all possible Code at this point, independent of savings

			ArrayList alsCode = new ArrayList();
			alsCode.AddRange(FindMatchCodes(ibSrc));

			// See which approach yields the smallest size over the next cLookAhead
			// encodings. It is easy to get different results here. The larger
			// cLookAhead is the better (but slower)

			int icodeBest = 0;
			int cBitsSavingsBest = Int32.MinValue;
			for (int i = 0; i < alsCode.Count; i++) {
				Code code = (Code)alsCode[i];
				int cBitsSavingsT = code.cBitsSavings;

				// Look ahead if asked to
		
				if (cLookAhead != 0 && ibSrc + code.cbSrc < m_abSrc.Length) {
					int ibSrcT = ibSrc + code.cbSrc;
					PushIndexes(ibSrc, code.cbSrc);
					for (int iT = 0; iT < cLookAhead; iT++) {
						Code codeT = FindBestCode(ibSrcT, 0);
						PushIndexes(ibSrcT, codeT.cbSrc);
						ibSrcT += codeT.cbSrc;
						cBitsSavingsT += codeT.cBitsSavings;
						if (ibSrcT >= m_abSrc.Length)
							break;
					}
					PopIndexes(ibSrc, ibSrcT - ibSrc);
				}

				// Check against best so far

				if (cBitsSavingsT > cBitsSavingsBest) {
					cBitsSavingsBest = cBitsSavingsT;
					icodeBest = i;
				}
			}

			// Get and return the best

			return (Code)alsCode[icodeBest];
		}

		private Code[] Codify(int ibSrc) {
			// Get the codes for this stream

			ArrayList alsCodes = new ArrayList();
			while (ibSrc < m_abSrc.Length) {
				Code code = FindBestCode(ibSrc, 0); // 16);
				PushIndexes(ibSrc, code.cbSrc);
				alsCodes.Add(code);
				ibSrc += code.cbSrc;
			}

			// Add the End code and done

			Code codeEnd;
			codeEnd.ab = new byte[2];
			codeEnd.ab[0] = (byte)0;
			codeEnd.ab[1] = (byte)0;
			codeEnd.cBitsSavings = 0;
			codeEnd.cbSrc = 0;
			codeEnd.ibSrc = 0;
			codeEnd.cotype = CodeType.End;
			alsCodes.Add(codeEnd);
			return (Code[])alsCodes.ToArray(typeof(Code));
		}

		public byte[] Compress(byte[] abSrc) {
			// Create compression codes

			m_abSrc = abSrc;
			Code[] acode = Codify(0);
			
			// Turn into a byte stream

			ArrayList alsDst = new ArrayList();
			ArrayList alsDstT = new ArrayList();
			int ibitFlags = 7;
			byte bFlags = 0;
			for (int icode = 0; icode < acode.Length; icode++) {
				Code code = acode[icode];
				if (code.cotype == CodeType.Literal)
					bFlags |= (byte)(1 << ibitFlags);
				alsDstT.AddRange(code.ab);
				ibitFlags--;
				if (ibitFlags < 0 || icode == acode.Length - 1) {
					ibitFlags = 7;
					alsDst.Add(bFlags);
					alsDst.AddRange((byte[])alsDstT.ToArray(typeof(byte)));
					bFlags = 0;
					alsDstT.Clear();
				}
			}

			// Done

			return (byte[])alsDst.ToArray(typeof(byte));
		}

		public static byte[] DecompressChunk(byte[] abChunk) {
			ArrayList alsDst = new ArrayList();
			int ib = 0;
			bool fDone = false;
			while (!fDone) {
				// Get next flags

				byte bFlags = abChunk[ib];
				ib++;

				for (int ibitFlags = 7; ibitFlags >= 0; ibitFlags--) {
					// Literal or code?

					if ((bFlags & (1 << ibitFlags)) != 0) {
						alsDst.Add(abChunk[ib]);
						ib++;
						continue;
					}

					// Get a code

					ushort code = (ushort)(abChunk[ib] << 8);
					ib++;
					code |= (ushort)abChunk[ib];
					ib++;

					// Extended code? Check for count == 0.

					int ibBackwards = (int)(code & 0x1fff);
					int cb;
					if ((code & 0xe000) != 0) {
						// Count != 0, so not an extended code

						cb = (byte)((code & 0xe000) >> 13) + 1;
					} else {
						// Extended code. End?

						if (ibBackwards == 0) {
							fDone = true;
							break;
						}

						// Extended match

						ibBackwards = (int)(code << 1) | (int)(abChunk[ib] >> 7);
						cb = (int)(abChunk[ib] & 0x7f) + 2;
						ib++;
					}

					// Copy this chunk into the output

					byte[] abAdd = new byte[cb];
					alsDst.CopyTo(alsDst.Count - ibBackwards, abAdd, 0, cb);
					alsDst.AddRange(abAdd);
				}
			}

			return (byte[])alsDst.ToArray(typeof(byte));
		}
	}
}



