#include "cruisettingswidget.h"
#include "crui.h"
#include "cruimain.h"
#include "stringresource.h"
#include "cruiconfig.h"
#include "lvrend.h"

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
        if (settings->fixedValueIconSize()) {
            _righticon->setMinWidth(MIN_ITEM_PX - PT_TO_PX(4));
            _righticon->setMaxWidth(MIN_ITEM_PX - PT_TO_PX(4));
            _righticon->setMinHeight(MIN_ITEM_PX - PT_TO_PX(4));
            _righticon->setMaxHeight(MIN_ITEM_PX - PT_TO_PX(4));
        } else {
            _righticon->setMinWidth(UNSPECIFIED);
            _righticon->setMaxWidth(UNSPECIFIED);
            _righticon->setMinHeight(UNSPECIFIED);
            _righticon->setMaxHeight(UNSPECIFIED);
        }

    }
    virtual ~CRUISettingsListItemWidgetBase() {}
};

class CRUIOptionListItemWidget : public CRUIHorizontalLayout {
protected:
    const CRUIOptionItem * _item;
    CRUITextWidget * _title;
    //CRUITextWidget * _description;
    CRUIImageWidget * _lefticon;
    CRUIImageWidget * _righticon;
public:
    CRUIOptionListItemWidget() {
        _title = new CRUITextWidget();
        //_description = new CRUITextWidget();
        _lefticon = new CRUIImageWidget();
        _lefticon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        _righticon = new CRUIImageWidget();
        _righticon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        _righticon->setMargin(PT_TO_PX(3));
        _title->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        //_lefticon->setBackground(0x80FFFFFF);
        //_title->setBackground(0x80C0C0C0);
        //_righticon->setBackground(0x80A0A0A0);
        addChild(_lefticon);
        addChild(_title);
        addChild(_righticon);
        _title->setStyle("SETTINGS_ITEM_TITLE");
        _lefticon->setStyle("SETTINGS_ITEM_ICON");
        _righticon->setStyle("SETTINGS_ITEM_ICON");
        //_description->setStyle("SETTINGS_ITEM_DESCRIPTION");
        setStyle("SETTINGS_ITEM");
    }

