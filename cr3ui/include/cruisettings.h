#ifndef CRUISETTINGS_H
#define CRUISETTINGS_H

#include "lvdocviewprops.h"

// setting group paths
#define SETTINGS_PATH_READER "reader"
#define SETTINGS_PATH_BROWSER "browser"
#define SETTINGS_PATH_READER_FONTSANDCOLORS "reader/fontsandcolors"
#define SETTINGS_PATH_READER_FONTRENDERING "reader/fontsandcolors/fontrendering"
#define SETTINGS_PATH_READER_TEXTFORMATTING "reader/fontrendering"
#define SETTINGS_PATH_READER_PAGELAYOUT "reader/fontrendering"
#define SETTINGS_PATH_READER_INTERFACE "reader/interface"
#define SETTINGS_PATH_CONTROLS "reader/controls"
#define SETTINGS_PATH_TAP_ZONES_NORMAL "reader/controls/tapzonesnormal"
#define SETTINGS_PATH_TAP_ZONES_DOUBLE "reader/controls/tapzonesdouble"

// properties
#define PROP_APP_THEME "app.ui.theme"
#define PROP_APP_THEME_VALUE_LIGHT "@light"
#define PROP_APP_THEME_VALUE_DARK "@dark"
#define PROP_APP_THEME_VALUE_WHITE "@white"
#define PROP_APP_THEME_VALUE_BLACK "@black"
#define PROP_APP_THEME_DAY "app.ui.theme.day"
#define PROP_APP_THEME_NIGHT "app.ui.theme.night"
#define PROP_APP_LOCALE "app.locale.name"
#define PROP_NIGHT_MODE "crengine.night.mode"
#define PROP_APP_INTERFACE_LANGUAGE "app.ui.lang"
#define PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM "@system"
#define PROP_APP_FULLSCREEN "app.fullscreen"

#define PROP_APP_SCREEN_ORIENTATION "app.screen.orientation"
#define PROP_APP_SCREEN_BACKLIGHT_TIMEOUT "app.screen.backlight.timeout"
#define PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS "app.screen.backlight.brightness"
#define PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS_DAY "app.screen.backlight.brightness.day"
#define PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS_NIGHT "app.screen.backlight.brightness.night"

#define PROP_APP_TTS_VOICE "app.tts.voice"
#define PROP_APP_TTS_VOICE_VALUE_SYSTEM "@system"
#define PROP_APP_TTS_RATE "app.tts.rate"

#define PROP_APP_READER_SHOW_SCROLLBAR "app.reader.scrollbar"
#define PROP_APP_READER_SHOW_SCROLLBAR_VALUE_ON "1"
#define PROP_APP_READER_SHOW_SCROLLBAR_VALUE_OFF "0"

#define PROP_APP_READER_SHOW_TOOLBAR "app.reader.toolbar"
#define PROP_APP_READER_SHOW_TOOLBAR_VALUE_OFF "0"
#define PROP_APP_READER_SHOW_TOOLBAR_VALUE_TOP "1"
#define PROP_APP_READER_SHOW_TOOLBAR_VALUE_LEFT "2"
#define PROP_APP_READER_SHOW_TOOLBAR_VALUE_SHORT_SIDE "3"
#define PROP_APP_READER_SHOW_TOOLBAR_VALUE_LONG_SIDE "4"

#define PROP_APP_BOOK_COVER_VISIBLE "app.reader.cover.visible"
#define PROP_APP_BOOK_COVER_VISIBLE_VALUE_OFF "0"
#define PROP_APP_BOOK_COVER_VISIBLE_VALUE_ON "1"
#define PROP_APP_BOOK_COVER_COLOR "app.reader.cover.color"

#define PROP_APP_SCREEN_UPDATE_MODE "app.screen.update.mode"
#define PROP_APP_SCREEN_UPDATE_INTERVAL "app.screen.update.interval"

#define PROP_APP_CONTROLS_VOLUME_KEYS "app.controls.volumekeys"

#define PROP_PAGE_VIEW_MODE_VALUE_SCROLL "0"
#define PROP_PAGE_VIEW_MODE_VALUE_1PAGE "1"
#define PROP_PAGE_VIEW_MODE_VALUE_2PAGES "2"

