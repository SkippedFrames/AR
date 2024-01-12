class SCR_BudgetProgressEditorUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("")]
	protected string m_sProgressBar;
	
	[Attribute("")]
	protected string m_sProgressText;
	
	[Attribute("")]
	protected string m_sProgressAddText;
	
	protected SCR_WLibProgressBarComponent m_ProgressBar;
	protected TextWidget m_ProgressText;
	protected TextWidget m_ProgressAddText;

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ProgressBar = SCR_WLibProgressBarComponent.Cast(SCR_WLibProgressBarComponent.GetProgressBar(m_sProgressBar, w));
		m_ProgressText = TextWidget.Cast(w.FindAnyWidget(m_sProgressText));
		m_ProgressAddText = TextWidget.Cast(w.FindAnyWidget(m_sProgressAddText));
	}
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void ShowBudget(float value)
	{
		m_ProgressBar.SetValue(value);
		
		float perc = Math.Floor(value  * 100);
		m_ProgressText.SetText(perc.ToString() + "%");
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowBudgetChange(float value, int currentBudget)
	{
		m_ProgressAddText.SetVisible(true);
		m_ProgressBar.SetChange(value * 0.01);
		
		float perc = Math.Floor(value);

		if (value < 0)
		{
			m_ProgressAddText.SetText(perc.ToString() + "%");
		}
		else if (value > 1)
		{
			m_ProgressAddText.SetText("+" + perc.ToString() + "%");
		}
		else
		{
			m_ProgressAddText.SetText("+<1%");
		}
		
		// Colorize progress change 
		if (currentBudget > 100)
		{
			m_ProgressAddText.SetColor(UIColors.WARNING);
			m_ProgressBar.SetSliderChangeColor(UIColors.WARNING);
		}
		else
		{
			m_ProgressAddText.SetColor(UIColors.CONFIRM);
			m_ProgressBar.SetSliderChangeColor(UIColors.CONFIRM);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void HideBudgetChange()
	{
		m_ProgressAddText.SetVisible(false);
		m_ProgressBar.SetChange(0);
	}
}
