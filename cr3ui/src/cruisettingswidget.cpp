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
    CRUISettingsListItemWidgetBase();
    virtual void setSetting(CRUISettingsItemBase * settings) {
        _settings = settings;
        _title->setText(_settings->getName());
        _description->setText(_settings->getDescription());
    }
    virtual ~CRUISettingsListItemWidgetBase() {}
};

class CRUISettingsListItemWidget : public CRUISettingsListItemWidgetBase {
public:
};

class CRUISettingsValueListItemWidget : public CRUISettingsListItemWidgetBase {
public:
};

CRUISettingsListItemWidgetBase::CRUISettingsListItemWidgetBase() : _settings(NULL) {
    _title = new CRUITextWidget();
    _description = new CRUITextWidget();
    _righticon = new CRUIImageWidget();
    CRUIVerticalLayout * layout = new CRUIVerticalLayout();
    layout->addChild(_title);
    layout->addChild(_description);
    addChild(layout);
    addChild(_righticon);
    _title->setStyle("SETTINGS_ITEM_TITLE");
    _description->setStyle("SETTINGS_ITEM_DESCRIPTION");
    setStyle("SETTINGS_ITEM");
}




/// no-rtti workaround for dynamic_cast<CRUISettingsList *>
CRUISettingsList * CRUISettingsList::asList() {
    return (CRUISettingsList *)this;
}

CRUISettingsWidget::CRUISettingsWidget(CRUIMainWidget * main, CRUISettingsItemBase * settings) : CRUIWindowWidget(main), _settings(settings)
{
    _titlebar = new CRUITitleBarWidget(settings->getName(), this, false);
    _list = NULL;
    addChild(_titlebar);
    if (settings->isList()) {
        int childCount = settings->childCount();
        CRUISettingsList * list = settings->asList();
        childCount = list->childCount();
        _list = new CRUISettingsListWidget(settings->asList());
        _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _list->setOnItemClickListener(this);
        addChild(_list);
    }
    setStyle("SETTINGS_WIDGET");
}

bool CRUISettingsWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    CRUISettingsItemBase * setting = _settings->getChild(itemIndex);
    _main->showSettings(setting);
    return true;
}

bool CRUISettingsWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        _main->back();
    }
    return true;
}

CRUISettingsListWidget::CRUISettingsListWidget(CRUISettingsList * settings) : CRUIListWidget(false), _settings(settings) {
    setAdapter(this);
    _settingsListItem = new CRUISettingsListItemWidget(); // child setting list widget
    _optionListItem = new CRUISettingsValueListItemWidget(); // child is setting with list of possible options
    setStyle("SETTINGS_ITEM_LIST");
}

lString16 CRUISettingsItemBase::getName() const {
    return _16(_nameRes.c_str());
}

lString16 CRUISettingsItemBase::getDescription() const {
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
        _settingsListItem->setSetting(item->asList());
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
