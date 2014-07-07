/*
 * Author: Marcus Lorentzon, 2001
 *         d98malor@dtek.chalmers.se
 * 
 * Freeware: Do not remove this header
 * 
 * File: SerialStream.cs
 * 
 * Description: Implements a Stream for asynchronous
 *              transfers and COMM.
 *
 * Version: 2.0 beta
 * 
 */

namespace LoMaN.IO {

	using System;
	using System.IO;
	using System.Threading;
	using System.Runtime.InteropServices;

	public class SerialStream : Stream {

		public class SerialAsyncResult : IAsyncResult {
			public SerialAsyncResult(object asyncObject) {
				m_AsyncObject = asyncObject;
				m_WaitHandle = new ManualResetEvent(false);
			}

			internal void Init(object stateObject, AsyncCallback callback, bool bIsRead) {
				m_StateObject = stateObject;
				m_Callback = callback;
				m_bIsRead = bIsRead;
				m_bCompleted = false;
				m_WaitHandle.Reset();
			}

			internal void Reset() {
				m_StateObject = null;
				m_Callback = null;
				m_bCompleted = true;
				m_WaitHandle.Reset();
			}

			internal bool m_bIsRead;
			internal bool m_bCompleted = true;

			public bool IsCompleted { get { return m_bCompleted; } }
			public bool CompletedSynchronously { get { return false; } }

			private object m_AsyncObject;
			public object AsyncObject { get { return m_AsyncObject; } }

			private object m_StateObject;
			public object AsyncState { get { return m_StateObject; } }

			private ManualResetEvent m_WaitHandle;
			public WaitHandle AsyncWaitHandle { get { return m_WaitHandle; } }

			private AsyncCallback m_Callback;
			public AsyncCallback Callback { get { return m_Callback; } }
		}
		
		private unsafe void AsyncFSCallback(uint errorCode, uint numBytes, NativeOverlapped* pOverlapped) {
			SerialAsyncResult sar = (SerialAsyncResult)Overlapped.Unpack(pOverlapped).AsyncResult;

			if (sar.m_bIsRead)
				m_iReadCount = (int)numBytes;
			else
				m_iWriteCount = (int)numBytes;
			((ManualResetEvent)sar.AsyncWaitHandle).Set();

			if (errorCode == ERROR_OPERATION_ABORTED)
				sar.m_bCompleted = false;
			else
				sar.m_bCompleted = true;

			if (sar.Callback != null)
				sar.Callback.Invoke(sar);
		}

		private IOCompletionCallback m_IOCompletionCallback;
		private int m_hFile = 0;

		private string m_sPort;
		public string Port {
			get {
				return m_sPort;
			}
			set {
				if (m_sPort != value) {
					Close();
					Open(value);
				}
			}
		}

		private const uint GENERIC_READ = 0x80000000;
		private const uint GENERIC_WRITE = 0x40000000;
		private const uint ERROR_OPERATION_ABORTED = 995;
		private const uint ERROR_IO_PENDING = 997;

		public void Open(string port) {
			m_hFile = CreateFile(port, (uint)((m_bRead?GENERIC_READ:0)|(m_bWrite?GENERIC_WRITE:0)), 0, 0, 3, 0x40000000, 0);
			if (m_hFile <= 0) {
				m_hFile = 0;
				throw new FileNotFoundException("Unable to open " + port);
			}
			m_sPort = port;

			ThreadPool.BindHandle(new IntPtr(m_hFile));
		}

		public SerialStream(string port, FileAccess access) {
			m_bRead  = ((int)access & (int)FileAccess.Read) != 0;
			m_bWrite = ((int)access & (int)FileAccess.Write) != 0;
			Open(port);
			unsafe {
				m_IOCompletionCallback = new IOCompletionCallback(AsyncFSCallback);
			}
		}

		private bool m_bRead;
		public override bool CanRead { get { return m_bRead; } }

		private bool m_bWrite;
		public override bool CanWrite { get { return m_bWrite; } }

		public override bool CanSeek { get { return false; } }

		public bool Closed  { get { return m_hFile <= 0; } }

		public override long Length { get { return 0; } }

