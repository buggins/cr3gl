#ifndef CRUISETTINGS_H
#define CRUISETTINGS_H

#include "lvdocviewprops.h"

// setting group paths
#define SETTINGS_PATH_READER "reader"
#define SETTINGS_PATH_BROWSER "browser"
#define SETTINGS_PATH_READER_FONTSANDCOLORS "reader/fontsandcolors"

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

//#define PROP_FONT_COLOR "font.color.default"
#define PROP_FONT_COLOR_DAY        "font.color.day"
#define PROP_FONT_COLOR_NIGHT      "font.color.night"
#define PROP_BACKGROUND_IMAGE_ENABLED "background.image.enabled"
#define PROP_BACKGROUND_IMAGE_ENABLED_DAY "background.image.enabled.day"
#define PROP_BACKGROUND_IMAGE_ENABLED_NIGHT "background.image.enabled.night"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS "background.image.correction.brightness"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_DAY "background.image.correction.brightness.day"
#define PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_NIGHT "background.image.correction.brightness.night"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST "background.image.correction.contrast"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_DAY "background.image.correction.contrast.day"
#define PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_NIGHT "background.image.correction.contrast.night"
#define PROP_BACKGROUND_COLOR_DAY "background.color.day"
#define PROP_BACKGROUND_COLOR_NIGHT "background.color.night"
#define PROP_BACKGROUND_IMAGE_DAY "background.image.filename.day"
#define PROP_BACKGROUND_IMAGE_NIGHT "background.image.filename.night"

//#define PROP_FONT_ANTIALIASING       "font.antialiasing.mode"
//#define PROP_FONT_FACE               "font.face.default"
//#define PROP_FONT_HINTING            "font.hinting.mode"
//#define PROP_FONT_GAMMA              "font.gamma"
#define PROP_FONT_GAMMA_DAY          "font.gamma.day"
#define PROP_FONT_GAMMA_NIGHT        "font.gamma.night"

#define PROP_APP_TAP_ZONE_ACTION_NORMAL "app.tapzone."
#define PROP_APP_TAP_ZONE_ACTION_LONG "app.tapzone.long."
#define PROP_APP_TAP_ZONE_ACTION_DOUBLE "app.tapzone.dbl."

#endif // CRUISETTINGS_H