    virtual void setSetting(const CRUIOptionItem * item, bool isSelected) {
        _item = item;
        _title->setText(_item->getName());
        _lefticon->setImage(isSelected ? "btn_radio_on" : "btn_radio_off");
        CRUIImageRef img = item->getRightImage();
        int sz = MIN_ITEM_PX - PT_TO_PX(3*2);
        _righticon->setMinWidth(sz);
        _righticon->setMaxWidth(sz);
        _righticon->setMinHeight(sz);
        _righticon->setMaxHeight(sz);
        if (img.isNull()) {
            _righticon->setVisibility(GONE);
        } else {
            _righticon->setVisibility(VISIBLE);
            _righticon->setImage(img);
        }
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


CRUIFontSampleWidget::CRUIFontSampleWidget(CRPropRef props) : CRUISettingsSampleWidget(props), _lastPropsHash(0) {
    _docview = new CRUIDocView();
    lString16 sample = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    LVArray<int> fontSizes;
    for (int i = crconfig.minFontSize; i <= crconfig.maxFontSize; i++)
        fontSizes.add(i);
    _docview->setFontSizes(fontSizes, false);
    _docview->createDefaultDocument(lString16(), sample);
    _docview->setViewMode(DVM_SCROLL, 1);
}

CRUIFontSampleWidget::~CRUIFontSampleWidget() {
    delete _docview;
}

/// measure dimensions
void CRUIFontSampleWidget::measure(int baseWidth, int baseHeight) {
    int dx = 0;
    int dy = _props->getIntDef(PROP_FONT_SIZE) * 2 * 120 / 100;
    if (dy < baseHeight / 6)
        dy = _props->getIntDef(PROP_FONT_SIZE) * 3 * 120 / 100;
    defMeasure(baseWidth, baseHeight, dx, dy);
}

void CRUIFontSampleWidget::format() {
    lUInt32 h = 123;
    for (int i = 0; i < _props->getCount(); i++) {
        lString8 key(_props->getName(i));
        lString16 value = _props->getValue(i);
        h = h * 31 + getHash(key);
        h = h * 31 + getHash(value);
    }
    bool changed = false;
    if (_lastPropsHash != h) {
        _lastPropsHash = h;
        changed = true;
    }
    if (_lastSize.x != _pos.width() || _lastSize.y != _pos.height()) {
        _lastSize.x = _pos.width();
        _lastSize.y = _pos.height();
        changed = true;
    }
    if (changed) {
        lvRect rc = _pos;
        applyMargin(rc);
        lvRect margins;
        getPadding(margins);
        _docview->Resize(rc.width(), rc.height());
        _docview->setPageMargins(margins);
        CRPropRef propsForDocview = LVCreatePropsContainer();
        static const char * props_for_copy[] = {
            PROP_FONT_COLOR,
            PROP_FONT_SIZE,
            PROP_FONT_FACE,
            PROP_FONT_GAMMA_INDEX,
            PROP_FONT_ANTIALIASING,
            PROP_FONT_WEIGHT_EMBOLDEN,
            PROP_FONT_HINTING,
            PROP_INTERLINE_SPACE,
            PROP_PAGE_MARGINS,
            NULL
        };
        for (int i = 0; props_for_copy[i]; i++) {
            propsForDocview->setString(props_for_copy[i], _props->getStringDef(props_for_copy[i]));
        }
//        lUInt32 textColor = _props->getColorDef(PROP_FONT_COLOR, 0);
//        int fontSize = _props->getIntDef(PROP_FONT_SIZE);
//        lString8 face = UnicodeToUtf8(_props->getStringDef(PROP_FONT_FACE));
//        int gammaIndex = _props->getIntDef(PROP_FONT_GAMMA_INDEX, 15);
//        int oldGammaIndex = fontMan->GetGammaIndex();
//        if (oldGammaIndex != gammaIndex) {
//            fontMan->SetGammaIndex(gammaIndex);
//            _docview->clearImageCache();
//            invalidate();
//        }
//        int antialiasingMode = _props->getIntDef(PROP_FONT_ANTIALIASING, 2);
//        if (antialiasingMode == 1) {
//            antialiasingMode = 2;
//        }
//        if (fontMan->GetAntialiasMode() != antialiasingMode) {
//            fontMan->SetAntialiasMode(antialiasingMode);
//            _docview->clearImageCache();
//            invalidate();
//        }
//        bool bold = _props->getBoolDef(PROP_FONT_WEIGHT_EMBOLDEN, false);
//        int v = bold ? STYLE_FONT_EMBOLD_MODE_EMBOLD
//                : STYLE_FONT_EMBOLD_MODE_NORMAL;
//        if (v != LVRendGetFontEmbolden()) {
//            LVRendSetFontEmbolden(v);
//        }
//        bool bytecode = _props->getBoolDef(PROP_FONT_HINTING, 1);
//        int hintingMode = bytecode ? HINTING_MODE_BYTECODE_INTERPRETOR : HINTING_MODE_AUTOHINT;
//        if ((int)fontMan->GetHintingMode() != hintingMode && hintingMode >= 0 && hintingMode <= 2) {
//            //CRLog::debug("Setting hinting mode to %d", mode);
//            fontMan->SetHintingMode((hinting_mode_t)hintingMode);
//            _docview->clearImageCache();
//            invalidate();
//        }

//        CRUIImageRef bgImage = resourceResolver->getBackgroundImage(_props);
//        //SimpleTitleFormatter fmt(sample, face, false, false, textColor, rc.width(), rc.height(), fontSize);
//        _docview->setTextColor(textColor);
//        _docview->setBackground(bgImage);
//        _docview->setFontSize(fontSize);
//        _docview->setDefaultFontFace(face);
        _docview->propsApply(propsForDocview);
        _docview->Render(0, 0, &_pageList);
    }
}

/// draws widget with its children to specified surface
void CRUIFontSampleWidget::draw(LVDrawBuf * buf) {
    CRUIWidget::draw(buf);
    lvRect rc = _pos;
    LVDrawStateSaver saver(*buf);
    CR_UNUSED(saver);
    setClipRect(buf, rc);
    applyMargin(rc);
    lvRect margins;
    getPadding(margins);
    format();
    CRUIImageRef bgImage = resourceResolver->getBackgroundImage(_props);
    LVRendPageInfo * page = _pageList[0];
    bgImage->draw(buf, rc, 0, 0);
    lUInt32 textColor = _props->getColorDef(PROP_FONT_COLOR, 0);
    buf->SetTextColor(textColor);
    _docview->drawPageTo(buf, *page, &rc, 1, 0);
}

static lString16 formatInterlineSpace(int sz) {
	char s[32];
	sprintf(s, "%d%%", sz);
    return lString16(s);
}

static lString16 formatPageMargins(int sz) {
	char s[32];
	sprintf(s, "%d.%02d%%", sz / 100, sz % 100);
    return lString16(s);
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
    _sizetext->setMinHeight(MIN_ITEM_PX);
    _slider = new CRUISliderWidget(crconfig.minFontSize, crconfig.maxFontSize, sz);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    addChild(_sizetext);
    addChild(_slider);
    CRUITextWidget * separator = new CRUITextWidget(lString16(""));
    separator->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    separator->setPadding(PT_TO_PX(2));
    separator->setBackground(0xE0FFFFFF);
    addChild(separator);
    _sample = new CRUIFontSampleWidget(props);
    _sample->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _sample->setMaxHeight(deviceInfo.shortSide / 3);
    _sample->setMinHeight(deviceInfo.shortSide / 5);
    _sample->setBackground(0xE0808080);
    _sample->setPadding(PT_TO_PX(4));
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

CRUIInterlineSpaceEditorWidget::CRUIInterlineSpaceEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    int sz = props->getIntDef(PROP_INTERLINE_SPACE, 120);
    _sizetext = new CRUITextWidget();
    _sizetext->setAlign(ALIGN_CENTER);
    _sizetext->setPadding(PT_TO_PX(6));
    _sizetext->setText(formatInterlineSpace(sz));
    _sizetext->setFontSize(FONT_SIZE_XLARGE);
    _sizetext->setMinHeight(MIN_ITEM_PX);
    _slider = new CRUISliderWidget(80, 200, sz);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    addChild(_sizetext);
    addChild(_slider);
    CRUITextWidget * separator = new CRUITextWidget(lString16(""));
    separator->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    separator->setPadding(PT_TO_PX(2));
    separator->setBackground(0xE0FFFFFF);
    addChild(separator);
    _sample = new CRUIFontSampleWidget(props);
    _sample->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _sample->setMaxHeight(deviceInfo.shortSide / 3);
    _sample->setMinHeight(deviceInfo.shortSide / 5);
    _sample->setBackground(0xE0808080);
    _sample->setPadding(PT_TO_PX(4));
    addChild(_sample);
}

bool CRUIInterlineSpaceEditorWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    int sz = pos;
    _sizetext->setText(formatInterlineSpace(sz));
    _props->setInt(PROP_INTERLINE_SPACE, sz);
    _sample->invalidate();
    return true;
}

CRUIPageMarginsEditorWidget::CRUIPageMarginsEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    int sz = props->getIntDef(PROP_PAGE_MARGINS, 500);
    _sizetext = new CRUITextWidget();
    _sizetext->setAlign(ALIGN_CENTER);
    _sizetext->setPadding(PT_TO_PX(6));
    _sizetext->setText(formatPageMargins(sz));
    _sizetext->setFontSize(FONT_SIZE_XLARGE);
    _sizetext->setMinHeight(MIN_ITEM_PX);
    _slider = new CRUISliderWidget(100, 2000, sz);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    addChild(_sizetext);
    addChild(_slider);
    CRUITextWidget * separator = new CRUITextWidget(lString16(""));
    separator->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    separator->setPadding(PT_TO_PX(2));
    separator->setBackground(0xE0FFFFFF);
    addChild(separator);
    _sample = new CRUIFontSampleWidget(props);
    _sample->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _sample->setMaxHeight(deviceInfo.shortSide / 3);
    _sample->setMinHeight(deviceInfo.shortSide / 5);
    _sample->setBackground(0xE0808080);
    _sample->setPadding(PT_TO_PX(4));
    addChild(_sample);
}

bool CRUIPageMarginsEditorWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    int sz = pos;
    _sizetext->setText(formatPageMargins(sz));
    _props->setInt(PROP_PAGE_MARGINS, sz);
    _sample->invalidate();
    return true;
}



static CRUISliderWidget * createColorSlider(const char * id, int value, CRUIOnScrollPosCallback * callback, lUInt32 color1, lUInt32 color2) {
    CRUISliderWidget * _slider = new CRUISliderWidget(0, 255, value);
    _slider->setId(id);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(callback);
    _slider->setColors(color1, color2);
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

static lUInt32 replaceColor(lUInt32 color, int shift, int value) {
    color = color & ~(255 << shift);
    color = color | ((value & 255) << shift);
    return color;
}

bool CRUIColorEditorWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "ENABLE_TEXTURE") {
        _enableTextureSetting->toggle(_props);
        _checkbox->setSetting(_enableTextureSetting, _props);
        _props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
        _props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
        _sliderRB->setScrollPos(brightnessSettingToSlider(_props, 16));
        _sliderGB->setScrollPos(brightnessSettingToSlider(_props, 8));
        _sliderBB->setScrollPos(brightnessSettingToSlider(_props, 0));
        _sliderRC->setScrollPos(contrastSettingToSlider(_props, 16));
        _sliderGC->setScrollPos(contrastSettingToSlider(_props, 8));
        _sliderBC->setScrollPos(contrastSettingToSlider(_props, 0));
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

    _sliderR = createColorSlider("R",((cl >> 16) & 255), this, replaceColor(cl, 16, 0), replaceColor(cl, 16, 255));
    _sliderG = createColorSlider("G",((cl >> 8) & 255), this, replaceColor(cl, 8, 0), replaceColor(cl, 8, 255));
    _sliderB = createColorSlider("B",((cl >> 0) & 255), this, replaceColor(cl, 0, 0), replaceColor(cl, 0, 255));

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

        _sliderRB = createColorSlider("RB", brightnessSettingToSlider(_props, 16), this, 0x80000000, 0x80FF0000);
        _sliderGB = createColorSlider("GB", brightnessSettingToSlider(_props, 8), this, 0x80000000, 0x8000FF00);
        _sliderBB = createColorSlider("BB", brightnessSettingToSlider(_props, 0), this, 0x80000000, 0x800000FF);
        _sliderRC = createColorSlider("RC", contrastSettingToSlider(_props, 16), this, 0x80000000, 0x80FF0000);
        _sliderGC = createColorSlider("GC", contrastSettingToSlider(_props, 8), this, 0x80000000, 0x8000FF00);
        _sliderBC = createColorSlider("BC", contrastSettingToSlider(_props, 0), this, 0x80000000, 0x800000FF);

        CRUITextWidget * colorCorrectionTitle = new CRUITextWidget(STR_SETTINGS_COLOR_CORRECTION);
        colorCorrectionTitle->setStyle("SLIDER_TITLE");
        _colorCorrectionPane->addChild(colorCorrectionTitle);
        CRUITableLayout * table = new CRUITableLayout(2);
        //
        CRUITextWidget * brightnessTitle = new CRUITextWidget(STR_SETTINGS_BRIGHTNESS);
        brightnessTitle->setStyle("SLIDER_TITLE");
        CRUITextWidget * contrastTitle = new CRUITextWidget(STR_SETTINGS_CONTRAST);
        contrastTitle->setStyle("SLIDER_TITLE");
        table->addChild(brightnessTitle); table->addChild(contrastTitle);
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
    _sample->setPadding(PT_TO_PX(4));
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
    bool updateRgb = false;
    if (widget->getId() == "R") {
        cl = (cl & 0x00FFFF) | (pos << 16);
        updateRgb = true;
    }
    if (widget->getId() == "G") {
        cl = (cl & 0xFF00FF) | (pos << 8);
        updateRgb = true;
    }
    if (widget->getId() == "B") {
        cl = (cl & 0xFFFF00) | (pos << 0);
        updateRgb = true;
    }
    if (updateRgb) {
        _sliderR->setColors(replaceColor(cl, 16, 0), replaceColor(cl, 16, 255));
        _sliderG->setColors(replaceColor(cl, 8, 0), replaceColor(cl, 8, 255));
        _sliderB->setColors(replaceColor(cl, 0, 0), replaceColor(cl, 0, 255));
    }
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
    CRUIImageRef img(new CRUISolidFillImage(cl, MIN_ITEM_PX - PT_TO_PX(3)));
    return img;
}

