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
        CRUIImageRef icon = _settings->getValueIcon(props);
        _righticon->setImage(icon);
        int dx = UNSPECIFIED;
        int dy = UNSPECIFIED;
        if (settings->fixedValueIconSize()) {
            dx = dy = MIN_ITEM_PX - PT_TO_PX(4);
        } else {
            if (!icon.isNull()) {
                dx = icon->originalWidth();
                dy = icon->originalHeight();
            }
        }
        _righticon->setMinWidth(dx);
        _righticon->setMaxWidth(dx);
        _righticon->setMinHeight(dy);
        _righticon->setMaxHeight(dy);

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
        if (img.isNull()) {
            _righticon->setVisibility(GONE);
        } else {
            _righticon->setVisibility(VISIBLE);
            int sz = item->getRightImageSize();
            if (sz == 0) {
                sz = UNSPECIFIED;
                _righticon->setMinWidth(img->originalWidth() + 2*PT_TO_PX(3));
                _righticon->setMaxWidth(img->originalWidth() + 2*PT_TO_PX(3));
                _righticon->setMinHeight(img->originalHeight() + 2*PT_TO_PX(3));
                _righticon->setMaxHeight(img->originalHeight() + 2*PT_TO_PX(3));
            } else {
                _righticon->setMinWidth(sz);
                _righticon->setMaxWidth(sz);
                _righticon->setMinHeight(sz);
                _righticon->setMaxHeight(sz);
            }
            _righticon->setImage(img);
            measure(500, 100);
            layout(0, 0, 500, 100);
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

/// measure dimensions
void CRUISettingsEditor::measure(int baseWidth, int baseHeight) {
    if (baseWidth != UNSPECIFIED && baseHeight != UNSPECIFIED) {
        setVertical(baseWidth < baseHeight);
    }
    CRUILinearLayout::measure(baseWidth, baseHeight);
}

CRUISettingsListEditor::CRUISettingsListEditor(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting), _list(NULL) {
    _list = new CRUIListWidget(true);
    _list->setAdapter(this);
    _list->setOnItemClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _list->setLayoutParams(FILL_PARENT, FILL_PARENT);
    addChildControl(_list);
}


CRUIFontSampleWidget::CRUIFontSampleWidget(CRPropRef props) : CRUISettingsSampleWidget(props), _lastPropsHash(0) {
    _docview = new CRUIDocView();
    lString16 sample = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    LVArray<int> fontSizes;
    for (int i = crconfig.minFontSize; i <= crconfig.maxFontSize; i++)
        fontSizes.add(i);
    _docview->setFontSizes(fontSizes, false);
    _docview->createDefaultDocument(lString16(), sample + L"\n" + sample + L"\n" + sample + L"\n");
    _docview->setViewMode(DVM_PAGES, 1);
    _docview->setStatusMode(0, false, false, false, false, false, false, false);
    setLayoutParams(FILL_PARENT, FILL_PARENT);
    setMaxHeight(deviceInfo.shortSide / 3);
    setMinHeight(deviceInfo.shortSide / 5);
    setMaxWidth(deviceInfo.shortSide / 3);
    setMinWidth(deviceInfo.shortSide / 5);
    setBackground(0xE0808080);
    setPadding(PT_TO_PX(4));
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
    lvRect frameWidths = _docview->calcCoverFrameWidths(_pos);
    lvRect pageRc = _pos;
    pageRc.shrinkBy(frameWidths);
    if (_lastSize.x != pageRc.width() || _lastSize.y != pageRc.height()) {
        _lastSize.x = pageRc.width();
        _lastSize.y = pageRc.height();
        changed = true;
    }
    if (changed) {
        lvRect rc = _pos;
        applyMargin(rc);
        lvRect margins;
        getPadding(margins);
        rc.shrinkBy(frameWidths);
        _docview->Resize(rc.width(), rc.height());
        //_docview->setPageMargins(margins);
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
            PROP_APP_BOOK_COVER_COLOR,
            PROP_APP_BOOK_COVER_VISIBLE,
            //PROP_PAGE_MARGIN_LEFT,
            //PROP_PAGE_MARGIN_RIGHT,
            //PROP_PAGE_MARGIN_TOP,
            //PROP_PAGE_MARGIN_BOTTOM,
            NULL
        };
        for (int i = 0; props_for_copy[i]; i++) {
            propsForDocview->setString(props_for_copy[i], _props->getStringDef(props_for_copy[i]));
        }
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

    lvRect frameWidths = _docview->calcCoverFrameWidths(_pos);
    lvRect pageRc = rc;
    pageRc.shrinkBy(frameWidths);

    _docview->drawCoverFrame(*buf, rc, pageRc);
    rc = pageRc;

    CRUIImageRef bgImage = resourceResolver->getBackgroundImage(_props);
    LVRendPageInfo * page = _pageList[0];
    bgImage->draw(buf, rc, 0, 0);

    //lvRect rc2 = rc;
    //rc2.shrink(3);
    //buf->FillRect(rc2, 0xC0FFC080);

    lUInt32 textColor = _props->getColorDef(PROP_FONT_COLOR, 0);
    buf->SetTextColor(textColor);
    _docview->drawPageTo(buf, *page, &rc, 1, 0);
    lvRect rc2 = rc;
    rc2.right = rc2.left + rc2.width() / 7;
    buf->GradientRect(rc2.left, rc2.top, rc2.right, rc2.bottom, 0xD0000000, 0xFF000000, 0xFF000000, 0xD0000000);
    rc2 = rc;
    rc2.left = rc2.right - rc2.width() / 7;
    buf->GradientRect(rc2.left, rc2.top, rc2.right, rc2.bottom, 0xFF000000, 0xD0000000, 0xD0000000, 0xFF000000);
}

static lString16 formatInterlineSpace(int sz) {
	char s[32];
	sprintf(s, "%d%%", sz);
    return lString16(s);
}

static lString16 formatScreenBrightness(int sz) {
    if (sz == -1)
        return _16(STR_SETTINGS_APP_SCREEN_BACKLIGHT_BRIGHTNESS_VALUE_SYSTEM);
    return lString16::itoa(sz) + L"%";
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
    addChildControl(_sizetext);
    addChildControl(_slider);
    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);
}

