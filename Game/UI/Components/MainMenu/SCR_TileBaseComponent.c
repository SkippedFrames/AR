//------------------------------------------------------------------------------------------------
class SCR_TileBaseComponent : ScriptedWidgetComponent
{
	[Attribute("1")]
	float m_fSaturationSelected;
	
	[Attribute("0.5")]
	float m_fSaturationDeselected;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_ColorSelected;
	
	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker)]
	ref Color m_ColorDeselected;
	
	[Attribute("0.2")]
	float m_fAnimationTime;
	
	[Attribute("Image")]
	string m_sContentName;
	
	Widget m_wRoot;
	ImageWidget m_wImage;
	float m_fAnimationRate;
	
	ref ScriptInvoker m_OnFocused = new ScriptInvoker();
	ref ScriptInvoker m_OnFocusLost = new ScriptInvoker();
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wImage = ImageWidget.Cast(w.FindAnyWidget(m_sContentName));
		
		SetInitialAnimationRate();
		
		// Set initial color
		if (!m_wImage)
			return;

		m_wImage.SetColor(m_ColorDeselected);
		m_wImage.SetSaturation(m_fSaturationDeselected);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetInitialAnimationRate()
	{
		m_fAnimationRate = SCR_WLibComponentBase.START_ANIMATION_RATE;
		GetGame().GetCallqueue().CallLater(SetAnimationRate, SCR_WLibComponentBase.START_ANIMATION_PERIOD);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetAnimationRate()
	{
		if (m_fAnimationTime <= 0)
			m_fAnimationRate = 1000;
		else
			m_fAnimationRate = 1 / m_fAnimationTime;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		AnimateWidget.Saturation(m_wImage, m_fSaturationSelected, m_fAnimationRate);
		AnimateWidget.Color(m_wImage, m_ColorSelected, m_fAnimationRate);
		m_OnFocused.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		AnimateWidget.Saturation(m_wImage, m_fSaturationDeselected, m_fAnimationRate);
		AnimateWidget.Color(m_wImage, m_ColorDeselected, m_fAnimationRate);
		m_OnFocusLost.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_OnClicked.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
};
