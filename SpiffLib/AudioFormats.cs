using System;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace SpiffLib
{
	/// <summary>
	/// Summary description for AudioFormats.
	/// </summary>
	public class AudioFormats
	{
		// Fibonacci

		static int[] s_anFibonacci = { -34, -21, -13, -8, -5, -3, -2, -1, 0, 1, 2, 3, 5, 8, 13, 21 };

		public static byte[] EncodeFibonacciDelta(byte[] abPcm) {
			return Encode8BitDelta(abPcm, s_anFibonacci);
		}

		public static byte[] DecodeFibonacciDelta(byte[] abEncoded) {
			return Decode8BitDelta(abEncoded, s_anFibonacci);
		}

		// Exponential

		static int[] s_anExponential = { -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64 };

		public static byte[] EncodeExponentialDelta(byte[] abPcm) {
			return Encode8BitDelta(abPcm, s_anExponential);
		}

		public static byte[] DecodeExponentialDelta(byte[] abEncoded) {
			return Decode8BitDelta(abEncoded, s_anExponential);
		}

		static byte[] Encode8BitDelta(byte[] abPcm, int[] anDelta) {
			byte[] abEncoded = new byte[1 + (abPcm.Length + 1) / 2];
			int nLast = (int)abPcm[0];
			abEncoded[0] = abPcm[0];
			for (int i = 1; i < abPcm.Length; i++) {
				// Important - the shortest path to the next sample may be by overflowing or underflowing.
				// If this isn't done, the results will be poor.

				sbyte sbDelta = (sbyte)(abPcm[i] - nLast);
				int iEntry = -1;
				int nDiffSmallest = Int32.MaxValue;
				for (int j = 0; j < anDelta.Length; j++) {
					int nDiff = (anDelta[j] - sbDelta);
					if (nDiff < 0)
						nDiff = -nDiff;
					if (nDiff < nDiffSmallest) {
						nDiffSmallest = nDiff;
						iEntry = j;
					}
				}
				Debug.Assert(iEntry >= 0 && iEntry <= 15);

				if ((i & 1) != 0) {
					abEncoded[(i - 1) / 2 + 1] = (byte)((iEntry << 4) + 8);
				} else {
					abEncoded[(i - 1) / 2 + 1] = (byte)((byte)(abEncoded[(i - 1) / 2 + 1] & 0xf0) | (byte)iEntry);
				}
				nLast += anDelta[iEntry];
			}

			return abEncoded;
		}

		static byte[] Decode8BitDelta(byte[] abEncoded, int[] anDelta) {
			byte[] abPcm = new byte[(abEncoded.Length - 1) * 2 + 1];
			byte bSample = abEncoded[0];
			abPcm[0] = bSample;
			for (int i = 1; i < abEncoded.Length; i++) {
				bSample += (byte)anDelta[abEncoded[i] >> 4];
				abPcm[(i - 1) * 2 + 1] = bSample;
				bSample += (byte)anDelta[abEncoded[i] & 0x0f];
				abPcm[(i - 1) * 2 + 2] = bSample;
			}

			return abPcm;
		}

		// Wrappers

		public unsafe static byte[] EncodeImaAdpcm(short[] ashPcm, int cBits, bool fPreClamp) {
			byte[] abEncoded = new byte[ashPcm.Length / 2];
			short *pshPcm = (short *)Marshal.UnsafeAddrOfPinnedArrayElement(ashPcm, 0);
			byte *pbOut = (byte *)Marshal.UnsafeAddrOfPinnedArrayElement(abEncoded, 0);
			adpcm_state state;
			state.valprev = 0;
			state.index = 0;
			adpcm_coder(pshPcm, pbOut, ashPcm.Length, &state, cBits, fPreClamp);
			return abEncoded;
		}

		public unsafe static short[] DecodeImaAdpcm(byte[] abEncoded, bool fPreClamp) {
			short[] ashOut = new short[abEncoded.Length * 2];
			byte *pbEncoded = (byte *)Marshal.UnsafeAddrOfPinnedArrayElement(abEncoded, 0);
			short *pshOut = (short *)Marshal.UnsafeAddrOfPinnedArrayElement(ashOut, 0);
			adpcm_state state;
			state.valprev = 0;
			state.index = 0;
			adpcm_decoder(pbEncoded, pshOut, abEncoded.Length * 2, &state, fPreClamp);
			return ashOut;
		}

		// IMD ADPCM
		// This implementation Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
		// Netherlands.

		/* Intel ADPCM step variation table */
		static int[] indexTable = new int[16] {
			-1, -1, -1, -1, 2, 4, 6, 8,
			-1, -1, -1, -1, 2, 4, 6, 8,
		};

		static int[] stepsizeTable = new int[89] {
			7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
			19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
			50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
			130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
			337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
			876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
			2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
			5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
			15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
		};
	    
		struct adpcm_state {
			public int valprev;
			public int index;
		};

		unsafe static void adpcm_coder(short *indata, byte *outdata, int len, adpcm_state *state, int cBits, bool fPreClamp)
		{
			short *inp;			/* Input buffer pointer */
			byte *outp;		/* output buffer pointer */
			int val;			/* Current input sample value */
			int sign;			/* Current adpcm sign bit */
			int delta;			/* Current adpcm output value */
			int diff;			/* Difference between val and valprev */
			int step;			/* Stepsize */
			int valpred;		/* Predicted output value */
			int vpdiff;			/* Current change to valpred */
			int index;			/* Current step change index */
			int outputbuffer;		/* place to keep previous 4-bit value */
			int bufferstep;		/* toggle between outputbuffer/output */

			outp = outdata;
			inp = indata;

			valpred = state->valprev;
			index = state->index;
			step = stepsizeTable[index];
		    
			outputbuffer = 0;
			bufferstep = 1;

			for ( ; len > 0 ; len-- ) {
				val = *inp++;

				/* Step 1 - compute difference with previous value */
				diff = val - valpred;
				sign = (diff < 0) ? 8 : 0;
				if (sign != 0) diff = (-diff);

				// If we preclamp, then at decode time we don't need to clamp
				// at all. It results in some quantization error however, but
				// it is so slight it fails the blind test.

				int vpdiffMax = int.MaxValue;
				if (fPreClamp) {
					if (sign != 0) {
						vpdiffMax = valpred - -32768;
					} else {
						vpdiffMax = 32767 - valpred;
					}
				}

				/* Step 2 - Divide and clamp */
				/* Note:
				** This code *approximately* computes:
				**    delta = diff*4/step;
				**    vpdiff = (delta+0.5)*step/4;
				** but in shift step bits are dropped. The net result of this is
				** that even if you have fast mul/div hardware you cannot put it to
				** good use since the fixup would be too expensive.
				*/
				delta = 0;

				if (fPreClamp) {
					vpdiff = 0;
				} else {
					vpdiff = (step >> 3);
				}

				if ( diff >= step ) {
					delta = 4;
					diff -= step;
					vpdiff += step;

					if (vpdiff > vpdiffMax) {
						vpdiff -= step;
						delta &= ~4;
					}
				}

				if (cBits > 2) {
					step >>= 1;
					if ( diff >= step  ) {
						delta |= 2;
						diff -= step;
						vpdiff += step;

						if (vpdiff > vpdiffMax) {
							vpdiff -= step;
							delta &= ~2;
						}
					}
				}

				if (cBits > 3) {
					step >>= 1;
					if ( diff >= step ) {
						delta |= 1;
						vpdiff += step;

						if (vpdiff > vpdiffMax) {
							vpdiff -= step;
							delta &= ~1;
						}
					}
				}

				/* Step 3 - Update previous value */
				if (sign != 0)
					valpred -= vpdiff;
				else
					valpred += vpdiff;

				/* Step 4 - Clamp previous value to 16 bits */
				if ( valpred > 32767 ) {
					valpred = 32767;
					Debug.Assert(!fPreClamp);
				} else if ( valpred < -32768 ) {
					valpred = -32768;
					Debug.Assert(!fPreClamp);
				}

				/* Step 5 - Assemble value, update index and step values */
				delta |= sign;
				
				index += indexTable[delta];
				if ( index < 0 ) index = 0;
				if ( index > 88 ) index = 88;
				step = stepsizeTable[index];

				/* Step 6 - Output value */
				if (bufferstep != 0 ) {
					outputbuffer = (delta << 4) & 0xf0;
				} else {
					*outp++ = (byte)((delta & 0x0f) | outputbuffer);
				}
				bufferstep ^= 1;
			}

			/* Output last step, if needed */
			if (bufferstep == 0)
				*outp++ = (byte)outputbuffer;
		    
			state->valprev = valpred;
			state->index = index;
		}

		unsafe static void adpcm_decoder(byte *indata, short *outdata, int len, adpcm_state *state, bool fPreClamp)
		{
			byte *inp;		/* Input buffer pointer */
			short *outp;		/* output buffer pointer */
			int sign;			/* Current adpcm sign bit */
			int delta;			/* Current adpcm output value */
			int step;			/* Stepsize */
			int valpred;		/* Predicted value */
			int vpdiff;			/* Current change to valpred */
			int index;			/* Current step change index */
			int inputbuffer;		/* place to keep next 4-bit value */
			int bufferstep;		/* toggle between inputbuffer/input */

			outp = outdata;
			inp = indata;

			valpred = state->valprev;
			index = state->index;
			step = stepsizeTable[index];

			inputbuffer = 0;
			bufferstep = 0;
		    
			for ( ; len > 0 ; len-- ) {
	
				/* Step 1 - get the delta value */
				if (bufferstep != 0) {
					delta = inputbuffer & 0xf;
				} else {
					inputbuffer = *inp++;
					delta = (inputbuffer >> 4) & 0xf;
				}
				bufferstep ^= 1;

				/* Step 2 - Find new index value (for later) */
				index += indexTable[delta];
				if ( index < 0 ) index = 0;
				if ( index > 88 ) index = 88;

				/* Step 3 - Separate sign and magnitude */
				sign = delta & 8;
				delta = delta & 7;

				/* Step 4 - Compute difference and new predicted value */
				/*
				** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
				** in adpcm_coder.
				*/

				if (fPreClamp) {
					vpdiff = 0;
				} else {
					vpdiff = step >> 3;
				}

				if ((delta & 4) != 0) vpdiff += step;
				if ((delta & 2) != 0) vpdiff += step>>1;
				if ((delta & 1) != 0) vpdiff += step>>2;

				if (sign != 0)
					valpred -= vpdiff;
				else
					valpred += vpdiff;

				/* Step 5 - clamp output value */
				if (!fPreClamp) {
					if ( valpred > 32767 )
						valpred = 32767;
					else if ( valpred < -32768 )
						valpred = -32768;
				}

				/* Step 6 - Update step value */
				step = stepsizeTable[index];

				/* Step 7 - Output value */
				*outp++ = (short)valpred;
			}

			state->valprev = valpred;
			state->index = index;
		}

		static int[] s_anStepFractions = new int[] {
			1,1,1,1,1,2,2,2,2,2,2,3,3,3,4,4,4,5,5,6,6,7,8,8,9,10,11,12,13,15,16,18,20,22,24,26,29,32,35,38,42,46,51,56,62,68,75,82,91,100,110,120,133,146,160,176,194,213,235,258,284,312,344,378,416,458,503,554,609,670,737,811,892,981,1079,1187,1305,1436,1579,1737,1911,2102,2313,2544,2798,3078,3386,3724,4096,
			2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,9,10,11,13,14,15,17,18,20,22,24,27,30,33,36,39,43,48,52,58,63,70,77,84,93,102,112,124,136,150,165,181,199,219,241,265,292,321,353,388,427,470,517,568,625,687,756,832,915,1007,1107,1218,1340,1474,1621,1783,1961,2158,2373,2611,2872,3159,3475,3822,4205,4625,5088,5596,6156,6772,7449,8192,
			4,4,5,5,6,6,7,7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,30,33,37,40,44,49,54,59,65,72,79,87,95,105,115,127,140,154,169,186,204,225,247,272,299,329,362,398,438,482,530,583,641,706,776,854,939,1033,1136,1250,1375,1512,1664,1830,2013,2214,2436,2679,2947,3242,3566,3923,4315,4747,5221,5744,6318,6950,7645,8409,9250,10175,11193,12312,13543,14897,16384,
			5,6,7,8,8,9,10,11,12,13,14,16,17,19,21,23,26,28,31,34,38,41,45,50,55,60,66,73,80,89,98,107,118,130,143,157,173,190,209,230,253,278,306,337,371,408,449,494,543,597,657,722,795,875,962,1058,1164,1280,1409,1550,1704,1874,2062,2268,2495,2745,3020,3321,3653,4019,4421,4863,5349,5884,6473,7120,7832,8615,9476,10424,11467,12614,13875,15263,16789,18467,20315,22346,24575,
			7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767,
			9,10,11,13,14,15,16,18,20,21,24,26,29,31,35,39,43,46,51,56,63,69,75,83,91,100,110,121,134,148,163,179,196,216,238,261,288,316,349,384,421,464,510,561,618,680,748,823,905,995,1095,1204,1325,1458,1603,1764,1940,2134,2348,2583,2840,3124,3436,3780,4159,4575,5033,5535,6089,6698,7368,8105,8915,9806,10788,11866,13053,14359,15794,17374,19111,21023,23125,25438,27981,30779,33858,37243,40959,
			11,12,14,15,17,18,20,21,24,26,29,32,35,38,42,47,51,56,62,68,75,83,90,99,110,120,132,146,161,177,195,215,236,260,285,314,345,380,419,461,506,557,612,674,741,816,897,987,1086,1194,1314,1445,1590,1749,1923,2117,2328,2561,2817,3099,3408,3749,4124,4536,4991,5490,6039,6642,7307,8037,8841,9726,10698,11768,12945,14240,15663,17231,18953,20849,22934,25227,27750,30525,33578,36935,40629,44691,49151,
			12,14,16,18,19,21,23,25,28,30,33,37,40,44,49,54,60,65,72,79,88,96,105,116,128,140,154,170,187,207,228,250,275,303,333,366,403,443,488,537,590,649,714,786,865,952,1047,1152,1267,1393,1533,1685,1855,2041,2244,2469,2716,2987,3287,3616,3976,4373,4811,5292,5822,6405,7046,7749,8524,9377,10315,11347,12481,13729,15103,16613,18274,20102,22111,24323,26756,29432,32375,35613,39174,43090,47401,52140,57342,
			-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-8,-9,-10,-11,-12,-13,-15,-16,-18,-20,-22,-24,-26,-29,-32,-35,-38,-42,-46,-51,-56,-62,-68,-75,-82,-91,-100,-110,-120,-133,-146,-160,-176,-194,-213,-235,-258,-284,-312,-344,-378,-416,-458,-503,-554,-609,-670,-737,-811,-892,-981,-1079,-1187,-1305,-1436,-1579,-1737,-1911,-2102,-2313,-2544,-2798,-3078,-3386,-3724,-4096,
			-2,-2,-2,-3,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-9,-9,-10,-11,-13,-14,-15,-17,-18,-20,-22,-24,-27,-30,-33,-36,-39,-43,-48,-52,-58,-63,-70,-77,-84,-93,-102,-112,-124,-136,-150,-165,-181,-199,-219,-241,-265,-292,-321,-353,-388,-427,-470,-517,-568,-625,-687,-756,-832,-915,-1007,-1107,-1218,-1340,-1474,-1621,-1783,-1961,-2158,-2373,-2611,-2872,-3159,-3475,-3822,-4205,-4625,-5088,-5596,-6156,-6772,-7449,-8192,
			-4,-4,-5,-5,-6,-6,-7,-7,-8,-9,-10,-11,-12,-13,-14,-16,-17,-19,-21,-23,-25,-28,-30,-33,-37,-40,-44,-49,-54,-59,-65,-72,-79,-87,-95,-105,-115,-127,-140,-154,-169,-186,-204,-225,-247,-272,-299,-329,-362,-398,-438,-482,-530,-583,-641,-706,-776,-854,-939,-1033,-1136,-1250,-1375,-1512,-1664,-1830,-2013,-2214,-2436,-2679,-2947,-3242,-3566,-3923,-4315,-4747,-5221,-5744,-6318,-6950,-7645,-8409,-9250,-10175,-11193,-12312,-13543,-14897,-16384,
			-5,-6,-7,-8,-8,-9,-10,-11,-12,-13,-14,-16,-17,-19,-21,-23,-26,-28,-31,-34,-38,-41,-45,-50,-55,-60,-66,-73,-80,-89,-98,-107,-118,-130,-143,-157,-173,-190,-209,-230,-253,-278,-306,-337,-371,-408,-449,-494,-543,-597,-657,-722,-795,-875,-962,-1058,-1164,-1280,-1409,-1550,-1704,-1874,-2062,-2268,-2495,-2745,-3020,-3321,-3653,-4019,-4421,-4863,-5349,-5884,-6473,-7120,-7832,-8615,-9476,-10424,-11467,-12614,-13875,-15263,-16789,-18467,-20315,-22346,-24575,
			-7,-8,-9,-10,-11,-12,-13,-14,-16,-17,-19,-21,-23,-25,-28,-31,-34,-37,-41,-45,-50,-55,-60,-66,-73,-80,-88,-97,-107,-118,-130,-143,-157,-173,-190,-209,-230,-253,-279,-307,-337,-371,-408,-449,-494,-544,-598,-658,-724,-796,-876,-963,-1060,-1166,-1282,-1411,-1552,-1707,-1878,-2066,-2272,-2499,-2749,-3024,-3327,-3660,-4026,-4428,-4871,-5358,-5894,-6484,-7132,-7845,-8630,-9493,-10442,-11487,-12635,-13899,-15289,-16818,-18500,-20350,-22385,-24623,-27086,-29794,-32767,
			-9,-10,-11,-13,-14,-15,-16,-18,-20,-21,-24,-26,-29,-31,-35,-39,-43,-46,-51,-56,-63,-69,-75,-83,-91,-100,-110,-121,-134,-148,-163,-179,-196,-216,-238,-261,-288,-316,-349,-384,-421,-464,-510,-561,-618,-680,-748,-823,-905,-995,-1095,-1204,-1325,-1458,-1603,-1764,-1940,-2134,-2348,-2583,-2840,-3124,-3436,-3780,-4159,-4575,-5033,-5535,-6089,-6698,-7368,-8105,-8915,-9806,-10788,-11866,-13053,-14359,-15794,-17374,-19111,-21023,-23125,-25438,-27981,-30779,-33858,-37243,-40959,
			-11,-12,-14,-15,-17,-18,-20,-21,-24,-26,-29,-32,-35,-38,-42,-47,-51,-56,-62,-68,-75,-83,-90,-99,-110,-120,-132,-146,-161,-177,-195,-215,-236,-260,-285,-314,-345,-380,-419,-461,-506,-557,-612,-674,-741,-816,-897,-987,-1086,-1194,-1314,-1445,-1590,-1749,-1923,-2117,-2328,-2561,-2817,-3099,-3408,-3749,-4124,-4536,-4991,-5490,-6039,-6642,-7307,-8037,-8841,-9726,-10698,-11768,-12945,-14240,-15663,-17231,-18953,-20849,-22934,-25227,-27750,-30525,-33578,-36935,-40629,-44691,-49151,
			-12,-14,-16,-18,-19,-21,-23,-25,-28,-30,-33,-37,-40,-44,-49,-54,-60,-65,-72,-79,-88,-96,-105,-116,-128,-140,-154,-170,-187,-207,-228,-250,-275,-303,-333,-366,-403,-443,-488,-537,-590,-649,-714,-786,-865,-952,-1047,-1152,-1267,-1393,-1533,-1685,-1855,-2041,-2244,-2469,-2716,-2987,-3287,-3616,-3976,-4373,-4811,-5292,-5822,-6405,-7046,-7749,-8524,-9377,-10315,-11347,-12481,-13729,-15103,-16613,-18274,-20102,-22111,-24323,-26756,-29432,-32375,-35613,-39174,-43090,-47401,-52140,-57342
		};

		static int[] s_anStepIndexMap = new int[] {
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,
			4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,
			6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,88,88,
			8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,88,88,88,88,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
			2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,
			4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,
			6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,88,88,
			8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,88,88,88,88,88,88,88,88
		};

		public static byte[] EncodeImaAdpcmTableDriven(short[] ashPcm) {
			ArrayList alsEncoded = new ArrayList();

			short shPredicted = 0;
			int nStepIndex = 0;

			int[] anStep = new int[2];
			for (int i = 0; i < ashPcm.Length; i += 2) {
				// Encoded size is a nibble, so 2 per byte

				for (int k = 0; k < 2; k++) {
					anStep[k] = 0;
					if (i + k == ashPcm.Length)
						break;
					int nDelta = ashPcm[i + k] - shPredicted;
					int iStepSmallest = -1;
					int nDiffSmallest = int.MaxValue;
					for (int j = 0; j < 16; j++) {
						int nStepFraction = s_anStepFractions[j * 89 + nStepIndex];
						if (shPredicted + nStepFraction < -32768 || shPredicted + nStepFraction > 32767)
							continue;
						int nDiff = Math.Abs(nStepFraction - nDelta);
						if (nDiff < nDiffSmallest) {
							nDiffSmallest = nDiff;
							iStepSmallest = j;
						}
					}
					anStep[k] = iStepSmallest;
					shPredicted += (short)s_anStepFractions[iStepSmallest * 89 + nStepIndex];
					nStepIndex = s_anStepIndexMap[iStepSmallest * 89 + nStepIndex];
				}

				// Add 1 byte to output

				alsEncoded.Add((byte)((anStep[0] << 4) | anStep[1]));
			}

			return (byte[])alsEncoded.ToArray(typeof(byte));
		}

		public static short[] DecodeImaAdpcmTableDriven(byte[] abIn) {
			ArrayList alsPcm = new ArrayList();

			short shPredicted = 0;
			int nStepIndex = 0;

			foreach (byte bT in abIn) {
				int nDelta = bT >> 4;
				shPredicted += (short)s_anStepFractions[nDelta * 89 + nStepIndex];
				alsPcm.Add(shPredicted);
				nStepIndex = s_anStepIndexMap[nDelta * 89 + nStepIndex];
				nDelta = bT & 0xf;
				shPredicted += (short)s_anStepFractions[nDelta * 89 + nStepIndex];
				alsPcm.Add(shPredicted);
				nStepIndex = s_anStepIndexMap[nDelta * 89 + nStepIndex];
			}

			return (short[])alsPcm.ToArray(typeof(short));
		}

		static int[] s_anStepFractions8BitEncode = new int[] {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,4,4,4,5,5,6,6,7,8,8,9,10,11,12,14,15,16,18,20,22,24,26,29,32,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,5,5,5,6,7,7,8,9,10,11,12,13,14,15,17,19,20,22,25,27,30,33,36,40,44,48,53,58,64,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,49,54,60,66,72,79,87,96,106,116,128,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,4,4,5,5,5,5,6,7,8,8,9,10,11,12,14,14,16,18,20,21,23,26,29,32,35,38,42,46,50,56,61,67,74,81,89,98,108,119,131,144,158,174,191,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,5,5,6,6,7,7,8,9,10,11,12,13,15,16,18,19,21,24,26,28,31,34,38,42,46,50,56,61,67,74,81,89,98,108,119,131,144,158,174,192,211,232,255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,4,4,4,4,5,5,6,6,8,8,9,9,10,11,13,14,15,16,19,20,23,24,26,30,33,35,39,43,48,53,58,63,70,76,84,93,101,111,123,135,149,164,180,198,218,240,264,290,319,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,5,5,5,5,6,6,8,8,9,9,11,11,12,14,15,17,18,20,23,24,27,29,32,36,39,42,47,51,57,63,69,75,84,92,101,111,122,134,147,162,179,197,216,237,261,288,317,348,383,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,5,5,5,5,7,7,9,9,11,11,12,12,14,16,18,19,21,23,26,28,32,33,37,42,46,49,54,60,67,74,81,88,98,107,117,130,142,156,172,189,208,229,252,277,305,336,369,406,446,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-8,-9,-10,-11,-12,-14,-15,-16,-18,-20,-22,-24,-26,-29,-32,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-4,-4,-5,-5,-5,-6,-7,-7,-8,-9,-10,-11,-12,-13,-14,-15,-17,-19,-20,-22,-25,-27,-30,-33,-36,-40,-44,-48,-53,-58,-64,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-8,-9,-10,-11,-12,-13,-14,-16,-17,-19,-21,-23,-25,-28,-31,-34,-37,-41,-45,-49,-54,-60,-66,-72,-79,-87,-96,-106,-116,-128,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-4,-4,-5,-5,-5,-5,-6,-7,-8,-8,-9,-10,-11,-12,-14,-14,-16,-18,-20,-21,-23,-26,-29,-32,-35,-38,-42,-46,-50,-56,-61,-67,-74,-81,-89,-98,-108,-119,-131,-144,-158,-174,-191,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-3,-3,-3,-3,-4,-4,-5,-5,-6,-6,-7,-7,-8,-9,-10,-11,-12,-13,-15,-16,-18,-19,-21,-24,-26,-28,-31,-34,-38,-42,-46,-50,-56,-61,-67,-74,-81,-89,-98,-108,-119,-131,-144,-158,-174,-192,-211,-232,-255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-3,-3,-3,-3,-3,-4,-4,-4,-4,-5,-5,-6,-6,-8,-8,-9,-9,-10,-11,-13,-14,-15,-16,-19,-20,-23,-24,-26,-30,-33,-35,-39,-43,-48,-53,-58,-63,-70,-76,-84,-93,-101,-111,-123,-135,-149,-164,-180,-198,-218,-240,-264,-290,-319,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-5,-5,-5,-5,-6,-6,-8,-8,-9,-9,-11,-11,-12,-14,-15,-17,-18,-20,-23,-24,-27,-29,-32,-36,-39,-42,-47,-51,-57,-63,-69,-75,-84,-92,-101,-111,-122,-134,-147,-162,-179,-197,-216,-237,-261,-288,-317,-348,-383,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-4,-4,-4,-4,-4,-5,-5,-5,-5,-7,-7,-9,-9,-11,-11,-12,-12,-14,-16,-18,-19,-21,-23,-26,-28,-32,-33,-37,-42,-46,-49,-54,-60,-67,-74,-81,-88,-98,-107,-117,-130,-142,-156,-172,-189,-208,-229,-252,-277,-305,-336,-369,-406,-446
		};

		public static byte[] EncodeImaAdpcmTableDriven8Bit(byte[] abPcm) {
			ArrayList alsEncoded = new ArrayList();

			byte bPredicted = 128;
			int nStepIndex = 0;

			int[] anStep = new int[2];
			for (int i = 0; i < abPcm.Length; i += 2) {
				// Encoded size is a nibble, so 2 per byte

				for (int k = 0; k < 2; k++) {
					anStep[k] = 0;
					if (i + k == abPcm.Length)
						break;
					int nDelta = abPcm[i + k] - bPredicted;
					int iStepSmallest = -1;
					int nDiffSmallest = int.MaxValue;
					for (int j = 0; j < 16; j++) {
						int nStepFraction = s_anStepFractions8BitEncode[j * 89 + nStepIndex];
						if (bPredicted + nStepFraction < 0 || bPredicted + nStepFraction > 255)
							continue;
						int nDiff = Math.Abs(nStepFraction - nDelta);
						if (nDiff <= nDiffSmallest) {
							nDiffSmallest = nDiff;
							iStepSmallest = j;
						}
					}
					anStep[k] = iStepSmallest;
					bPredicted += (byte)s_anStepFractions8BitEncode[iStepSmallest * 89 + nStepIndex];
					nStepIndex = s_anStepIndexMap[iStepSmallest * 89 + nStepIndex];
				}

				// Add 1 byte to output

				alsEncoded.Add((byte)((anStep[0] << 4) | anStep[1]));
			}

			return (byte[])alsEncoded.ToArray(typeof(byte));
		}

		static byte[] s_anStepFractions8BitDecode = new byte[] {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,4,4,4,5,5,6,6,7,8,8,9,10,11,12,14,15,16,18,20,22,24,26,29,32,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,5,5,5,6,7,7,8,9,10,11,12,13,14,15,17,19,20,22,25,27,30,33,36,40,44,48,53,58,64,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,49,54,60,66,72,79,87,96,106,116,128,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,4,4,5,5,5,5,6,7,8,8,9,10,11,12,14,14,16,18,20,21,23,26,29,32,35,38,42,46,50,56,61,67,74,81,89,98,108,119,131,144,158,174,191,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,5,5,6,6,7,7,8,9,10,11,12,13,15,16,18,19,21,24,26,28,31,34,38,42,46,50,56,61,67,74,81,89,98,108,119,131,144,158,174,192,211,232,255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,4,4,4,4,5,5,6,6,8,8,9,9,10,11,13,14,15,16,19,20,23,24,26,30,33,35,39,43,48,53,58,63,70,76,84,93,101,111,123,135,149,164,180,198,218,240,255,255,255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,5,5,5,5,6,6,8,8,9,9,11,11,12,14,15,17,18,20,23,24,27,29,32,36,39,42,47,51,57,63,69,75,84,92,101,111,122,134,147,162,179,197,216,237,255,255,255,255,255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,5,5,5,5,7,7,9,9,11,11,12,12,14,16,18,19,21,23,26,28,32,33,37,42,46,49,54,60,67,74,81,88,98,107,117,130,142,156,172,189,208,229,252,255,255,255,255,255,255,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,253,253,253,252,252,252,251,251,250,250,249,248,248,247,246,245,244,242,241,240,238,236,234,232,230,227,224,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,253,253,253,253,252,252,251,251,251,250,249,249,248,247,246,245,244,243,242,241,239,237,236,234,231,229,226,223,220,216,212,208,203,198,192,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,253,253,253,253,252,252,252,251,251,250,250,249,248,248,247,246,245,244,243,242,240,239,237,235,233,231,228,225,222,219,215,211,207,202,196,190,184,177,169,160,150,140,128,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,254,254,254,253,253,252,252,251,251,251,251,250,249,248,248,247,246,245,244,242,242,240,238,236,235,233,230,227,224,221,218,214,210,206,200,195,189,182,175,167,158,148,137,125,112,98,82,65,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,253,253,253,253,252,252,251,251,250,250,249,249,248,247,246,245,244,243,241,240,238,237,235,232,230,228,225,222,218,214,210,206,200,195,189,182,175,167,158,148,137,125,112,98,82,64,45,24,1,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,253,253,253,253,253,252,252,252,252,251,251,250,250,248,248,247,247,246,245,243,242,241,240,237,236,233,232,230,226,223,221,217,213,208,203,198,193,186,180,172,163,155,145,133,121,107,92,76,58,38,16,1,1,1,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,254,254,254,254,254,254,254,254,254,254,253,253,253,253,253,251,251,251,251,250,250,248,248,247,247,245,245,244,242,241,239,238,236,233,232,229,227,224,220,217,214,209,205,199,193,187,181,172,164,155,145,134,122,109,94,77,59,40,19,1,1,1,1,1,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,254,254,254,254,254,254,254,254,254,254,252,252,252,252,252,251,251,251,251,249,249,247,247,245,245,244,244,242,240,238,237,235,233,230,228,224,223,219,214,210,207,202,196,189,182,175,168,158,149,139,126,114,100,84,67,48,27,4,1,1,1,1,1,1
		};
		
		public static byte[] DecodeImaAdpcmTableDriven8Bit(byte[] abIn) {
			ArrayList alsPcm = new ArrayList();

			byte bPredicted = 128;
			int nStepIndex = 0;

			foreach (byte bT in abIn) {
				int nDelta = bT >> 4;
				bPredicted += s_anStepFractions8BitDecode[nDelta * 89 + nStepIndex];
				alsPcm.Add(bPredicted);
				nStepIndex = s_anStepIndexMap[nDelta * 89 + nStepIndex];
				nDelta = bT & 0xf;
				bPredicted += s_anStepFractions8BitDecode[nDelta * 89 + nStepIndex];
				alsPcm.Add(bPredicted);
				nStepIndex = s_anStepIndexMap[nDelta * 89 + nStepIndex];
			}

			return (byte[])alsPcm.ToArray(typeof(byte));
		}

		public static byte[] EncodeYamahaAdpcm(short[] ashPcm) {
			ArrayList alsEncoded = new ArrayList();
			int nPcmPredicted = 0;
			int nStep = 0x7f;
			byte bOutput = 0;

			for (int ish = 0; ish < ashPcm.Length; ish++) {
				//Console.WriteLine("ish: " + ish + " nPcmPredicted: " + nPcmPredicted + " nStep: " + nStep);

				//if (ish == 256)
				//	Debug.Assert(false);

				int nPcm = ashPcm[ish];

				int nDiff = nPcm - nPcmPredicted;
				bool fNegative = false;
				if (nDiff < 0) {
					fNegative = true;
					nDiff = -nDiff;
				}

				// Get fraction in multiples of 1/4ths. Note by first shifting down by two, resolution is lost.
				// I.e. x / 4 * 2 != x / 2.

				int cTimes = 0;
				int nStep4th = nStep / 4;
				int nFracT = nStep4th;
				while (cTimes < 7) {
					if (nDiff < nFracT)
						break;
					nFracT += nStep4th;
					cTimes++;
				}

				// Interestingly this "rebuilding" from bits has more precision that the above
				// because of the above nStep / 4. The nStep / 2 below has more precision than
				// 2 * (nStep / 4). For example 23 / 4 * 2 == 10 vs. 23 / 2 == 11.

				int nStepFraction = 0;
				if ((cTimes & 1) != 0)
					nStepFraction += nStep / 4;
				if ((cTimes & 2) != 0)
					nStepFraction += nStep / 2;
				if ((cTimes & 4) != 0)
					nStepFraction += nStep;
				nStepFraction += nStep / 8;

				// Add to predicted, do range check

				byte b = (byte)cTimes;
				if (fNegative) {
					nPcmPredicted -= nStepFraction;
					b |= 8;
				} else {
					nPcmPredicted += nStepFraction;
				}
				if (nPcmPredicted > 32767) {
					nPcmPredicted = 32767;
				} else if (nPcmPredicted < -32768) {
					nPcmPredicted = -32768;
				}

				// Yamaha calcs the next step like this. Same as MS ADPCM in terms of constants used

				switch (b & 7) {
				case 0:
				case 1:
				case 2:
				case 3:
					nStep = nStep * 230 / 256;
					break;

				case 4:
					nStep = nStep * 307 / 256;
					break;

				case 5:
					nStep = nStep * 409 / 256;
					break;	

				case 6:
					nStep = nStep * 512 / 256;
					break;

				case 7:
					nStep = nStep * 614 / 256;
					break;
				}

				// Bounds check the step

				if (nStep < 0x7f) {
					nStep = 0x7f;
				} else if (nStep > 0x6000) {
					nStep = 0x6000;
				}

				// Output a byte. Yamaha uses "little endian" nibble order.

				if ((ish & 1) == 0) {
					bOutput = (byte)(b & 0x0f);
				} else {
					bOutput |= (byte)((b << 4) & 0xf0);
					alsEncoded.Add(bOutput);
				}
			}

			return (byte[])alsEncoded.ToArray(typeof(byte));
		}
		
		public static byte[] EncodeYamahaAdpcmFaster(short[] ashPcm) {
			ArrayList alsEncoded = new ArrayList();
			int nPcmPredicted = 0;
			int nStep = 0x7f;

			// Encoding test for "fastest" encoding. Essentially this unrolls the conditionals as much as possible.
			// This is an algorithmic test before encoding this in assembly language

			for (int ish = 0; ish < ashPcm.Length; ish += 2) {
				//Console.WriteLine("ish: " + ish + " nPcmPredicted: " + nPcmPredicted + " nStep: " + nStep);

				//if (ish == 256)
				//	Debug.Assert(false);

				byte bOut;
				int nDiff;
				
				// Low nibble (first sample of byte)

				nDiff = ashPcm[ish] - nPcmPredicted;
				if (nDiff >= 0) {
					// nDiff positive

					int nStepT = nStep & ~3;
					if (nDiff >= nStepT) {
						nDiff -= nStepT;
						nStepT = nStepT >> 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x07

								nPcmPredicted += nStep + (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 614) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x07;
							} else {
								// case 0x06

								nPcmPredicted += nStep + (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep <<= 1;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x06;
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x05

								nPcmPredicted += nStep + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 409) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x05;
							} else {
								// case 0x04

								nPcmPredicted += nStep + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 307) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x04;
							}
						}
					} else {
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x03

								nPcmPredicted += (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x03;
							} else {
								// case 0x02

								nPcmPredicted += (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x02;
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x01

								nPcmPredicted += nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x01;
							} else {
								// case 0x00

								nPcmPredicted += (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x00;
							}
						}
					}
				} else {
					// nDiff negative

					nDiff = -nDiff;
					int nStepT = nStep & ~3;
					if (nDiff >= nStepT) {
						nDiff -= nStepT;
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0f

								nPcmPredicted -= nStep + (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 614) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x0f;
							} else {
								// case 0x0e

								nPcmPredicted -= nStep + (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep <<= 1;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x0e;
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0d

								nPcmPredicted -= nStep + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 409) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x0d;
							} else {
								// case 0x0c

								nPcmPredicted -= nStep + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 307) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								bOut = 0x0c;
							}
						}
					} else {
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0b

								nPcmPredicted -= (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x0b;
							} else {
								// case 0x0a

								nPcmPredicted -= (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x0a;
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x09

								nPcmPredicted -= nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x09;
							} else {
								// case 0x08

								nPcmPredicted -= (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								bOut = 0x08;
							}
						}
					}
				}

				// High nibble, second sample of byte

				nDiff = ashPcm[ish + 1] - nPcmPredicted;
				if (nDiff >= 0) {
					// nDiff positive

					int nStepT = nStep & ~3;
					if (nDiff >= nStepT) {
						nDiff -= nStepT;
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x07

								nPcmPredicted += nStep + (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 614) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0x70 + bOut));
							} else {
								// case 0x06

								nPcmPredicted += nStep + (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep <<= 1;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0x60 + bOut));
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x05

								nPcmPredicted += nStep + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 409) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0x50 + bOut));
							} else {
								// case 0x04

								nPcmPredicted += nStep + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 307) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0x40 + bOut));
							}
						}
					} else {
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x03

								nPcmPredicted += (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x30 + bOut));
							} else {
								// case 0x02

								nPcmPredicted += (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x20 + bOut));
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x01

								nPcmPredicted += nStepT + (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x10 + bOut));
							} else {
								// case 0x00

								nPcmPredicted += (nStepT >> 1);
								if (nPcmPredicted > 32767)
									nPcmPredicted = 32767;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x00 + bOut));
							}
						}
					}
				} else {
					// nDiff negative

					nDiff = -nDiff;
					int nStepT = nStep & ~3;
					if (nDiff >= nStepT) {
						nDiff -= nStepT;
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0f

								nPcmPredicted -= nStep + (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 614) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0xf0 + bOut));
							} else {
								// case 0x0e

								nPcmPredicted -= nStep + (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep <<= 1;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0xe0 + bOut));
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0d

								nPcmPredicted -= nStep + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 409) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0xd0 + bOut));
							} else {
								// case 0x0c

								nPcmPredicted -= nStep + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 307) >> 8;
								if (nStep > 0x6000)
									nStep = 0x6000;
								alsEncoded.Add((byte)(0xc0 + bOut));
							}
						}
					} else {
						nStepT >>= 1;
						if (nDiff >= nStepT) {
							nDiff -= nStepT;
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x0b

								nPcmPredicted -= (nStep >> 1) + nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0xb0 + bOut));
							} else {
								// case 0x0a

								nPcmPredicted -= (nStep >> 1) + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0xa0 + bOut));
							}
						} else {
							nStepT >>= 1;
							if (nDiff >= nStepT) {
								// case 0x09

								nPcmPredicted -= nStepT + (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x90 + bOut));
							} else {
								// case 0x08

								nPcmPredicted -= (nStepT >> 1);
								if (nPcmPredicted < -32768)
									nPcmPredicted = -32768;
								nStep = (nStep * 230) >> 8;
								if (nStep < 0x7f)
									nStep = 0x7f;
								alsEncoded.Add((byte)(0x80 + bOut));
							}
						}
					}
				}
			}

			return (byte[])alsEncoded.ToArray(typeof(byte));
		}

		public static void Write8BitAdpcmDecodeTable() {
			TextWriter tw = new StreamWriter("decode.s");

			tw.WriteLine("Decode8BitAdpcm:");
			for (int nStepIndex = 0; nStepIndex < 89; nStepIndex++) {
				tw.WriteLine("| Step Index " + nStepIndex);
				tw.Write(".byte ");
				for (int nDelta = 0; nDelta < 16; nDelta++)
					tw.Write(s_anStepFractions8BitDecode[nDelta * 89 + nStepIndex] + (nDelta == 15 ? "\n" : ", "));
				tw.Write(".word ");
				for (int nDelta = 0; nDelta < 16; nDelta++) {
					int offCurrentIndex = 48 * nStepIndex;
					int nStepIndexNew = s_anStepIndexMap[nDelta * 89  + nStepIndex];
					int offNewIndex = 48 * nStepIndexNew;
					tw.Write("0x" + ((ushort)(offNewIndex - offCurrentIndex)).ToString("x") + (nDelta == 15 ? "\n" : ", "));
				}
			}

			tw.Close();
		}

		public static void Write8BitAdpcmDecodeTableC() {
			TextWriter tw = new StreamWriter("decode.cpp");

			tw.WriteLine("byte gabAdpcmSteppings[89][16] = {");
			for (int nStepIndex = 0; nStepIndex < 89; nStepIndex++) {
				tw.Write("    ");
				for (int nDelta = 0; nDelta < 16; nDelta++)
					tw.Write(s_anStepFractions8BitDecode[nDelta * 89 + nStepIndex] + (nDelta == 15 ? ",\n" : ", "));
			}
			tw.WriteLine("};");

			tw.WriteLine("char s_anStepIndexMap[89][16] = {");
			for (int nStepIndex = 0; nStepIndex < 89; nStepIndex++) {
				tw.Write("    ");
				for (int nDelta = 0; nDelta < 16; nDelta++)
					tw.Write(s_anStepIndexMap[nDelta * 89 + nStepIndex] + (nDelta == 15 ? ",\n" : ", "));
			}
			tw.WriteLine("};");
			tw.Close();
		}
	}
	
	class Pcm {
		int m_cBits;
		int m_hz;
		byte[] m_ab;
		short[] m_ash;

		// See http://www.intersrv.com/~dcross/wavio.html for format info, or
		// http://ccrma-www.stanford.edu/courses/422/projects/WaveFormat/

		[StructLayout(LayoutKind.Sequential)]
			struct FourCC {
			public byte b0;
			public byte b1;
			public byte b2;
			public byte b3;

			public FourCC(string str) {
				b0 = (byte)str[0];
				b1 = (byte)str[1];
				b2 = (byte)str[2];
				b3 = (byte)str[3];
			}
		};

		[StructLayout(LayoutKind.Sequential)]
			struct WavFileHeader { 
			public FourCC fccRIFF;
			public uint cbRiffBytes;
			public FourCC fccWAVE;
			public FourCC fccfmt;
			public uint cbfmtBytes;
			public ushort wFormatTag; 
			public ushort nChannels; 
			public uint nSamplesPerSec; 
			public uint nAvgBytesPerSec; 
			public ushort nBlockAlign; 
			public ushort nBitsPerSample; 
			public FourCC fccdata;
			public uint cbdataBytes;
		};

		unsafe public Pcm (string strWav) {
			if (strWav.EndsWith(".wav") || strWav.EndsWith(".WAV")) {
				LoadWavFile(strWav);
			}
			if (strWav.EndsWith(".snd")) {
				LoadSndFile(strWav);
			}
		}

		unsafe void LoadWavFile(string strWav) {
			BinaryReader brdr = new BinaryReader(new FileStream(strWav, FileMode.Open, FileAccess.Read, FileShare.Read));
			byte[] ab = brdr.ReadBytes((int)brdr.BaseStream.Length);
			brdr.Close();

			WavFileHeader *pwavhdr = (WavFileHeader *)Marshal.UnsafeAddrOfPinnedArrayElement(ab, 0);
			bool fOk = true;
			if (pwavhdr->wFormatTag != 1)
				fOk = false;
			if (pwavhdr->nBitsPerSample != 8 && pwavhdr->nBitsPerSample != 16)
				fOk = false;
			if (!fOk)
				throw new Exception("Only support reading 8 / 16 bit raw pcm!");
			m_cBits = pwavhdr->nBitsPerSample;
			m_hz = (int)pwavhdr->nSamplesPerSec;
			int cbSizeType = Marshal.SizeOf(pwavhdr->GetType());
			switch (m_cBits) {
			case 8:
				m_ab = new byte[pwavhdr->cbdataBytes];
				for (int i = 0; i < m_ab.Length; i++)
					m_ab[i] = ab[i + cbSizeType];
				break;

			case 16:
				m_ash = new short[pwavhdr->cbdataBytes / 2];
				for (int i = 0; i < pwavhdr->cbdataBytes; i += 2)
					m_ash[i / 2] = (short)(ab[i + cbSizeType] + ab[1 + i + cbSizeType] * 256);
				break;
			}
		}

		void LoadSndFile(string strSnd) {
			BinaryReader brdr = new BinaryReader(new FileStream(strSnd, FileMode.Open, FileAccess.Read, FileShare.Read));
			byte[] ab = brdr.ReadBytes((int)brdr.BaseStream.Length);
			brdr.Close();

			m_cBits = 8;
			m_hz = 8000;
			m_ab = new byte[ab.Length - 6];
			for (int i = 0; i < m_ab.Length; i++)
				m_ab[i] = ab[i + 6];
		}

		public Pcm(int hz, byte[] ab) {
			m_cBits = 8;
			m_hz = hz;
			m_ab = ab;
		}

		public Pcm(int hz, short[] ash) {
			m_cBits = 16;
			m_hz = hz;
			m_ash = ash;
		}

		public int BitsPerSample {
			get {
        		return m_cBits;
			}
		}

		public int Hertz {
			get {
				return m_hz;
			}
		}

		public byte[] Data8Bit {
			get {
				return (byte[])m_ab.Clone();
			}
			set {
				m_cBits = 8;
				m_ash = null;
				m_ab = (byte[])value.Clone();
			}
		}

		public short[] Data16Bit {
			get {
				return (short[])m_ash.Clone();
			}
			set {
				m_cBits = 16;
				m_ab = null;
				m_ash = (short[])value.Clone();
			}
		}

		public void ConvertTo16Bit() {
			if (m_cBits == 16)
				return;

			// 8 bit unsigned (0,127==-128,-1 and 128,255==0,127) to
			// 16 bit signed (-32768 -> 32767)
			short[] ash = new short[m_ab.Length];
			for (int i = 0; i < ash.Length; i++) {
				ash[i] = (short)((short)m_ab[i] * 256);
				ash[i] = (short)(ash[i] ^ 0x8000);
			}
			Data16Bit = ash;
		}

		public void ConvertTo8Bit() {
			if (m_cBits == 8)
				return;

			// 16 bit signed (-32768 -> 32767) to
			// 8 bit unsigned (0,127==-128,-1 and 128,255==0,127)

			byte[] ab = new byte[m_ash.Length];
			for (int i = 0; i < ab.Length; i++)
				ab[i] = (byte)((m_ash[i] ^ 0x8000) / 256);
			Data8Bit = ab;
		}

		public void SetMaxSize(int cbMax) {
			if (m_cBits == 8) {
				if (m_ab.Length > cbMax) {
					byte[] abT = new byte[32700];
					Array.Copy(m_ab, 0, abT, 0, abT.Length);
					m_ab = abT;
				}
			} else {
				if (m_ash.Length * 2 > cbMax) {
					short[] ashT = new short[cbMax / 2];
					Array.Copy(m_ash, 0, ashT, 0, ashT.Length);
					m_ash = ashT;
				}
			}
		}

		[DllImportAttribute("winmm.dll")]
		private static extern int PlaySoundW(IntPtr pbSound, IntPtr hModule, uint fdwSound);
		const UInt32 SND_SYNC = 0x0000; // play synchronously (default)
		const UInt32 SND_ASYNC = 0x0001; // play asynchronously
		const UInt32 SND_NODEFAULT = 0x0002;  // silence (!default) if sound not found
		const UInt32 SND_MEMORY = 0x0004; // pszSound points to a memory file
		const UInt32 SND_LOOP = 0x0008; // loop the sound until next sndPlaySound
		const UInt32 SND_NOSTOP = 0x0010;  // don't stop any currently playing sound
		const UInt32 SND_NOWAIT = 0x00002000; // don't wait if the driver is busy
		const UInt32 SND_ALIAS = 0x00010000; // name is a registry alias
		const UInt32 SND_ALIAS_ID = 0x00110000; // alias is a predefined ID
		const UInt32 SND_FILENAME = 0x00020000; // name is file name
		const UInt32 SND_RESOURCE = 0x00040004; // name is resource name or atom
		const UInt32 SND_PURGE = 0x0040;
		const UInt32 SND_APPLICATION = 0x0080; // look for application specific association	

		unsafe public void Play() {
			// Fill in the header

			if (m_ab == null && m_ash == null)
				return;

			int cbData = m_cBits == 8 ? m_ab.Length : m_ash.Length * 2;

			WavFileHeader wavhdr;
			wavhdr.fccRIFF = new FourCC("RIFF");
			wavhdr.cbRiffBytes = (uint)(Marshal.SizeOf(typeof(WavFileHeader)) - 8 + cbData);
			wavhdr.fccWAVE = new FourCC("WAVE");
			wavhdr.fccfmt = new FourCC("fmt ");
			wavhdr.cbfmtBytes = 16;
			wavhdr.wFormatTag = 1;
			wavhdr.nChannels = 1;
			wavhdr.nSamplesPerSec = (uint)m_hz;
			wavhdr.nAvgBytesPerSec = (uint)(m_cBits / 8 * m_hz);
			wavhdr.nBlockAlign = (ushort)(m_cBits / 8);
			wavhdr.nBitsPerSample = (ushort)m_cBits;
			wavhdr.fccdata = new FourCC("data");
			wavhdr.cbdataBytes = (uint)cbData;

			// Serialize the data

			byte[] ab = SerializeStructure(null, &wavhdr, Marshal.SizeOf(wavhdr.GetType()));
			switch (m_cBits) {
			case 8:
				ab = SerializeStructure(ab, (void *)Marshal.UnsafeAddrOfPinnedArrayElement(m_ab, 0), m_ab.Length);
				break;

			case 16:
				ab = SerializeStructure(ab, (void *)Marshal.UnsafeAddrOfPinnedArrayElement(m_ash, 0), m_ash.Length * 2);
				break;
			}

			// Play the sound

			IntPtr ptr = Marshal.UnsafeAddrOfPinnedArrayElement(ab, 0);
			int nSuccess = PlaySoundW(ptr, IntPtr.Zero, SND_ASYNC | SND_MEMORY);
			Debug.Assert(nSuccess != 0);
		}

		unsafe static byte[] SerializeStructure(byte[] ab, void *pv, int cb) {
			byte *pb = (byte *)pv;
			byte[] abNew = null;
			if (ab == null) {
				abNew = new byte[cb];
				for (int i = 0; i < cb; i++)
					abNew[i] = pb[i];
			} else {
				abNew = new byte[ab.Length + cb];
				for (int i = 0; i < ab.Length; i++)
					abNew[i] = ab[i];
				for (int i = 0; i < cb; i++)
					abNew[i + ab.Length] = pb[i];
			}

			return abNew;
		}

		unsafe public static void PlayWavFile(string strWav) {
			// Get the bytes

			BinaryReader brdr = new BinaryReader(new FileStream(strWav, FileMode.Open));
			byte[] ab = brdr.ReadBytes((int)brdr.BaseStream.Length);
			brdr.Close();

			// Play the wav

			IntPtr ptr = Marshal.UnsafeAddrOfPinnedArrayElement(ab, 0);
			int nSuccess = PlaySoundW(ptr, IntPtr.Zero, SND_ASYNC | SND_MEMORY);
			Debug.Assert(nSuccess != 0);
		}

		public byte[] GetSndEncoding() {
			ConvertTo8Bit();
			SetMaxSize(32700 * 2);
			return AudioFormats.EncodeImaAdpcmTableDriven8Bit(Data8Bit);
		}

#if false
		static byte[] MassageSample(byte[] abPcm) {
			// Need to make sure that the encoded size is a multiple of 2 bytes in size, which means the
			// raw size needs to be a multiple of 4 in size, and need to trail the amplitude to 128 to
			// reduce click sounds.

			byte[] abPcmNew = new byte[(abPcm.Length + 7) & ~3];

			// Copy in the existing pcm

			Array.Copy(abPcm, 0, abPcmNew, 0, abPcm.Length);

			// Perform fade

			int cSamplesFade = abPcmNew.Length - abPcm.Length;
			int nSampleLast = (int)abPcm[abPcm.Length - 1];
			int nStep = -((nSampleLast - 128) / cSamplesFade);
			for (int iSample = abPcm.Length; iSample < abPcmNew.Length; iSample++) {
				if (iSample == abPcmNew.Length - 1) {
					nSampleLast = 0;
				} else {
					nSampleLast += nStep;
				}
				abPcmNew[iSample] = (byte)nSampleLast;
			}

			return abPcmNew;
		}
#endif
	}
}
