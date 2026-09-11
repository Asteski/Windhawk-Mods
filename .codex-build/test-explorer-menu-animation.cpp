#include "../mods/explorer-context-menu-animation.wh.cpp"
#include <cassert>
#include <cstdio>

struct FakeFlyout {
    void** vtable;
    ULONG refs = 1;
    bool enabled = false;
    bool failSet = false;
    unsigned sets = 0;
};
ULONG WINAPI AddRefFake(FakeFlyout* f) { return ++f->refs; }
ULONG WINAPI ReleaseFake(FakeFlyout* f) { return --f->refs; }
HRESULT WINAPI GetEnabledFake(FakeFlyout* f, bool* result) {
    *result = f->enabled;
    return S_OK;
}
HRESULT WINAPI SetEnabledFake(FakeFlyout* f, bool value) {
    ++f->sets;
    if (f->failSet) return E_FAIL;
    f->enabled = value;
    return S_OK;
}
void* expectedStorage;
void* expectedHost;
const POINT* expectedPoint;
unsigned calls;
void OriginalFake(void* host, void* storage, const POINT* point, unsigned flags) {
    assert(host == expectedHost && storage == expectedStorage);
    assert(point == expectedPoint && flags == 0x12345678);
    if (storage && *static_cast<void**>(storage)) {
        auto f = *static_cast<FakeFlyout**>(storage);
        assert(f->refs == 1);
    }
    ++calls;
}
int main() {
    void* vtable[20]{};
    vtable[1] = reinterpret_cast<void*>(AddRefFake);
    vtable[2] = reinterpret_cast<void*>(ReleaseFake);
    vtable[18] = reinterpret_cast<void*>(GetEnabledFake);
    vtable[19] = reinterpret_cast<void*>(SetEnabledFake);
    FakeFlyout f{vtable};
    FakeFlyout* storage = &f;
    POINT point{100,200};
    expectedHost = &point;
    expectedPoint = &point;
    expectedStorage = &storage;
    g_showXamlFlyoutNow = OriginalFake;
    ShowXamlFlyoutNow_Hook(expectedHost, expectedStorage, &point, 0x12345678);
    assert(f.enabled && f.sets == 1 && calls == 1);
    ShowXamlFlyoutNow_Hook(expectedHost, expectedStorage, &point, 0x12345678);
    assert(f.enabled && f.sets == 1 && calls == 2);
    f.enabled = false;
    f.failSet = true;
    ShowXamlFlyoutNow_Hook(expectedHost, expectedStorage, &point, 0x12345678);
    assert(!f.enabled && f.sets == 2 && calls == 3);
    storage = nullptr;
    ShowXamlFlyoutNow_Hook(expectedHost, expectedStorage, &point, 0x12345678);
    assert(calls == 4);
    expectedStorage = nullptr;
    ShowXamlFlyoutNow_Hook(expectedHost, expectedStorage, &point, 0x12345678);
    assert(calls == 5);
    puts("PASS: enable, already-enabled, setter failure, null object, null storage; arguments and reference count preserved");
}
