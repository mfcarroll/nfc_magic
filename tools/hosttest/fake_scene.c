#include "fake_scene.h"

#include <gui/view_dispatcher.h>
#include <nfc_magic_dev_icons.h>

#include <stdarg.h>

FakeScene fake_scene;

// Sentinel objects. The scenes only ever compare or pass the addresses.
const NotificationSequence sequence_success = {0};
const NotificationSequence sequence_error = {0};
const Icon I_WarningDolphinFlip_45x42;

void fake_scene_reset(uint32_t scene_state) {
    memset(&fake_scene, 0, sizeof(fake_scene));
    fake_scene.scene_state = scene_state;
}

static FakeElement* push_element(FakeElementKind kind) {
    // Overflow is a test bug, not something to paper over: a scene drawing more than this either
    // changed shape or is looping.
    furi_check(fake_scene.element_count < FAKE_SCENE_MAX_ELEMENTS);
    FakeElement* e = &fake_scene.elements[fake_scene.element_count++];
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    return e;
}

static void set_text(FakeElement* e, const char* text) {
    // Truncation would silently weaken an assertion, so refuse instead.
    furi_check(text != NULL);
    furi_check(strlen(text) < sizeof(e->text));
    strcpy(e->text, text);
}

// ---- widget ---------------------------------------------------------------------------------------

void widget_reset(Widget* widget) {
    UNUSED(widget);
    fake_scene.widget_was_reset = true;
}

void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    UNUSED(widget);
    FakeElement* e = push_element(FakeElementString);
    e->x = x;
    e->y = y;
    e->horizontal = horizontal;
    e->vertical = vertical;
    e->font = font;
    set_text(e, text);
}

void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    UNUSED(widget);
    FakeElement* e = push_element(FakeElementMultiline);
    e->x = x;
    e->y = y;
    e->horizontal = horizontal;
    e->vertical = vertical;
    e->font = font;
    set_text(e, text);
}

void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text) {
    UNUSED(widget);
    UNUSED(width);
    UNUSED(height);
    FakeElement* e = push_element(FakeElementScroll);
    e->x = x;
    e->y = y;
    set_text(e, text);
}

void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context) {
    UNUSED(widget);
    UNUSED(context);
    FakeElement* e = push_element(FakeElementButton);
    e->button = button_type;
    e->callback = callback;
    set_text(e, text);
}

void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon) {
    UNUSED(widget);
    UNUSED(icon);
    FakeElement* e = push_element(FakeElementIcon);
    e->x = x;
    e->y = y;
}

// ---- notification ---------------------------------------------------------------------------------

void notification_message(NotificationApp* app, const NotificationSequence* sequence) {
    UNUSED(app);
    if(sequence == &sequence_success) fake_scene.played_success = true;
    if(sequence == &sequence_error) fake_scene.played_error = true;
}

// ---- scene manager --------------------------------------------------------------------------------

static void push_nav(FakeNavKind kind, uint32_t scene_id) {
    furi_check(fake_scene.nav_count < FAKE_SCENE_MAX_NAV);
    fake_scene.navs[fake_scene.nav_count].kind = kind;
    fake_scene.navs[fake_scene.nav_count].scene_id = scene_id;
    fake_scene.nav_count++;
}

uint32_t scene_manager_get_scene_state(const SceneManager* scene_manager, uint32_t scene_id) {
    UNUSED(scene_manager);
    UNUSED(scene_id);
    return fake_scene.scene_state;
}

void scene_manager_set_scene_state(SceneManager* scene_manager, uint32_t scene_id, uint32_t state) {
    UNUSED(scene_manager);
    UNUSED(scene_id);
    fake_scene.scene_state = state;
}

void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id) {
    UNUSED(scene_manager);
    push_nav(FakeNavNext, next_scene_id);
}

bool scene_manager_previous_scene(SceneManager* scene_manager) {
    UNUSED(scene_manager);
    push_nav(FakeNavPrevious, 0);
    return true;
}

bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    UNUSED(scene_manager);
    push_nav(FakeNavSearchPrevious, scene_id);
    return true;
}

bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id) {
    UNUSED(scene_manager);
    push_nav(FakeNavSearchAnother, scene_id);
    return true;
}

