#include "cruisettingswidget.h"
#include "crui.h"
#include "cruimain.h"
#include "stringresource.h"

using namespace CRUI;


class CRUISettingsListItemWidgetBase : public CRUIHorizontalLayout {
protected:
    CRUISettingsItem * _settings;
    CRUITextWidget * _title;
    CRUITextWidget * _description;
    CRUIImageWidget * _righticon;
public:
    void setDescription(lString16 s) {
        _description->setText(s);
        if (!s.empty()) {
            _description->setVisibility(VISIBLE);
        } else {
            _description->setVisibility(GONE);
        }
    }
    void setDescription(const char * s) {
        _description->setText(s);
        if (s && s[0]) {
            _description->setVisibility(VISIBLE);
        } else {
            _description->setVisibility(GONE);
        }
    }

    CRUISettingsListItemWidgetBase() {
        _title = new CRUITextWidget();
        _description = new CRUITextWidget();
        _righticon = new CRUIImageWidget();
        CRUIVerticalLayout * layout = new CRUIVerticalLayout();
        layout->addChild(_title);
        layout->addChild(_description);
        layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        layout->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
        addChild(layout);
        addChild(_righticon);
        layout->setStyle("SETTINGS_ITEM_TEXT_LAYOUT");
        _righticon->setStyle("SETTINGS_ITEM_ICON");
        _title->setStyle("SETTINGS_ITEM_TITLE");
        _description->setStyle("SETTINGS_ITEM_DESCRIPTION");
        setStyle("SETTINGS_ITEM");
    }

    virtual void setSetting(CRUISettingsItem * settings, CRPropRef props) {
        CR_UNUSED(props);
        _settings = settings;
        _title->setText(_settings->getName());
        setDescription(_settings->getDescription(props));
        _righticon->setImage("ic_menu_forward");
    }
    virtual ~CRUISettingsListItemWidgetBase() {}
};

class CRUISettingsCheckboxWidget : public CRUISettingsListItemWidgetBase {
public:
    virtual void setSetting(CRUISettingsItem * settings, CRPropRef props) {
        _settings = settings;
        _title->setText(_settings->getName());
        setDescription(_settings->getDescription(props));
        if (settings->isChecked(props))
            _righticon->setImage("btn_check_on");
        else
            _righticon->setImage("btn_check_off");

    }
    virtual ~CRUISettingsCheckboxWidget() {}
};

class CRUIOptionListItemWidget : public CRUIHorizontalLayout {
protected:
    const CRUIOptionItem * _item;
    CRUITextWidget * _title;
    //CRUITextWidget * _description;
    CRUIImageWidget * _lefticon;
public:
    CRUIOptionListItemWidget() {
        _title = new CRUITextWidget();
        //_description = new CRUITextWidget();
        _lefticon = new CRUIImageWidget();
        _lefticon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        addChild(_lefticon);
        addChild(_title);
        _title->setStyle("SETTINGS_ITEM_TITLE");
        _lefticon->setStyle("SETTINGS_ITEM_ICON");
        //_description->setStyle("SETTINGS_ITEM_DESCRIPTION");
        setStyle("SETTINGS_ITEM");
    }

    virtual void setSetting(const CRUIOptionItem * item, bool isSelected) {
        _item = item;
        _title->setText(_item->getName());
        _lefticon->setImage(isSelected ? "btn_radio_on" : "btn_radio_off");
        //_description->setText(_item->getDescription());
    }
    virtual ~CRUIOptionListItemWidget() {}
};

class CRUISettingsListItemWidget : public CRUISettingsListItemWidgetBase {
public:
};

class CRUISettingsValueListItemWidget : public CRUISettingsListItemWidgetBase {
public:
};

/// no-rtti workaround for dynamic_cast<CRUISettingsList *>
CRUISettingsList * CRUISettingsList::asList() {
    return this;
}

CRUISettingsListEditor::CRUISettingsListEditor(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting), _list(NULL) {
    _list = new CRUIListWidget(true);
    _list->setAdapter(this);
    _list->setOnItemClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
    addChild(_list);
}



