/*
 * classic_cursor — apply the cape embedded at build time (macos_15_cursors.cape) to the
 * macOS cursors, once, and exit.
 *
 * Cursors stay replaced until logout/reboot; there is no backup or restore.
 */
#include <ApplicationServices/ApplicationServices.h>
#include <ImageIO/ImageIO.h>
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#define MAX_FRAMES 24
#define MAX_SYSTEM_CURSORS 128
#define NAME_LEN 256

/* Private WindowServer cursor API (exported by CoreGraphics, no public header). */
extern int CGSMainConnectionID(void);
extern CGError CGSRegisterCursorWithImages(int cid, char *name, bool set_globally,
    bool instantly, CGSize size, CGPoint hotspot, unsigned long frame_count,
    CGFloat frame_duration, CFArrayRef images, int *seed);
extern char *CGSCursorNameForSystemCursor(int cursor_id);
extern void CGSSetDockCursorOverride(int cid, bool flag);

struct cursor_spec {
    unsigned long frame_count;
    CGFloat frame_duration;
    CGPoint hotspot;
    CGSize size;
    CFArrayRef images;
};

/*
 * embedded_cape — locate the cape plist linked into this binary.
 *
 * Output:
 *   new CFData wrapping the embedded bytes (caller releases), or NULL if
 *   the section is missing.
 */
static CFDataRef
embedded_cape(void)
{
    unsigned long size = 0;
    uint8_t *bytes = getsectiondata(&_mh_execute_header, "__DATA", "cape", &size);

    if (bytes == NULL) {
        return NULL;
    }
    return CFDataCreateWithBytesNoCopy(NULL, bytes, (CFIndex)size, kCFAllocatorNull);
}

/*
 * get_number — read a numeric value from a dictionary.
 *
 * Inputs:
 *   dict — dictionary to read
 *   key  — key name
 *
 * Output:
 *   the value as a double, or 0 if missing or not a number.
 */
static double
get_number(CFDictionaryRef dict, const char *key)
{
    CFStringRef cf_key = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
    CFTypeRef value = CFDictionaryGetValue(dict, cf_key);
    double result = 0;

    if (value != NULL && CFGetTypeID(value) == CFNumberGetTypeID()) {
        CFNumberGetValue(value, kCFNumberDoubleType, &result);
    }

    CFRelease(cf_key);
    return result;
}

/*
 * decode_images — turn the cape's Representations (TIFF/PNG data) into CGImages.
 *
 * Inputs:
 *   reps — CFArray of CFData
 *
 * Output:
 *   new CFArray of CGImageRef tagged sRGB (caller releases), or NULL if
 *   no representation could be decoded.
 */
static CFArrayRef
decode_images(CFArrayRef reps)
{
    CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CFMutableArrayRef images = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

    for (CFIndex i = 0; i < CFArrayGetCount(reps); i++) {
        CFTypeRef rep = CFArrayGetValueAtIndex(reps, i);
        if (CFGetTypeID(rep) != CFDataGetTypeID()) {
            continue;
        }

        CGImageSourceRef src = CGImageSourceCreateWithData(rep, NULL);
        CGImageRef img = src ? CGImageSourceCreateImageAtIndex(src, 0, NULL) : NULL;
        if (img != NULL) {
            /* Reinterpret (not convert) the pixels as sRGB, as Mousecape does. */
            CGImageRef tagged = CGImageCreateCopyWithColorSpace(img, srgb);
            CFArrayAppendValue(images, tagged != NULL ? tagged : img);
            if (tagged != NULL) {
                CFRelease(tagged);
            }
            CFRelease(img);
        }
        if (src != NULL) {
            CFRelease(src);
        }
    }

    CGColorSpaceRelease(srgb);
    if (CFArrayGetCount(images) == 0) {
        CFRelease(images);
        return NULL;
    }
    return images;
}

/*
 * register_cursor — register one cursor image set under one name.
 *
 * Inputs:
 *   name — cursor identifier
 *   spec — decoded cursor data
 *
 * Output:
 *   0 on success, -1 on failure.
 */
static int
register_cursor(const char *name, const struct cursor_spec *spec)
{
    char mutable_name[NAME_LEN];
    int seed = 0;

    snprintf(mutable_name, sizeof(mutable_name), "%s", name);
    CGError err = CGSRegisterCursorWithImages(CGSMainConnectionID(), mutable_name,
        true, true, spec->size, spec->hotspot, spec->frame_count,
        spec->frame_duration, spec->images, &seed);

    return err == kCGErrorSuccess ? 0 : -1;
}

