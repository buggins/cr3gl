#include "cruisettingswidget.h"
#include "crui.h"
#include "cruimain.h"

using namespace CRUI;


class CRUISettingsListItemWidgetBase : public CRUIHorizontalLayout {
protected:
    CRUISettingsItemBase * _settings;
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

    virtual void setSetting(CRUISettingsItemBase * settings, CRPropRef props) {
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
    virtual void setSetting(CRUISettingsItemBase * settings, CRPropRef props) {
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

CRUISettingsOptionsListEditorWidget::CRUISettingsOptionsListEditorWidget(CRPropRef props, CRUISettingsItemBase * setting) : CRUISettingsEditor(props, setting), _list(NULL), _onItemClickListener(NULL) {
    _list = new CRUIListWidget(true);
    _list->setAdapter(this);
    _list->setOnItemClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _optionListItem = new CRUIOptionListItemWidget(); // child is setting with list of possible options
    _currentValue = UnicodeToUtf8(_props->getStringDef(_settings->getSettingId().c_str()));
    addChild(_list);
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
    const CRUIOptionItem * item = _settings->getOption(itemIndex);
    _props->setString(_settings->getSettingId().c_str(), item->getValue().c_str());
    if (_onItemClickListener)
        return _onItemClickListener->onListItemClick(widget, itemIndex);
    return true;
}

CRUISettingsListWidget::CRUISettingsListWidget(CRPropRef props, CRUISettingsItemBase * settings) : CRUISettingsEditor(props, settings), CRUIListWidget(true) {
    setAdapter(this);
    _settingsListItem = new CRUISettingsListItemWidget(); // child setting list widget
    _optionListItem = new CRUISettingsValueListItemWidget(); // child is setting with list of possible options
    _checkboxListItem = new CRUISettingsCheckboxWidget();
    setStyle("SETTINGS_ITEM_LIST");
}

lString16 CRUISettingsItemBase::getName() const {
    return _16(_nameRes.c_str());
}

lString16 CRUISettingsItemBase::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return !_descriptionRes.empty() ? _16(_descriptionRes.c_str()) : lString16();
}

//CRUISettingsItemBase * CRUISettingsItemBase::findSetting(lString8 name) {
//    if (getSettingId() == name)
//        return this;
//    for (int i = 0; i <childCount(); i++) {
//        CRUISettingsItemBase * res = getChild(i)->findSetting(name);
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
    CRUISettingsItemBase * item = _settings->getChild(index);
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

lString16 CRUISettingsCheckbox::getDescription(CRPropRef props) const {
    bool checked = isChecked(props);
    if (checked && !_checkedDescriptionRes.empty())
        return _16(_checkedDescriptionRes.c_str());
    if (!checked && !_uncheckedDescriptionRes.empty())
        return _16(_uncheckedDescriptionRes.c_str());
    return CRUISettingsItemBase::getDescription(props);
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
    for (int i = 0; i < _list.length(); i++)
        if (_list[i]->getValue() == value)
            return _list[i];
    return NULL;
}

//const CRUIOptionItem * CRUISettingsOptionList::getDefaultOption() const {
//    return findOption(_defaultValue);
//}

const CRUIOptionItem * CRUISettingsOptionList::getSelectedOption(CRPropRef props) const {
    lString8 value = UnicodeToUtf8(props->getStringDef(getSettingId().c_str()));
    if (value.empty())
        return NULL;
    return findOption(value);
}

lString16 CRUISettingsOptionList::getDescription(CRPropRef props) const {
    const CRUIOptionItem * opt = getSelectedOption(props);
    if (!opt)
        return CRUISettingsItemBase::getDescription(props);
    return opt->getName();
}





CRUISettingsWidget::CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItemBase * settings) : CRUIWindowWidget(main), _settings(settings)
{
    _titlebar = new CRUITitleBarWidget(settings->getName(), this, false);
    addChild(_titlebar);
    if (settings->asList()) {
        CRUISettingsListWidget * _list;
        _list = new CRUISettingsListWidget(main->getNewSettings(), settings);
        _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _list->setOnItemClickListener(this);
        addChild(_list);
    } else if (settings->asOptionList()) {
        CRUISettingsOptionsListEditorWidget * _list;
        _list = new CRUISettingsOptionsListEditorWidget(main->getNewSettings(), settings);
        _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _list->setOnItemClickListener(this);
        addChild(_list);
    }
    setStyle("SETTINGS_WIDGET");
}

bool CRUISettingsWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    CRUISettingsItemBase * setting = _settings->getChild(itemIndex);
    if (_settings->asList()) {
        if (setting->asList() || setting->asOptionList()) {
            _main->showSettings(setting);
        } else if (setting->isToggle()) {
            setting->toggle(_main->getNewSettings());
            invalidate();
        }
    } else if (_settings->asOptionList()) {
        _main->back();
    }
    return true;
}

bool CRUISettingsWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        _main->back();
    }
    return true;
}