bool CRUIFontSizeEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
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
    addChildControl(_sizetext);
    addChildControl(_slider);
    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);
}

bool CRUIInterlineSpaceEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    int sz = pos;
    _sizetext->setText(formatInterlineSpace(sz));
    _props->setInt(PROP_INTERLINE_SPACE, sz);
    _sample->invalidate();
    return true;
}

class CRUIScreenBrightnessSystemSettingsCheckbox : public CRUISettingsCheckbox {
protected:
    lString8 _checkedDescriptionRes;
    lString8 _uncheckedDescriptionRes;
public:
    CRUIScreenBrightnessSystemSettingsCheckbox() : CRUISettingsCheckbox(STR_SETTINGS_APP_SCREEN_BACKLIGHT_BRIGHTNESS_VALUE_SYSTEM, NULL, PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, NULL, NULL)
    {

    }
    virtual ~CRUIScreenBrightnessSystemSettingsCheckbox() {}
    virtual void toggle(CRPropRef props) const {
        int sz = props->getIntDef(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, -1);
        if (sz != -1) {
            sz = -1;
            props->setInt(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, sz);
        }
    }

    virtual bool isChecked(CRPropRef props) const {
        int sz = props->getIntDef(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, -1);
        return sz == -1;
    }

    virtual lString8 getValueIconRes(CRPropRef props) const {
        if (isChecked(props))
            return lString8("checked_checkbox");
        else
            return lString8("unchecked_checkbox");
    }
};

