#ifndef CRUISETTINGSWIDGET_CPP
#define CRUISETTINGSWIDGET_CPP

#include "cruiwindow.h"

class CRUISettingsItemBase {
    lString8 _nameRes; // settings name resource
    lString8 _descriptionRes; // settings name resource
    lString8 _settingId;
public:
    CRUISettingsItemBase(const char * nameRes, const char * descriptionRes, const char * settingId)
        : _nameRes(nameRes), _descriptionRes(descriptionRes), _settingId(settingId) {}
    virtual ~CRUISettingsItemBase() {}
    virtual lString8 getSettingId() const { return _settingId; }
    virtual int childCount() const { return 0; }
    virtual CRUISettingsItemBase * getChild(int index) const { CR_UNUSED(index); return NULL; }
    /// returns true if it's list of subsettings, false if single setting
    virtual bool isList() const { return false; }
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


class CRUISettingsWidget : public CRUIWindowWidget {
public:
    CRUISettingsWidget(CRUIMainWidget * main);
};

#endif // CRUISETTINGSWIDGET_CPP
