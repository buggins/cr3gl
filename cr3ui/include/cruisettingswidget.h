#ifndef CRUISETTINGSWIDGET_CPP
#define CRUISETTINGSWIDGET_CPP

#include "cruitheme.h"
#include "cruiwindow.h"
#include "cruilist.h"
#include "cruisettings.h"

class CRUISettingsList;
class CRUISettingsOptionList;
class CRUIOptionItem;
class CRUISettingsEditor;

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
    virtual CRUIImageRef getValueIcon(CRPropRef props) const;
    virtual bool fixedValueIconSize() const { return true; }
    virtual lString8 getValueIconRes(CRPropRef props) const;
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
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props) { CR_UNUSED(props); return NULL; }
    virtual bool hasCustomEditor() { return false; }
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
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
};

enum {
    TAPZONE_MODIFIER_NONE = 0,
    TAPZONE_MODIFIER_TWOFINGER = 1
};

class CRUITapZoneSettingsList : public CRUISettingsList {
    int modifier;
public:
    CRUITapZoneSettingsList(const char * nameRes, const char * descriptionRes, int modifier);
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
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
    virtual lString8 getValueIconRes(CRPropRef props) const;
    virtual bool fixedValueIconSize() const { return false; }
};

/// option item for option list setting
class CRUIOptionItem {
protected:
    lString8 _value;
    lString8 _nameRes;
    lString16 _name;
public:
    CRUIOptionItem(lString8 value, lString8 nameRes) : _value(value), _nameRes(nameRes) {}
    CRUIOptionItem(const char * value, const char * nameRes) : _value(value), _nameRes(nameRes) {}
    CRUIOptionItem(lString8 value, lString16 name) : _value(value), _name(name) {}
    virtual lString16 getName() const;
    virtual const lString8 & getValue() const { return _value; }
    virtual CRUIImageRef getRightImage() const { return CRUIImageRef(); }
    virtual int getRightImageSize() const { return 0; }
    virtual ~CRUIOptionItem() {}
};

/// option item for option list setting
class CRUITextureOptionItem : public CRUIOptionItem {
    CRUIBackgroundImageResource * _resource;
public:
    CRUITextureOptionItem(const CRUIBackgroundImageResource * item) : CRUIOptionItem(item->getId(), item->getName()), _resource(new CRUIBackgroundImageResource(*item)) {}
    virtual CRUIImageRef getRightImage() const;
    virtual int getRightImageSize() const;
    virtual lString16 getName() const;
    virtual ~CRUITextureOptionItem() { delete _resource; }
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
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    CRUISettingsOptionList(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {

    }
    virtual ~CRUISettingsOptionList() {}
};

class CRUIBackgroundTextureSetting : public CRUISettingsOptionList {
public:
    CRUIBackgroundTextureSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsOptionList(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual CRUIImageRef getValueIcon(CRPropRef props) const;
};

class CRUIFontFaceSetting : public CRUISettingsOptionList {
public:
    CRUIFontFaceSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsOptionList(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
};

class CRUIFontSizeSetting : public CRUISettingsItem {
public:
    CRUIFontSizeSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual lString16 getDescription(CRPropRef props) const;
};

class CRUIInterlineSpaceSetting : public CRUISettingsItem {
public:
	CRUIInterlineSpaceSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual lString16 getDescription(CRPropRef props) const;
};

class CRUIPageMarginsSetting : public CRUISettingsItem {
public:
	CRUIPageMarginsSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual lString16 getDescription(CRPropRef props) const;
};

class CRUIColorSetting : public CRUISettingsItem {
public:
    CRUIColorSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual lString16 getDescription(CRPropRef props) const;
    virtual CRUIImageRef getValueIcon(CRPropRef props) const;
};

class CRUIFontRenderingSetting : public CRUISettingsItem {
public:
    CRUIFontRenderingSetting(const char * nameRes, const char * descriptionRes, const char * settingId) : CRUISettingsItem(nameRes, descriptionRes, settingId) {
    }
    /// create editor widget based on option type
    virtual CRUISettingsEditor * createEditor(CRPropRef props);
    virtual bool hasCustomEditor() { return true; }
    virtual lString16 getDescription(CRPropRef props) const;
};

class CRUISettingsEditorCallback {
public:
    virtual void onSettingChange(CRUISettingsItem * newSubitem, bool done) = 0;
    virtual ~CRUISettingsEditorCallback() {}
};

class CRUISettingsEditor : public CRUIVerticalLayout {
protected:
    CRPropRef _props;
    CRUISettingsItem * _settings;
    CRUISettingsEditorCallback * _callback;
    CRUIVerticalLayout * _controls;
public:
    CRUISettingsEditor(CRPropRef props, CRUISettingsItem * setting) : _props(props), _settings(setting), _callback(NULL) {
        _controls = new CRUIVerticalLayout();
        _controls->setLayoutParams(CRUI::FILL_PARENT, CRUI::FILL_PARENT);
        addChild(_controls);
    }
    void addChildControl(CRUIWidget * widget) {
        _controls->addChild(widget);
    }