CRUIScreenBrightnessEditorWidget::CRUIScreenBrightnessEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    int sz = props->getIntDef(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, -1);
    _checkbox = new CRUIScreenBrightnessSystemSettingsCheckbox();
    _checkboxWidget = new CRUISettingsListItemWidget();
    _checkboxWidget->setId("ENABLE_TEXTURE");
    _checkboxWidget->setSetting(_checkbox, _props);
    _checkboxWidget->setOnClickListener(this);

    _sizetext = new CRUITextWidget();
    _sizetext->setAlign(ALIGN_CENTER);
    _sizetext->setPadding(PT_TO_PX(6));
    _sizetext->setText(sz >= 0 ? formatScreenBrightness(sz) : lString16("-"));
    _sizetext->setFontSize(FONT_SIZE_LARGE);
    _sizetext->setMinHeight(MIN_ITEM_PX);
    _slider = new CRUISliderWidget(0, 100, sz >= 0 ? sz : 100);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    addChildControl(_checkboxWidget);
    //addChildSpacer();
    addChildControl(_sizetext);
    addChildControl(_slider);
    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);

}

void CRUIScreenBrightnessEditorWidget::format() {
    int sz = _props->getIntDef(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, -1);
    _sizetext->setText(sz >= 0 ? formatScreenBrightness(sz) : lString16("-"));
    _sample->invalidate();
    _platform->setScreenBacklightBrightness(sz);
    _checkboxWidget->setSetting(_checkbox, _props);
}

bool CRUIScreenBrightnessEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    if (!manual)
        return false;
    int sz = pos;
    _props->setInt(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, sz);
    format();
    return true;
}

bool CRUIScreenBrightnessEditorWidget::onClick(CRUIWidget * widget) {
    CR_UNUSED(widget);
    _checkbox->toggle(_props);
    format();
    return true;
}

CRUIPageMarginsEditorWidget::CRUIPageMarginsEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsEditor(props, setting) {
    int sz = props->getIntDef(PROP_PAGE_MARGINS, 300);
    _sizetext = new CRUITextWidget();
    _sizetext->setAlign(ALIGN_CENTER);
    _sizetext->setPadding(PT_TO_PX(6));
    _sizetext->setText(formatPageMargins(sz));
    _sizetext->setFontSize(FONT_SIZE_XLARGE);
    _sizetext->setMinHeight(MIN_ITEM_PX);
    _slider = new CRUISliderWidget(100, 1000, sz);
    _slider->setPadding(PT_TO_PX(4));
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    addChildControl(_sizetext);
    addChildControl(_slider);
    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);
}

bool CRUIPageMarginsEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
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

    _colorSample = NULL;

    if (setting->getSettingId() == PROP_BACKGROUND_COLOR) {
        _enableTextureSetting = new CRUISettingsCheckbox(STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED, NULL, PROP_BACKGROUND_IMAGE_ENABLED, STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED_VALUE_ON, STR_SETTINGS_BACKGROUND_TEXTURE_ENABLED_VALUE_OFF);
        _checkbox = new CRUISettingsListItemWidget();
        _checkbox->setId("ENABLE_TEXTURE");
        _checkbox->setSetting(_enableTextureSetting, _props);
        _checkbox->setOnClickListener(this);
        addChildControl(_checkbox);

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
    if (setting->getSettingId() != PROP_BACKGROUND_COLOR) {
        _colorSample = new CRUITextWidget(lString16());
        _colorSample->setStyle("TOOL_BAR");
        _colorSample->setMinHeight(MIN_ITEM_PX * 3 / 2);
        _colorSample->setMaxHeight(MIN_ITEM_PX * 3 / 2);
        _colorSample->setMargin(PT_TO_PX(5));
        _colorSample->setPadding(PT_TO_PX(3));
        _colorSample->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _colorSample->setAlign(ALIGN_CENTER);
        _colorSample->setFontSize(FONT_SIZE_LARGE);
        char buf[32];
        sprintf(buf, "#%06x", cl);
        _colorSample->setText(Utf8ToUnicode(buf));
        _colorSample->setBackground(cl);
        if ((cl & 255) + ((cl >> 8) & 255) + ((cl >> 8) & 255) < 128 * 3)
            _colorSample->setTextColor(0xFFFFFF);
        else
            _colorSample->setTextColor(0x000000);

        _colorPane->addChild(_colorSample);
    }

    CRUIFrameLayout * frame = new CRUIFrameLayout();
    frame->addChild(_colorPane);
    frame->addChild(_colorCorrectionPane);
    addChildControl(frame);

    _colorCorrectionPane->setVisibility(INVISIBLE);

    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);

    updateMode();
}