#define PROP_PAGE_VIEW_ANIMATION "crengine.page.view.animation"
#define PROP_PAGE_VIEW_ANIMATION_VALUE_NONE "0"
#define PROP_PAGE_VIEW_ANIMATION_VALUE_SLIDE1 "1"
#define PROP_PAGE_VIEW_ANIMATION_VALUE_SLIDE2 "2"
#define PROP_PAGE_VIEW_ANIMATION_VALUE_FADE "3"
#define PROP_PAGE_VIEW_ANIMATION_VALUE_3D "4"

//#define PROP_FONT_COLOR "font.color.default"
#define PROP_FONT_COLOR_DAY        "font.color.default.day"
#define PROP_FONT_COLOR_NIGHT      "font.color.default.night"
#define PROP_BACKGROUND_IMAGE_ENABLED "background.image.enabled"
#define PROP_BACKGROUND_IMAGE_ENABLED_DAY "background.image.enabled.day"
#define PROP_BACKGROUND_IMAGE_ENABLED_NIGHT "background.image.enabled.night"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS "background.image.correction.brightness"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_DAY "background.image.correction.brightness.day"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_NIGHT "background.image.correction.brightness.night"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST "background.image.correction.contrast"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_DAY "background.image.correction.contrast.day"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_NIGHT "background.image.correction.contrast.night"
#define PROP_BACKGROUND_COLOR_DAY "background.color.default.day"
#define PROP_BACKGROUND_COLOR_NIGHT "background.color.default.night"
#define PROP_BACKGROUND_IMAGE_DAY "background.image.filename.day"
#define PROP_BACKGROUND_IMAGE_NIGHT "background.image.filename.night"
#define PROP_APP_BOOK_COVER_COLOR_DAY "app.reader.cover.color.day"
#define PROP_APP_BOOK_COVER_COLOR_NIGHT "app.reader.cover.color.night"

#define PROP_PAGE_MARGINS "page.margins"

//#define PROP_FONT_ANTIALIASING       "font.antialiasing.mode"
//#define PROP_FONT_FACE               "font.face.default"
//#define PROP_FONT_HINTING            "font.hinting.mode"
//#define PROP_FONT_GAMMA              "font.gamma"
#define PROP_FONT_GAMMA_INDEX_DAY          "font.gamma.index.day"
#define PROP_FONT_GAMMA_INDEX_NIGHT        "font.gamma.index.night"

#define PROP_APP_TAP_ZONE_ACTION_NORMAL "app.tapzone."
#define PROP_APP_TAP_ZONE_ACTION_LONG "app.tapzone.long."
#define PROP_APP_TAP_ZONE_ACTION_DOUBLE "app.tapzone.dbl."

#define PROP_HIGHLIGHT_SELECTION_COLOR_DAY "crengine.highlight.selection.color.day"
#define PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT_DAY "crengine.highlight.bookmarks.color.comment.day"
#define PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION_DAY "crengine.highlight.bookmarks.color.correction.day"
#define PROP_HIGHLIGHT_SELECTION_COLOR_NIGHT "crengine.highlight.selection.color.night"
#define PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT_NIGHT "crengine.highlight.bookmarks.color.comment.night"
#define PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION_NIGHT "crengine.highlight.bookmarks.color.correction.night"

#define PROP_APP_WINDOW_STATE "app.window.state"
#define PROP_APP_WINDOW_X "app.window.x"
#define PROP_APP_WINDOW_Y "app.window.y"
#define PROP_APP_WINDOW_WIDTH "app.window.width"
#define PROP_APP_WINDOW_HEIGHT "app.window.height"

enum WindowState {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_FULLSCREEN
};

enum SCREEN_ORIENTATION {
    SCREEN_ORIENTATION_SYSTEM = 0,
    SCREEN_ORIENTATION_SENSOR = 1,
    SCREEN_ORIENTATION_0 = 2,
    SCREEN_ORIENTATION_90 = 3,
    SCREEN_ORIENTATION_180 = 4,
    SCREEN_ORIENTATION_270 = 5,
    SCREEN_ORIENTATION_PORTRAIT = 6,
    SCREEN_ORIENTATION_LANDSCAPE = 7,
};
#endif // CRUISETTINGS_H