    virtual void setCallback(CRUISettingsEditorCallback * callback) { _callback = callback; }
    virtual void setOnDragListener(CRUIDragListener * listener) { CR_UNUSED(listener); }
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
};

class CRUISettingsListEditor : public CRUISettingsEditor, public CRUIListAdapter, public CRUIOnListItemClickListener {
protected:
    CRUIListWidget * _list;
public:
    CRUISettingsListEditor(CRPropRef props, CRUISettingsItem * setting);
    virtual void setOnDragListener(CRUIDragListener * listener) { _list->setOnDragListener(listener); }
    virtual ~CRUISettingsListEditor() {}
};

class CRUIOptionListItemWidget;
class CRUISettingsOptionsListEditorWidget : public CRUISettingsListEditor {
    CRUIOptionListItemWidget * _optionListItem; // child is setting with list of possible options
protected:
    lString8 _currentValue;
public:
    CRUISettingsOptionsListEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};

class CRUISettingsSampleWidget : public CRUIWidget {
protected:
    CRPropRef _props;
public:
    CRUISettingsSampleWidget(CRPropRef props) : _props(props) {}
};

class CRUIDocView;
class CRUIFontSampleWidget : public CRUISettingsSampleWidget {
protected:
    CRUIDocView * _docview;
    lUInt32 _lastPropsHash;
    lvPoint _lastSize;
    LVRendPageList _pageList;
public:
    CRUIFontSampleWidget(CRPropRef props);
    virtual ~CRUIFontSampleWidget();
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    void format();
};

class CRUIFontFaceEditorWidget : public CRUISettingsOptionsListEditorWidget {
protected:
    CRUIFontSampleWidget * _sample;
public:
    CRUIFontFaceEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};

class CRUIBackgroundTextureEditorWidget : public CRUISettingsOptionsListEditorWidget {
protected:
    CRUIFontSampleWidget * _sample;
public:
    CRUIBackgroundTextureEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
};

class CRUIFontSizeEditorWidget : public CRUISettingsEditor, public CRUIOnScrollPosCallback {
protected:
    CRUISliderWidget * _slider;
    CRUITextWidget * _sizetext;
    CRUIFontSampleWidget * _sample;
public:
    CRUIFontSizeEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
};

class CRUIInterlineSpaceEditorWidget : public CRUISettingsEditor, public CRUIOnScrollPosCallback {
protected:
    CRUISliderWidget * _slider;
    CRUITextWidget * _sizetext;
    CRUIFontSampleWidget * _sample;
public:
    CRUIInterlineSpaceEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);
};

class CRUIPageMarginsEditorWidget : public CRUISettingsEditor, public CRUIOnScrollPosCallback {
protected:
    CRUISliderWidget * _slider;
    CRUITextWidget * _sizetext;
    CRUIFontSampleWidget * _sample;
public:
    CRUIPageMarginsEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);
};

class CRUISettingsListItemWidget;
class CRUIColorEditorWidget : public CRUISettingsEditor, public CRUIOnScrollPosCallback, public CRUIOnClickListener {
protected:
    CRUISettingsCheckbox * _enableTextureSetting;
    CRUISettingsListItemWidget * _checkbox;
    CRUISliderWidget * _sliderR;
    CRUISliderWidget * _sliderG;
    CRUISliderWidget * _sliderB;
    CRUISliderWidget * _sliderRB;
    CRUISliderWidget * _sliderGB;
    CRUISliderWidget * _sliderBB;
    CRUISliderWidget * _sliderRC;
    CRUISliderWidget * _sliderGC;
    CRUISliderWidget * _sliderBC;
    CRUIVerticalLayout * _colorPane;
    CRUIVerticalLayout * _colorCorrectionPane;
    CRUIFontSampleWidget * _sample;
public:
    CRUIColorEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual ~CRUIColorEditorWidget() {
        if (_enableTextureSetting)
            delete _enableTextureSetting;
    }
    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);
    virtual bool onClick(CRUIWidget * widget);
    void updateMode();

};

class CRUIFontRenderingOptionsEditorWidget : public CRUISettingsEditor, public CRUIOnScrollPosCallback, public CRUIOnClickListener {
    CRUISettingsListItemWidget * _cbBold;
    CRUISettingsListItemWidget * _cbAntialiasing;
    CRUISettingsListItemWidget * _cbBytecodeInterpretor;
    CRUISettingsCheckbox * _settingBold;
    CRUISettingsCheckbox * _settingAntialiasing;
    CRUISettingsCheckbox * _settingBytecodeInterpretor;
    CRUISliderWidget * _sliderGamma;
    CRUIFontSampleWidget * _sample;
public:
    CRUIFontRenderingOptionsEditorWidget(CRPropRef props, CRUISettingsItem * setting);
    virtual ~CRUIFontRenderingOptionsEditorWidget();

    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);
    virtual bool onClick(CRUIWidget * widget);
};

class CRUISettingsListItemWidget;
class CRUISettingsValueListItemWidget;
class CRUISettingsListWidget : public CRUISettingsListEditor {
    CRUISettingsListItemWidget * _settingsListItem; // child setting list widget
    CRUISettingsValueListItemWidget * _optionListItem; // child is setting with list of possible options
public:
    CRUISettingsListWidget(CRPropRef props, CRUISettingsItem * settings);
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual ~CRUISettingsListWidget() {}
};

/// option item for option list setting
class CRUIActionOptionItem : public CRUIOptionItem {
public:
    CRUIActionOptionItem(const char * name, const CRUIAction * action);
    virtual CRUIImageRef getRightImage() const;
};

class CRUISettingsActionListItemWidget;
class CRUISettingsTapZoneListEditor : public CRUISettingsListWidget {
    CRUISettingsActionListItemWidget * _actionItem;
public:
    CRUISettingsTapZoneListEditor(CRPropRef props, CRUISettingsItem * setting);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    virtual ~CRUISettingsTapZoneListEditor();
};

class CRUISettingsWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnLongClickListener, public CRUISettingsEditorCallback {
    CRUISettingsItem * _settings;
    CRUITitleBarWidget * _titlebar;
public:
    CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItem * settings);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    virtual void onSettingChange(CRUISettingsItem * newSubitem, bool done);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
};


#endif // CRUISETTINGSWIDGET_CPP