// ---- view dispatcher ------------------------------------------------------------------------------

void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event) {
    UNUSED(view_dispatcher);
    fake_scene.custom_event = event;
    fake_scene.custom_event_sent = true;
}

void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id) {
    UNUSED(view_dispatcher);
    UNUSED(view_id);
    fake_scene.switched_view = true;
}

// ---- queries --------------------------------------------------------------------------------------

const char* fake_scene_button(GuiButtonType which) {
    for(uint16_t i = 0; i < fake_scene.element_count; i++) {
        const FakeElement* e = &fake_scene.elements[i];
        if(e->kind == FakeElementButton && e->button == which) return e->text;
    }
    return NULL;
}

static char joined[4096];

const char* fake_scene_all_text(void) {
    joined[0] = '\0';
    for(uint16_t i = 0; i < fake_scene.element_count; i++) {
        const FakeElement* e = &fake_scene.elements[i];
        if(e->kind == FakeElementIcon) continue;
        furi_check(strlen(joined) + strlen(e->text) + 2 < sizeof(joined));
        strcat(joined, e->text);
        strcat(joined, "\n");
    }
    return joined;
}

bool fake_scene_text_contains(const char* needle) {
    return strstr(fake_scene_all_text(), needle) != NULL;
}

const char* fake_scene_scroll_text(void) {
    for(uint16_t i = 0; i < fake_scene.element_count; i++) {
        if(fake_scene.elements[i].kind == FakeElementScroll) return fake_scene.elements[i].text;
    }
    return NULL;
}

bool fake_scene_press(GuiButtonType which, void* context) {
    for(uint16_t i = 0; i < fake_scene.element_count; i++) {
        FakeElement* e = &fake_scene.elements[i];
        if(e->kind == FakeElementButton && e->button == which) {
            if(e->callback) e->callback(which, InputTypeShort, context);
            return true;
        }
    }
    return false;
}

// ---- FuriString -----------------------------------------------------------------------------------
// Real behaviour, not a stub: every user-visible string in these scenes is assembled here.

struct FuriString {
    char* buf;
    size_t len;
    size_t cap;
};

static void fs_grow(FuriString* s, size_t need) {
    if(s->cap >= need) return;
    size_t cap = s->cap ? s->cap : 64;
    while(cap < need) cap *= 2;
    s->buf = realloc(s->buf, cap);
    furi_check(s->buf != NULL);
    s->cap = cap;
}

FuriString* furi_string_alloc(void) {
    FuriString* s = calloc(1, sizeof(FuriString));
    furi_check(s != NULL);
    fs_grow(s, 64);
    s->buf[0] = '\0';
    return s;
}

void furi_string_free(FuriString* s) {
    if(!s) return;
    free(s->buf);
    free(s);
}

static void fs_vappend(FuriString* s, const char* fmt, va_list ap) {
    va_list probe;
    va_copy(probe, ap);
    int n = vsnprintf(NULL, 0, fmt, probe);
    va_end(probe);
    furi_check(n >= 0);
    fs_grow(s, s->len + (size_t)n + 1);
    vsnprintf(s->buf + s->len, (size_t)n + 1, fmt, ap);
    s->len += (size_t)n;
}

void furi_string_printf(FuriString* s, const char* fmt, ...) {
    s->len = 0;
    s->buf[0] = '\0';
    va_list ap;
    va_start(ap, fmt);
    fs_vappend(s, fmt, ap);
    va_end(ap);
}

void furi_string_cat_printf(FuriString* s, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fs_vappend(s, fmt, ap);
    va_end(ap);
}

void furi_string_cat_str(FuriString* s, const char* str) {
    furi_string_cat_printf(s, "%s", str);
}

void furi_string_set_str(FuriString* s, const char* str) {
    furi_string_printf(s, "%s", str);
}

void furi_string_push_back(FuriString* s, char c) {
    fs_grow(s, s->len + 2);
    s->buf[s->len++] = c;
    s->buf[s->len] = '\0';
}

const char* furi_string_get_cstr(const FuriString* s) {
    return s->buf;
}

size_t furi_string_size(const FuriString* s) {
    return s->len;
}
