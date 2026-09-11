// ==WindhawkMod==
// @id              start-menu-direction
// @name            Start Menu Direction
// @description     Sets the start menu appearance animation direction (bottom/top/left/right) for Old or New Start menu.
// @version         0.1.0
// @author          Asteski
// @include         explorer.exe
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- startMenuVersion: old
    $name: Start Menu Version
    $description: Apply direction to Old or New start menu
    $options:
    - old: Old
    - new: New
- direction: bottom
    $name: Start Menu Direction
    $description: Choose animation direction
    $options:
    - bottom: Bottom
    - top: Top
    - left: Left
    - right: Right
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <string>
#include <vector>

// Helper: set string setting if API available
static BOOL SetStringSettingIfAvailable(PCWSTR key, PCWSTR value) {
    typedef BOOL (WINAPI *PFN_Wh_SetStringSetting)(PCWSTR, PCWSTR);
    HMODULE h = GetModuleHandleW(L"windhawk64.dll");
    if (!h) {
        Wh_Log(L"Start Menu Direction: windhawk64.dll not found; cannot set setting %s", key);
        return FALSE;
    }
    FARPROC proc = GetProcAddress(h, "Wh_SetStringSetting");
    if (!proc) {
        Wh_Log(L"Start Menu Direction: Wh_SetStringSetting not available; key=%s", key);
        return FALSE;
    }
    PFN_Wh_SetStringSetting pfn = (PFN_Wh_SetStringSetting)proc;
    return pfn(key, value);
}

// Apply the given list of (key,value) pairs to Windhawk settings
static void ApplySettingsPairs(const std::vector<std::pair<std::wstring,std::wstring>>& pairs) {
    for (const auto &p : pairs) {
        const std::wstring &k = p.first;
        const std::wstring &v = p.second;
        Wh_Log(L"Start Menu Direction: setting %s = %s", k.c_str(), v.c_str());
        SetStringSettingIfAvailable(k.c_str(), v.c_str());
    }
}

// Data from the user's attachment (left/right/top). For "bottom" we clear controlStyles.
static std::vector<std::pair<std::wstring,std::wstring>> g_pairs_left = {
    {L"controlStyles[0].target", L"StartDocked.StartSizingFrame"},
    {L"controlStyles[0].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[0].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[1].target", L"StartDocked.LauncherFrame > Grid > Grid"},
    {L"controlStyles[1].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[1].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[2].target", L"Cortana.UI.Views.TaskbarSearchPage"},
    {L"controlStyles[2].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[2].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[3].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid"},
    {L"controlStyles[3].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[3].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[4].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid > Grid"},
    {L"controlStyles[4].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"0\" />"},
    {L"controlStyles[4].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[5].target", L"Grid#QueryFormulationRoot"},
    {L"controlStyles[5].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[5].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[6].target", L"Grid#QueryFormulationRoot > Cortana.UI.Views.QueryFormulationControl#QueryFormulation > Grid"},
    {L"controlStyles[6].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[6].styles[1]", L"RenderTransformOrigin=0.5,0.5"}
};

static std::vector<std::pair<std::wstring,std::wstring>> g_pairs_right = {
    {L"controlStyles[0].target", L"StartDocked.StartSizingFrame"},
    {L"controlStyles[0].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[0].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[1].target", L"StartDocked.LauncherFrame > Grid > Grid"},
    {L"controlStyles[1].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[1].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[2].target", L"Cortana.UI.Views.TaskbarSearchPage"},
    {L"controlStyles[2].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[2].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[3].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid"},
    {L"controlStyles[3].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[3].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[4].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid > Grid"},
    {L"controlStyles[4].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"0\" />"},
    {L"controlStyles[4].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[5].target", L"Grid#QueryFormulationRoot"},
    {L"controlStyles[5].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"270\" />"},
    {L"controlStyles[5].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[6].target", L"Grid#QueryFormulationRoot > Cortana.UI.Views.QueryFormulationControl#QueryFormulation > Grid"},
    {L"controlStyles[6].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"90\" />"},
    {L"controlStyles[6].styles[1]", L"RenderTransformOrigin=0.5,0.5"}
};

