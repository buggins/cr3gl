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

#define PROP_APP_TTS_VOICE "app.tts.voice"
#define PROP_APP_TTS_VOICE_VALUE_SYSTEM "@system"
#define PROP_APP_TTS_RATE "app.tts.rate"

#define PROP_APP_SCREEN_UPDATE_MODE "app.screen.update.mode"
#define PROP_APP_SCREEN_UPDATE_INTERVAL "app.screen.update.interval"

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


#endif // CRUISETTINGS_H
