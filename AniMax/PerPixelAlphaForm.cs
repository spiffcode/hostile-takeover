//
// Copyright © 2002 Rui Godinho Lopes <rui@ruilopes.com>
// All rights reserved.
//
// This source file(s) may be redistributed unmodified by any means
// PROVIDING they are not sold for profit without the authors expressed
// written consent, and providing that this notice and the authors name
// and all copyright notices remain intact.
//
// Any use of the software in source or binary forms, with or without
// modification, must include, in the user documentation ("About" box and
// printed documentation) and internal comments to the code, notices to
// the end user as follows:
//
// "Portions Copyright © 2002 Rui Godinho Lopes"
//
// An email letting me know that you are using it would be nice as well.
// That's not much to ask considering the amount of work that went into
// this.
//
// THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED. USE IT AT YOUT OWN RISK. THE AUTHOR ACCEPTS NO
// LIABILITY FOR ANY DATA DAMAGE/LOSS THAT THIS PRODUCT MAY CAUSE.
//

using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace SpiffCode {

	// a static class to expose needed win32 gdi functions.
	class Win32 {

		public enum Bool {
			False= 0,
			True
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct Point {
			public Int32 x;
			public Int32 y;

			public Point(Int32 x, Int32 y) { this.x= x; this.y= y; }
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Size {
			public Int32 cx;
			public Int32 cy;

			public Size(Int32 cx, Int32 cy) { this.cx= cx; this.cy= cy; }
		}

		[StructLayout(LayoutKind.Sequential, Pack=1)]
		struct ARGB {
			public byte Blue;
			public byte Green;
			public byte Red;
			public byte Alpha;
		}

		[StructLayout(LayoutKind.Sequential, Pack=1)]
		public struct BLENDFUNCTION {
			public byte BlendOp;
			public byte BlendFlags;
			public byte SourceConstantAlpha;
			public byte AlphaFormat;
		}

		public const Int32 ULW_COLORKEY = 0x00000001;
		public const Int32 ULW_ALPHA    = 0x00000002;
		public const Int32 ULW_OPAQUE   = 0x00000004;

		public const byte AC_SRC_OVER  = 0x00;
		public const byte AC_SRC_ALPHA = 0x01;


		[DllImport("user32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool UpdateLayeredWindow(IntPtr hwnd, IntPtr hdcDst, ref Point pptDst, ref Size psize, IntPtr hdcSrc, ref Point pprSrc, Int32 crKey, ref BLENDFUNCTION pblend, Int32 dwFlags);

		[DllImport("user32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern IntPtr GetDC(IntPtr hWnd);

		[DllImport("user32.dll", ExactSpelling=true)]
		public static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern IntPtr CreateCompatibleDC(IntPtr hDC);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool DeleteDC(IntPtr hdc);

		[DllImport("gdi32.dll", ExactSpelling=true)]
		public static extern IntPtr SelectObject(IntPtr hDC, IntPtr hObject);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool DeleteObject(IntPtr hObject);

		[DllImport("user32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool PrintWindow(IntPtr hwnd, IntPtr hdcBlt, UInt32 fl);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool MoveToEx(IntPtr hdc, Int32 x, Int32 y, IntPtr lpPoint);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool LineTo(IntPtr hdc, Int32 x, Int32 y);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Bool GdiFlush();

		[DllImport("user32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Int32 SetWindowLong(IntPtr hwnd, Int32 index, Int32 value);
	}

	/// <para>PerPixel forms should derive from this base class</para>
	/// <author><name>Rui Godinho Lopes</name><email>rui@ruilopes.com</email></author>
	public class PerPixelAlphaForm : Form {

		/// <para>Changes the current bitmap.</para>
		public void SetBitmap(Bitmap bitmap) {
			SetBitmap(bitmap, 255);
		}

#if false
		/// <para>Changes the current bitmap with a custom opacity level.  Here is where all happens!</para>
		public void SetBitmap(Bitmap bitmap, byte opacity) {
			if (bitmap.PixelFormat != PixelFormat.Format32bppArgb)
				throw new ApplicationException("The bitmap must be 32bpp with alpha-channel.");

			Bitmap bmWindow = new Bitmap(Width, Height, PixelFormat.Format32bppArgb);	// NEW:
	//		Bitmap bmWindow = bitmap;
			Graphics g = Graphics.FromImage(bmWindow);
			g.Clear(Color.FromArgb(0, 0, 0, 0));
			g.DrawImage(bitmap, 0, 0, bmWindow.Width, bmWindow.Height);
			g.Flush();

			IntPtr memDc = g.GetHdc();

	//		Win32.Bool fSuccess = Win32.PrintWindow(Handle, memDc, 0); // NEW:

			int err = Marshal.GetLastWin32Error();
			if (err != 1400) {
				Win32.Size size = new Win32.Size(bmWindow.Width, bmWindow.Height);
				Win32.Point pointSource = new Win32.Point(0, 0);
				Win32.Point topPos = new Win32.Point(Left, Top);
				Win32.BLENDFUNCTION blend = new Win32.BLENDFUNCTION();
				blend.BlendOp             = Win32.AC_SRC_OVER;
				blend.BlendFlags          = 0;
				blend.SourceConstantAlpha = opacity;
				blend.AlphaFormat         = Win32.AC_SRC_ALPHA;
				Win32.GdiFlush();
	//			BackgroundImage = bmWindow;
				IntPtr screenDc = Win32.GetDC(IntPtr.Zero);
				Win32.UpdateLayeredWindow(Handle, screenDc, ref topPos, ref size, memDc, ref pointSource, 0, ref blend, Win32.ULW_ALPHA);
				Win32.ReleaseDC(IntPtr.Zero, screenDc);
			}
			g.ReleaseHdc(memDc);
			g.Dispose();
		}
#else
		public void SetBitmap(Bitmap bitmap, byte opacity) {
			if (bitmap.PixelFormat != PixelFormat.Format32bppArgb)
				throw new ApplicationException("The bitmap must be 32bpp with alpha-channel.");
			IntPtr screenDc = Win32.GetDC(IntPtr.Zero);
			IntPtr memDc = Win32.CreateCompatibleDC(screenDc);
			IntPtr hBitmap = IntPtr.Zero;
			IntPtr oldBitmap = IntPtr.Zero;

			try {
				hBitmap = bitmap.GetHbitmap(Color.FromArgb(0));  // grab a GDI handle from this GDI+ bitmap
				oldBitmap = Win32.SelectObject(memDc, hBitmap);

				Win32.Size size = new Win32.Size(bitmap.Width, bitmap.Height);
				Win32.Point pointSource = new Win32.Point(0, 0);
				Win32.Point topPos = new Win32.Point(Left, Top);
				Win32.BLENDFUNCTION blend = new Win32.BLENDFUNCTION();
				blend.BlendOp             = Win32.AC_SRC_OVER;
				blend.BlendFlags          = 0;
				blend.SourceConstantAlpha = opacity;
				blend.AlphaFormat         = Win32.AC_SRC_ALPHA;

				Win32.UpdateLayeredWindow(Handle, screenDc, ref topPos, ref size, memDc, ref pointSource, 0, ref blend, Win32.ULW_ALPHA);
			}
			finally {
				Win32.ReleaseDC(IntPtr.Zero, screenDc);
				if (hBitmap != IntPtr.Zero) {
					Win32.SelectObject(memDc, oldBitmap);
					//Windows.DeleteObject(hBitmap); // The documentation says that we have to use the Windows.DeleteObject... but since there is no such method I use the normal DeleteObject from Win32 GDI and it's working fine without any resource leak.
					Win32.DeleteObject(hBitmap);
				}
				Win32.DeleteDC(memDc);
			}
		}
#endif

		protected override CreateParams CreateParams	{
			get {
				CreateParams cp = base.CreateParams;
				cp.ExStyle |= 0x00080000; // This form has to have the WS_EX_LAYERED extended style
				return cp;
			}
		}

		public Bitmap SnapshotWindow() {
			Bitmap bm = new Bitmap(Width, Height, PixelFormat.Format32bppArgb);
#if false
			IntPtr screenDc = Win32.GetDC(IntPtr.Zero);
			IntPtr memDc = Win32.CreateCompatibleDC(screenDc);
			IntPtr hBitmap = IntPtr.Zero;
			IntPtr oldBitmap = IntPtr.Zero;
			hBitmap = bm.GetHbitmap();  // grab a GDI handle from this GDI+ bitmap
			oldBitmap = Win32.SelectObject(memDc, hBitmap);
			Win32.PrintWindow(Handle, memDc, 0);
			Win32.ReleaseDC(IntPtr.Zero, screenDc);
			Win32.SelectObject(memDc, oldBitmap);
	//		Win32.DeleteObject(hBitmap);
			Win32.DeleteDC(memDc);
			bm = Bitmap.FromHbitmap(hBitmap);
#else
			Graphics g = Graphics.FromImage(bm);
			IntPtr hdcMem = g.GetHdc();
			Win32.PrintWindow(Handle, hdcMem, 0);
			g.ReleaseHdc(hdcMem);
			g.Dispose();
#endif
			return bm;
		}

		protected override void OnPaint(PaintEventArgs e) {
	//		base.OnPaint(e);
		}

		protected override void OnPaintBackground(PaintEventArgs e) {
	//		base.OnPaintBackground(e);
		}
	}
}
