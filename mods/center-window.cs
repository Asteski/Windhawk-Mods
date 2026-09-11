using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace CenterWin
{
    internal class Program : ApplicationContext
    {
        private const int MOD_WIN = 0x0008;
        private const int WM_HOTKEY = 0x0312;

        private NotifyIcon trayIcon;

        [DllImport("user32.dll")]
        private static extern bool RegisterHotKey(IntPtr hWnd, int id, int fsModifiers, Keys vk);

        [DllImport("user32.dll")]
        private static extern bool UnregisterHotKey(IntPtr hWnd, int id);

        [DllImport("user32.dll")]
        private static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int X, int Y, int cx, int cy, uint uFlags);

        [StructLayout(LayoutKind.Sequential)]
        private struct RECT { public int Left, Top, Right, Bottom; }

        public Program()
        {
            // Initialize system tray icon
            trayIcon = new NotifyIcon()
            {
                Icon = new System.Drawing.Icon("icon.ico"), // tray icon, must be in output directory before debugging
                Text = "CenterWin",
                Visible = true,
                ContextMenuStrip = new ContextMenuStrip()
            };
            trayIcon.ContextMenuStrip.Items.Add("Exit", null, (s, e) => Application.Exit());

            // Register Win+C hotkey
            RegisterHotKey(IntPtr.Zero, 1, MOD_WIN, Keys.J);

            // Add message filter to handle hotkey
            Application.AddMessageFilter(new HotkeyMessageFilter(CenterActiveWindow));
        }

        private void CenterActiveWindow()
        {
            IntPtr hWnd = GetForegroundWindow();
            if (hWnd == IntPtr.Zero) return;

            GetWindowRect(hWnd, out RECT r);
            int width = r.Right - r.Left;
            int height = r.Bottom - r.Top;

            var screen = Screen.FromHandle(hWnd).Bounds;
            int newX = (screen.Width - width) / 2;
            int newY = (screen.Height - height) / 2;

            SetWindowPos(hWnd, IntPtr.Zero, newX, newY, width, height, 0);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing) trayIcon.Dispose();
            UnregisterHotKey(IntPtr.Zero, 1);
            base.Dispose(disposing);
        }

        [STAThread]
        static void Main()
        {
            Application.Run(new Program());
        }
    }

    internal class HotkeyMessageFilter : IMessageFilter
    {
        private const int WM_HOTKEY = 0x0312;
        private readonly Action onHotkey;

        public HotkeyMessageFilter(Action onHotkey) => this.onHotkey = onHotkey;

        public bool PreFilterMessage(ref Message m)
        {
            if (m.Msg == WM_HOTKEY)
            {
                onHotkey();
                return true;
            }
            return false;
        }
    }
}