		public override void SetLength(long nLength) { }

		public override void Close() {
			CloseHandle(m_hFile);
			m_hFile = 0;
			m_sPort = null;
		}

		private int m_iReadCount;
		public override IAsyncResult BeginRead(byte[] buffer, int offset, int count, AsyncCallback callback, object state) {
			SerialAsyncResult ar = new SerialAsyncResult(this);
			ar.Init(state, callback, true);
			Overlapped ov = new Overlapped(0, 0, ar.AsyncWaitHandle.Handle.ToInt32(), ar);
			unsafe { fixed (byte* data = &buffer[0]) {
				int read = 0;
				NativeOverlapped* nov = ov.Pack(m_IOCompletionCallback);
				ReadFile(m_hFile, data, count, &read, nov);
			} }

			if (GetLastError() == ERROR_IO_PENDING)
				return ar;
			else
				throw new Exception("Unable to initialize read. Errorcode: " + GetLastError().ToString());
		}

		private int m_iWriteCount;
		public override IAsyncResult BeginWrite(byte[] buffer, int offset, int count, AsyncCallback callback, object state) {
			SerialAsyncResult ar = new SerialAsyncResult(this);
			ar.Init(state, callback, false);
			Overlapped ov = new Overlapped(0, 0, ar.AsyncWaitHandle.Handle.ToInt32(), ar);
			unsafe { fixed (byte* data = &buffer[0]) {
				int write = 0;
				NativeOverlapped* nov = ov.Pack(m_IOCompletionCallback);
				WriteFile(m_hFile, data, count, &write, nov);
			} }
			if (GetLastError() == ERROR_IO_PENDING)
				return ar;
			else
				throw new Exception("Unable to initialize write. Errorcode: " + GetLastError().ToString());
		}
		
		public override int EndRead(IAsyncResult asyncResult) {
			SerialAsyncResult sar = (SerialAsyncResult)asyncResult;
			if (!sar.m_bIsRead)
				throw new Exception("Invalid parameter: IAsyncResult is not from a read");
			sar.AsyncWaitHandle.WaitOne();
			if (!sar.m_bCompleted) {
				((ManualResetEvent)sar.AsyncWaitHandle).Reset();
				sar.AsyncWaitHandle.WaitOne();
			}
			sar.Reset();

			return m_iReadCount;
		}
		
		public override void EndWrite(IAsyncResult asyncResult) {
			SerialAsyncResult sar = (SerialAsyncResult)asyncResult;
			if (sar.m_bIsRead)
				throw new Exception("Invalid parameter: IAsyncResult is from a read");
			sar.AsyncWaitHandle.WaitOne();
			if (!sar.m_bCompleted) {
				((ManualResetEvent)sar.AsyncWaitHandle).Reset();
				sar.AsyncWaitHandle.WaitOne();
			}
			sar.Reset();
		}

		public override int Read(byte[] buffer, int offset, int count) {
			return EndRead(BeginRead(buffer, offset, count, null, null));
		}

		public override void Write(byte[] buffer, int offset, int count) {
			EndWrite(BeginWrite(buffer, offset, count, null, null));
		}

		public override void Flush() { FlushFileBuffers(m_hFile); }

		private const uint PURGE_TXABORT = 0x0001;  // Kill the pending/current writes to the comm port.
		private const uint PURGE_RXABORT = 0x0002;  // Kill the pending/current reads to the comm port.
		private const uint PURGE_TXCLEAR = 0x0004;  // Kill the transmit queue if there.
		private const uint PURGE_RXCLEAR = 0x0008;  // Kill the typeahead buffer if there.

		public bool PurgeRead() { return(PurgeComm(m_hFile, PURGE_RXCLEAR)); }
		public bool PurgeWrite() { return(PurgeComm(m_hFile, PURGE_TXCLEAR)); }
		public bool CancelRead() { return(PurgeComm(m_hFile, PURGE_RXABORT)); }
		public bool CancelWrite() { return(PurgeComm(m_hFile, PURGE_TXABORT)); }

		public override long Seek(long offset, SeekOrigin origin) { return 0; }

		public override long Position { get { return 0; } set { } }