bool CRUIColorEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
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
        if (_colorSample) {
            _colorSample->setBackground(cl);
            if ((cl & 255) + ((cl >> 8) & 255) + ((cl >> 8) & 255) < 128 * 3)
                _colorSample->setTextColor(0xFFFFFF);
            else
                _colorSample->setTextColor(0x000000);
            char buf[32];
            sprintf(buf, "#%06x", cl);
            _colorSample->setText(Utf8ToUnicode(buf));
        }
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
    addChildControl(_cbBold);
    addChildControl(_cbAntialiasing);
    addChildControl(_cbBytecodeInterpretor);
    CRUITextWidget * gammaTitle = new CRUITextWidget(STR_SETTINGS_FONT_GAMMA);
    gammaTitle->setStyle("SLIDER_TITLE");
    gammaTitle->setAlign(ALIGN_BOTTOM | ALIGN_HCENTER);
    addChildControl(gammaTitle);
    int gammaIndex = _props->getIntDef(PROP_FONT_GAMMA_INDEX, 15);
    _sliderGamma = new CRUISliderWidget(0, 30, gammaIndex);
    _sliderGamma->setId("GAMMA");
    _sliderGamma->setPadding(PT_TO_PX(4));
    _sliderGamma->setScrollPosCallback(this);
    addChildControl(_sliderGamma);

    addChildSpacer();

    _sample = new CRUIFontSampleWidget(props);
    addChild(_sample);
}

bool CRUIFontRenderingOptionsEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
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
    CR_UNUSED(props);
	return lString16();
}

/// create editor widget based on option type
CRUISettingsEditor * CRUIScreenBrightnessSetting::createEditor(CRPropRef props) {
    return new CRUIScreenBrightnessEditorWidget(props, this);
}

lString16 CRUIScreenBrightnessSetting::getDescription(CRPropRef props) const {
    int sz = props->getIntDef(PROP_APP_SCREEN_BACKLIGHT_BRIGHTNESS, -1);
    return formatScreenBrightness(sz);
}

/// create editor widget based on option type
CRUISettingsEditor * CRUIPageMarginsSetting::createEditor(CRPropRef props) {
	return new CRUIPageMarginsEditorWidget(props, this);
}

lString16 CRUIPageMarginsSetting::getDescription(CRPropRef props) const {
    CR_UNUSED(props);
    return lString16();
}

