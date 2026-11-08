// ==WindhawkMod==
// @id              ultimate-modern-context-menu
// @name            Ultimate Modern Context Menu
// @description     Configurable buttons and submenus in Explorer's native Windows 11 context menu.
// @version         0.1.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lshell32 -luser32 -lgdi32 -lgdiplus -lshlwapi -lole32 -loleaut32 -lruntimeobject -lshcore -luuid
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*

# Ultimate Modern Context Menu

Configurable buttons and submenus in Explorer's native Windows 11 context menu.

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- ButtonsSettings:
  - buttons:
    - - label: Menu
        $name: Name
      - type: menu
        $name: Item type
        $options:
        - button: Button
        - menu: Dropdown menu
      - enabled: true
        $name: Enabled
      - visibility: ''
        $name: Show on
        $options:
        - all: Everywhere
        - desktop: Desktop background
        - folders: Folders
        - drives: Drives
        - files: All files
        - extensions: Specific file extensions
      - extensions: ''
        $name: File extensions
        $description: Used for Specific file extensions. For example .png; .jpg; .txt. All selected files must match.
      - icon: ''
        $name: Icon
        $description: Segoe Fluent Icons hex code, or .exe/.dll/.ico path with optional ,index.
      - action: ''
        $name: Action
      - arguments: ''
        $name: Arguments
      - position: after
        $name: Position
        $options:
        - bottom: Bottom
        - top: Top
        - before: Before an item
        - after: After an item
      - relativeTo: New
        $name: Relative to
        $description: Existing root menu item's displayed name, in your Windows language. Used for Before/After; if absent, adds the item at the
          bottom.
      - subItems:
        - - label: Settings
            $name: Name
          - type: button
            $name: Item type
            $options:
            - button: Menu item
            - menu: Submenu
          - enabled: true
            $name: Enabled
          - visibility: ''
            $name: Show on
            $options:
            - all: Everywhere
            - desktop: Desktop background
            - folders: Folders
            - drives: Drives
            - files: All files
            - extensions: Specific file extensions
          - extensions: ''
            $name: File extensions
            $description: Used for Specific file extensions. For example .png; .jpg; .txt. All selected files must match.
          - icon: E713
            $name: Icon
          - action: ms-settings:home
            $name: Action
          - arguments: ''
            $name: Arguments
          - separatorAfter: false
            $name: Separator after
          - subItems:
            - - label: ''
                $name: Name
              - enabled: false
                $name: Enabled
              - visibility: ''
                $name: Show on
                $options:
                - all: Everywhere
                - desktop: Desktop background
                - folders: Folders
                - drives: Drives
                - files: All files
                - extensions: Specific file extensions
              - extensions: ''
                $name: File extensions
                $description: Used for Specific file extensions. For example .png; .jpg; .txt. All selected files must match.
              - icon: ''
                $name: Icon
              - action: ''
                $name: Action
              - arguments: ''
                $name: Arguments
              - separatorAfter: false
                $name: Separator after
            $name: Submenu items
        - - label: About
          - type: button
          - enabled: true
          - visibility: ''
          - extensions: ''
          - icon: E946
          - action: winver.exe
          - arguments: ''
          - separatorAfter: false
          - subItems:
            - - label: ''
              - enabled: false
              - visibility: ''
              - extensions: ''
              - icon: ''
              - action: ''
              - arguments: ''
              - separatorAfter: false
        - - label: Paint
          - type: button
          - enabled: true
          - visibility: ''
          - extensions: ''
          - icon: mspaint.exe,0
          - action: mspaint.exe
          - arguments: ''
          - separatorAfter: false
          - subItems:
            - - label: ''
              - enabled: false
              - visibility: ''
              - extensions: ''
              - icon: ''
              - action: ''
              - arguments: ''
              - separatorAfter: false
        $name: Menu items
    - - label: Test
      - type: button
      - enabled: true
      - visibility: all
      - extensions: ''
      - icon: F196
      - action: explorer.exe
      - arguments: ''
      - position: top
      - relativeTo: ''
      - subItems:
        - - label: ''
          - type: ''
          - enabled: false
          - visibility: ''
          - extensions: ''
          - icon: ''
          - action: ''
          - arguments: ''
          - separatorAfter: true
          - subItems:
            - - label: ''
              - enabled: false
              - visibility: ''
              - extensions: ''
              - icon: ''
              - action: ''
              - arguments: ''
              - separatorAfter: false
    - - label: Control Panel
      - type: button
      - enabled: true
      - visibility: ''
      - extensions: ''
      - icon: ''
      - action: control.exe
      - arguments: ''
      - position: bottom
      - relativeTo: ''
      - subItems:
        - - label: ''
          - type: ''
          - enabled: false
          - visibility: ''
          - extensions: ''
          - icon: ''
          - action: ''
          - arguments: ''
          - separatorAfter: false
          - subItems:
            - - label: ''
              - enabled: false
              - visibility: ''
              - extensions: ''
              - icon: ''
              - action: ''
              - arguments: ''
              - separatorAfter: false
  $name: Buttons
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <winstring.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shcore.h>
#include <winrt/Windows.Storage.Streams.h>
#include <algorithm>
#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