		public void SetTimeouts(int ReadIntervalTimeout, int ReadTotalTimeoutMultiplier, int ReadTotalTimeoutConstant, 
								int WriteTotalTimeoutMultiplier, int WriteTotalTimeoutConstant) {
			SerialTimeouts Timeouts = new SerialTimeouts(ReadIntervalTimeout, ReadTotalTimeoutMultiplier, ReadTotalTimeoutConstant, 
										 WriteTotalTimeoutMultiplier, WriteTotalTimeoutConstant);
			unsafe { SetCommTimeouts(m_hFile, &Timeouts); }
		}

		public enum Parity {None, Odd, Even, Mark, Space};
		public enum StopBits {One, OneAndHalf, Two};
		public enum FlowControl {None, XOnXOff, Hardware};

		[StructLayout(LayoutKind.Sequential)]
		public struct DCB {
			public int DCBlength;
			public uint BaudRate;
			public uint Flags;
			public uint fBinary { get { return Flags&0x0001; } 
								  set { Flags = Flags & ~1U | value; } }
			public uint fParity { get { return (Flags>>1)&1; }
								  set { Flags = Flags & ~(1U << 1) | (value << 1); } }
			public uint fOutxCtsFlow { get { return (Flags>>2)&1; }
								  set { Flags = Flags & ~(1U << 2) | (value << 2); } }
			public uint fOutxDsrFlow { get { return (Flags>>3)&1; }
								  set { Flags = Flags & ~(1U << 3) | (value << 3); } }
			public uint fDtrControl { get { return (Flags>>4)&3; }
								  set { Flags = Flags & ~(3U << 4) | (value << 4); } }
			public uint fDsrSensitivity { get { return (Flags>>6)&1; }
								  set { Flags = Flags & ~(1U << 6) | (value << 6); } }
			public uint fTXContinueOnXoff { get { return (Flags>>7)&1; }
								  set { Flags = Flags & ~(1U << 7) | (value << 7); } }
			public uint fOutX { get { return (Flags>>8)&1; }
								  set { Flags = Flags & ~(1U << 8) | (value << 8); } }
			public uint fInX { get { return (Flags>>9)&1; }
								  set { Flags = Flags & ~(1U << 9) | (value << 9); } }
			public uint fErrorChar { get { return (Flags>>10)&1; }
								  set { Flags = Flags & ~(1U << 10) | (value << 10); } }
			public uint fNull { get { return (Flags>>11)&1; }
								  set { Flags = Flags & ~(1U << 11) | (value << 11); } }
			public uint fRtsControl { get { return (Flags>>12)&3; }
								  set { Flags = Flags & ~(3U << 12) | (value << 12); } }
			public uint fAbortOnError { get { return (Flags>>13)&1; }
								  set { Flags = Flags & ~(1U << 13) | (value << 13); } }
			public ushort wReserved;
			public ushort XonLim;
			public ushort XoffLim;
			public byte ByteSize;
			public byte Parity;
			public byte StopBits;
			public sbyte XonChar;
			public sbyte XoffChar;
			public sbyte ErrorChar;
			public sbyte EofChar;
			public sbyte EvtChar;
			public ushort wReserved1;

			public override string ToString() {
				return "DCBlength: " + DCBlength + "\r\n" +
					"BaudRate: " + BaudRate + "\r\n" +
					"fBinary: " + fBinary + "\r\n" +
					"fParity: " + fParity + "\r\n" +
					"fOutxCtsFlow: " + fOutxCtsFlow + "\r\n" +
					"fOutxDsrFlow: " + fOutxDsrFlow + "\r\n" +
					"fDtrControl: " + fDtrControl + "\r\n" +
					"fDsrSensitivity: " + fDsrSensitivity + "\r\n" +
					"fTXContinueOnXoff: " + fTXContinueOnXoff + "\r\n" +
					"fOutX: " + fOutX + "\r\n" +
					"fInX: " + fInX + "\r\n" +
					"fErrorChar: " + fErrorChar + "\r\n" +
					"fNull: " + fNull + "\r\n" +
					"fRtsControl: " + fRtsControl + "\r\n" +
					"fAbortOnError: " + fAbortOnError + "\r\n" +
					"XonLim: " + XonLim + "\r\n" +
					"XoffLim: " + XoffLim + "\r\n" +
					"ByteSize: " + ByteSize + "\r\n" +
					"Parity: " + Parity + "\r\n" +
					"StopBits: " + StopBits + "\r\n" +
					"XonChar: " + XonChar + "\r\n" +
					"XoffChar: " + XoffChar + "\r\n" +
					"EofChar: " + EofChar + "\r\n" +
					"EvtChar: " + EvtChar + "\r\n";
			}
		}

