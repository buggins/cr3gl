#ifndef CRUISETTINGSWIDGET_CPP
#define CRUISETTINGSWIDGET_CPP

#include "cruiwindow.h"
#include "cruilist.h"
#include "cruisettings.h"

class CRUISettingsList;
class CRUISettingsOptionList;
class CRUIOptionItem;
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
    /// no-rtti workaround for dynamic_cast<CRUISettingsList *>
    virtual CRUISettingsList * asList() { return NULL; }
    /// no-rtti workaround for dynamic_cast<CRUISettingsOptionList *>
    virtual CRUISettingsOptionList * asOptionList() { return NULL; }
    //virtual CRUISettingsItemBase * findSetting(lString8 name);


    virtual int getOptionCount() const { return 0; }
    virtual const CRUIOptionItem * getOption(int index) const { CR_UNUSED(index); return NULL; }
    virtual const CRUIOptionItem * findOption(lString8 value) const { CR_UNUSED(value); return NULL; }
    virtual const CRUIOptionItem * getDefaultOption() const { return NULL; }
    virtual const CRUIOptionItem * getSelectedOption() const { return NULL; }

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
    /// no-rtti workaround for dynamic_cast<CRUISettingsList *>
    virtual CRUISettingsList * asList();
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
    /// no-rtti workaround for dynamic_cast<CRUISettingsOptionList *>
    virtual CRUISettingsOptionList * asOptionList() { return this; }
    CRUISettingsOptionList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {

    }
    virtual ~CRUISettingsOptionList() {}
};

class CRUISettingsEditor {
protected:
    CRPropRef _props;
    CRUISettingsItemBase * _settings;
public:
    CRUISettingsEditor(CRPropRef props, CRUISettingsItemBase * setting) : _props(props), _settings(setting) {}
};

class CRUISettingsEditorWidget : public CRUISettingsEditor, public CRUIVerticalLayout {
public:
    CRUISettingsEditorWidget(CRPropRef props, CRUISettingsItemBase * setting) : CRUISettingsEditor(props, setting) {}
};

class CRUIOptionListItemWidget;
class CRUISettingsOptionsListEditorWidget : public CRUISettingsEditor, public CRUIListWidget, public CRUIListAdapter {
    CRUIOptionListItemWidget * _optionListItem; // child is setting with list of possible options
public:
    CRUISettingsOptionsListEditorWidget(CRPropRef props, CRUISettingsItemBase * setting);
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
};

class CRUISettingsListItemWidget;
class CRUISettingsValueListItemWidget;
class CRUISettingsListWidget : public CRUISettingsEditor, public CRUIListWidget, public CRUIListAdapter {
    CRUISettingsListItemWidget * _settingsListItem; // child setting list widget
    CRUISettingsValueListItemWidget * _optionListItem; // child is setting with list of possible options
public:
    CRUISettingsListWidget(CRPropRef props, CRUISettingsItemBase * settings);
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual ~CRUISettingsListWidget() {}
};

class CRUISettingsWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnListItemClickListener {
    CRUISettingsItemBase * _settings;
    CRUITitleBarWidget * _titlebar;
public:
    CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItemBase * settings);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};


#endif // CRUISETTINGSWIDGET_CPP
