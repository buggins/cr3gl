#ifndef CRUISETTINGSWIDGET_CPP
#define CRUISETTINGSWIDGET_CPP

#include "cruiwindow.h"
#include "cruilist.h"
#include "cruisettings.h"

class CRUISettingsList;
class CRUISettingsOptionList;
class CRUIOptionItem;

class CRUISettingsItem {
    lString8 _nameRes; // settings name resource
    lString8 _descriptionRes; // settings name resource
    lString8 _settingId;
public:
    CRUISettingsItem(const char * nameRes, const char * descriptionRes, const char * settingId)
        : _nameRes(nameRes), _descriptionRes(descriptionRes), _settingId(settingId) {}
    virtual ~CRUISettingsItem() {}
    virtual lString8 getSettingId() const { return _settingId; }
    virtual lString16 getName() const;
    virtual lString8 getValue(CRPropRef props) const { CR_UNUSED(props); return lString8(); }
    virtual lString16 getDescription(CRPropRef props) const;
    virtual int childCount() const { return 0; }
    virtual CRUISettingsItem * getChild(int index) const { CR_UNUSED(index); return NULL; }
    /// no-rtti workaround for dynamic_cast<CRUISettingsList *>
    virtual CRUISettingsList * asList() { return NULL; }
    /// no-rtti workaround for dynamic_cast<CRUISettingsOptionList *>
    virtual CRUISettingsOptionList * asOptionList() { return NULL; }
    //virtual CRUISettingsItem * findSetting(lString8 name);


    virtual int getOptionCount() const { return 0; }
    virtual const CRUIOptionItem * getOption(int index) const { CR_UNUSED(index); return NULL; }
    virtual const CRUIOptionItem * findOption(lString8 value) const { CR_UNUSED(value); return NULL; }
    //virtual const CRUIOptionItem * getDefaultOption() const { return NULL; }
    virtual const CRUIOptionItem * getSelectedOption(CRPropRef props) const { CR_UNUSED(props); return NULL; }
    virtual bool isToggle() const { return false; }
    virtual bool isChecked(CRPropRef props) const { CR_UNUSED(props); return false; }
    virtual void toggle(CRPropRef props) const { CR_UNUSED(props); }

};

class CRUISettingsList : public CRUISettingsItem {
protected:
    LVPtrVector<CRUISettingsItem> _list;
public:
    CRUISettingsList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {

    }
    virtual void addChild(CRUISettingsItem * child) {
        _list.add(child);
    }
    virtual ~CRUISettingsList() {}
    virtual int childCount() const { return _list.length(); }
    virtual CRUISettingsItem * getChild(int index) const { return _list[index]; }
    /// no-rtti workaround for dynamic_cast<CRUISettingsList *>
    virtual CRUISettingsList * asList();
};

class CRUISettingsCheckbox : public CRUISettingsItem {
protected:
    lString8 _checkedDescriptionRes;
    lString8 _uncheckedDescriptionRes;
public:
    CRUISettingsCheckbox(const char * nameRes, const char * descriptionRes, const char * settingId, const char * checkedRes = NULL, const char * uncheckedRes = NULL)
        : CRUISettingsItem(nameRes, descriptionRes, settingId)
        , _checkedDescriptionRes(checkedRes)
        , _uncheckedDescriptionRes(uncheckedRes)
    {

    }
    virtual ~CRUISettingsCheckbox() {}
    virtual bool isToggle() const { return true; }
    virtual void toggle(CRPropRef props) const;
    virtual bool isChecked(CRPropRef props) const;
    virtual lString16 getDescription(CRPropRef props) const;
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
    //virtual const CRUIOptionItem * getDefaultOption() const;
    virtual const CRUIOptionItem * getSelectedOption(CRPropRef props) const;
    virtual void addOption(CRUIOptionItem * option) { _list.add(option); }
    virtual void clearOptions() { _list.clear(); }
    /// no-rtti workaround for dynamic_cast<CRUISettingsOptionList *>
    virtual CRUISettingsOptionList * asOptionList() { return this; }
    virtual lString8 getValue(CRPropRef props) const;
    virtual lString16 getDescription(CRPropRef props) const;
    CRUISettingsOptionList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {

    }
    virtual ~CRUISettingsOptionList() {}
};

class CRUISettingsEditor : public CRUIVerticalLayout {
protected:
    CRPropRef _props;
    CRUISettingsItem * _settings;
public:
    CRUISettingsEditor(CRPropRef props, CRUISettingsItem * setting) : _props(props), _settings(setting) {}
};

class CRUIOptionListItemWidget;
class CRUISettingsOptionsListEditorWidget : public CRUISettingsEditor, public CRUIListAdapter, public CRUIOnListItemClickListener {
    CRUIOptionListItemWidget * _optionListItem; // child is setting with list of possible options
    CRUIListWidget * _list;
    lString8 _currentValue;
    CRUIOnListItemClickListener * _onItemClickListener;
public:
    CRUISettingsOptionsListEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual void setOnItemClickListener(CRUIOnListItemClickListener * listener) { _onItemClickListener = listener; }
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};

class CRUISettingsListItemWidget;
class CRUISettingsValueListItemWidget;
class CRUISettingsCheckboxWidget;
class CRUISettingsListWidget : public CRUISettingsEditor, public CRUIListAdapter, public CRUIOnListItemClickListener {
    CRUIListWidget * _list;
    CRUIOnListItemClickListener * _onItemClickListener;
    CRUISettingsListItemWidget * _settingsListItem; // child setting list widget
    CRUISettingsValueListItemWidget * _optionListItem; // child is setting with list of possible options
    CRUISettingsCheckboxWidget * _checkboxListItem; // checkbox
public:
    CRUISettingsListWidget(CRPropRef props, CRUISettingsItem * settings);
    virtual void setOnItemClickListener(CRUIOnListItemClickListener * listener) { _onItemClickListener = listener; }
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual ~CRUISettingsListWidget() {}
};

class CRUISettingsWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnListItemClickListener {
    CRUISettingsItem * _settings;
    CRUITitleBarWidget * _titlebar;
public:
    CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItem * settings);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};


#endif // CRUISETTINGSWIDGET_CPP
