#ifndef CRUISETTINGSWIDGET_CPP
#define CRUISETTINGSWIDGET_CPP

#include "cruiwindow.h"
#include "cruilist.h"

class CRUISettingsItemBase {
    lString8 _nameRes; // settings name resource
    lString8 _descriptionRes; // settings name resource
    lString8 _settingId;
public:
    CRUISettingsItemBase(const char * nameRes, const char * descriptionRes, const char * settingId)
        : _nameRes(nameRes), _descriptionRes(descriptionRes), _settingId(settingId) {}
    virtual ~CRUISettingsItemBase() {}
    virtual lString8 getSettingId() const { return _settingId; }
    virtual lString16 getName() const;
    virtual lString16 getDescription() const;
    virtual int childCount() const { return 0; }
    virtual CRUISettingsItemBase * getChild(int index) const { CR_UNUSED(index); return NULL; }
    /// returns true if it's list of subsettings, false if single setting
    virtual bool isList() const { return false; }
    virtual CRUISettingsItemBase * findSetting(lString8 name);
};

class CRUISettingsList : public CRUISettingsItemBase {
protected:
    LVPtrVector<CRUISettingsItemBase> _list;
public:
    CRUISettingsList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItemBase(nameRes, descriptionRes, settingId) {

    }
    virtual void addChild(CRUISettingsItemBase * child) {
        _list.add(child);
    }
    virtual ~CRUISettingsList() {}
    virtual int childCount() const { return _list.length(); }
    virtual CRUISettingsItemBase * getChild(int index) const { return _list[index]; }
    /// returns true if it's list of subsettings, false if single setting
    virtual bool isList() const { return true; }
};

class CRUISettingsItem : public CRUISettingsItemBase {
protected:
    lString8 _defaultValue;
    lString8 _value;
public:
    CRUISettingsItem(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItemBase(nameRes, descriptionRes, settingId) {

    }
    virtual ~CRUISettingsItem() {}
    virtual void setDefaultValue(lString8 defaultValue) { _defaultValue = defaultValue; }
    virtual void setDefaultValue(const char * defaultValue) { _defaultValue = defaultValue; }
    virtual lString8 getDefaultValue() { return _defaultValue; }
    virtual lString8 getValue() { return _value; }
    virtual void setValue(lString8 value) { _value = value; }
};

/// option item for option list setting
class CRUIOptionItem {
    lString8 _value;
    lString8 _nameRes;
    lString16 _name;
public:
    CRUIOptionItem(lString8 value, lString8 nameRes) : _value(value), _nameRes(nameRes) {}
    CRUIOptionItem(const char * value, const char * nameRes) : _value(value), _nameRes(nameRes) {}
    CRUIOptionItem(lString8 value, lString16 name) : _value(value), _name(name) {}
    lString16 getName() const;
    const lString8 & getValue() const { return _value; }
};

class CRUISettingsOptionList : public CRUISettingsItem {
protected:
    LVPtrVector<CRUIOptionItem> _list;
public:
    virtual int getOptionCount() const { return _list.length(); }
    virtual const CRUIOptionItem * getOption(int index) const { return _list[index]; }
    virtual const CRUIOptionItem * findOption(lString8 value) const;
    virtual const CRUIOptionItem * getDefaultOption() const;
    virtual const CRUIOptionItem * getSelectedOption() const;
    virtual void addOption(CRUIOptionItem * option) { _list.add(option); }
    virtual void clearOptions() { _list.clear(); }
    CRUISettingsOptionList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {

    }
    virtual ~CRUISettingsOptionList() {}
};

class CRUISettingsListItemWidget : public CRUIHorizontalLayout {
    CRUISettingsList * _settings;
    CRUITextWidget * _title;
    CRUITextWidget * _description;
    CRUIImageWidget * _righticon;
public:
    CRUISettingsListItemWidget();
    void setSetting(CRUISettingsList * settings);
};

class CRUISettingsValueListItemWidget : public CRUIHorizontalLayout {
    CRUISettingsItemBase * _settings;
    CRUITextWidget * _title;
    CRUITextWidget * _description;
    CRUIImageWidget * _righticon;
public:
    CRUISettingsValueListItemWidget();
    void setSetting(CRUISettingsItemBase * _settings);
};

class CRUISettingsListWidget : public CRUIListWidget, public CRUIListAdapter {
    CRUISettingsList * _settings;
    CRUISettingsListItemWidget * _settingsListItem; // child setting list widget
    CRUISettingsValueListItemWidget * _optionListItem; // child is setting with list of possible options
public:
    CRUISettingsListWidget(CRUISettingsList * settings);
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual ~CRUISettingsListWidget() {}
};

class CRUISettingsWidget : public CRUIWindowWidget, public CRUIOnClickListener {
    CRUISettingsList * _settings;
    CRUITitleBarWidget * _titlebar;
    CRUISettingsListWidget * _list;
public:
    CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsList * settings);
    virtual bool onClick(CRUIWidget * widget);
};

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
#define PROP_FONT_COLOR_DAY        "font.color.day"
#define PROP_FONT_COLOR_NIGHT      "font.color.night"
#define PROP_FONT_COLOR "font.color.default"
#define PROP_BACKGROUND_COLOR_DAY "background.color.day"
#define PROP_BACKGROUND_COLOR_NIGHT "background.color.night"

#define PROP_FONT_ANTIALIASING       "font.antialiasing.mode"
#define PROP_FONT_FACE               "font.face.default"
#define PROP_FONT_HINTING            "font.hinting.mode"
#define PROP_FONT_GAMMA              "font.gamma"
#define PROP_FONT_GAMMA_DAY          "font.gamma.day"
#define PROP_FONT_GAMMA_NIGHT        "font.gamma.night"

#endif // CRUISETTINGSWIDGET_CPP
