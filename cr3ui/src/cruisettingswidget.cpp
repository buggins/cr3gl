#include "cruisettingswidget.h"

using namespace CRUI;

CRUISettingsWidget::CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsList * settings) : CRUIWindowWidget(main), _settings(settings)
{
    _titlebar = new CRUITitleBarWidget(settings->getName(), this, false);
    _list = new CRUISettingsListWidget(settings);
    _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
    addChild(_titlebar);
    addChild(_list);
    setStyle("SETTINGS_WIDGET");
}

bool CRUISettingsWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        // TODO: handle title bar back button
    }
    return true;
}

CRUISettingsListWidget::CRUISettingsListWidget(CRUISettingsList * settings) : CRUIListWidget(false), _settings(settings) {
    setAdapter(this);
    _settingsListItem = new CRUISettingsListItemWidget(); // child setting list widget
    _optionListItem = new CRUISettingsValueListItemWidget(); // child is setting with list of possible options
}

lString16 CRUISettingsItemBase::getName() const {
    return _16(_nameRes.c_str());
}

lString16 CRUISettingsItemBase::getDescription() const {
    return !_descriptionRes.empty() ? _16(_descriptionRes.c_str()) : lString16();
}

CRUISettingsItemBase * CRUISettingsItemBase::findSetting(lString8 name) {
    if (getSettingId() == name)
        return this;
    for (int i = 0; i <childCount(); i++) {
        CRUISettingsItemBase * res = getChild(i)->findSetting(name);
        if (res)
            return res;
    }
    return NULL;
}

CRUISettingsListItemWidget::CRUISettingsListItemWidget() : _settings(NULL) {
    _title = new CRUITextWidget();
    _description = new CRUITextWidget();
    _righticon = new CRUIImageWidget();
    CRUIVerticalLayout * layout = new CRUIVerticalLayout();
    layout->addChild(_title);
    layout->addChild(_description);
    addChild(layout);
    addChild(_righticon);
}

void CRUISettingsListItemWidget::setSetting(CRUISettingsList * settings)
{
    _settings = settings;
    _title->setText(_settings->getName());
    _description->setText(_settings->getDescription());
}


CRUISettingsValueListItemWidget::CRUISettingsValueListItemWidget() : _settings(NULL) {
    _title = new CRUITextWidget();
    _description = new CRUITextWidget();
    _righticon = new CRUIImageWidget();
    CRUIVerticalLayout * layout = new CRUIVerticalLayout();
    layout->addChild(_title);
    layout->addChild(_description);
    addChild(layout);
    addChild(_righticon);
    setStyle("SETTINGS_ITEM_LIST");
}

void CRUISettingsValueListItemWidget::setSetting(CRUISettingsItemBase * settings) {
    _settings = settings;
}

int CRUISettingsListWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _settings->childCount();
}

CRUIWidget * CRUISettingsListWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    CRUISettingsItemBase * item = _settings->getChild(index);
    if (item->isList()) {
        _settingsListItem->setSetting((CRUISettingsList*)item);
        return _settingsListItem;
    }
    // TODO: more item types
    _optionListItem->setSetting(item);
    return _optionListItem;
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

const CRUIOptionItem * CRUISettingsOptionList::getDefaultOption() const {
    return findOption(_defaultValue);
}

const CRUIOptionItem * CRUISettingsOptionList::getSelectedOption() const {
    return findOption(_value);
}