static std::vector<std::pair<std::wstring,std::wstring>> g_pairs_top = {
    {L"controlStyles[0].target", L"StartDocked.StartSizingFrame"},
    {L"controlStyles[0].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[0].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[1].target", L"StartDocked.LauncherFrame > Grid > Grid"},
    {L"controlStyles[1].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[1].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[2].target", L"Cortana.UI.Views.TaskbarSearchPage"},
    {L"controlStyles[2].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[2].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[3].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid"},
    {L"controlStyles[3].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[3].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[4].target", L"Cortana.UI.Views.TaskbarSearchPage > Grid > Grid"},
    {L"controlStyles[4].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[4].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[5].target", L"Grid#QueryFormulationRoot"},
    {L"controlStyles[5].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[5].styles[1]", L"RenderTransformOrigin=0.5,0.5"},
    {L"controlStyles[6].target", L"Grid#QueryFormulationRoot > Cortana.UI.Views.QueryFormulationControl#QueryFormulation > Grid"},
    {L"controlStyles[6].styles[0]", L"RenderTransform:=<RotateTransform Angle=\"180\" />"},
    {L"controlStyles[6].styles[1]", L"RenderTransformOrigin=0.5,0.5"}
};

// Clear entries (used for bottom/default)
static std::vector<std::pair<std::wstring,std::wstring>> g_pairs_clear = {
    {L"controlStyles[0].target", L""},
    {L"controlStyles[0].styles[0]", L""},
    {L"controlStyles[0].styles[1]", L""},
    {L"controlStyles[1].target", L""},
    {L"controlStyles[1].styles[0]", L""},
    {L"controlStyles[1].styles[1]", L""},
    {L"controlStyles[2].target", L""},
    {L"controlStyles[2].styles[0]", L""},
    {L"controlStyles[2].styles[1]", L""},
    {L"controlStyles[3].target", L""},
    {L"controlStyles[3].styles[0]", L""},
    {L"controlStyles[3].styles[1]", L""},
    {L"controlStyles[4].target", L""},
    {L"controlStyles[4].styles[0]", L""},
    {L"controlStyles[4].styles[1]", L""},
    {L"controlStyles[5].target", L""},
    {L"controlStyles[5].styles[0]", L""},
    {L"controlStyles[5].styles[1]", L""},
    {L"controlStyles[6].target", L""},
    {L"controlStyles[6].styles[0]", L""},
    {L"controlStyles[6].styles[1]", L""}
};

// Load settings and apply
static void ApplyChosenDirection() {
    PCWSTR ver = Wh_GetStringSetting(L"startMenuVersion");
    PCWSTR dir = Wh_GetStringSetting(L"direction");
    std::wstring version = ver && wcslen(ver) ? ver : L"old";
    std::wstring direction = dir && wcslen(dir) ? dir : L"bottom";

    if (ver) Wh_FreeStringSetting(ver);
    if (dir) Wh_FreeStringSetting(dir);

    Wh_Log(L"Start Menu Direction: applying version=%s direction=%s", version.c_str(), direction.c_str());

    // For now the same controlStyles are used regardless of "Old" or "New" selection.
    if (direction == L"left") {
        ApplySettingsPairs(g_pairs_left);
    } else if (direction == L"right") {
        ApplySettingsPairs(g_pairs_right);
    } else if (direction == L"top") {
        ApplySettingsPairs(g_pairs_top);
    } else {
        // bottom/default => clear custom controlStyles so Start uses default animation
        ApplySettingsPairs(g_pairs_clear);
    }
}

// Called by Windhawk when settings change
void Wh_ModSettingsChanged() {
    ApplyChosenDirection();
}

BOOL Wh_ModInit() {
    Wh_Log(L"Start Menu Direction: initializing");
    ApplyChosenDirection();
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Start Menu Direction: uninitializing - clearing custom styles");
    ApplySettingsPairs(g_pairs_clear);
}