lString16 CRUIColorSetting::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return lString16();
//    lUInt32 cl = props->getColorDef(getSettingId().c_str(), 0);
//    char str[32];
//    sprintf(str, "#%06X", cl);
//    return Utf8ToUnicode(str);
}


CRUIFontRenderingOptionsEditorWidget::~CRUIFontRenderingOptionsEditorWidget() {
    delete _settingBold;
    delete _settingAntialiasing;
    delete _settingBytecodeInterpretor;
}

CRUIFontRenderingOptionsEditorWidget::CRUIFontRenderingOptionsEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    _settingBold = new CRUISettingsCheckbox(STR_SETTINGS_FONT_EMBOLDEN, NULL, PROP_FONT_WEIGHT_EMBOLDEN, STR_SETTINGS_FONT_EMBOLDEN_VALUE_ON, STR_SETTINGS_FONT_EMBOLDEN_VALUE_OFF);
    _settingAntialiasing = new CRUISettingsCheckbox(STR_SETTINGS_FONT_ANTIALIASING, NULL, PROP_FONT_ANTIALIASING, STR_SETTINGS_FONT_ANTIALIASING_VALUE_ON, STR_SETTINGS_FONT_ANTIALIASING_VALUE_OFF);
    _settingBytecodeInterpretor = new CRUISettingsCheckbox(STR_SETTINGS_FONT_BYTECODE_INTERPRETOR, NULL, PROP_FONT_HINTING, STR_SETTINGS_FONT_BYTECODE_INTERPRETOR_VALUE_ON, STR_SETTINGS_FONT_BYTECODE_INTERPRETOR_VALUE_OFF);
    _cbBold = new CRUISettingsListItemWidget();
    _cbBold->setId("ENABLE_BOLD");
    _cbBold->setSetting(_settingBold, _props);
    _cbBold->setOnClickListener(this);
    _cbAntialiasing = new CRUISettingsListItemWidget();
    _cbAntialiasing->setId("ENABLE_ANTIALIASING");
    _cbAntialiasing->setSetting(_settingAntialiasing, _props);
    _cbAntialiasing->setOnClickListener(this);
    _cbBytecodeInterpretor = new CRUISettingsListItemWidget();
    _cbBytecodeInterpretor->setId("ENABLE_BYTECODE_INTERPRETOR");
    _cbBytecodeInterpretor->setSetting(_settingBytecodeInterpretor, _props);
    _cbBytecodeInterpretor->setOnClickListener(this);
    addChild(_cbBold);
    addChild(_cbAntialiasing);
    addChild(_cbBytecodeInterpretor);
    CRUITextWidget * gammaTitle = new CRUITextWidget(STR_SETTINGS_FONT_GAMMA);
    gammaTitle->setStyle("SLIDER_TITLE");
    gammaTitle->setAlign(ALIGN_BOTTOM | ALIGN_HCENTER);
    addChild(gammaTitle);
    int gammaIndex = _props->getIntDef(PROP_FONT_GAMMA_INDEX, 15);
    _sliderGamma = new CRUISliderWidget(0, 30, gammaIndex);
    _sliderGamma->setId("GAMMA");
    _sliderGamma->setPadding(PT_TO_PX(4));
    _sliderGamma->setScrollPosCallback(this);
    addChild(_sliderGamma);


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
    _sample->setPadding(PT_TO_PX(4));
    addChild(_sample);
}

bool CRUIFontRenderingOptionsEditorWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    CR_UNUSED(manual);
    if (widget->getId() == "GAMMA") {
        _props->setInt(PROP_FONT_GAMMA_INDEX, pos);
        _sample->format();
    }
    return true;
}

bool CRUIFontRenderingOptionsEditorWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "ENABLE_BOLD") {
        _settingBold->toggle(_props);
        _cbBold->setSetting(_settingBold, _props);
        _sample->format();
    } else if (widget->getId() == "ENABLE_ANTIALIASING") {
        _settingAntialiasing->toggle(_props);
        _cbAntialiasing->setSetting(_settingAntialiasing, _props);
        _sample->format();
    } else if (widget->getId() == "ENABLE_BYTECODE_INTERPRETOR") {
        _settingBytecodeInterpretor->toggle(_props);
        _cbBytecodeInterpretor->setSetting(_settingBytecodeInterpretor, _props);
        _sample->format();
    }
    return true;
}

CRUISettingsEditor * CRUIFontRenderingSetting::createEditor(CRPropRef props) {
    return new CRUIFontRenderingOptionsEditorWidget(props, this);
}

lString16 CRUIFontRenderingSetting::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return _16(STR_SETTINGS_FONT_RENDERING_DESCRIPTION);
}