struct Entry {
    std::wstring label, type, icon, target, arguments;
    std::wstring position, relativeTo;
    std::wstring visibility, extensions;
    bool enabled = true, separator = false;
    std::vector<Entry> children;
};
std::mutex g_mutex;
std::vector<Entry> g_entries;
// A snapshot belongs to a presenter, not the settings array: changing settings
// cannot redirect an already-open menu to another command.
std::map<void*, std::map<UINT, Entry>> g_active;
std::atomic<bool> g_stopping{false};
thread_local unsigned g_renderDepth = 0;

// MSVC x64 nontrivial return ABI: RCX=this, RDX=caller return storage,
// R8=HMENU. Never pass an MSVC STL/WinRT object through the MinGW ABI.
using CreateItems = void*(WINAPI*)(void*, void*, HMENU);
using Invoke = void(WINAPI*)(void*, UINT);
CreateItems CreateItems_Original;
Invoke Invoke_Original;

enum class MenuLocation { Unknown, Desktop, FolderBackground, Selection };
struct SelectedItem { bool folder = false; std::wstring extension; bool drive = false; };
struct MenuContext { MenuLocation location = MenuLocation::Unknown; std::vector<SelectedItem> items; };
std::map<void*, MenuContext> g_contexts;
std::wstring LowerCase(std::wstring text) {
    if (!text.empty()) CharLowerBuffW(text.data(), (DWORD)text.size());
    return text;
}
std::set<std::wstring> ExtensionSet(const std::wstring& raw) {
    std::set<std::wstring> result;
    for (size_t i = 0; i < raw.size();) {
        i = raw.find_first_not_of(L" ;,\t\r\n", i);
        if (i == std::wstring::npos) break;
        auto end = raw.find_first_of(L" ;,\t\r\n", i);
        auto value = LowerCase(raw.substr(i, end - i));
        if (value.starts_with(L"*.")) value.erase(0, 1);
        if (!value.empty() && value.front() != L'.') value.insert(0, 1, L'.');
        if (value.size() > 1 && value.find_first_of(L"*?\\/") == std::wstring::npos) result.insert(value);
        if (end == std::wstring::npos) break;
        i = end + 1;
    }
    return result;
}
bool VisibleOn(const Entry& entry, const MenuContext& context) {
    if (entry.visibility.empty() || entry.visibility == L"all") return true;
    if (entry.visibility == L"desktop") return context.location == MenuLocation::Desktop;
    if (entry.visibility == L"folders" && context.location == MenuLocation::FolderBackground) return true;
    if (context.location != MenuLocation::Selection || context.items.empty()) return false;
    if (entry.visibility == L"folders")
        return std::all_of(context.items.begin(), context.items.end(), [](const auto& item) { return item.folder && !item.drive; });
    if (entry.visibility == L"drives")
        return std::all_of(context.items.begin(), context.items.end(), [](const auto& item) { return item.drive; });
    if (entry.visibility == L"files")
        return std::all_of(context.items.begin(), context.items.end(), [](const auto& item) { return !item.folder && !item.drive; });
    if (entry.visibility == L"extensions") {
        auto extensions = ExtensionSet(entry.extensions);
        return std::all_of(context.items.begin(), context.items.end(), [&](const auto& item) {
            return !item.folder && !item.drive && extensions.count(LowerCase(item.extension));
        });
    }
    return false;
}
std::vector<Entry> VisibleEntries(const std::vector<Entry>& entries, const MenuContext& context) {
    std::vector<Entry> result;
    for (const auto& entry : entries) {
        if (!VisibleOn(entry, context)) continue;
        Entry copy = entry;
        if (copy.type == L"menu") {
            copy.children = VisibleEntries(copy.children, context);
            if (copy.children.empty()) continue;
        }
        result.push_back(std::move(copy));
    }
    // A separator belongs between visible rows, never after the final row.
    if (!result.empty()) result.back().separator = false;
    return result;
}
MenuContext CaptureMenuContext(IUnknown* site, IContextMenu* menu, IShellItem* item) {
    MenuContext context;
    winrt::com_ptr<IObjectWithSelection> selection;
    winrt::com_ptr<IShellItemArray> items;
    winrt::com_ptr<IServiceProvider> services;
    winrt::com_ptr<IShellView> view;
    if (site && SUCCEEDED(site->QueryInterface(IID_PPV_ARGS(services.put()))))
        services->QueryService(SID_SFolderView, IID_PPV_ARGS(view.put()));
    if (menu && SUCCEEDED(menu->QueryInterface(IID_PPV_ARGS(selection.put()))))
        selection->GetSelection(IID_PPV_ARGS(items.put()));
    if (!items && view) {
        winrt::com_ptr<IDataObject> data;
        if (SUCCEEDED(view->GetItemObject(SVGIO_SELECTION, IID_PPV_ARGS(data.put()))))
            SHCreateShellItemArrayFromDataObject(data.get(), IID_PPV_ARGS(items.put()));
    }
    if (items) {
        DWORD count = 0;
        if (FAILED(items->GetCount(&count)) || count > 4096) return context;
        for (DWORD i = 0; i < count; ++i) {
            winrt::com_ptr<IShellItem> selected;
            if (FAILED(items->GetItemAt(i, selected.put()))) return {};
            SFGAOF attributes = 0;
            if (FAILED(selected->GetAttributes(SFGAO_FOLDER, &attributes))) return {};
            PWSTR name = nullptr;
            SelectedItem info;
            info.folder = (attributes & SFGAO_FOLDER) != 0;
            if (info.folder && SUCCEEDED(selected->GetDisplayName(SIGDN_FILESYSPATH, &name))) {
                // Drive roots, including removable/optical and mapped network
                // drives, are distinct from ordinary folders. No disk access
                // or drive probing is needed to classify the shell item's path.
                info.drive = PathIsRootW(name) != FALSE;
                CoTaskMemFree(name);
                name = nullptr;
            }
            if (!info.folder && SUCCEEDED(selected->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &name))) {
                info.extension = LowerCase(PathFindExtensionW(name));
                CoTaskMemFree(name);
            }
            context.items.push_back(std::move(info));
        }
        if (!context.items.empty()) { context.location = MenuLocation::Selection; return context; }
    }
    // Desktop is identified by its shell view's owner, not by comparing a
    // filesystem path (opening the Desktop folder in Explorer is still a folder).
    HWND owner = nullptr;
    winrt::com_ptr<IOleWindow> window;
    if (site) site->QueryInterface(IID_PPV_ARGS(window.put()));
    if (!window && view) view->QueryInterface(IID_PPV_ARGS(window.put()));
    if (!window && menu) menu->QueryInterface(IID_PPV_ARGS(window.put()));
    if (window) window->GetWindow(&owner);
    if (owner) {
        wchar_t name[100]{};
        GetClassNameW(GetAncestor(owner, GA_ROOT), name, ARRAYSIZE(name));
        if (_wcsicmp(name, L"Progman") == 0 || _wcsicmp(name, L"WorkerW") == 0) {
            context.location = MenuLocation::Desktop; return context;
        }
    }
    // Background context normally supplies the current folder as the last
    // argument. Never infer a file selection from a foreground Explorer window.
    if (view) {
        context.location = MenuLocation::FolderBackground;
        return context;
    }
    if (item) {
        SFGAOF attributes = 0;
        if (SUCCEEDED(item->GetAttributes(SFGAO_FOLDER, &attributes)) && (attributes & SFGAO_FOLDER))
            context.location = MenuLocation::FolderBackground;
    }
    return context;
}
using Prepare = HRESULT(WINAPI*)(void*, UINT, IUnknown*, POINT, IContextMenu*, UINT, UINT, BOOL, const void*, const GUID*, IShellItem*);
Prepare Prepare_Original;
HRESULT WINAPI Prepare_Hook(void* self, UINT flags, IUnknown* site, POINT point, IContextMenu* menu,
                            UINT first, UINT last, BOOL unknown, const void* verbs, const GUID* activity, IShellItem* item) {
    MenuContext context;
    try { context = CaptureMenuContext(site, menu, item); }
    catch (...) { Wh_Log(L"Could not identify context menu target"); }
    {
        std::lock_guard lock(g_mutex);
        if (g_contexts.size() >= 256) g_contexts.clear();
        g_contexts[self] = std::move(context);
    }
    return Prepare_Original(self, flags, site, point, menu, first, last, unknown, verbs, activity, item);
}
std::wstring ReadSetting(const std::wstring& key) {
    auto value = Wh_GetStringSetting(L"%s", key.c_str());
    std::wstring result = value ? value : L"";
    if (value) Wh_FreeStringSetting(value);
    return result;
}
Entry ReadEntry(const std::wstring& prefix, int depth) {
    Entry e;
    e.label = ReadSetting(prefix + L".label");
    if (e.label.empty()) return e;
    e.type = depth == 2 ? L"button" : ReadSetting(prefix + L".type");
    e.icon = ReadSetting(prefix + L".icon");
    e.target = ReadSetting(prefix + L".action");
    e.arguments = ReadSetting(prefix + L".arguments");
    e.visibility = ReadSetting(prefix + L".visibility");
    e.extensions = ReadSetting(prefix + L".extensions");
    if (depth == 0) {
        e.position = ReadSetting(prefix + L".position");
        e.relativeTo = ReadSetting(prefix + L".relativeTo");
    }
    e.enabled = Wh_GetIntSetting(L"%s", (prefix + L".enabled").c_str()) != 0;
    e.separator = depth && Wh_GetIntSetting(L"%s", (prefix + L".separatorAfter").c_str()) != 0;
    if (e.type == L"menu" && depth < 2) {
        for (int i = 0; i < 64; ++i) {
            auto child = ReadEntry(prefix + L".subItems[" + std::to_wstring(i) + L"]", depth + 1);
            if (!child.label.empty()) e.children.push_back(std::move(child));
        }
    }
    return e;
}
void LoadSettings() {
    std::vector<Entry> entries;
    for (int i = 0; i < 64; ++i) {
        auto e = ReadEntry(L"ButtonsSettings.buttons[" + std::to_wstring(i) + L"]", 0);
        if (!e.label.empty()) entries.push_back(std::move(e));
    }
    std::lock_guard lock(g_mutex);
    g_entries = std::move(entries);
}
std::wstring Expand(const std::wstring& input) {
    DWORD size = ExpandEnvironmentStringsW(input.c_str(), nullptr, 0);
    if (!size) return input;
    std::vector<wchar_t> buffer(size);
    if (!ExpandEnvironmentStringsW(input.c_str(), buffer.data(), size)) return input;
    return buffer.data();
}
void Launch(const Entry& entry) {
    auto target = Expand(entry.target);
    auto args = Expand(entry.arguments);
    SHELLEXECUTEINFOW sei{sizeof(sei)};
    sei.lpVerb = L"open";
    sei.lpFile = target.c_str();
    sei.lpParameters = args.empty() ? nullptr : args.c_str();
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&sei)) Wh_Log(L"Launch failed: %s, error %lu", target.c_str(), GetLastError());
}
void CollectIds(HMENU menu, std::set<UINT>& used, unsigned depth = 0) {
    if (!menu || depth > 32) return;
    for (int i = 0, count = GetMenuItemCount(menu); i < count; ++i) {
        MENUITEMINFOW item{sizeof(item)};
        item.fMask = MIIM_ID | MIIM_SUBMENU;
        if (GetMenuItemInfoW(menu, i, TRUE, &item)) {
            used.insert(item.wID);
            CollectIds(item.hSubMenu, used, depth + 1);
        }
    }
}
// Own only temporary HMENU additions. Windows creates the actual native UI
// and its event handlers; none of our COM objects survive mod unload.
struct Additions {
    HMENU root;
    std::vector<UINT> topIds;
    explicit Additions(HMENU value) : root(value) {}
    ~Additions() {
        // DeleteMenu destroys attached submenus too.
        for (auto id : topIds) DeleteMenu(root, id, MF_BYCOMMAND);
    }
};
// Use the original menu as an immutable set of insertion boundaries. This
// keeps shared anchors stable and avoids reversing items placed at the top.
std::wstring PlacementLabel(const std::wstring& raw) {
    std::wstring label;
    for (size_t i = 0; i < raw.size() && raw[i] != L'\t'; ++i) {
        if (raw[i] == L'&') {
            if (i + 1 < raw.size() && raw[i + 1] == L'&') ++i;
            else continue;
        }
        label += raw[i];
    }
    auto first = label.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos) return {};
    label.erase(0, first);
    label.erase(label.find_last_not_of(L" \t\r\n") + 1);
    if (!label.empty() && label.back() == L'\u2026') label.pop_back();
    else if (label.size() >= 3 && label.compare(label.size() - 3, 3, L"...") == 0)
        label.resize(label.size() - 3);
    auto last = label.find_last_not_of(L" \t");
    if (last == std::wstring::npos) return {};
    label.resize(last + 1);
    return label;
}
std::vector<std::wstring> PlacementAnchors(HMENU root) {
    std::vector<std::wstring> labels;
    int count = GetMenuItemCount(root);
    for (int i = 0; i < count; ++i) {
        int length = GetMenuStringW(root, i, nullptr, 0, MF_BYPOSITION);
        std::vector<wchar_t> text(length + 1);
        GetMenuStringW(root, i, text.data(), (int)text.size(), MF_BYPOSITION);
        labels.push_back(PlacementLabel(text.data()));
    }
    return labels;
}
size_t PlacementBoundary(const Entry& entry, const std::vector<std::wstring>& anchors) {
    if (entry.position == L"top") return 0;
    if (entry.position == L"before" || entry.position == L"after") {
        auto label = PlacementLabel(entry.relativeTo);
        if (!label.empty()) {
            for (size_t i = 0; i < anchors.size(); ++i) {
                if (CompareStringOrdinal(label.c_str(), -1, anchors[i].c_str(), -1,
                                         TRUE) == CSTR_EQUAL)
                    return i + (entry.position == L"after" ? 1 : 0);
            }
        }
    }
    return anchors.size();
}
std::map<UINT, Entry> AddEntries(HMENU root, const std::vector<Entry>& entries,
                                Additions& added) {
    std::set<UINT> used;
    CollectIds(root, used);
    UINT next = 0xD800;
    auto allocate = [&]() -> UINT {
        while (next < 0xF000 && used.count(next)) ++next;
        if (next == 0xF000) return 0;
        used.insert(next);
        return next++;
    };
    auto anchors = PlacementAnchors(root);
    std::map<size_t, std::vector<const Entry*>> placements;
    for (const auto& entry : entries)
        placements[PlacementBoundary(entry, anchors)].push_back(&entry);
    std::map<UINT, Entry> actions;
    auto append = [&](auto&& self, HMENU dest, const Entry& entry, bool top,
                      UINT position) -> void {
        UINT id = allocate();
        if (!id) return;
        HMENU child = entry.type == L"menu" ? CreatePopupMenu() : nullptr;
        if (entry.type == L"menu" && !child) return;
        if (child) for (const auto& sub : entry.children) self(self, child, sub, false, GetMenuItemCount(child));
        MENUITEMINFOW item{sizeof(item)};
        item.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
        item.wID = id;
        item.dwTypeData = const_cast<PWSTR>(entry.label.c_str());
        item.fState = entry.enabled ? MFS_ENABLED : MFS_DISABLED;
        if (child) { item.fMask |= MIIM_SUBMENU; item.hSubMenu = child; }
        if (!InsertMenuItemW(dest, position, TRUE, &item)) {
            if (child) DestroyMenu(child);
            return;
        }
        if (top) added.topIds.push_back(id);
        if (!child && entry.enabled && !entry.target.empty()) actions.emplace(id, entry);
        if (!top && entry.separator) AppendMenuW(dest, MF_SEPARATOR, 0, nullptr);
    };
    size_t inserted = 0;
    for (const auto& [boundary, group] : placements) {
        for (const auto* entry : group) {
            auto previousCount = added.topIds.size();
            append(append, root, *entry, true, (UINT)(boundary + inserted));
            inserted += added.topIds.size() - previousCount;
        }
    }
    return actions;
}

