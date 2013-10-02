#include "cruisettingswidget.h"
#include "crui.h"
#include "cruimain.h"
#include "stringresource.h"
#include "cruiconfig.h"

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
        _righticon->setImage(_settings->getValueIcon(props));
    }
    virtual ~CRUISettingsListItemWidgetBase() {}
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
    lUInt32 textColor = _props->getColorDef(PROP_FONT_COLOR, 0);
    CRUIImageRef bgImage = resourceResolver->getBackgroundImage(_props);
    int fontSize = _props->getIntDef(PROP_FONT_SIZE);
    lString8 face = UnicodeToUtf8(_props->getStringDef(PROP_FONT_FACE));
    lString16 sample = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    SimpleTitleFormatter fmt(sample, face, false, false, textColor, rc.width(), rc.height(), fontSize);

    // draw background
    bgImage->draw(buf, rc, 0, 0);
    // draw text
    fmt.draw(*buf, rc, 0, 0);
}

static lString16 formatFontSize(int sz) {
    int pt10 = PX_TO_PT(sz * 10);
    int pt10_prev = PX_TO_PT((sz-1) * 10);
    int pt10_next = PX_TO_PT((sz+1) * 10);
    if (pt10_next - pt10_prev > 20) // integer part only
        return lString16::itoa(PX_TO_PT(sz)) + "";
    if (pt10_next - pt10_prev > 10) {
        pt10 = pt10 / 5 * 5; // .0 and .5 only
//        if (pt10 % 10  == 0)
//            return lString16::itoa(PX_TO_PT(pt10 / 10)) + "";
    }
    return lString16::itoa(pt10/10) + "." + lString16::itoa(pt10 % 10) + "";
}

CRUIFontSizeEditorWidget::CRUIFontSizeEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    int sz = props->getIntDef(PROP_FONT_SIZE, 24);
    _sizetext = new CRUITextWidget();
    _sizetext->setAlign(ALIGN_CENTER);
    _sizetext->setPadding(PT_TO_PX(6));
    _sizetext->setText(formatFontSize(sz));
    _sizetext->setFontSize(FONT_SIZE_XLARGE);
    _slider = new CRUISliderWidget(crconfig.minFontSize, crconfig.maxFontSize, sz);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    addChild(_sizetext);
    addChild(_slider);
    CRUITextWidget * separator = new CRUITextWidget(lString16(""));
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

bool CRUIFontSizeEditorWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    int sz = pos;
    _sizetext->setText(formatFontSize(sz));
    _props->setInt(PROP_FONT_SIZE, sz);
    _sample->invalidate();
    return true;
}

/// updates widget position based on specified rectangle
void CRUIFontSizeEditorWidget::layout(int left, int top, int right, int bottom) {
    if (_slider->getMinScrollPos() != crconfig.minFontSize || _slider->getMaxScrollPos() != crconfig.maxFontSize) {
        _slider->setMinScrollPos(crconfig.minFontSize);
        _slider->setMaxScrollPos(crconfig.maxFontSize);
    }
    CRUISettingsEditor::layout(left, top, right, bottom);
}

static CRUISliderWidget * createColorSlider(const char * id, int value, CRUIOnScrollPosCallback * callback) {
    CRUISliderWidget * _slider = new CRUISliderWidget(0, 255, value);
    _slider->setId(id);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(callback);
    return _slider;
}

static int brightnessSettingToSlider(CRPropRef props, int shift) {
    lUInt32 color = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
    int v = (color >> shift) & 255;
    return v;
}

static int contrastSettingToSlider(CRPropRef props, int shift) {
    lUInt32 color = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
    int v = (color >> shift) & 255;
    return v;
}

static void brightnessSettingFromSlider(CRPropRef props, int shift, int pos) {
    lUInt32 color = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
    color = color & ~(255 << shift);
    color = color | ((pos & 255) << shift);
    props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, color);
}

static void contrastSettingFromSlider(CRPropRef props, int shift, int pos) {
    lUInt32 color = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
    color = color & ~(255 << shift);
    color = color | ((pos & 255) << shift);
    props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, color);
}

bool CRUIColorEditorWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "ENABLE_TEXTURE") {
        _enableTextureSetting->toggle(_props);
        _checkbox->setSetting(_enableTextureSetting, _props);
        updateMode();
    }
    return true;
}

void CRUIColorEditorWidget::updateMode() {
    bool textureEnabled = (_settings->getSettingId() == PROP_BACKGROUND_COLOR) && _props->getBoolDef(PROP_BACKGROUND_IMAGE_ENABLED, true);
    if (textureEnabled) {
        _colorPane->setVisibility(INVISIBLE);
        _colorCorrectionPane->setVisibility(VISIBLE);
    } else {
        _colorPane->setVisibility(VISIBLE);
        _colorCorrectionPane->setVisibility(INVISIBLE);
    }
    invalidate();
}

