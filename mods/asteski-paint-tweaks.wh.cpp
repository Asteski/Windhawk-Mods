// ==WindhawkMod==
// @id              asteski-paint-tweaks
// @name            Paint Tweaks
// @description     Customize Paint and open images from Photos in Paint
// @version         0.4.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         mspaint.exe
// @include         Photos.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -luuid -lshell32 -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Paint Tweaks

Includes Photos integration: redirect Edit to Paint, choose whether to close the
Photos viewer afterwards, and add Paint to Open with. Open with keeps Photos open.
Paint and Photos run separate implementations selected by executable name; Paint
save-dialog and discard hooks never run in Photos. Disable the old standalone
Photos mod before enabling this combined version to avoid duplicate hooks.

For the Windows 11 Microsoft Store Paint app (developed against 11.2605.81.0).

Choose PNG, JPEG, GIF, TIFF, HEIC, Paint project, or any of Paint's four BMP
variants as the default in the Save As dialog. PNG is the initial setting.
The dialog remains editable: you can select another format for an individual
save. Only formats offered by Paint are used. Exporting to a flat image does
not preserve editable project layers; Paint's normal warnings still apply.

Sign in/account, What's new, and Copilot each have an independent hide switch.
The "Close without asking to save" dropdown controls closing Paint windows:
keep the normal prompt (Default), discard only untitled/never-saved work,
discard only changes to previously saved/opened files, or always discard.
Saving a new image makes it a saved file for subsequent closes. Discarding
does not write changes to disk. Explicit Save and Save As still work normally.

Hover toolbar is optional: enter the menu bar to reveal the ribbon, then move
into it to use the tools. It stays open while a menu/flyout or toolbar keyboard
focus is active, or while the pointer moves onto Paint's title bar. Move back
to the canvas to collapse it after the hide delay. The Show toolbar / Hide
toolbar button is hidden by default; turn its hide switch off to restore it.
Hover reveal only operates in Paint's "Automatically hide toolbar" mode.
"Always show toolbar" retains Paint's normal behavior.
Hover reveal is independent of the hidden Show/Hide toolbar button.
Temporary hover reveal does not change the button's name or native state.

Enable "Customize Copilot button" to change its caption, accessible name,
tooltip and icon. The defaults retain Generate and Assets/GenerativeFill.svg.
Choose Original, SVG image, or Font glyph. SVG supports Paint's Assets/... paths
(its image control selects the light/dark/contrast variant), absolute local SVG
paths, and ms-appx:/// URIs. External SVGs use their own colors in both themes.
Glyphs accept a pasted character or a hex code such as E7C3 or U+E7C3, with a
configurable font (Segoe Fluent Icons by default). Restart Paint to apply edits.
The original menu actions are retained. Hide Copilot takes precedence.
Settings changes require a Paint restart. Disabling restores tracked visibility.

Experimental: compiled successfully; live UI validation is still required.
What's new uses its English accessibility label as a fallback where Paint does
not expose a stable element name. Other display languages may need an update.

XAML diagnostic connection/factory scaffolding is adapted from m417z's Windows
11 Settings Styler (GPL-3.0), based on the UWPSpy VisualTreeWatcher.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- closeWithoutSaving: default
  $name: Close without asking to save
  $description: Discard changes in the selected scenarios. Previously saved includes files opened through Paint or Explorer. Restart Paint after changing this setting.
  $options:
  - default: Default — ask to save as usual
  - unsaved: Only when the file was not previously saved
  - saved: Only when the file was previously saved or opened
  - always: Always — discard changes without asking
- defaultFormat: png
  $name: Default Save As format
  $options:
  - original: Paint default
  - bmp1: Monochrome Bitmap
  - bmp4: 16 Colour Bitmap
  - bmp8: 256 Colour Bitmap
  - bmp24: 24-bit Bitmap
  - jpeg: JPEG
  - gif: GIF
  - tiff: TIFF
  - png: PNG
  - heic: HEIC
  - paint: Microsoft Paint Project
- hideSignIn: true
  $name: Hide sign in / account button
- hideWhatsNew: true
  $name: Hide What's new button
- hideCopilot: true
  $name: Hide Copilot button
- generateButton: false
  $name: Customize Copilot button
  $description: Apply the name and icon below. Hide Copilot takes precedence. Restart Paint after changing settings.
- copilotName: Generate
  $name: Copilot button name
- copilotIconType: svg
  $name: Copilot icon type
  $options:
  - original: Original Copilot icon
  - svg: SVG image
  - glyph: Font glyph
- copilotSvg: Assets/GenerativeFill.svg
  $name: Copilot SVG path
  $description: Use Assets/GenerativeFill.svg for Paint's theme-aware asset, another Assets/... path, an absolute local SVG path, or an ms-appx:/// URI. Used only with SVG image.
- copilotGlyph: E7C3
  $name: Copilot glyph
  $description: Enter a hexadecimal Unicode code point (E7C3, U+E7C3, or 0xE7C3), or paste the glyph itself. Used only with Font glyph.
- copilotGlyphFont: Segoe Fluent Icons
  $name: Copilot glyph font
- hoverToolbar: false
  $name: Show toolbar on menu-bar hover
  $description: Only active when Paint is set to Automatically hide toolbar. Always show toolbar stays visible.
- hideToolbarToggle: true
  $name: Hide Show toolbar / Hide toolbar button
  $description: Remove the toolbar toggle from the menu bar. Toolbar mode remains available in the View menu.
- hideDelay: 600
  $name: Toolbar hide delay (milliseconds)
- photosEditInPaint: true
  $name: "Photos: Make Edit open images in Paint"
- photosAfterEdit: keep
  $name: "Photos: After Edit opens Paint"
  $options:
  - keep: Keep Photos open
  - close: Close this Photos viewer
- photosAddToOpenWith: true
  $name: "Photos: Add Paint to Open with"
*/
// ==/WindhawkModSettings==


#undef GetCurrentTime
#include <windows.h>
#include <shobjidl.h>
#include <ocidl.h>
#include <xamlom.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <atomic>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <Unknwn.h>
#include <combaseapi.h>
#include <appmodel.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>

