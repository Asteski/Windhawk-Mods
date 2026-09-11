#include <array>
#include <string>
#include <cassert>
#include <cstdio>
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
namespace wux { enum class ElementTheme { Default, Light, Dark }; }
enum class GifPlaybackMode {
    Always,
    Hover,
    Pressed,
    Stopped,
};

enum class GifAnimationStatus {
    Unknown,
    NotAnimated,
    Playing,
    Stopped,
    SystemAnimationsDisabled,
};

enum class IconState : size_t {
    Normal,
    Hover,
    Pressed,
    Activated,
    Count,
};

constexpr size_t kIconStateCount = static_cast<size_t>(IconState::Count);

struct ModSettings {
    std::wstring imageSource;

    std::wstring hoverImageSource;
    std::wstring pressedImageSource;
    std::wstring activatedImageSource;

    std::array<std::wstring, kIconStateCount> lightThemeImages;
    std::array<std::wstring, kIconStateCount> darkThemeImages;

    int hoverFadeDuration = 120;
    int pressedFadeDuration = 80;

    int iconSize = 34;

    GifPlaybackMode gifPlayback = GifPlaybackMode::Hover;

    double hoverScale = 1.15;
    double hoverRotation = 4.0;
    double hoverOpacity = 1.0;
    int hoverDuration = 120;

    double pressedScale = 0.95;
    double pressedRotation = -4.0;
    double pressedOpacity = 1.0;
    int pressedDuration = 80;

    int releaseDuration = 140;

    bool respectSystemAnimations = true;
    bool showImageLoadFailureWarnings = true;
};


bool IsBlank(const std::wstring& value) {
    return value.find_first_not_of(L" \t\r\n") == std::wstring::npos;
}
void ApplyThemeImageSources(ModSettings& settings, wux::ElementTheme theme) {
    const auto& sources = theme == wux::ElementTheme::Light
                              ? settings.lightThemeImages
                              : settings.darkThemeImages;
    std::wstring* targets[] = {
        &settings.imageSource, &settings.hoverImageSource,
        &settings.pressedImageSource, &settings.activatedImageSource,
    };
    static_assert(ARRAYSIZE(targets) == kIconStateCount);
    for (size_t i = 0; i < kIconStateCount; i++) {
        if (!IsBlank(sources[i])) {
            *targets[i] = sources[i];
        }
    }
}
int main() {
    ModSettings base;
    base.imageSource = L"base-normal";
    base.hoverImageSource = L"base-hover";
    base.pressedImageSource = L"base-pressed";
    base.activatedImageSource = L"base-activated";
    auto unchanged = base;
    ApplyThemeImageSources(unchanged, wux::ElementTheme::Light);
    assert(unchanged.imageSource == base.imageSource);
    assert(unchanged.hoverImageSource == base.hoverImageSource);
    assert(unchanged.pressedImageSource == base.pressedImageSource);
    assert(unchanged.activatedImageSource == base.activatedImageSource);
    base.lightThemeImages = {L"light-normal", L" \t\r\n", L"light-pressed", L""};
    base.lightThemeImages[1] = L" \t\r\n";
    // Actual whitespace, rather than a configured path containing backslashes.
    base.lightThemeImages[1] = std::wstring{L' ', L'\t', L'\r', L'\n'};
    base.darkThemeImages = {L"dark-normal", L"dark-hover", L"dark-pressed", L"dark-activated"};
    auto light = base;
    ApplyThemeImageSources(light, wux::ElementTheme::Light);
    assert(light.imageSource == L"light-normal");
    assert(light.hoverImageSource == L"base-hover");
    assert(light.pressedImageSource == L"light-pressed");
    assert(light.activatedImageSource == L"base-activated");
    auto dark = base;
    ApplyThemeImageSources(dark, wux::ElementTheme::Dark);
    assert(dark.imageSource == L"dark-normal");
    assert(dark.hoverImageSource == L"dark-hover");
    assert(dark.pressedImageSource == L"dark-pressed");
    assert(dark.activatedImageSource == L"dark-activated");
    auto lightAgain = base;
    ApplyThemeImageSources(lightAgain, wux::ElementTheme::Light);
    assert(lightAgain.imageSource == light.imageSource);
    assert(lightAgain.activatedImageSource == light.activatedImageSource);
    assert(base.imageSource == L"base-normal");
    puts("Theme selection and fallback checks passed");
}