/// measure dimensions
void CRUIFontSampleWidget::measure(int baseWidth, int baseHeight) {
    int dx = 0;
    int dy = _props->getIntDef(PROP_FONT_SIZE) * 2 * 120 / 100;
    if (dy < baseHeight / 6)
        dy = _props->getIntDef(PROP_FONT_SIZE) * 3 * 120 / 100;
    defMeasure(baseWidth, baseHeight, dx, dy);
}

/// draws widget with its children to specified surface
void CRUIFontSampleWidget::draw(LVDrawBuf * buf) {
    CRUIWidget::draw(buf);
    lvRect rc = _pos;
    applyMargin(rc);
    LVDrawStateSaver saver(*buf);
    setClipRect(buf, rc);
    applyPadding(rc);
    // draw
    lUInt32 textColor = 0;
    int fontSize = _props->getIntDef(PROP_FONT_SIZE);
    lString8 face = UnicodeToUtf8(_props->getStringDef(PROP_FONT_FACE));
    lString16 sample = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    SimpleTitleFormatter fmt(sample, face, false, false, textColor, rc.width(), rc.height(), fontSize);
    fmt.draw(*buf, rc, 0, 0);
}

CRUIFontFaceEditorWidget::CRUIFontFaceEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsOptionsListEditorWidget(props, setting) {
    CRUITextWidget * separator = new CRUITextWidget(lString16("Sample:"));
    separator->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    separator->setPadding(PT_TO_PX(2));
    separator->setBackground(0xC0FFFFFF);
    addChild(separator);
    _sample = new CRUIFontSampleWidget(props);
    _sample->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _sample->setMaxHeight(deviceInfo.shortSide / 3);
    _sample->setMinHeight(deviceInfo.shortSide / 5);
    _sample->setBackground(0xC0808080);
    _sample->setPadding(PT_TO_PX(3));
    addChild(_sample);
}

bool CRUIFontFaceEditorWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    const CRUIOptionItem * item = _settings->getOption(itemIndex);
    _currentValue = item->getValue();
    _props->setString(_settings->getSettingId().c_str(), _currentValue);
    invalidate();
    return true;
}

CRUISettingsOptionsListEditorWidget::CRUISettingsOptionsListEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsListEditor(props, setting) {
    _optionListItem = new CRUIOptionListItemWidget(); // child is setting with list of possible options
    _currentValue = _settings->getValue(_props);
}

int CRUISettingsOptionsListEditorWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _settings->getOptionCount();
}

CRUIWidget * CRUISettingsOptionsListEditorWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    const CRUIOptionItem * item = _settings->getOption(index);
    bool isSelected = item->getValue() == _currentValue;
    _optionListItem->setSetting(item, isSelected);
    return _optionListItem;
}

bool CRUISettingsOptionsListEditorWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    const CRUIOptionItem * item = _settings->getOption(itemIndex);
    _props->setString(_settings->getSettingId().c_str(), item->getValue().c_str());
    if (_callback)
        _callback->onSettingChange(NULL, true);
    return true;
}

CRUISettingsListWidget::CRUISettingsListWidget(CRPropRef props, CRUISettingsItem * settings) : CRUISettingsListEditor(props, settings) {
    _settingsListItem = new CRUISettingsListItemWidget(); // child setting list widget
    _optionListItem = new CRUISettingsValueListItemWidget(); // child is setting with list of possible options
    _checkboxListItem = new CRUISettingsCheckboxWidget();
}

lString16 CRUISettingsItem::getName() const {
    return _16(_nameRes.c_str());
}

lString16 CRUISettingsItem::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return !_descriptionRes.empty() ? _16(_descriptionRes.c_str()) : lString16();
}

//CRUISettingsItem * CRUISettingsItem::findSetting(lString8 name) {
//    if (getSettingId() == name)
//        return this;
//    for (int i = 0; i <childCount(); i++) {
//        CRUISettingsItem * res = getChild(i)->findSetting(name);
//        if (res)
//            return res;
//    }
//    return NULL;
//}



int CRUISettingsListWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _settings->childCount();
}