lString16 CRUIFontSizeSetting::getDescription(CRPropRef props) const {
    int sz = props->getIntDef(PROP_FONT_SIZE, 24);
    return formatFontSize(sz);
}

/// create editor widget based on option type
CRUISettingsEditor * CRUIInterlineSpaceSetting::createEditor(CRPropRef props) {
	return new CRUIInterlineSpaceEditorWidget(props, this);
}

lString16 CRUIInterlineSpaceSetting::getDescription(CRPropRef props) const {
	return lString16();
}

/// create editor widget based on option type
CRUISettingsEditor * CRUIPageMarginsSetting::createEditor(CRPropRef props) {
	return new CRUIPageMarginsEditorWidget(props, this);
}

lString16 CRUIPageMarginsSetting::getDescription(CRPropRef props) const {
	return lString16();
}

CRUIBackgroundTextureEditorWidget::CRUIBackgroundTextureEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsOptionsListEditorWidget(props, setting) {
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
    _sample->setPadding(PT_TO_PX(4));
    addChild(_sample);
}

bool CRUIBackgroundTextureEditorWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    const CRUIOptionItem * item = _settings->getOption(itemIndex);
    if (item->getValue() == _currentValue)
        return true;
    _currentValue = item->getValue();
    _props->setString(_settings->getSettingId().c_str(), _currentValue);
    _props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
    _props->setColor(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
    invalidate();
    return true;
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
    _sample->setBackground(0xE0808080);
    _sample->setPadding(PT_TO_PX(4));
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
    //return lString8("ic_menu_forward");
    //return lString8("00_button_right");
    //return lString8("arrow");
    return lString8("forward");
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
#if 1
    if (isChecked(props))
        return lString8("checked_checkbox");
    else
        return lString8("unchecked_checkbox");
//    if (isChecked(props))
//        return lString8("00_button_on");
//    else
//        return lString8("00_button_off");
#else
    if (isChecked(props))
        return lString8("btn_check_on");
    else
        return lString8("btn_check_off");
#endif
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

lString16 CRUITextureOptionItem::getName() const {
    return _resource->getName();
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
    _titlebar = new CRUITitleBarWidget(settings->getName(), this, this, false);
    _body->addChild(_titlebar);
    setStyle("SETTINGS_WIDGET");
    CRUISettingsEditor * editor = settings->createEditor(main->getNewSettings());
    if (editor) {
        editor->setCallback(this);
        editor->setOnDragListener(this);
        editor->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _body->addChild(editor);
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

bool CRUISettingsWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        // todo: close all settings instantly
        _main->back(true);
    }
    return true;
}

/// motion event handler, returns true if it handled event
bool CRUISettingsWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    int delta = event->getX() - event->getStartX();
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        break;
    case ACTION_UP:
        break;
    case ACTION_FOCUS_IN:
        break;
    case ACTION_FOCUS_OUT:
        return false; // to continue tracking
        break;
    case ACTION_CANCEL:
        break;
    case ACTION_MOVE:
        if ((delta > DRAG_THRESHOLD_X) || (-delta > DRAG_THRESHOLD_X))
            getMain()->startDragging(event, false);
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
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

/// create editor widget based on option type
CRUISettingsEditor * CRUIBackgroundTextureSetting::createEditor(CRPropRef props) {
    return new CRUIBackgroundTextureEditorWidget(props, this);
}

CRUIImageRef CRUITextureOptionItem::getRightImage() const {
    return resourceResolver->getBackgroundImage(_resource->getId());
}

CRUIImageRef CRUIBackgroundTextureSetting::getValueIcon(CRPropRef props) const {
    lString8 id = UnicodeToUtf8(props->getStringDef(PROP_BACKGROUND_IMAGE));
    return resourceResolver->getBackgroundImage(id);
}