CRUIBackgroundTextureEditorWidget::CRUIBackgroundTextureEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsOptionsListEditorWidget(props, setting) {
    _sample = new CRUIFontSampleWidget(props);
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



CRUITTSSettingEditorWidget::CRUITTSSettingEditorWidget(CRPropRef props, CRUISettingsItem * setting)
    : CRUISettingsOptionsListEditorWidget(props, setting)
{
    CRUIVerticalLayout * controls = new CRUIVerticalLayout();
    controls->setLayoutParams(FILL_PARENT, FILL_PARENT);
    controls->setPadding(PT_TO_PX(4));
    int rate = props->getIntDef(PROP_APP_TTS_RATE, 70);
    CRUITextWidget * _label = new CRUITextWidget(STR_SETTINGS_TTS_RATE);
    _label->setPadding(PT_TO_PX(4));
    CRUISliderWidget * _slider = new CRUISliderWidget(0, 100, rate);
    _slider->setScrollPosCallback(this);
    _slider->setMinHeight(MIN_ITEM_PX);
    _slider->setPadding(lvRect(0, 0, 0, PT_TO_PX(6)));
    CRUITextWidget * _label2 = new CRUITextWidget(STR_SETTINGS_TTS_SAMPLE);
    _label2->setPadding(PT_TO_PX(4));
    _edit = new CRUIEditWidget();
    _edit->setText(lString16("Text To Speech voice test sample"));
    _edit->setPadding(PT_TO_PX(6));
    CRUIButton * _btnTest = new CRUIButton(_16(STR_SETTINGS_TTS_SAMPLE_PLAY));
    _btnTest->setPadding(PT_TO_PX(4));
    controls->addChild(_label);
    controls->addChild(_slider);
    controls->addChild(_label2);
    controls->addChild(_edit);
    controls->addChild(_btnTest);
    _btnTest->setOnClickListener(this);
    controls->addChild(new CRUIVSpacer());
    insertChild(controls, 0);
}

bool CRUITTSSettingEditorWidget::onClick(CRUIWidget * widget) {
    CR_UNUSED(widget);
    if (getPlatform()->getTextToSpeech()) {
        if (getPlatform()->getTextToSpeech()->isSpeaking())
            getPlatform()->getTextToSpeech()->stop();
        getPlatform()->getTextToSpeech()->tell(_edit->getText());
    }
    return true;
}

bool CRUITTSSettingEditorWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
    CR_UNUSED(widget);
    _props->setInt(PROP_APP_TTS_RATE, pos);
    if (manual && getPlatform()->getTextToSpeech()) {
        //getPlatform()->getTextToSpeech()->stop();
        getPlatform()->getTextToSpeech()->setRate(pos);
    }
    return true;
}

bool CRUITTSSettingEditorWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    const CRUIOptionItem * item = _settings->getOption(itemIndex);
    _currentValue = item->getValue();
    _props->setString(_settings->getSettingId().c_str(), _currentValue);
    if (getPlatform()->getTextToSpeech()) {
        if (getPlatform()->getTextToSpeech()->isSpeaking())
            getPlatform()->getTextToSpeech()->stop();
        getPlatform()->getTextToSpeech()->setCurrentVoice(_currentValue);
    }
    invalidate();
    return true;
}