// Only intercept creation while our temporary menu tree is being converted.
thread_local std::map<std::wstring, std::wstring> g_renderIcons;
using CreateCommand = void*(WINAPI*)(void*, void*, void*, void*, void*, void*);
CreateCommand CreateCommand_Original;
ULONG_PTR g_gdiplus = 0;
winrt::Windows::Storage::Streams::IRandomAccessStreamReference MakeIcon(const std::wstring& raw) {
    using namespace Gdiplus;
    auto spec = Expand(raw);
    Bitmap image(32, 32, PixelFormat32bppARGB);
    Graphics graphics(&image);
    graphics.Clear(Color(0, 0, 0, 0));
    bool glyph = spec.size() >= 4 && spec.size() <= 6 && spec.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos;
    if (glyph) {
        unsigned long value = wcstoul(spec.c_str(), nullptr, 16);
        if (value > 0xFFFF) return nullptr;
        wchar_t text[] = {static_cast<wchar_t>(value), 0};
        Font font(L"Segoe Fluent Icons", 32, FontStyleRegular, UnitPixel);
        SolidBrush brush(Color(255, 128, 128, 128));
        StringFormat format(StringFormat::GenericTypographic());
        format.SetFormatFlags(format.GetFormatFlags() | StringFormatFlagsNoClip);
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        graphics.DrawString(text, 1, &font, RectF(0, 0, 32, 32), &format, &brush);
    } else {
        int index = 0;
        auto comma = spec.rfind(L',');
        if (comma != std::wstring::npos) {
            index = _wtoi(spec.c_str() + comma + 1);
            spec.resize(comma);
        }
        // Resolve bare DLL/EXE names before extraction. Shell associations also
        // cover packaged applications whose launcher has no embedded icon.
        wchar_t resolved[32768];
        DWORD length = SearchPathW(nullptr, spec.c_str(), nullptr,
                                   ARRAYSIZE(resolved), resolved, nullptr);
        if (length && length < ARRAYSIZE(resolved)) spec = resolved;
        HICON icon = nullptr;
        UINT iconId = 0;
        PrivateExtractIconsW(spec.c_str(), index, 32, 32, &icon, &iconId, 1, 0);
        if (!icon && index == 0) {
            SHFILEINFOW info{};
            if (SHGetFileInfoW(spec.c_str(), 0, &info, sizeof(info),
                               SHGFI_ICON | SHGFI_LARGEICON)) icon = info.hIcon;
        }
        if (!icon) {
            Wh_Log(L"Could not extract icon: %s, index %d", spec.c_str(), index);
            return nullptr;
        }
        // GDI+ Bitmap::FromHICON can lose the alpha channel. Render into a
        // top-down BGRA DIB instead, preserving alpha and legacy AND masks.
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = 32;
        info.bmiHeader.biHeight = -32;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        void* pixels = nullptr;
        HDC dc = CreateCompatibleDC(nullptr);
        HBITMAP dib = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS,
                                       &pixels, nullptr, 0);
        if (!dc || !dib) {
            if (dib) DeleteObject(dib);
            if (dc) DeleteDC(dc);
            DestroyIcon(icon);
            return nullptr;
        }
        auto old = SelectObject(dc, dib);
        ZeroMemory(pixels, 32 * 32 * 4);
        BOOL drawn = DrawIconEx(dc, 0, 0, icon, 32, 32, 0, nullptr, DI_NORMAL);
        GdiFlush();
        auto data = static_cast<DWORD*>(pixels);
        bool hasAlpha = false;
        for (int i = 0; i < 32 * 32; ++i) hasAlpha |= (data[i] >> 24) != 0;
        if (drawn && !hasAlpha) {
            DWORD color[32 * 32];
            CopyMemory(color, pixels, sizeof(color));
            DrawIconEx(dc, 0, 0, icon, 32, 32, 0, nullptr, DI_MASK);
            GdiFlush();
            for (int i = 0; i < 32 * 32; ++i)
                data[i] = (color[i] & 0xFFFFFF) |
                          ((data[i] & 0xFFFFFF) ? 0 : 0xFF000000);
        }
        if (drawn) {
            Bitmap bitmap(32, 32, 32 * 4, PixelFormat32bppPARGB,
                          static_cast<BYTE*>(pixels));
            graphics.DrawImage(&bitmap, 0, 0, 32, 32);
        }
        SelectObject(dc, old);
        DeleteObject(dib);
        DeleteDC(dc);
        DestroyIcon(icon);
        if (!drawn) return nullptr;
    }
    CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0,0,0xf8,0x1e,0xf3,0x2e}};
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) return nullptr;
    auto status = image.Save(stream, &png, nullptr);
    LARGE_INTEGER zero{};
    stream->Seek(zero, STREAM_SEEK_SET, nullptr);
    winrt::Windows::Storage::Streams::IRandomAccessStream random{nullptr};
    // System-owned stream implementations remain valid after the mod unloads.
    auto hr = CreateRandomAccessStreamOverStream(stream, BSOS_DEFAULT,
        winrt::guid_of<winrt::Windows::Storage::Streams::IRandomAccessStream>(), winrt::put_abi(random));
    stream->Release();
    if (status != Ok || FAILED(hr)) return nullptr;
    return winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromStream(random);
}
void* WINAPI CreateCommand_Hook(void* self, void* result, void* id, void* label, void* state, void* icon) {
    try {
        if (g_renderDepth && label) {
            auto handle = *static_cast<HSTRING*>(label);
            UINT32 length = 0;
            auto text = WindowsGetStringRawBuffer(handle, &length);
            auto it = g_renderIcons.find(std::wstring(text, length));
            if (it != g_renderIcons.end()) {
                auto reference = MakeIcon(it->second);
                if (reference) return CreateCommand_Original(self, result, id, label, state, &reference);
            }
        }
    } catch (...) { Wh_Log(L"Could not create configured icon"); }
    return CreateCommand_Original(self, result, id, label, state, icon);
}
void* WINAPI CreateItems_Hook(void* self, void* result, HMENU menu) {
    if (g_stopping || g_renderDepth || !IsMenu(menu))
        return CreateItems_Original(self, result, menu);
    struct Depth {
        Depth() { ++g_renderDepth; }
        ~Depth() { --g_renderDepth; }
    } depth;
    Additions added(menu);
    try {
        std::vector<Entry> entries;
        MenuContext context;
        { std::lock_guard lock(g_mutex);
          entries = g_entries;
          auto it = g_contexts.find(self); if (it != g_contexts.end()) context = it->second;
        }
        entries = VisibleEntries(entries, context);
        g_renderIcons.clear();
        auto collect = [&](auto&& visit, const Entry& entry) -> void {
            if (!entry.icon.empty()) g_renderIcons[entry.label] = entry.icon;
            for (const auto& child : entry.children) visit(visit, child);
        };
        for (const auto& entry : entries) collect(collect, entry);
        auto actions = AddEntries(menu, entries, added);
        {
            std::lock_guard lock(g_mutex);
            // Bound stale presenters; addresses are refreshed on each menu build.
            if (g_active.size() > 256) g_active.clear();
            g_active[self] = std::move(actions);
        }
        Wh_Log(L"Modern menu model: %u custom top-level entries", (unsigned)added.topIds.size());
    } catch (...) {
        Wh_Log(L"Failed to build custom entries; leaving native menu available");
        // added is cleaned before rendering in the error case.
        for (auto id : added.topIds) DeleteMenu(menu, id, MF_BYCOMMAND);
        added.topIds.clear();
        std::lock_guard lock(g_mutex);
        g_active.erase(self);
    }
    return CreateItems_Original(self, result, menu);
}
void WINAPI Invoke_Hook(void* self, UINT id) {
    Entry entry;
    bool found = false;
    {
        std::lock_guard lock(g_mutex);
        auto presenter = g_active.find(self);
        if (presenter != g_active.end()) {
            auto action = presenter->second.find(id);
            if (action != presenter->second.end()) { entry = action->second; found = true; }
        }
    }
    if (!found) { Invoke_Original(self, id); return; }
    // ID 0 follows Explorer's cancellation path: dismiss the native UI before
    // launching. Custom IDs must never reach its original command dispatcher.
    Invoke_Original(self, 0);
    if (!g_stopping) Launch(entry);
}
BOOL Wh_ModInit() {
    LoadSettings();
    Gdiplus::GdiplusStartupInput startup;
    if (Gdiplus::GdiplusStartup(&g_gdiplus, &startup, nullptr) != Gdiplus::Ok) return FALSE;
    HMODULE module = LoadLibraryW(L"Windows.UI.FileExplorer.dll");
    if (!module) return FALSE;
    // Match decorated names directly: DIA and DbgHelp can format the same
    // undecorated signature differently (notably __ptr64 qualifiers).
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{L"?PrepareForContextMenu@ContextMenuPresenter@@UEAAJW4LOCATION_CONTEXT_FLAGS@@PEAUIUnknown@@UtagPOINT@@PEAUIContextMenu@@IIHPEBUICIVERBTOIDMAP@@U_GUID@@PEAUIShellItem@@@Z"}, &Prepare_Original, Prepare_Hook},
        {{L"?CreateMenuItems@ContextMenuPresenter@@QEAA?AU?$IIterable@UMenuItem@UI@WindowsUdk@winrt@@@Collections@Foundation@Windows@winrt@@PEAUHMENU__@@@Z"}, &CreateItems_Original, CreateItems_Hook},
        {{L"?Invoke@ContextMenuPresenter@@UEAAXI@Z"}, &Invoke_Original, Invoke_Hook},
        {{L"?CreateCommandMenuItem@?$consume_WindowsUdk_UI_IMenuItemStatics@UIMenuItemStatics@UI@WindowsUdk@winrt@@@impl@winrt@@QEBA@AEBUhstring@param@3@0AEBW4MenuItemStates@UI@WindowsUdk@3@AEBUIRandomAccessStreamReference@Streams@Storage@Windows@3@@Z"}, &CreateCommand_Original, CreateCommand_Hook, true},
    };
    WH_HOOK_SYMBOLS_OPTIONS options{};
    options.optionsSize = sizeof(options);
    options.noUndecoratedSymbols = TRUE;
    if (!WindhawkUtils::HookSymbols(module, hooks, ARRAYSIZE(hooks), &options)) {
        Wh_Log(L"Required Explorer hooks unavailable; initialization failed");
        return FALSE;
    }
    Wh_Log(L"Icon hook resolved: %d; configured roots: %u", CreateCommand_Original != nullptr, (unsigned)g_entries.size());
    Wh_Log(L"Modern context menu hooks installed");
    return TRUE;
}
void Wh_ModSettingsChanged() { LoadSettings(); }
void Wh_ModBeforeUninit() { g_stopping = true; }
void Wh_ModUninit() {
    std::lock_guard lock(g_mutex);
    g_active.clear();
    g_entries.clear();
    g_contexts.clear();
}