namespace PaintMod {
using namespace winrt;
namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Microsoft::UI::Xaml;
using namespace wux;
std::wstring g_format;
std::wstring g_copilotName,g_iconType,g_svgPath,g_glyph,g_glyphFont;
bool g_signIn, g_whatsNew, g_copilot, g_hover, g_generate, g_hideToolbarToggle;
DWORD g_delay;
std::atomic<bool> g_stopping{false};
HANDLE g_stopEvent, g_worker;

enum class ClosePolicy { Default, Unsaved, Saved, Always };
ClosePolicy g_closePolicy = ClosePolicy::Default;
static bool SkipSavePrompt(ClosePolicy policy, bool saved) {
    return policy == ClosePolicy::Always ||
        (policy == ClosePolicy::Saved && saved) ||
        (policy == ClosePolicy::Unsaved && !saved);
}

// Derive these ABI offsets from the matching MFC routines, never from Paint's
// title or a remembered file-dialog selection (which may have been cancelled).
size_t g_documentPathOffset;
using CanCloseFrame_t = BOOL(__fastcall*)(void*, void*);
CanCloseFrame_t g_canCloseFrame;

static bool ShouldDiscardDocument(void* document) {
    if (!document || g_closePolicy == ClosePolicy::Default || g_stopping) return false;
    const wchar_t* path = nullptr;
    wchar_t first = 0;
    SIZE_T read;
    if (!g_documentPathOffset ||
        !ReadProcessMemory(GetCurrentProcess(), static_cast<char*>(document) +
            g_documentPathOffset, &path, sizeof(path), &read) || !path ||
        !ReadProcessMemory(GetCurrentProcess(), path, &first, sizeof(first), &read))
        return false; // Unknown state must retain Paint's prompt.
    return SkipSavePrompt(g_closePolicy, first != 0);
}
BOOL __fastcall CanCloseFrameHook(void* document, void* frame) {
    if (ShouldDiscardDocument(document)) return TRUE;
    return g_canCloseFrame(document, frame);
}
static bool InitClosePolicy() {
    if (g_closePolicy == ClosePolicy::Default) return true;
    auto mfc = GetModuleHandleW(L"mfc140u.dll");
    if (!mfc) return false;
    // MFC 14 x64 exports these functions by ordinal. Verified against the
    // matching Microsoft PDB; validate the ABI before installing any hook.
    auto clear = reinterpret_cast<const unsigned char*>(GetProcAddress(mfc, MAKEINTRESOURCEA(2779)));
    auto close = reinterpret_cast<const unsigned char*>(GetProcAddress(mfc, MAKEINTRESOURCEA(2662)));
    if (!clear || !close) return false;
    // ClearPathName: add rcx, <CString member>; jmp CString::Empty.
    if (clear[0] != 0x48 || clear[1] != 0x83 || clear[2] != 0xc1 ||
        clear[3] < sizeof(void*) || clear[3] > 0x78 || clear[4] != 0xe9) return false;
    const unsigned char prefix[] = {0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,
                                   0x01,0x48,0x8b,0xd9,0x48,0x8b,0x80,0xe0,0,0,0};
    if (memcmp(close, prefix, sizeof(prefix)) != 0) return false;
    g_documentPathOffset = clear[3];
    return Wh_SetFunctionHook(const_cast<unsigned char*>(close),
        reinterpret_cast<void*>(CanCloseFrameHook), reinterpret_cast<void**>(&g_canCloseFrame));
}

// Match extension tokens, not localized display strings or absolute indices.
struct Filter { std::wstring spec; };
std::mutex g_dialogMutex;
std::map<IFileDialog*, std::vector<Filter>> g_filters;
using SetTypes_t = HRESULT(WINAPI*)(IFileDialog*,UINT,const COMDLG_FILTERSPEC*);
using Show_t = HRESULT(WINAPI*)(IFileDialog*,HWND);
SetTypes_t g_setTypes;
Show_t g_show;
static bool HasExtension(const std::wstring& spec, const wchar_t* ext) {
    size_t pos=0;
    while(pos<spec.size()) {
        size_t end=spec.find(L';',pos);
        auto token=spec.substr(pos,end==std::wstring::npos?end:end-pos);
        token.erase(std::remove_if(token.begin(),token.end(),iswspace),token.end());
        if(_wcsicmp(token.c_str(),(std::wstring(L"*.")+ext).c_str())==0) return true;
        if(end==std::wstring::npos) break;
        pos=end+1;
    }
    return false;
}
static UINT ChooseFilter(const std::vector<Filter>& filters) {
    const auto bmpCount=std::count_if(filters.begin(),filters.end(),[](const Filter& f) { return HasExtension(f.spec,L"bmp"); });
    int bmp=0;
    for(size_t i=0;i<filters.size();i++) {
        const auto& s=filters[i].spec;
        if(HasExtension(s,L"bmp")) {
            ++bmp;
            if(bmpCount==4 && ((g_format==L"bmp1" && bmp==1)||(g_format==L"bmp4" && bmp==2)||
               (g_format==L"bmp8" && bmp==3)||(g_format==L"bmp24" && bmp==4))) return i+1;
        } else if((g_format==L"jpeg" && (HasExtension(s,L"jpg")||HasExtension(s,L"jpeg")))||
                  (g_format==L"tiff" && (HasExtension(s,L"tif")||HasExtension(s,L"tiff")))||
                  (g_format==L"heic" && HasExtension(s,L"heic"))||
                  (g_format==L"png" && HasExtension(s,L"png"))||
                  (g_format==L"gif" && HasExtension(s,L"gif"))||
                  (g_format==L"paint" && HasExtension(s,L"paint"))) return i+1;
    }
    return 0;
}
HRESULT WINAPI SetTypesHook(IFileDialog* self,UINT count,const COMDLG_FILTERSPEC* types) {
    HRESULT hr=g_setTypes(self,count,types);
    if(SUCCEEDED(hr)) {
        com_ptr<IFileSaveDialog> save;
        if(SUCCEEDED(self->QueryInterface(IID_PPV_ARGS(save.put())))) {
            std::vector<Filter> filters;
            for(UINT i=0;i<count;i++) filters.push_back({types[i].pszSpec?types[i].pszSpec:L""});
            std::lock_guard lock(g_dialogMutex);
            g_filters[self]=std::move(filters);
        }
    }
    return hr;
}
HRESULT WINAPI ShowHook(IFileDialog* self,HWND owner) {
    UINT index=0;
    {
        std::lock_guard lock(g_dialogMutex);
        auto it=g_filters.find(self);
        if(it!=g_filters.end()) { index=ChooseFilter(it->second); g_filters.erase(it); }
    }
    com_ptr<IFileSaveDialog> save;
    if(index && SUCCEEDED(self->QueryInterface(IID_PPV_ARGS(save.put())))) {
        self->SetFileTypeIndex(index);
        const wchar_t* ext=g_format.c_str();
        if(g_format.starts_with(L"bmp")) ext=L"bmp";
        else if(g_format==L"jpeg") ext=L"jpg";
        else if(g_format==L"tiff") ext=L"tif";
        self->SetDefaultExtension(ext);
        Wh_Log(L"Save dialog default: %s, index %u",ext,index);
    }
    return g_show(self,owner);
}


// ABI prefixes from Paint 11.2605.81.0 PaintUI.winmd. QueryInterface checks
// the metadata IID before use; no private object offsets or guessed vtables.
struct PaintRibbon : ::IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_ViewModel(::IInspectable**) = 0;
};
struct PaintRibbonViewModel : ::IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_AreToolsEnabled(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ToolsViewModel(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ImageToolbarViewModel(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ColorsViewModel(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_BrushSizeViewModel(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_StickersViewModel(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_FeaturePromotionService(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_RibbonBehavior(::IInspectable**) = 0;
};
struct PaintRibbonBehavior : ::IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_IsRibbonAlwaysDisplayed(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsRibbonAutomaticallyHidden(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsShowHideRibbonVisible(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsShowHideRibbonEnabled(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsShowHideRibbonEnabled(boolean) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsRibbonVisible(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_SetRibbonDisplayMode(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ToggleRibbonVisibility(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE LightDismissRibbon() = 0;
};
struct PaintRibbonGroup : ::IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_Content(::IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Content(::IInspectable*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Label(void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Label(void*) = 0;
};
struct PaintThemeAwareImage : ::IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_IsEnabled(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsEnabled(boolean) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ImagePath(void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_ImagePath(void*) = 0;
};
constexpr GUID IID_PaintRibbon={0x698f9aa4,0x8804,0x59bd,{0xa5,0xa8,0x1e,0x0f,0x68,0x81,0x7a,0x38}};
constexpr GUID IID_PaintRibbonViewModel={0xc6ac4a16,0x0f3e,0x5437,{0x8e,0xb2,0xbc,0x37,0xac,0xa9,0x1e,0x5b}};
constexpr GUID IID_PaintRibbonBehavior={0x73eb39e3,0xbc20,0x52fc,{0xb2,0x6c,0x93,0xbf,0x95,0xba,0x3d,0x92}};
constexpr GUID IID_PaintRibbonGroup={0x7f572d47,0x6797,0x532d,{0xaa,0xbb,0xea,0x9d,0xfc,0x6e,0xf8,0x46}};
constexpr GUID IID_PaintThemeAwareImage={0x8b086e5d,0x0957,0x5456,{0xaf,0x65,0x41,0x61,0xd8,0xd4,0x9b,0x62}};
template<class T> com_ptr<T> PaintInterface(wf::IInspectable const& object,const GUID& iid) {
    com_ptr<T> result;
    if(object) reinterpret_cast<::IUnknown*>(get_abi(object))->QueryInterface(iid,result.put_void());
    return result;
}
struct Tracked { weak_ref<FrameworkElement> element; Visibility original; int64_t visibilityToken; };
thread_local std::vector<Tracked> t_hidden;
thread_local weak_ref<FrameworkElement> t_menu,t_ribbon;
thread_local weak_ref<FrameworkElement> t_nativeRibbon,t_generateButton,t_generateGroup,t_generateImage;
struct HoverVisibility {
    weak_ref<FrameworkElement> element;
    wf::IInspectable original{nullptr};
};
thread_local std::vector<HoverVisibility> t_hoverVisibility;
static void SetHoverVisibility(FrameworkElement element,Visibility visibility) {
    if(!element) return;
    auto property=UIElement::VisibilityProperty();
    auto found=std::find_if(t_hoverVisibility.begin(),t_hoverVisibility.end(),
        [&](const auto& entry) { return entry.element.get()==element; });
    if(found==t_hoverVisibility.end())
        t_hoverVisibility.push_back({make_weak(element),element.ReadLocalValue(property)});
    if(element.Visibility()!=visibility) element.Visibility(visibility);
}
static void RestoreHoverVisibility() {
    auto saved=std::move(t_hoverVisibility);
    t_hoverVisibility.clear();
    for(auto& entry:saved) try {
        if(auto element=entry.element.get()) {
            auto property=UIElement::VisibilityProperty();
            if(entry.original==DependencyProperty::UnsetValue()) element.ClearValue(property);
            else element.SetValue(property,entry.original);
        }
    } catch(...) {}
}
thread_local hstring t_originalLabel,t_originalImage,t_originalName;
thread_local wf::IInspectable t_originalTooltip{nullptr};
thread_local bool t_generateButtonChanged=false;
thread_local FrameworkElement t_replacedIcon{nullptr},t_customIcon{nullptr};
thread_local weak_ref<FrameworkElement> t_iconParent;
static hstring ParseGlyph(std::wstring value) {
    if(value.starts_with(L"U+") || value.starts_with(L"u+") ||
       value.starts_with(L"0x") || value.starts_with(L"0X")) value.erase(0,2);
    if(!value.empty() && value.size()<=6 &&
       std::all_of(value.begin(),value.end(),[](wchar_t c){return iswxdigit(c)!=0;})) {
        unsigned long cp=wcstoul(value.c_str(),nullptr,16);
        if(cp==0 || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff)) return {};
        if(cp<=0xffff) return hstring(std::wstring(1,static_cast<wchar_t>(cp)));
        cp-=0x10000;
        std::wstring pair{static_cast<wchar_t>(0xd800+(cp>>10)),static_cast<wchar_t>(0xdc00+(cp&1023))};
        return hstring(pair);
    }
    return hstring(value);
}
static bool ReplaceIcon(FrameworkElement parent,FrameworkElement oldIcon,FrameworkElement newIcon) {
    if(auto panel=parent.try_as<Controls::Panel>()) {
        uint32_t index;
        if(panel.Children().IndexOf(oldIcon,index)) { panel.Children().SetAt(index,newIcon); return true; }
    } else if(auto content=parent.try_as<Controls::ContentControl>(); content && content.Content()==oldIcon) {
        content.Content(newIcon); return true;
    } else if(auto border=parent.try_as<Controls::Border>(); border && border.Child()==oldIcon) {
        border.Child(newIcon); return true;
    }
    return false;
}
static void ApplyCustomIcon(FrameworkElement image) {
    if(g_iconType==L"original") return;
    if(g_iconType==L"svg" && g_svgPath.starts_with(L"Assets/")) {
        auto native=PaintInterface<PaintThemeAwareImage>(image,IID_PaintThemeAwareImage);
        check_hresult(native->get_ImagePath(put_abi(t_originalImage)));
        check_hresult(native->put_ImagePath(get_abi(hstring(g_svgPath))));
        t_generateImage=make_weak(image);
        return;
    }
    FrameworkElement replacement{nullptr};
    if(g_iconType==L"glyph") {
        auto glyph=ParseGlyph(g_glyph);
        if(glyph.empty()) return;
        Controls::FontIcon icon;
        icon.Glyph(glyph);
        icon.FontFamily(Media::FontFamily(hstring(g_glyphFont)));
        icon.FontSize(24);
        replacement=icon;
    } else if(g_iconType==L"svg" && !g_svgPath.empty()) {
        auto uri=g_svgPath;
        if(uri.size()>2 && uri[1]==L':') {
            if(GetFileAttributesW(uri.c_str())==INVALID_FILE_ATTRIBUTES) return;
            std::replace(uri.begin(),uri.end(),L'\\',L'/');
            uri=L"file:///"+uri;
        } else if(!uri.starts_with(L"ms-appx:///")) return;
        Controls::Image icon;
        icon.Source(Media::Imaging::SvgImageSource(wf::Uri(hstring(uri))));
        icon.Stretch(Media::Stretch::Uniform);
        replacement=icon;
    }
    if(!replacement) return;
    replacement.Width(image.ActualWidth()>0?image.ActualWidth():24);
    replacement.Height(image.ActualHeight()>0?image.ActualHeight():24);
    replacement.HorizontalAlignment(image.HorizontalAlignment());
    replacement.VerticalAlignment(image.VerticalAlignment());
    replacement.Margin(image.Margin());
    replacement.IsHitTestVisible(false);
    auto parent=Media::VisualTreeHelper::GetParent(image).try_as<FrameworkElement>();
    if(parent && ReplaceIcon(parent,image,replacement)) {
        t_iconParent=make_weak(parent);
        t_replacedIcon=image;
        t_customIcon=replacement;
    }
}
static com_ptr<PaintRibbonBehavior> RibbonBehavior() {
    auto ribbon=PaintInterface<PaintRibbon>(t_nativeRibbon.get(),IID_PaintRibbon);
    if(!ribbon) return {};
    wf::IInspectable viewModel{nullptr},behavior{nullptr};
    if(FAILED(ribbon->get_ViewModel(reinterpret_cast<::IInspectable**>(put_abi(viewModel))))) return {};
    auto vm=PaintInterface<PaintRibbonViewModel>(viewModel,IID_PaintRibbonViewModel);
    if(!vm || FAILED(vm->get_RibbonBehavior(reinterpret_cast<::IInspectable**>(put_abi(behavior))))) return {};
    return PaintInterface<PaintRibbonBehavior>(behavior,IID_PaintRibbonBehavior);
}
static FrameworkElement FindGenerateImage(DependencyObject node,int depth=0) {
    if(!node || depth>12) return nullptr;
    if(PaintInterface<PaintThemeAwareImage>(node,IID_PaintThemeAwareImage)) return node.try_as<FrameworkElement>();
    for(int i=0,n=Media::VisualTreeHelper::GetChildrenCount(node);i<n;i++)
        if(auto match=FindGenerateImage(Media::VisualTreeHelper::GetChild(node,i),depth+1)) return match;
    return nullptr;
}
static void ApplyGenerate() {
    if(!g_generate || g_copilot) return;
    auto button=t_generateButton.get();
    if(!button) return;
    if(!t_generateButtonChanged) {
        t_originalName=Automation::AutomationProperties::GetName(button);
        t_originalTooltip=Controls::ToolTipService::GetToolTip(button);
        Automation::AutomationProperties::SetName(button,hstring(g_copilotName));
        Controls::ToolTipService::SetToolTip(button,box_value(hstring(g_copilotName)));
        t_generateButtonChanged=true;
    }
    if(!t_generateGroup.get()) {
        auto parent=Media::VisualTreeHelper::GetParent(button);
        for(int depth=0;parent && depth<12;depth++,parent=Media::VisualTreeHelper::GetParent(parent)) {
            if(auto group=PaintInterface<PaintRibbonGroup>(parent,IID_PaintRibbonGroup)) {
                check_hresult(group->get_Label(put_abi(t_originalLabel)));
                check_hresult(group->put_Label(get_abi(hstring(g_copilotName))));
                t_generateGroup=make_weak(parent.as<FrameworkElement>());
                break;
            }
        }
    }
    if(!t_generateImage.get() && !t_customIcon && g_iconType!=L"original") {
        if(auto image=FindGenerateImage(button)) {
            ApplyCustomIcon(image);
        }
    }
}
thread_local UINT_PTR t_timer=0;
thread_local bool t_open=false;
thread_local bool t_overMenu=false,t_overRibbon=false;
thread_local event_token t_menuEnter{},t_menuExit{},t_ribbonEnter{},t_ribbonExit{};
thread_local ULONGLONG t_lastHover=0;
static bool Descendant(DependencyObject node,DependencyObject parent) {
    for(int i=0;node && i<64;i++,node=Media::VisualTreeHelper::GetParent(node)) if(node==parent) return true;
    return false;
}
static bool PointerOverPaintTitleBar() {
    POINT cursor;
    if(!GetCursorPos(&cursor)) return false;
    HWND window=GetAncestor(WindowFromPoint(cursor),GA_ROOT);
    DWORD process=0;
    // Only this UI thread's main window, not another app or an owned dialog.
    if(!window || GetWindowThreadProcessId(window,&process)!=GetCurrentThreadId() ||
       process!=GetCurrentProcessId() || GetWindow(window,GW_OWNER) || IsIconic(window)) return false;
    LPARAM position=MAKELPARAM(static_cast<SHORT>(cursor.x),static_cast<SHORT>(cursor.y));
    switch(SendMessage(window,WM_NCHITTEST,0,position)) {
    case HTCAPTION: case HTSYSMENU: case HTMINBUTTON: case HTMAXBUTTON:
    case HTCLOSE: case HTHELP:
        return true;
    default:
        return false;
    }
}
void Track(FrameworkElement e,const wchar_t* type);
void CALLBACK UiTick(HWND,UINT,UINT_PTR,DWORD) try {
    if(g_stopping) return;
    for(auto it=t_hidden.begin();it!=t_hidden.end();) {
        if(auto e=it->element.get()) { if(e.Visibility()!=Visibility::Collapsed) e.Visibility(Visibility::Collapsed); ++it; }
        else it=t_hidden.erase(it);
    }
    ApplyGenerate();
    if(!g_hover) return;
    auto menu=t_menu.get(),ribbon=t_ribbon.get();
    if(!menu || !ribbon) return;
    // Names can be assigned after the diagnostics Add callback. Resolve the
    // actual MenuBar namescope as well, using Paint's exact named button.
    if(g_hideToolbarToggle) {
        if(auto button=menu.FindName(L"ShowHideRibbonButton").try_as<FrameworkElement>())
            Track(button,L"Microsoft.UI.Xaml.Controls.Button");
    }
    bool menuVisible=menu.IsLoaded();
    for(DependencyObject node=menu;node && menuVisible;node=Media::VisualTreeHelper::GetParent(node))
        if(auto visual=node.try_as<UIElement>()) menuVisible=visual.Visibility()==Visibility::Visible;
    if(!menuVisible) {
        t_open=false; t_overMenu=false; t_overRibbon=false;
        if(!t_hoverVisibility.empty()) {
            SetHoverVisibility(t_nativeRibbon.get(),Visibility::Collapsed);
            SetHoverVisibility(ribbon,Visibility::Collapsed);
        }
        return;
    }
    auto behavior=RibbonBehavior();
    boolean alwaysVisible=true;
    if(!behavior || FAILED(behavior->get_IsRibbonAlwaysDisplayed(&alwaysVisible)) || alwaysVisible) {
        RestoreHoverVisibility();
        t_open=false;
        return;
    }
    auto inner=t_nativeRibbon.get();
    if(!inner) return;
    bool visible=t_open;
    bool over=t_overMenu||(visible && t_overRibbon);
    // Retain an already revealed ribbon, but don't reveal it from the title
    // bar alone. Native non-client hover doesn't raise XAML PointerEntered.
    bool keep=(visible || t_open) && PointerOverPaintTitleBar();
    if(visible) {
        keep=keep || Media::VisualTreeHelper::GetOpenPopupsForXamlRoot(ribbon.XamlRoot()).Size()>0;
        auto focus=Input::FocusManager::GetFocusedElement(ribbon.XamlRoot()).try_as<DependencyObject>();
        if(focus && Descendant(focus,ribbon)) {
            auto control=focus.try_as<Controls::Control>();
            keep=keep||(control && control.FocusState()==FocusState::Keyboard);
        }
    }
    if(over||keep) { t_lastHover=GetTickCount64(); t_open=true; }
    else if(!(GetAsyncKeyState(VK_LBUTTON)&0x8000) && !(GetAsyncKeyState(VK_RBUTTON)&0x8000) &&
            GetTickCount64()-t_lastHover>=g_delay) t_open=false;
    SetHoverVisibility(inner,t_open?Visibility::Visible:Visibility::Collapsed);
    SetHoverVisibility(ribbon,t_open?Visibility::Visible:Visibility::Collapsed);
} catch(...) {}
void Track(FrameworkElement e,const wchar_t* type) {
    auto name=e.Name();
    auto id=Automation::AutomationProperties::GetAutomationId(e);
    auto label=Automation::AutomationProperties::GetName(e);
    Wh_Log(L"UI: %s / %s / %s / %s",type,name.c_str(),id.c_str(),label.c_str());
    bool hide=(g_hideToolbarToggle && name==L"ShowHideRibbonButton")||
              (g_copilot && name==L"CopilotDropDownButton")||
              (g_signIn && (name==L"UserAvatar" || std::wstring_view(type)==L"PaintUI.UserAvatar"))||
              (g_whatsNew && (name==L"WhatsNewButton" || id==L"WhatsNewButton" || label==L"What's new" || label==L"What’s new"));
    if(g_copilot && name==L"CopilotDropDownButton") {
        // Remove the complete ribbon group, including its caption and spacing.
        auto parent=Media::VisualTreeHelper::GetParent(e);
        for(int depth=0;parent && depth<12;depth++,parent=Media::VisualTreeHelper::GetParent(parent)) {
            if(get_class_name(parent)==L"PaintUI.RibbonGroup") {
                if(auto group=parent.try_as<FrameworkElement>()) e=group;
                break;
            }
        }
    }
    if(hide) {
        bool found=false; for(auto& t:t_hidden) if(t.element.get()==e) found=true;
        if(!found) {
            auto original=e.Visibility();
            auto token=e.RegisterPropertyChangedCallback(UIElement::VisibilityProperty(),
                [](DependencyObject const& object,DependencyProperty const&) {
                    if(g_stopping) return;
                    if(auto visual=object.try_as<FrameworkElement>(); visual && visual.Visibility()!=Visibility::Collapsed)
                        visual.Visibility(Visibility::Collapsed);
                });
            t_hidden.push_back({make_weak(e),original,token});
        }
        e.Visibility(Visibility::Collapsed);
    }
    if(name==L"CopilotDropDownButton" && g_generate && !g_copilot) t_generateButton=make_weak(e);
    if(g_hover && std::wstring_view(type)==L"PaintUI.Ribbon") t_nativeRibbon=make_weak(e);
    if(g_hover && name==L"menuBar" && !t_menu.get()) {
        t_menu=make_weak(e);
        t_menuEnter=e.PointerEntered([](auto&&,auto&&) { t_overMenu=true; UiTick(nullptr,0,0,0); });
        t_menuExit=e.PointerExited([](auto&&,auto&&) { t_overMenu=false; });
    }
    if(g_hover && name==L"ribbonContainer" && !t_ribbon.get()) {
        t_ribbon=make_weak(e); t_open=false;
        t_ribbonEnter=e.PointerEntered([](auto&&,auto&&) { t_overRibbon=true; });
        t_ribbonExit=e.PointerExited([](auto&&,auto&&) { t_overRibbon=false; });
    }
    if(!t_timer) t_timer=SetTimer(nullptr,0,100,UiTick);
}
void WINAPI CleanupUi(void*) {
    if(t_timer) { KillTimer(nullptr,t_timer); t_timer=0; }
    RestoreHoverVisibility();
    for(auto& t:t_hidden) try { if(auto e=t.element.get()) {
        e.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(),t.visibilityToken);
        e.Visibility(t.original);
    } } catch(...) {}
    t_hidden.clear();
    try { if(auto e=t_menu.get()) { e.PointerEntered(t_menuEnter); e.PointerExited(t_menuExit); } } catch(...) {}
    try { if(auto e=t_ribbon.get()) {
        e.PointerEntered(t_ribbonEnter); e.PointerExited(t_ribbonExit);
    } } catch(...) {}
    try {
        if(auto parent=t_iconParent.get(); parent && t_customIcon)
            ReplaceIcon(parent,t_customIcon,t_replacedIcon);
        if(auto group=PaintInterface<PaintRibbonGroup>(t_generateGroup.get(),IID_PaintRibbonGroup)) group->put_Label(get_abi(t_originalLabel));
        if(auto image=PaintInterface<PaintThemeAwareImage>(t_generateImage.get(),IID_PaintThemeAwareImage)) image->put_ImagePath(get_abi(t_originalImage));
        if(auto button=t_generateButton.get(); button && t_generateButtonChanged) {
            Automation::AutomationProperties::SetName(button,t_originalName);
            Controls::ToolTipService::SetToolTip(button,t_originalTooltip);
        }
    } catch(...) {}
    t_menu={}; t_ribbon={}; t_nativeRibbon={};
    t_generateButton={}; t_generateGroup={}; t_generateImage={};
    t_originalTooltip=nullptr; t_generateButtonChanged=false;
    t_iconParent={}; t_customIcon=nullptr; t_replacedIcon=nullptr;
}
HMODULE GetCurrentModuleHandle() {
    HMODULE module=nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle),&module);
    return module;
}
class VisualTreeWatcher : public winrt::implements<VisualTreeWatcher,IVisualTreeServiceCallback2,winrt::non_agile> {
public:
    com_ptr<IXamlDiagnostics> diagnostics;
    HANDLE worker=nullptr;
    VisualTreeWatcher(com_ptr<IUnknown> site):diagnostics(site.as<IXamlDiagnostics>()) {
        worker=CreateThread(nullptr,0,[](void* p)->DWORD {
            auto self=static_cast<VisualTreeWatcher*>(p);
            auto hr=self->diagnostics.as<IVisualTreeService3>()->AdviseVisualTreeChange(self);
            Wh_Log(L"XAML subscription: %08X",hr);
            return 0;
        },this,0,nullptr);
    }
    void UnadviseVisualTreeChange() {
        if(worker) { WaitForSingleObject(worker,INFINITE); CloseHandle(worker); worker=nullptr; }
        diagnostics.as<IVisualTreeService3>()->UnadviseVisualTreeChange(this);
    }
    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation,VisualElement element,VisualMutationType kind) override {
        if(g_stopping || kind!=Add) return S_OK;
        try {
            wf::IInspectable object{nullptr};
            check_hresult(diagnostics->GetIInspectableFromHandle(element.Handle,reinterpret_cast<::IInspectable**>(put_abi(object))));
            if(auto e=object.try_as<FrameworkElement>()) Track(e,element.Type?element.Type:L"");
        } catch(...) { Wh_Log(L"XAML tracking error: %08X",to_hresult()); }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle,VisualElementState,LPCWSTR) noexcept override { return S_OK; }
};
winrt::com_ptr<VisualTreeWatcher> g_visualTreeWatcher;

// {7152D421-5463-40E8-A432-F5916B6427E5}
static constexpr CLSID CLSID_WindhawkTAP = { 0x7152d421, 0x5463, 0x40e8, { 0xa4, 0x32, 0xf5, 0x91, 0x6b, 0x64, 0x27, 0xe5 } };

class WindhawkTAP : public winrt::implements<WindhawkTAP, IObjectWithSite, winrt::non_agile>
{
public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown *pUnkSite) override;
    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void **ppvSite) noexcept override;

private:
    winrt::com_ptr<IUnknown> site;
};

#pragma endregion  // tap_hpp

#pragma region tap_cpp

HRESULT WindhawkTAP::SetSite(IUnknown *pUnkSite) try
{
    // Only ever 1 VTW at once.
    if (g_visualTreeWatcher)
    {
        g_visualTreeWatcher->UnadviseVisualTreeChange();
        g_visualTreeWatcher = nullptr;
    }

    site.copy_from(pUnkSite);

    if (site)
    {
        // Decrease refcount increased by InitializeXamlDiagnosticsEx.
        FreeLibrary(GetCurrentModuleHandle());

        g_visualTreeWatcher = winrt::make_self<VisualTreeWatcher>(site);
    }

    return S_OK;
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

HRESULT WindhawkTAP::GetSite(REFIID riid, void **ppvSite) noexcept
{
    return site.as(riid, ppvSite);
}

#pragma endregion  // tap_cpp

#pragma region simplefactory_hpp


template<class T>
struct SimpleFactory : winrt::implements<SimpleFactory<T>, IClassFactory, winrt::non_agile>
{
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override try
    {
        if (!pUnkOuter)
        {
            *ppvObject = nullptr;
            return winrt::make<T>().as(riid, ppvObject);
        }
        else
        {
            return CLASS_E_NOAGGREGATION;
        }
    }
    catch (...)
    {
        HRESULT hr = winrt::to_hresult();
        Wh_Log(L"Error %08X", hr);
        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

#pragma endregion  // simplefactory_hpp

#pragma region module_cpp


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"

HRESULT WINAPI GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) try
{
    if (rclsid == CLSID_WindhawkTAP)
    {
        *ppv = nullptr;
        return winrt::make<SimpleFactory<WindhawkTAP>>().as(riid, ppv);
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

HRESULT WINAPI CanUnloadNow()
{
    if (winrt::get_module_lock())
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

#pragma clang diagnostic pop

#pragma endregion  // module_cpp

#pragma region api_cpp

using PFN_INITIALIZE_XAML_DIAGNOSTICS_EX = decltype(&InitializeXamlDiagnosticsEx);

HRESULT InjectWindhawkTAP() noexcept
{
    HMODULE module = GetCurrentModuleHandle();
    if (!module)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    WCHAR location[MAX_PATH];
    switch (GetModuleFileName(module, location, ARRAYSIZE(location)))
    {
    case 0:
    case ARRAYSIZE(location):
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const HMODULE wux(GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll"));
    if (!wux) [[unlikely]]
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const auto ixde = reinterpret_cast<PFN_INITIALIZE_XAML_DIAGNOSTICS_EX>(GetProcAddress(wux, "InitializeXamlDiagnosticsEx"));
    if (!ixde) [[unlikely]]
    {
        auto error=HRESULT_FROM_WIN32(GetLastError());
        
        return error;
    }

    // I didn't find a better way than trying many connections until one works.
    // Reference:
    // https://github.com/microsoft/microsoft-ui-xaml/blob/d74a0332cf0d5e58f12eddce1070fa7a79b4c2db/src/dxaml/xcp/dxaml/lib/DXamlCore.cpp#L2782
    HRESULT hr;
    for (int i = 0; i < 10000; i++)
    {
        if(g_stopping) {  return E_ABORT; }
        WCHAR connectionName[256];
        wsprintf(connectionName, L"WinUIVisualDiagConnection%d", i + 1);

        hr = ixde(connectionName, GetCurrentProcessId(), L"", location, CLSID_WindhawkTAP, nullptr);
        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            break;
        }
    }

    
    return hr;
}


using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);

bool RunFromWindowThread(HWND hWnd,
                         RunFromWindowThreadProc_t proc,
                         PVOID procParam) {
    static const UINT runFromWindowThreadRegisteredMsg =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct RUN_FROM_WINDOW_THREAD_PARAM {
        RunFromWindowThreadProc_t proc;
        PVOID procParam;
    };

    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (dwThreadId == 0) {
        return false;
    }

    if (dwThreadId == GetCurrentThreadId()) {
        proc(procParam);
        return true;
    }

    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HC_ACTION) {
                const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
                if (cwp->message == runFromWindowThreadRegisteredMsg) {
                    RUN_FROM_WINDOW_THREAD_PARAM* param =
                        (RUN_FROM_WINDOW_THREAD_PARAM*)cwp->lParam;
                    param->proc(param->procParam);
                }
            }

            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, dwThreadId);
    if (!hook) {
        return false;
    }

    RUN_FROM_WINDOW_THREAD_PARAM param;
    param.proc = proc;
    param.procParam = procParam;
    SendMessage(hWnd, runFromWindowThreadRegisteredMsg, 0, (LPARAM)&param);

    UnhookWindowsHookEx(hook);

    return true;
}


BOOL Wh_ModInit() {
    auto closePolicy = Wh_GetStringSetting(L"closeWithoutSaving");
    if (wcscmp(closePolicy, L"unsaved") == 0) g_closePolicy = ClosePolicy::Unsaved;
    else if (wcscmp(closePolicy, L"saved") == 0) g_closePolicy = ClosePolicy::Saved;
    else if (wcscmp(closePolicy, L"always") == 0) g_closePolicy = ClosePolicy::Always;
    Wh_FreeStringSetting(closePolicy);
    if (!InitClosePolicy()) {
        g_closePolicy = ClosePolicy::Default;
        Wh_Log(L"Close policy unavailable for this MFC version; keeping save prompts");
    }
    auto format=Wh_GetStringSetting(L"defaultFormat"); g_format=format; Wh_FreeStringSetting(format);
    g_signIn=Wh_GetIntSetting(L"hideSignIn"); g_whatsNew=Wh_GetIntSetting(L"hideWhatsNew");
    g_generate=Wh_GetIntSetting(L"generateButton");
    auto readString=[](const wchar_t* key) {
        auto value=Wh_GetStringSetting(key);
        std::wstring result=value; Wh_FreeStringSetting(value); return result;
    };
    g_copilotName=readString(L"copilotName");
    g_iconType=readString(L"copilotIconType");
    g_svgPath=readString(L"copilotSvg");
    g_glyph=readString(L"copilotGlyph");
    g_glyphFont=readString(L"copilotGlyphFont");
    g_hideToolbarToggle=Wh_GetIntSetting(L"hideToolbarToggle");
    g_copilot=Wh_GetIntSetting(L"hideCopilot"); g_hover=Wh_GetIntSetting(L"hoverToolbar");
    g_delay=std::clamp(Wh_GetIntSetting(L"hideDelay"),100,5000);
    HRESULT init=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    com_ptr<IFileSaveDialog> dialog;
    HRESULT hr=CoCreateInstance(CLSID_FileSaveDialog,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(dialog.put()));
    bool hooks=false;
    if(SUCCEEDED(hr)) {
        auto vtable=*reinterpret_cast<void***>(dialog.get());
        hooks=Wh_SetFunctionHook(vtable[4],reinterpret_cast<void*>(SetTypesHook),reinterpret_cast<void**>(&g_setTypes)) &&
              Wh_SetFunctionHook(vtable[3],reinterpret_cast<void*>(ShowHook),reinterpret_cast<void**>(&g_show));
        dialog=nullptr;
    }
    if(SUCCEEDED(init)) CoUninitialize();
    if(!hooks) { Wh_Log(L"Cannot hook save dialog: %08X",hr); return FALSE; }
    g_stopEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);
    return g_stopEvent!=nullptr;
}
void Wh_ModAfterInit() {
    g_worker=CreateThread(nullptr,0,[](void*)->DWORD {
        for(int i=0;i<120 && WaitForSingleObject(g_stopEvent,500)==WAIT_TIMEOUT;i++) {
            if(!GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll")) continue;
            HRESULT hr=InjectWindhawkTAP();
            Wh_Log(L"XAML connect: %08X",hr);
            if(SUCCEEDED(hr)) break;
        }
        return 0;
    },nullptr,0,nullptr);
}
void Wh_ModUninit() {
    g_stopping=true;
    SetEvent(g_stopEvent);
    if(g_worker) { WaitForSingleObject(g_worker,INFINITE); CloseHandle(g_worker); }
    if(g_visualTreeWatcher) { g_visualTreeWatcher->UnadviseVisualTreeChange(); g_visualTreeWatcher=nullptr; }
    EnumWindows([](HWND w,LPARAM)->BOOL {
        DWORD pid; GetWindowThreadProcessId(w,&pid);
        if(pid==GetCurrentProcessId()) RunFromWindowThread(w,CleanupUi,nullptr);
        return TRUE;
    },0);
    CloseHandle(g_stopEvent);
}
BOOL Wh_ModSettingsChanged(BOOL* reload) { *reload=TRUE; return TRUE; }




} // namespace PaintMod

namespace PhotosMod {
using namespace winrt;
namespace wf=winrt::Windows::Foundation;
namespace wux=winrt::Microsoft::UI::Xaml;
using namespace wux;
std::atomic<bool> g_stopping{false};
HANDLE g_stopEvent=nullptr,g_worker=nullptr;
std::wstring g_paintIcon,g_paintExe;
bool g_editInPaint=true,g_closeAfterEdit=false,g_addToOpenWith=true;

static Controls::IconElement PaintIcon() {
    if(!g_paintIcon.empty()) {
        Controls::BitmapIcon icon;
        icon.UriSource(wf::Uri(hstring(g_paintIcon)));
        icon.ShowAsMonochrome(false);
        return icon;
    }
    Controls::FontIcon icon;
    icon.FontFamily(Media::FontFamily(L"Segoe Fluent Icons"));
    icon.Glyph(L"\uE790");
    return icon;
}

// Interface IDs and getter slots from the installed Lightbox.winmd. Query each
// interface before calling its ABI, so unsupported Photos versions fail closed.
constexpr GUID IID_TitleBar={0x14bfcbe6,0xe086,0x5a4b,{0x95,0x48,0x5b,0xa3,0x89,0x9f,0x76,0x49}};
constexpr GUID IID_TitleVM={0xdce82107,0x9f0e,0x507e,{0xa7,0xfe,0x3c,0xee,0x2d,0x8a,0x15,0x08}};
constexpr GUID IID_Items={0x3e290619,0x891a,0x5ad7,{0x88,0x5d,0xd5,0x10,0xc1,0x07,0xee,0x3c}};
constexpr GUID IID_Item={0x9dc78563,0xf619,0x58db,{0x99,0xf6,0xa2,0x92,0x43,0x21,0xe5,0x87}};
template<class T> HRESULT ReadGetter(wf::IInspectable const& object,const GUID& iid,size_t slot,T* value) {
    if(!object) return E_POINTER;
    com_ptr<::IInspectable> iface;
    HRESULT hr=reinterpret_cast<::IUnknown*>(get_abi(object))->QueryInterface(iid,iface.put_void());
    if(FAILED(hr)) return hr;
    auto table=*reinterpret_cast<void***>(iface.get());
    return reinterpret_cast<HRESULT(STDMETHODCALLTYPE*)(::IInspectable*,T*)>(table[slot])(iface.get(),value);
}
static wf::IInspectable ObjectGetter(wf::IInspectable const& object,const GUID& iid,size_t slot) {
    wf::IInspectable result{nullptr};
    check_hresult(ReadGetter(object,iid,slot,reinterpret_cast<::IInspectable**>(put_abi(result))));
    return result;
}
static std::wstring CurrentPath(FrameworkElement const& toolbar) noexcept {
    try {
        DependencyObject parent=toolbar;
        for(int depth=0;parent && depth<24;depth++,parent=Media::VisualTreeHelper::GetParent(parent)) {
            com_ptr<::IInspectable> title;
            if(FAILED(reinterpret_cast<::IUnknown*>(get_abi(parent))->QueryInterface(IID_TitleBar,title.put_void()))) continue;
            auto vm=ObjectGetter(parent,IID_TitleBar,6);
            auto items=ObjectGetter(vm,IID_TitleVM,8);
            auto item=ObjectGetter(items,IID_Items,28);
            boolean video=true;
            check_hresult(ReadGetter(item,IID_Item,44,&video));
            if(video) return {};
            hstring path;
            check_hresult(ReadGetter(item,IID_Item,43,reinterpret_cast<HSTRING*>(put_abi(path))));
            if(path.empty()) return {};
            DWORD attributes=GetFileAttributesW(path.c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES || (attributes&FILE_ATTRIBUTE_DIRECTORY)) return {};
            return std::wstring(path);
        }
    } catch(...) {}
    return {};
}
static std::wstring FileUri(std::wstring const& path) {
    std::vector<wchar_t> uri(path.size()*3+32);
    DWORD count=static_cast<DWORD>(uri.size());
    if(FAILED(UrlCreateFromPathW(path.c_str(),uri.data(),&count,0))) return {};
    return uri.data();
}
static void FindPaintIcon() {
    UINT32 count=0,length=0;
    if(GetPackagesByPackageFamily(L"Microsoft.Paint_8wekyb3d8bbwe",&count,nullptr,&length,nullptr)!=ERROR_INSUFFICIENT_BUFFER) return;
    std::vector<PWSTR> names(count);
    std::vector<wchar_t> buffer(length);
    if(GetPackagesByPackageFamily(L"Microsoft.Paint_8wekyb3d8bbwe",&count,names.data(),&length,buffer.data())!=ERROR_SUCCESS) return;
    for(auto name:names) {
        UINT32 size=0;
        if(GetPackagePathByFullName(name,&size,nullptr)!=ERROR_INSUFFICIENT_BUFFER) continue;
        std::vector<wchar_t> path(size);
        if(GetPackagePathByFullName(name,&size,path.data())!=ERROR_SUCCESS) continue;
        std::wstring executable=std::wstring(path.data())+L"\\PaintApp\\mspaint.exe";
        if(GetFileAttributesW(executable.c_str())!=INVALID_FILE_ATTRIBUTES) g_paintExe=executable;
        std::wstring icon=std::wstring(path.data())+L"\\Assets\\PaintAppList.targetsize-32_altform-unplated.png";
        if(GetFileAttributesW(icon.c_str())!=INVALID_FILE_ATTRIBUTES) {g_paintIcon=FileUri(icon);break;}
    }
}
static bool OpenInPaint(FrameworkElement const& origin,bool closeViewer) {
    auto path=CurrentPath(origin);
    if(path.empty()) return false;
    auto argument=L"\""+path+L"\"";
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.fMask=SEE_MASK_FLAG_NO_UI;
    info.lpVerb=L"open"; info.lpFile=g_paintExe.c_str();
    info.lpParameters=argument.c_str(); info.nShow=SW_SHOWNORMAL;
    if(!ShellExecuteExW(&info)) {
        Wh_Log(L"Paint launch failed: %lu",GetLastError());
        return false;
    }
    if(closeViewer) try {
        auto id=origin.XamlRoot().ContentIslandEnvironment().AppWindowId();
        origin.DispatcherQueue().TryEnqueue([id] {
            if(g_stopping) return;
            try { winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(id).Destroy(); }
            catch(...) {Wh_Log(L"Couldn't close Photos viewer: %08X",to_hresult());}
        });
    } catch(...) {Wh_Log(L"Couldn't identify Photos viewer: %08X",to_hresult());}
    return true;
}
struct PaintEditCommand : implements<PaintEditCommand,Input::ICommand> {
    weak_ref<FrameworkElement> origin;
    Input::ICommand original{nullptr};
    event<wf::EventHandler<wf::IInspectable>> changed;
    PaintEditCommand(FrameworkElement const& element,Input::ICommand const& command):origin(make_weak(element)),original(command) {}
    bool CanExecute(wf::IInspectable const& parameter) {
        if(auto e=origin.get(); e && !CurrentPath(e).empty()) return true;
        return original && original.CanExecute(parameter);
    }
    void Execute(wf::IInspectable const& parameter) {
        if(g_stopping) return;
        if(auto e=origin.get(); e && !CurrentPath(e).empty()) {OpenInPaint(e,g_closeAfterEdit);return;}
        if(original && original.CanExecute(parameter)) original.Execute(parameter);
    }
    event_token CanExecuteChanged(wf::EventHandler<wf::IInspectable> const& handler) {return changed.add(handler);}
    void CanExecuteChanged(event_token const& token) noexcept {changed.remove(token);}
};
struct EditControl {
    weak_ref<Controls::Button> button;
    Input::ICommand native{nullptr},replacement{nullptr};
    wf::IInspectable originalLocal{nullptr};
};
struct OpenControl {
    weak_ref<Controls::Button> button;
    weak_ref<Controls::Panel> parent;
    Controls::Button item{nullptr};
    event_token click{};
};
thread_local std::vector<EditControl> t_edits;
thread_local std::vector<OpenControl> t_open;
thread_local UINT_PTR t_timer=0;
static FrameworkElement FindNamed(DependencyObject const& root,const wchar_t* name,int depth=0) {
    if(!root || depth>20) return nullptr;
    if(auto e=root.try_as<FrameworkElement>();e &&
       (e.Name()==name || Automation::AutomationProperties::GetAutomationId(e)==name)) return e;
    for(int i=0,n=Media::VisualTreeHelper::GetChildrenCount(root);i<n;i++)
        if(auto e=FindNamed(Media::VisualTreeHelper::GetChild(root,i),name,depth+1)) return e;
    return nullptr;
}
static void EnsureMenu(OpenControl& entry,Controls::Button const& origin) {
    if(entry.item) {entry.item.IsEnabled(!CurrentPath(origin).empty());return;}
    auto base=origin.Flyout();
    if(!base) base=Controls::Primitives::FlyoutBase::GetAttachedFlyout(origin);
    auto flyout=base.try_as<Controls::Flyout>();
    if(!flyout) return;
    auto first=FindNamed(flyout.Content(),L"OpenIn_FileExplorerButton");
    if(!first) return;
    auto panel=Media::VisualTreeHelper::GetParent(first).try_as<Controls::Panel>();
    if(!panel) return;
    uint32_t index;
    if(!panel.Children().IndexOf(first,index)) return;
    Controls::Button item;
    item.Name(L"WindhawkOpenWithPaint");
    item.HorizontalAlignment(HorizontalAlignment::Stretch);
    item.HorizontalContentAlignment(HorizontalAlignment::Stretch);
    item.BorderThickness(Thickness{0});
    item.Background(Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
    item.Padding(Thickness{12,8,12,8});
    Controls::StackPanel row;
    row.Orientation(Controls::Orientation::Horizontal); row.Spacing(12);
    auto icon=PaintIcon(); icon.Width(20); icon.Height(20);
    icon.VerticalAlignment(VerticalAlignment::Center);
    row.Children().Append(icon);
    Controls::StackPanel labels;
    Controls::TextBlock title;title.Text(L"Paint");title.FontSize(14);
    Controls::TextBlock description;description.Text(L"Edit image in Paint");description.FontSize(12);description.Opacity(0.75);
    labels.Children().Append(title); labels.Children().Append(description);
    row.Children().Append(labels); item.Content(row);
    Automation::AutomationProperties::SetName(item,L"Paint");
    auto weak=entry.button;
    entry.click=item.Click([weak](auto&&,auto&&) {
        if(g_stopping) return;
        if(auto button=weak.get()) {if(auto flyout=button.Flyout()) flyout.Hide();OpenInPaint(button,false);}
    });
    entry.item=item;entry.parent=make_weak(panel);
    panel.Children().InsertAt(index,item);
    item.IsEnabled(!CurrentPath(origin).empty());
}
void CALLBACK UiTick(HWND,UINT,UINT_PTR,DWORD) {
    if(g_stopping) return;
    for(auto& entry:t_edits) try {
        auto button=entry.button.get();if(!button || !button.IsLoaded()) continue;
        if(!entry.replacement) {
            entry.native=button.Command();
            entry.originalLocal=button.ReadLocalValue(Controls::Primitives::ButtonBase::CommandProperty());
            entry.replacement=make<PaintEditCommand>(button,entry.native);
            button.Command(entry.replacement);
        }
    } catch(...) {Wh_Log(L"Edit override: %08X",to_hresult());}
    for(auto& entry:t_open) try {
        if(auto button=entry.button.get();button && button.IsLoaded()) EnsureMenu(entry,button);
    } catch(...) {Wh_Log(L"Open with entry: %08X",to_hresult());}
}
static void Track(FrameworkElement const& element,const wchar_t*) {
    auto button=element.try_as<Controls::Button>();if(!button) return;
    if(g_editInPaint && element.Name()==L"EditButton") {
        if(std::none_of(t_edits.begin(),t_edits.end(),[&](auto& e){return e.button.get()==button;}))
            t_edits.push_back({make_weak(button)});
    } else if(g_addToOpenWith && (element.Name()==L"OpenInButton" || element.Name()==L"OpenInDropDownButton")) {
        if(std::none_of(t_open.begin(),t_open.end(),[&](auto& e){return e.button.get()==button;}))
            t_open.push_back({make_weak(button)});
    } else return;
    if(!t_timer) t_timer=SetTimer(nullptr,0,200,UiTick);
}
void WINAPI CleanupUi(void*) {
    if(t_timer) {KillTimer(nullptr,t_timer);t_timer=0;}
    for(auto& entry:t_edits) try {
        if(auto button=entry.button.get();button && entry.replacement && button.Command()==entry.replacement) {
            auto property=Controls::Primitives::ButtonBase::CommandProperty();
            if(entry.originalLocal==DependencyProperty::UnsetValue()) button.ClearValue(property);
            else button.SetValue(property,entry.originalLocal);
        }
    } catch(...) {}
    for(auto& entry:t_open) try {
        if(entry.item) entry.item.Click(entry.click);
        if(auto panel=entry.parent.get();panel && entry.item) {
            uint32_t index;if(panel.Children().IndexOf(entry.item,index)) panel.Children().RemoveAt(index);
        }
    } catch(...) {}
    t_edits.clear();t_open.clear();
}

HMODULE GetCurrentModuleHandle() {
    HMODULE module=nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle),&module);
    return module;
}
class VisualTreeWatcher : public winrt::implements<VisualTreeWatcher,IVisualTreeServiceCallback2,winrt::non_agile> {
public:
    com_ptr<IXamlDiagnostics> diagnostics;
    HANDLE worker=nullptr;
    VisualTreeWatcher(com_ptr<IUnknown> site):diagnostics(site.as<IXamlDiagnostics>()) {
        worker=CreateThread(nullptr,0,[](void* p)->DWORD {
            auto self=static_cast<VisualTreeWatcher*>(p);
            auto hr=self->diagnostics.as<IVisualTreeService3>()->AdviseVisualTreeChange(self);
            Wh_Log(L"XAML subscription: %08X",hr);
            return 0;
        },this,0,nullptr);
    }
    void UnadviseVisualTreeChange() {
        if(worker) { WaitForSingleObject(worker,INFINITE); CloseHandle(worker); worker=nullptr; }
        diagnostics.as<IVisualTreeService3>()->UnadviseVisualTreeChange(this);
    }
    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation,VisualElement element,VisualMutationType kind) override {
        if(g_stopping || kind!=Add) return S_OK;
        try {
            wf::IInspectable object{nullptr};
            check_hresult(diagnostics->GetIInspectableFromHandle(element.Handle,reinterpret_cast<::IInspectable**>(put_abi(object))));
            if(auto e=object.try_as<FrameworkElement>()) Track(e,element.Type?element.Type:L"");
        } catch(...) { Wh_Log(L"XAML tracking error: %08X",to_hresult()); }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle,VisualElementState,LPCWSTR) noexcept override { return S_OK; }
};
winrt::com_ptr<VisualTreeWatcher> g_visualTreeWatcher;

// {7152D421-5463-40E8-A432-F5916B6427E5}
static constexpr CLSID CLSID_WindhawkTAP = { 0xf1b89c20, 0x96a3, 0x4f56, { 0xb6, 0x10, 0x53, 0xc7, 0x0a, 0x95, 0x2d, 0x13 } };

class WindhawkTAP : public winrt::implements<WindhawkTAP, IObjectWithSite, winrt::non_agile>
{
public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown *pUnkSite) override;
    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void **ppvSite) noexcept override;

private:
    winrt::com_ptr<IUnknown> site;
};

#pragma endregion  // tap_hpp

#pragma region tap_cpp

HRESULT WindhawkTAP::SetSite(IUnknown *pUnkSite) try
{
    // Only ever 1 VTW at once.
    if (g_visualTreeWatcher)
    {
        g_visualTreeWatcher->UnadviseVisualTreeChange();
        g_visualTreeWatcher = nullptr;
    }

    site.copy_from(pUnkSite);

    if (site)
    {
        // Decrease refcount increased by InitializeXamlDiagnosticsEx.
        FreeLibrary(GetCurrentModuleHandle());

        g_visualTreeWatcher = winrt::make_self<VisualTreeWatcher>(site);
    }

    return S_OK;
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

HRESULT WindhawkTAP::GetSite(REFIID riid, void **ppvSite) noexcept
{
    return site.as(riid, ppvSite);
}

#pragma endregion  // tap_cpp

#pragma region simplefactory_hpp


template<class T>
struct SimpleFactory : winrt::implements<SimpleFactory<T>, IClassFactory, winrt::non_agile>
{
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override try
    {
        if (!pUnkOuter)
        {
            *ppvObject = nullptr;
            return winrt::make<T>().as(riid, ppvObject);
        }
        else
        {
            return CLASS_E_NOAGGREGATION;
        }
    }
    catch (...)
    {
        HRESULT hr = winrt::to_hresult();
        Wh_Log(L"Error %08X", hr);
        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

#pragma endregion  // simplefactory_hpp

#pragma region module_cpp


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"

HRESULT WINAPI GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) try
{
    if (rclsid == CLSID_WindhawkTAP)
    {
        *ppv = nullptr;
        return winrt::make<SimpleFactory<WindhawkTAP>>().as(riid, ppv);
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

HRESULT WINAPI CanUnloadNow()
{
    if (winrt::get_module_lock())
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

#pragma clang diagnostic pop

#pragma endregion  // module_cpp

#pragma region api_cpp

using PFN_INITIALIZE_XAML_DIAGNOSTICS_EX = decltype(&InitializeXamlDiagnosticsEx);

HRESULT InjectWindhawkTAP() noexcept
{
    HMODULE module = GetCurrentModuleHandle();
    if (!module)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    WCHAR location[MAX_PATH];
    switch (GetModuleFileName(module, location, ARRAYSIZE(location)))
    {
    case 0:
    case ARRAYSIZE(location):
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const HMODULE wux(GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll"));
    if (!wux) [[unlikely]]
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const auto ixde = reinterpret_cast<PFN_INITIALIZE_XAML_DIAGNOSTICS_EX>(GetProcAddress(wux, "InitializeXamlDiagnosticsEx"));
    if (!ixde) [[unlikely]]
    {
        auto error=HRESULT_FROM_WIN32(GetLastError());
        
        return error;
    }

    // I didn't find a better way than trying many connections until one works.
    // Reference:
    // https://github.com/microsoft/microsoft-ui-xaml/blob/d74a0332cf0d5e58f12eddce1070fa7a79b4c2db/src/dxaml/xcp/dxaml/lib/DXamlCore.cpp#L2782
    HRESULT hr;
    for (int i = 0; i < 10000; i++)
    {
        if(g_stopping) {  return E_ABORT; }
        WCHAR connectionName[256];
        wsprintf(connectionName, L"WinUIVisualDiagConnection%d", i + 1);

        hr = ixde(connectionName, GetCurrentProcessId(), L"", location, CLSID_WindhawkTAP, nullptr);
        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            break;
        }
    }

    
    return hr;
}


using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);

bool RunFromWindowThread(HWND hWnd,
                         RunFromWindowThreadProc_t proc,
                         PVOID procParam) {
    static const UINT runFromWindowThreadRegisteredMsg =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct RUN_FROM_WINDOW_THREAD_PARAM {
        RunFromWindowThreadProc_t proc;
        PVOID procParam;
    };

    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (dwThreadId == 0) {
        return false;
    }

    if (dwThreadId == GetCurrentThreadId()) {
        proc(procParam);
        return true;
    }

    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HC_ACTION) {
                const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
                if (cwp->message == runFromWindowThreadRegisteredMsg) {
                    RUN_FROM_WINDOW_THREAD_PARAM* param =
                        (RUN_FROM_WINDOW_THREAD_PARAM*)cwp->lParam;
                    param->proc(param->procParam);
                }
            }

            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, dwThreadId);
    if (!hook) {
        return false;
    }

    RUN_FROM_WINDOW_THREAD_PARAM param;
    param.proc = proc;
    param.procParam = procParam;
    SendMessage(hWnd, runFromWindowThreadRegisteredMsg, 0, (LPARAM)&param);

    UnhookWindowsHookEx(hook);

    return true;
}



BOOL Wh_ModInit() {
    auto read=[](PCWSTR key) {
        auto value=Wh_GetStringSetting(key);
        std::wstring result=value; Wh_FreeStringSetting(value); return result;
    };
    g_editInPaint=Wh_GetIntSetting(L"photosEditInPaint");
    g_closeAfterEdit=read(L"photosAfterEdit")==L"close";
    g_addToOpenWith=Wh_GetIntSetting(L"photosAddToOpenWith");
    wchar_t system[MAX_PATH];
    if(!GetSystemDirectoryW(system,ARRAYSIZE(system))) return FALSE;
    g_paintExe=std::wstring(system)+L"\\mspaint.exe";
    if(GetFileAttributesW(g_paintExe.c_str())==INVALID_FILE_ATTRIBUTES) g_paintExe=L"mspaint.exe";
    FindPaintIcon();
    g_stopEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);
    return g_stopEvent!=nullptr;
}
void Wh_ModAfterInit() {
    g_worker=CreateThread(nullptr,0,[](void*)->DWORD {
        for(int i=0;i<120 && WaitForSingleObject(g_stopEvent,500)==WAIT_TIMEOUT;i++) {
            if(!GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll")) continue;
            HRESULT hr=InjectWindhawkTAP();
            Wh_Log(L"Photos XAML connect: %08X",hr);
            if(SUCCEEDED(hr)) break;
        }
        return 0;
    },nullptr,0,nullptr);
}
void Wh_ModUninit() {
    g_stopping=true;
    SetEvent(g_stopEvent);
    if(g_worker) {WaitForSingleObject(g_worker,INFINITE);CloseHandle(g_worker);}
    if(g_visualTreeWatcher) {g_visualTreeWatcher->UnadviseVisualTreeChange();g_visualTreeWatcher=nullptr;}
    EnumWindows([](HWND window,LPARAM)->BOOL {
        DWORD pid;GetWindowThreadProcessId(window,&pid);
        if(pid==GetCurrentProcessId()) RunFromWindowThread(window,CleanupUi,nullptr);
        return TRUE;
    },0);
    CloseHandle(g_stopEvent);
}
BOOL Wh_ModSettingsChanged(BOOL* reload) { *reload=TRUE; return TRUE; }

} // namespace PhotosMod

// Only the selected application's implementation may initialize hooks or own UI.
enum class TargetApp { Unsupported, Paint, Photos };
static TargetApp g_targetApp = TargetApp::Unsupported;
BOOL Wh_ModInit() {
    wchar_t path[32768];
    DWORD size = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!size || size >= ARRAYSIZE(path)) return FALSE;
    auto name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    if (_wcsicmp(name, L"mspaint.exe") == 0) {
        g_targetApp = TargetApp::Paint;
        return PaintMod::Wh_ModInit();
    }
    if (_wcsicmp(name, L"Photos.exe") == 0) {
        g_targetApp = TargetApp::Photos;
        return PhotosMod::Wh_ModInit();
    }
    return FALSE;
}
void Wh_ModAfterInit() {
    if (g_targetApp == TargetApp::Paint) PaintMod::Wh_ModAfterInit();
    else if (g_targetApp == TargetApp::Photos) PhotosMod::Wh_ModAfterInit();
}
void Wh_ModUninit() {
    if (g_targetApp == TargetApp::Paint) PaintMod::Wh_ModUninit();
    else if (g_targetApp == TargetApp::Photos) PhotosMod::Wh_ModUninit();
}
BOOL Wh_ModSettingsChanged(BOOL* reload) { *reload = TRUE; return TRUE; }
__declspec(dllexport) HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void** object) {
    if (!object) return E_POINTER;
    *object = nullptr;
    if (g_targetApp == TargetApp::Paint) return PaintMod::GetClassObject(clsid, iid, object);
    if (g_targetApp == TargetApp::Photos) return PhotosMod::GetClassObject(clsid, iid, object);
    return CLASS_E_CLASSNOTAVAILABLE;
}
__declspec(dllexport) HRESULT WINAPI DllCanUnloadNow() {
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}
