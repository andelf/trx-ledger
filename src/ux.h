#include <bagl.h>

// type, userid, x, y, w, h, str, rad, fill, fg, bg, fid, iid, txt, touchparams...

// UI elements
#define UI_BACKGROUND()                                                                        \
    {                                                                                          \
        {BAGL_RECTANGLE, 0, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, \
            NULL, NULL, NULL                                                                   \
    }
#define UI_ICON_LEFT(userid, glyph)                                                            \
    {                                                                                          \
        {BAGL_ICON, userid, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0, 0, glyph}, NULL, 0, 0, 0, NULL, \
            NULL, NULL                                                                         \
    }
#define UI_ICON_RIGHT(userid, glyph)                                                             \
    {                                                                                            \
        {BAGL_ICON, userid, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0, 0, glyph}, NULL, 0, 0, 0, NULL, \
            NULL, NULL                                                                           \
    }

#define UI_TEXT(userid, x, y, w, text)                                  \
    {                                                                   \
        {BAGL_LABELINE,                                                 \
         userid,                                                        \
         x,                                                             \
         y,                                                             \
         w,                                                             \
         12,                                                            \
         0,                                                             \
         0,                                                             \
         0,                                                             \
         0xFFFFFF,                                                      \
         0,                                                             \
         BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, \
         0},                                                            \
            (char *)text, 0, 0, 0, NULL, NULL, NULL                     \
    }

#define UI_TITLE_TEXT(userid, text)                                     \
    {                                                                   \
        {BAGL_LABELINE,                                                 \
         userid,                                                        \
         0,                                                             \
         12,                                                            \
         128,                                                           \
         32,                                                            \
         0,                                                             \
         0,                                                             \
         0,                                                             \
         0xFFFFFF,                                                      \
         0x000000,                                                      \
         BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, \
         0},                                                            \
            (char *)text, 0, 0, 0, NULL, NULL, NULL                     \
    }

#define UI_CONTENT_TEXT(userid, text)                                     \
    {                                                                     \
        {BAGL_LABELINE,                                                   \
         userid,                                                          \
         23,                                                              \
         26,                                                              \
         82,                                                              \
         12,                                                              \
         0x80 | 10,                                                       \
         0,                                                               \
         0,                                                               \
         0xFFFFFF,                                                        \
         0x000000,                                                        \
         BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, \
         26},                                                             \
            (char *)text, 0, 0, 0, NULL, NULL, NULL                       \
    }