/*
 * alias_family — pick the system-cursor name family a cursor also lives under.
 *
 * macOS 26 renamed the Arrow and I-beam cursors, so those are also registered
 * under every system cursor name containing the family substring.
 *
 * Inputs:
 *   name — cape cursor identifier
 *
 * Output:
 *   "arrow", "ibeam", or NULL if the cursor has no aliases.
 */
static const char *
alias_family(const char *name)
{
    if (strcmp(name, "com.apple.coregraphics.Arrow") == 0 ||
        strcmp(name, "com.apple.coregraphics.ArrowCtx") == 0) {
        return "arrow";
    }
    if (strcmp(name, "com.apple.coregraphics.IBeam") == 0 ||
        strcmp(name, "com.apple.coregraphics.IBeamXOR") == 0) {
        return "ibeam";
    }
    return NULL;
}

/*
 * apply_cursor — register one cape entry (and its aliases) with WindowServer.
 *
 * Inputs:
 *   name  — cursor identifier, e.g. "com.apple.coregraphics.Arrow"
 *   entry — the cape's dictionary for that cursor
 *
 * Output:
 *   0 on success, -1 if the entry is invalid or registration failed.
 */
static int
apply_cursor(const char *name, CFDictionaryRef entry)
{
    struct cursor_spec spec;
    CFStringRef reps_key = CFSTR("Representations");
    CFTypeRef reps = CFDictionaryGetValue(entry, reps_key);

    spec.frame_count = (unsigned long)get_number(entry, "FrameCount");
    spec.frame_duration = get_number(entry, "FrameDuration");
    spec.hotspot = CGPointMake(get_number(entry, "HotSpotX"), get_number(entry, "HotSpotY"));
    spec.size = CGSizeMake(get_number(entry, "PointsWide"), get_number(entry, "PointsHigh"));

    if (spec.frame_count < 1 || spec.frame_count > MAX_FRAMES ||
        reps == NULL || CFGetTypeID(reps) != CFArrayGetTypeID()) {
        return -1;
    }

    spec.images = decode_images(reps);
    if (spec.images == NULL) {
        return -1;
    }

    int rc = register_cursor(name, &spec);

    const char *family = alias_family(name);
    for (int id = 0; family != NULL && id < MAX_SYSTEM_CURSORS; id++) {
        const char *alias = CGSCursorNameForSystemCursor(id);
        if (alias != NULL && strcasestr(alias, family) != NULL &&
            register_cursor(alias, &spec) == 0) {
            rc = 0;
        }
    }

    /* Stop the Dock from overriding the cursor right after registration. */
    CGSSetDockCursorOverride(CGSMainConnectionID(), false);

    CFRelease(spec.images);
    return rc;
}

struct apply_state {
    int failures;
};

/*
 * apply_each — CFDictionaryApplier: apply one entry of the cape's Cursors dict.
 *
 * Inputs:
 *   key, value — cursor identifier and its dictionary
 *   ctx        — struct apply_state, failures is incremented on error
 *
 * Output:
 *   none.
 */
static void
apply_each(const void *key, const void *value, void *ctx)
{
    struct apply_state *state = ctx;
    char name[NAME_LEN];

    if (CFGetTypeID(key) != CFStringGetTypeID() ||
        CFGetTypeID(value) != CFDictionaryGetTypeID() ||
        !CFStringGetCString(key, name, sizeof(name), kCFStringEncodingUTF8) ||
        apply_cursor(name, value) != 0) {
        fprintf(stderr, "classic_cursor: failed to apply a cursor entry\n");
        state->failures++;
    }
}

/*
 * main — parse the embedded cape plist and apply every cursor in it.
 *
 * Output:
 *   0 if all cursors applied, 1 otherwise.
 */
int
main(void)
{
    CFDataRef data = embedded_cape();
    if (data == NULL) {
        fprintf(stderr, "classic_cursor: no embedded cape\n");
        return 1;
    }

    CFPropertyListRef plist = CFPropertyListCreateWithData(NULL, data,
        kCFPropertyListImmutable, NULL, NULL);
    CFRelease(data);

    CFTypeRef cursors = NULL;
    if (plist != NULL && CFGetTypeID(plist) == CFDictionaryGetTypeID()) {
        cursors = CFDictionaryGetValue(plist, CFSTR("Cursors"));
    }
    if (cursors == NULL || CFGetTypeID(cursors) != CFDictionaryGetTypeID()) {
        fprintf(stderr, "classic_cursor: embedded cape is not valid\n");
        return 1;
    }

    struct apply_state state = { 0 };
    CFDictionaryApplyFunction(cursors, apply_each, &state);
    CFRelease(plist);

    return state.failures == 0 ? 0 : 1;
}
