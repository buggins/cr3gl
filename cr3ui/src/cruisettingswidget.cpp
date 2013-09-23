#include "cruisettingswidget.h"

CRUISettingsWidget::CRUISettingsWidget(CRUIMainWidget * main) : CRUIWindowWidget(main)
{

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