CRUIFontFaceEditorWidget::CRUIFontFaceEditorWidget(CRPropRef props, CRUISettingsItem * setting) : CRUISettingsOptionsListEditorWidget(props, setting) {
    _sample = new CRUIFontSampleWidget(props);
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
        editor->setPlatform(main->getPlatform());
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

CRUISettingsEditor * CRUITTSSetting::createEditor(CRPropRef props) {
    return new CRUITTSSettingEditorWidget(props, this);
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

int CRUITextureOptionItem::getRightImageSize() const {
    return crconfig.desktopMode ? deviceInfo.shortSide / 6 : MIN_ITEM_PX - PT_TO_PX(3*2);
}

CRUIImageRef CRUITextureOptionItem::getRightImage() const {
    return resourceResolver->getBackgroundImage(_resource->getId());
}

CRUIImageRef CRUIBackgroundTextureSetting::getValueIcon(CRPropRef props) const {
    lString8 id = UnicodeToUtf8(props->getStringDef(PROP_BACKGROUND_IMAGE));
    return resourceResolver->getBackgroundImage(id);
}

class CRUISettingsActionListItemWidget : public CRUISettingsListItemWidgetBase {
public:
    CRUISettingsActionListItemWidget() : CRUISettingsListItemWidgetBase()
    {
        setVertical(true);
        _title->setAlign(ALIGN_HCENTER);
        _righticon->setAlign(ALIGN_HCENTER);
        _description->setVisibility(CRUI::GONE);
        setBackground("home_frame.9");
        setMargin(PT_TO_PX(3));
    }

    virtual void setSetting(CRUISettingsItem * settings, CRPropRef props) {
        CR_UNUSED(props);
        _settings = settings;
        lString8 actionName = settings->getValue(props);
        const CRUIAction * action = CRUIActionByName(actionName.c_str());
        if (!action)
            action = ACTION_NO_ACTION;
        _title->setText(action->getName());
        if (!action->icon_res.empty())
            _righticon->setImage(resourceResolver->getIcon(action->icon_res.c_str()));
        else
            _righticon->setImage(CRUIImageRef());
    }
};

CRUISettingsTapZoneListEditor::CRUISettingsTapZoneListEditor(CRPropRef props, CRUISettingsItem * setting)
    : CRUISettingsListWidget(props, setting)
{
    _list->setColCount(3);
    _actionItem = new CRUISettingsActionListItemWidget();
}

CRUISettingsTapZoneListEditor::~CRUISettingsTapZoneListEditor() {
    delete _actionItem;
}

CRUIWidget * CRUISettingsTapZoneListEditor::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    CRUISettingsItem * item = _settings->getChild(index);
    _actionItem->setSetting(item, _props);
    return _actionItem;
}

CRUIActionOptionItem::CRUIActionOptionItem(const char * name, const CRUIAction * action) : CRUIOptionItem(name, action->name_res.c_str())
{

}

CRUIImageRef CRUIActionOptionItem::getRightImage() const {
    const CRUIAction * action = CRUIActionByName(_value.c_str());
    if (!action)
        CRUIImageRef();
    return resourceResolver->getIcon(action->icon_res.c_str());
}

static const char * TAP_ZONE_ACTION_LIST[] = {
    "NO_ACTION",
    "PAGE_UP",
    "PAGE_DOWN",
    "PAGE_UP_10",
    "PAGE_DOWN_10",
    "MENU",
    "SETTINGS",
    "TOC",
    "BACK",
    "LINK_BACK",
    "LINK_FORWARD",
    "BOOKMARKS",
    "FIND_TEXT",
    "HELP",
    "TOGGLE_NIGHT_MODE",
    "READER_HOME",
    "SHOW_FOLDER",
    "GOTO_PERCENT",
    "TTS_PLAY",
    "EXIT",
    NULL
};

CRUITapZoneSettingsList::CRUITapZoneSettingsList(const char * nameRes, const char * descriptionRes, int _modifier)
    : CRUISettingsList(nameRes, descriptionRes, "TAP_ZONE"), modifier(_modifier)
{
    for (int i = 1; i <= 9; i++) {
        lString8 s(modifier == 0 ? PROP_APP_TAP_ZONE_ACTION_NORMAL : PROP_APP_TAP_ZONE_ACTION_DOUBLE);
        s += lString8::itoa(i);
        CRUISettingsOptionList * options = new CRUISettingsOptionList("SETTINGS_CONTROLS_TAP_ZONE_ACTION", NULL, s.c_str());
        for (int a = 0; TAP_ZONE_ACTION_LIST[a]; a++) {
            const CRUIAction * action = CRUIActionByName(TAP_ZONE_ACTION_LIST[a]);
            if (!action) {
                CRLog::error("Unknown action for tap zone: %s", TAP_ZONE_ACTION_LIST[a]);
                continue;
            }
            CRUIActionOptionItem * option = new CRUIActionOptionItem(TAP_ZONE_ACTION_LIST[a], action);
            options->addOption(option);
        }
        addChild(options);
    }
}

/// create editor widget based on option type
CRUISettingsEditor * CRUITapZoneSettingsList::createEditor(CRPropRef props) {
    return new CRUISettingsTapZoneListEditor(props, this);
}
