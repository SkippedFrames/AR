//------------------------------------------------------------------------------------------------
class SCR_SliderComponent : SCR_ChangeableComponentBase
{
	[Attribute("0.5")]
	protected float m_fValue;

	[Attribute("0")]
	protected float m_fMinValue;

	[Attribute("1")]
	protected float m_fMaxValue;

	[Attribute("0.05")]
	protected float m_fStep;

	[Attribute("%1", UIWidgets.LocaleEditBox, "Localization friendly string: %1 saves the value. It can be overwritten by SetValueString() function")]
	protected string m_sFormatText;

	[Attribute("1", UIWidgets.EditBox, "Multiplies the internal value in the text. With 100 and percentage format, value 1 is visualized as 100%")]
	protected float m_fShownValueMultiplier;
	
	[Attribute("0", UIWidgets.CheckBox, "Should the value text be rounded?")]
	private bool m_bRoundValue;	
	
	[Attribute("0")]
	protected int m_iDecimalPrecision;

	[Attribute(SCR_SoundEvent.SOUND_FE_ITEM_CHANGE)]
	protected string m_sChangeSound;

	protected TextWidget m_wText;
	protected SliderWidget m_wSlider;
	protected float m_fOldValue;

	protected ref SCR_EventHandlerComponent m_Handler;
	protected ref ScriptInvoker m_OnChangedFinal;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wSlider = SliderWidget.Cast(w.FindAnyWidget("Slider"));
		m_wText = TextWidget.Cast(w.FindAnyWidget("SliderText"));

		if (!m_wSlider)
			return;

		// Get slider handling
		m_Handler = SCR_EventHandlerComponent.Cast(m_wSlider.FindHandler(SCR_EventHandlerComponent));
		if (!m_Handler)
			return;

		SetSliderSettings(m_fMinValue, m_fMaxValue, m_fStep, m_sFormatText);

		m_Handler.GetOnFocus().Insert(OnSliderFocus);
		m_Handler.GetOnFocusLost().Insert(OnSliderFocusLost);
		AddListeners();

		if (m_wText)
			m_wText.SetTextFormat(m_sFormatText, RoundValue(m_wSlider.GetCurrent() * m_fShownValueMultiplier, m_iDecimalPrecision));
		RemoveListeners();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		// Do not focus parent, call the super function on slider focus

		// Set focus to handler
		GetGame().GetWorkspace().SetFocusedWidget(m_wSlider);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		// Do not call super.OnFocusLost, it will be called by OnSliderFocusLost
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnValueChanged(Widget w)
	{
		float value;
		if (m_wSlider)
			value = m_wSlider.GetCurrent();
		
		if (m_wText && m_wText.IsVisible())
			m_wText.SetTextFormat(m_sFormatText, RoundValue(value * m_fShownValueMultiplier, m_iDecimalPrecision));

		if (m_sChangeSound != string.Empty && m_fOldValue != value)
			PlaySound(m_sChangeSound);

		m_fOldValue = value;
		
		m_OnChanged.Invoke(this, value);
	}

	//------------------------------------------------------------------------------------------------
	protected float RoundValue(float value, int precision)
	{
		if (!m_bRoundValue)
			return value;
		
		float coef = Math.Pow(10, m_iDecimalPrecision);
		value = Math.Round(value * coef) / coef;

		return value;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnValueFinal(Widget w)
	{
		float value;
		if (m_wSlider)
			value = m_wSlider.GetCurrent();

		if (m_OnChangedFinal)
			m_OnChangedFinal.Invoke(this, value);
	}

	//------------------------------------------------------------------------------------------------
	void OnSliderFocus()
	{
		AddListeners();
		m_wRoot.SetFlags(WidgetFlags.NOFOCUS);

		super.OnFocus(m_wRoot, 0, 0); // Emulate focus on a parent class
	}

	//------------------------------------------------------------------------------------------------
	void OnSliderFocusLost()
	{
		RemoveListeners();
		m_wRoot.ClearFlags(WidgetFlags.NOFOCUS);

		super.OnFocusLost(m_wRoot, 0, 0); // Emulate focus on a parent class
	}

	//------------------------------------------------------------------------------------------------
	//! Add listeners for handler invokers
	protected void AddListeners()
	{
		if (!m_Handler)
			return;

		m_Handler.GetOnChange().Insert(OnValueChanged);
		m_Handler.GetOnChangeFinal().Insert(OnValueFinal);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove listeners for handler invokers
	protected void RemoveListeners()
	{
		if (!m_Handler)
			return;

		m_Handler.GetOnChange().Remove(OnValueChanged);
	}

	// User API
	//------------------------------------------------------------------------------------------------
	void SetValue(float value)
	{
		if (!m_wSlider)
			return;

		m_fValue = value;
		m_wSlider.SetCurrent(value);
		OnValueChanged(m_wSlider);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChangedFinal()
	{
		if (!m_OnChangedFinal)
			m_OnChangedFinal = new ScriptInvoker();
		return m_OnChangedFinal;
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_wSlider.GetCurrent();
	}

	//------------------------------------------------------------------------------------------------
	void SetFormatText(string text)
	{
		m_sFormatText = text;
	}

	//------------------------------------------------------------------------------------------------
	string GetFormatText()
	{
		return m_sFormatText;
	}

	//------------------------------------------------------------------------------------------------
	void ShowCustomValue(string value)
	{
		if (!m_wText || !m_wText.IsVisible())
			return;

		m_wText.SetTextFormat(m_sFormatText, value);
	}

	//------------------------------------------------------------------------------------------------
	void SetSliderSettings(float min, float max, float step, string formatText = string.Empty)
	{
		m_wSlider.SetMin(min);
		m_wSlider.SetMax(max);
		m_wSlider.SetStep(step);
		if (formatText != string.Empty)
			m_sFormatText = formatText;
	}

	//------------------------------------------------------------------------------------------------
	void SetMin(float min)
	{
		m_wSlider.SetMin(min);
	}

	//------------------------------------------------------------------------------------------------
	void SetMax(float max)
	{
		m_wSlider.SetMax(max);
	}

	//------------------------------------------------------------------------------------------------
	void SetStep(float step)
	{
		m_wSlider.SetStep(step);
	}

	//------------------------------------------------------------------------------------------------
	float GetMin()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetMin();
	}

	//------------------------------------------------------------------------------------------------
	float GetMax()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetMax();
	}

	//------------------------------------------------------------------------------------------------
	float GetStep()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetStep();
	}

	//------------------------------------------------------------------------------------------------
	void SetShownValueMultiplier(float multiplier)
	{
		m_fShownValueMultiplier = multiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetShownValueMultiplier()
	{
		return m_fShownValueMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_SliderComponent GetSliderComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		return SCR_SliderComponent.Cast(SCR_ScriptedWidgetComponent.GetComponent(SCR_SliderComponent, name, parent, searchAllChildren));
	}
};