CRUIColorEditorWidget::CRUIColorEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {

    _colorPane = new CRUIVerticalLayout();
    _colorCorrectionPane = new CRUIVerticalLayout();
    lUInt32 cl = props->getColorDef(setting->getSettingId().c_str(), 0);

    _sliderR = createColorSlider("R",((cl >> 16) & 255), this);
    _sliderG = createColorSlider("G",((cl >> 8) & 255), this);
    _sliderB = createColorSlider("B",((cl >> 0) & 255), this);

    _colorPane->addChild(_sliderR);
    _colorPane->addChild(_sliderG);
    _colorPane->addChild(_sliderB);

    if (setting->getSettingId() == PROP_BACKGROUND_COLOR) {
        _enableTextureSetting = new CRUISettingsCheckbox(STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED, NULL, PROP_BACKGROUND_IMAGE_ENABLED, STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED_VALUE_ON, STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED_VALUE_OFF);
        _checkbox = new CRUISettingsListItemWidget();
        _checkbox->setId("ENABLE_TEXTURE");
        _checkbox->setSetting(_enableTextureSetting, _props);
        _checkbox->setOnClickListener(this);
        addChild(_checkbox);

        _sliderRB = createColorSlider("RB", brightnessSettingToSlider(_props, 16), this);
        _sliderGB = createColorSlider("GB", brightnessSettingToSlider(_props, 8), this);
        _sliderBB = createColorSlider("BB", brightnessSettingToSlider(_props, 0), this);
        _sliderRC = createColorSlider("RC", contrastSettingToSlider(_props, 16), this);
        _sliderGC = createColorSlider("GC", contrastSettingToSlider(_props, 8), this);
        _sliderBC = createColorSlider("BC", contrastSettingToSlider(_props, 0), this);

        _colorCorrectionPane->addChild(new CRUITextWidget(lString16("Color correction")));
        CRUITableLayout * table = new CRUITableLayout(2);
        table->addChild(new CRUITextWidget(lString16("Brightness"))); table->addChild(new CRUITextWidget(lString16("Contrast")));
        table->addChild(_sliderRB); table->addChild(_sliderRC);
        table->addChild(_sliderGB); table->addChild(_sliderGC);
        table->addChild(_sliderBB); table->addChild(_sliderBC);
        _colorCorrectionPane->addChild(table);
    }


    CRUIFrameLayout * frame = new CRUIFrameLayout();
    frame->addChild(_colorPane);
    frame->addChild(_colorCorrectionPane);
    addChild(frame);

    _colorCorrectionPane->setVisibility(INVISIBLE);

    CRUITextWidget * separator = new CRUITextWidget(lString16(""));
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

    updateMode();
}

bool CRUIColorEditorWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    CRLog::trace("CRUIColorEditorWidget::onScrollPosChange(%s, %d)", widget->getId().c_str(), pos);
    pos &= 255;
    lUInt32 cl = _props->getColorDef(_settings->getSettingId().c_str(), 0);
    if (widget->getId() == "R")
        cl = (cl & 0x00FFFF) | (pos << 16);
    if (widget->getId() == "G")
        cl = (cl & 0xFF00FF) | (pos << 8);
    if (widget->getId() == "B")
        cl = (cl & 0xFFFF00) | (pos << 0);
    _props->setColor(_settings->getSettingId().c_str(), cl);
    if (widget->getId() == "RB")
        brightnessSettingFromSlider(_props, 16, pos);
    else if (widget->getId() == "GB")
        brightnessSettingFromSlider(_props, 8, pos);
    else if (widget->getId() == "BB")
        brightnessSettingFromSlider(_props, 0, pos);
    else if (widget->getId() == "RC")
        contrastSettingFromSlider(_props, 16, pos);
    else if (widget->getId() == "GC")
        contrastSettingFromSlider(_props, 8, pos);
    else if (widget->getId() == "BC")
        contrastSettingFromSlider(_props, 0, pos);
    _sample->invalidate();
    return true;
}


CRUISettingsEditor * CRUIColorSetting::createEditor(CRPropRef props) {
    return new CRUIColorEditorWidget(props, this);
}

CRUIImageRef CRUIColorSetting::getValueIcon(CRPropRef props) const {
    lUInt32 cl = props->getColorDef(getSettingId().c_str(), 0);
    CRUIImageRef img(new CRUISolidFillImage(cl, MIN_ITEM_PX * 2 / 3));
    return img;
}

lString16 CRUIColorSetting::getDescription(CRPropRef props) const {
    lUInt32 cl = props->getColorDef(getSettingId().c_str(), 0);
    char str[32];
    sprintf(str, "#%06X", cl);
    return Utf8ToUnicode(str);
}


lString16 CRUIFontSizeSetting::getDescription(CRPropRef props) const {
    int sz = props->getIntDef(PROP_FONT_SIZE, 24);
    return formatFontSize(sz);
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
}

lString16 CRUISettingsItem::getName() const {
    return _16(_nameRes.c_str());
}

lString16 CRUISettingsItem::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return !_descriptionRes.empty() ? _16(_descriptionRes.c_str()) : lString16();
}

CRUIImageRef CRUISettingsItem::getValueIcon(CRPropRef props) const {
    lString8 res = getValueIconRes(props);
    if (res.empty())
        return CRUIImageRef();
    return resourceResolver->getIcon(res.c_str());
}

lString8 CRUISettingsItem::getValueIconRes(CRPropRef props) const {
    CR_UNUSED(props);
    return lString8("ic_menu_forward");
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
    if (item->asOptionList()) {
        _optionListItem->setSetting(item, _props);
        return _optionListItem;
    } else {
        _settingsListItem->setSetting(item, _props);
        return _settingsListItem;
    }
    // TODO: more item types
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

lString8 CRUISettingsCheckbox::getValueIconRes(CRPropRef props) const {
    if (isChecked(props))
        return lString8("btn_check_on");
    else
        return lString8("btn_check_off");
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
        if (setting->asList() || setting->asOptionList() || setting->hasCustomEditor()) {
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

/// create editor widget based on option type
CRUISettingsEditor * CRUIFontSizeSetting::createEditor(CRPropRef props) {
    return new CRUIFontSizeEditorWidget(props, this);
}