CRUIWidget * CRUISettingsListWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    CRUISettingsItem * item = _settings->getChild(index);
    if (item->asList()) {
        _settingsListItem->setSetting(item, _props);
        return _settingsListItem;
    } else if (item->isToggle()) {
        _checkboxListItem->setSetting(item, _props);
        return _checkboxListItem;
    }
    // TODO: more item types
    _optionListItem->setSetting(item, _props);
    return _optionListItem;
}

bool CRUISettingsListWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    CRUISettingsItem * setting = _settings->getChild(itemIndex);
    if (setting->isToggle()) {
        setting->toggle(_props);
        invalidate();
    } else {
        if (_callback)
            _callback->onSettingChange(setting, false);
    }
    return true;
}

lString16 CRUISettingsCheckbox::getDescription(CRPropRef props) const {
    bool checked = isChecked(props);
    if (checked && !_checkedDescriptionRes.empty())
        return _16(_checkedDescriptionRes.c_str());
    if (!checked && !_uncheckedDescriptionRes.empty())
        return _16(_uncheckedDescriptionRes.c_str());
    return CRUISettingsItem::getDescription(props);
}

bool CRUISettingsCheckbox::isChecked(CRPropRef props) const {
    return props->getBoolDef(getSettingId().c_str(), false);
}

void CRUISettingsCheckbox::toggle(CRPropRef props) const {
    bool value = props->getBoolDef(getSettingId().c_str(), false);
    value = !value;
    props->setBool(getSettingId().c_str(), value);
}

lString16 CRUIOptionItem::getName() const {
    return _name.empty() ? _16(_nameRes.c_str()) : _name;
}

const CRUIOptionItem * CRUISettingsOptionList::findOption(lString8 value) const {
    if (value.empty())
        return NULL;
    for (int i = 0; i < _list.length(); i++)
        if (_list[i]->getValue() == value)
            return _list[i];
    return NULL;
}

const CRUIOptionItem * CRUISettingsOptionList::getSelectedOption(CRPropRef props) const {
    return findOption(getValue(props));
}

lString16 CRUISettingsOptionList::getDescription(CRPropRef props) const {
    const CRUIOptionItem * opt = getSelectedOption(props);
    if (!opt)
        return CRUISettingsItem::getDescription(props);
    return opt->getName();
}

lString8 CRUISettingsOptionList::getValue(CRPropRef props) const {
    return UnicodeToUtf8(props->getStringDef(getSettingId().c_str()));
}




CRUISettingsWidget::CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItem * settings) : CRUIWindowWidget(main), _settings(settings)
{
    _titlebar = new CRUITitleBarWidget(settings->getName(), this, false);
    addChild(_titlebar);
    setStyle("SETTINGS_WIDGET");
    CRUISettingsEditor * editor = settings->createEditor(main->getNewSettings());
    if (editor) {
        editor->setCallback(this);
        editor->setLayoutParams(FILL_PARENT, FILL_PARENT);
        addChild(editor);
    }
}

void CRUISettingsWidget::onSettingChange(CRUISettingsItem * setting, bool done) {
    if (_settings->asList()) {
        if (setting->asList() || setting->asOptionList()) {
            _main->showSettings(setting);
        } else if (setting->isToggle()) {
            setting->toggle(_main->getNewSettings());
            invalidate();
        }
    } else if (_settings->asOptionList()) {
        if (done)
            _main->back();
    }
}

bool CRUISettingsWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        _main->back();
    }
    return true;
}

/// create editor widget based on option type
CRUISettingsEditor * CRUISettingsOptionList::createEditor(CRPropRef props) {
    return new CRUISettingsOptionsListEditorWidget(props, this);
}

/// create editor widget based on option type
CRUISettingsEditor * CRUIFontFaceSetting::createEditor(CRPropRef props) {
    return new CRUIFontFaceEditorWidget(props, this);
}

/// create editor widget based on option type
CRUISettingsEditor * CRUISettingsList::createEditor(CRPropRef props) {
    return new CRUISettingsListWidget(props, this);
}