		public bool SetPortSettings(uint baudrate, byte databits, StopBits stopbits, Parity parity, FlowControl flowcontrol) {
			unsafe {
				DCB dcb = new DCB();
				dcb.DCBlength = sizeof(DCB);
				dcb.BaudRate = baudrate;
				dcb.ByteSize = databits;
				dcb.StopBits = (byte)stopbits;
				dcb.Parity = (byte)parity;
				dcb.fParity = (parity > 0)? 1U : 0U;
				dcb.fBinary = dcb.fDtrControl = dcb.fTXContinueOnXoff = 1;
				dcb.fOutxCtsFlow = dcb.fAbortOnError = (flowcontrol == FlowControl.Hardware)? 1U : 0U;
				dcb.fOutX = dcb.fInX = (flowcontrol == FlowControl.XOnXOff)? 1U : 0U;
				dcb.fRtsControl = (flowcontrol == FlowControl.Hardware)? 2U : 1U;
				dcb.XonLim = 2048;
				dcb.XoffLim = 512;
				dcb.XonChar = 0x11; // Ctrl-Q
				dcb.XoffChar = 0x13; // Ctrl-S
				return SetCommState(m_hFile, &dcb);
			}
		}

		public bool GetPortSettings(out DCB dcb) {
			unsafe {
				DCB dcb2 = new DCB();
				dcb2.DCBlength = sizeof(DCB);
				bool ret = GetCommState(m_hFile, &dcb2);
				dcb = dcb2;
				return ret;
			}
		}

		[DllImport("kernel32.dll", EntryPoint="CreateFileW",  SetLastError=true,
			CharSet=CharSet.Unicode, ExactSpelling=true)]
		static extern int CreateFile(string filename, uint access, uint sharemode, uint security_attributes, uint creation, uint flags, uint template);

		[DllImport("kernel32.dll", SetLastError=true)]
		static extern bool CloseHandle(int handle);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool ReadFile(int hFile, byte* lpBuffer, int nNumberOfBytesToRead, int* lpNumberOfBytesRead, NativeOverlapped* lpOverlapped);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool WriteFile(int hFile, byte* lpBuffer, int nNumberOfBytesToWrite, int* lpNumberOfBytesWritten, NativeOverlapped* lpOverlapped);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool SetCommTimeouts(int hFile, SerialTimeouts* lpCommTimeouts);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool SetCommState(int hFile, DCB* lpDCB);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool GetCommState(int hFile, DCB* lpDCB);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool BuildCommDCB(string def, DCB* lpDCB);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern int GetLastError();

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool FlushFileBuffers(int hFile);

		[DllImport("kernel32.dll", SetLastError=true)]
		static unsafe extern bool PurgeComm(int hFile, uint dwFlags);
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct SerialTimeouts {
		public int ReadIntervalTimeout;
		public int ReadTotalTimeoutMultiplier;
		public int ReadTotalTimeoutConstant;
		public int WriteTotalTimeoutMultiplier;
		public int WriteTotalTimeoutConstant;

		public SerialTimeouts(int r1, int r2, int r3, int w1, int w2) {
			ReadIntervalTimeout = r1;
			ReadTotalTimeoutMultiplier = r2;
			ReadTotalTimeoutConstant = r3;
			WriteTotalTimeoutMultiplier = w1;
			WriteTotalTimeoutConstant = w2;
		}

		public override string ToString() {
			return "ReadIntervalTimeout: " + ReadIntervalTimeout + "\r\n" +
				   "ReadTotalTimeoutMultiplier: " + ReadTotalTimeoutMultiplier + "\r\n" +
				   "ReadTotalTimeoutConstant: " + ReadTotalTimeoutConstant + "\r\n" +
				   "WriteTotalTimeoutMultiplier: " + WriteTotalTimeoutMultiplier + "\r\n" +
				   "WriteTotalTimeoutConstant: " + WriteTotalTimeoutConstant + "\r\n";
		}
	}

	public class SerialSettings {
		private bool m_bDirty = false;
		public bool Dirty { get { return m_bDirty; } }

		// Timeouts
		private SerialTimeouts Timeouts = new SerialTimeouts(1, 0, 0, 0, 0);

		public int ReadIntervalTimeout {
			get { return Timeouts.ReadIntervalTimeout; }
			set { if (Timeouts.ReadIntervalTimeout != value) {
					  Timeouts.ReadIntervalTimeout = value;
					  m_bDirty = true;
				  }
			}
		}

		public int ReadTotalTimeoutMultiplier {
			get { return Timeouts.ReadTotalTimeoutMultiplier; }
			set { if (Timeouts.ReadTotalTimeoutMultiplier != value) {
					Timeouts.ReadTotalTimeoutMultiplier = value;
					m_bDirty = true;
				  }
			}
		}

		public int ReadTotalTimeoutConstant {
			get { return Timeouts.ReadTotalTimeoutConstant; }
			set { if (Timeouts.ReadTotalTimeoutConstant != value) {
					  Timeouts.ReadTotalTimeoutConstant = value;
					  m_bDirty = true;
				  }
			}
		}

		public int WriteTotalTimeoutMultiplier {
			get { return Timeouts.WriteTotalTimeoutMultiplier; }
			set { if (Timeouts.WriteTotalTimeoutMultiplier != value) {
					  Timeouts.WriteTotalTimeoutMultiplier = value;
					  m_bDirty = true;
				  }
			}
		}

		public int WriteTotalTimeoutConstant {
			get { return Timeouts.WriteTotalTimeoutConstant; }
			set { if (Timeouts.WriteTotalTimeoutConstant != value) {
					  Timeouts.WriteTotalTimeoutConstant = value;
					  m_bDirty = true;
				  }
			}
		}

		// Valid stuff
		private static readonly object[] s_iValidBitRates = new object[] {75u, 110u, 134u, 150u, 300u, 600u, 1200u, 1800u, 2400u, 4800u, 
				7200u, 9600u, 14400u, 19200u, 38400u, 57600u, 115200u, 128000u};
		public static object[] ValidBitRates {
			get { return s_iValidBitRates; }
		}

		private static readonly object[] s_iValidDataBits = new object[] {(byte)5, (byte)6, (byte)7, (byte)8};
		public static object[] ValidDataBits {
			get { return s_iValidDataBits; }
		}

		// Port settings
		private uint m_iBitRate = 57600;
		public uint BitRate {
			get { return m_iBitRate; }
			set { if (m_iBitRate != value) {
					  m_iBitRate = value;
					  m_bDirty = true;
				  }
			}
		}

		private byte m_iDataBits = 8;
		public byte DataBits {
			get { return m_iDataBits; }
			set { if (m_iDataBits != value) {
					  m_iDataBits = value;
					  m_bDirty = true;
				  }
			}
		}

		private SerialStream.Parity m_Parity = SerialStream.Parity.None;
		public SerialStream.Parity Parity {
			get { return m_Parity; }
			set { if (m_Parity != value) {
					  m_Parity = value;
					  m_bDirty = true;
				  }
			}
		}

		private SerialStream.StopBits m_StopBits = SerialStream.StopBits.One;
		public SerialStream.StopBits StopBits {
			get { return m_StopBits; }
			set { if (m_StopBits != value) {
					  m_StopBits = value;
					  m_bDirty = true;
				  }
			}
		}

		private SerialStream.FlowControl m_FlowControl = SerialStream.FlowControl.None;
		public SerialStream.FlowControl FlowControl {
			get { return m_FlowControl; }
			set { if (m_FlowControl != value) {
					  m_FlowControl = value;
					  m_bDirty = true;
				  }
			}
		}
	}

}
