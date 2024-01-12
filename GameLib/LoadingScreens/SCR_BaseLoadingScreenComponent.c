//#define DEBUG_LOADING_SCREENS

class SCR_BaseLoadingScreenComponent : ScriptedWidgetComponent
{
	Widget m_wRoot;

	ref SCR_ScenarioLoadingScreenWidgets m_Widgets;	
	SCR_LoadingSpinner m_SpinnerComp;
	
	const float FADE_TIME_BLACK_OVERLAY = 1;
	
	protected float m_fLoadingTime;
	
	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> HandlerAttached | widget: %2", this, w);
		#endif			
		
		m_wRoot = w;

		m_Widgets = new SCR_ScenarioLoadingScreenWidgets();
		m_Widgets.Init(m_wRoot);		

		// Get spinner component
		if (m_Widgets.m_wSpinner)
		{
			Widget compWidget = m_Widgets.m_wSpinner.FindAnyWidget("Spinner");
			
			if (compWidget)
				m_SpinnerComp = SCR_LoadingSpinner.Cast(compWidget.FindHandler(SCR_LoadingSpinner));		
		}		
				
		InitWidgets();
		
		// Get total elapsed loading time & set the animations progress
		m_fLoadingTime = GetLoadingTime();
		
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> HandlerAttached | widget: %2 | m_fLoadingTime: %3", this, w, m_fLoadingTime);
		#endif			
		
		Update(m_fLoadingTime);
	}

	//---------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		m_Widgets.m_wContent.SetVisible(true);
		m_Widgets.m_wContentOverlay.SetVisible(true);
		m_Widgets.m_wPreloadContent.SetVisible(true);
		m_Widgets.m_wBlackOverlay.SetVisible(true);		
	}	
		
	//---------------------------------------------------------------------------------------------
	void Update(float timeSlice, float progress = 0, float minDurationRatio = 0)
	{
		m_fLoadingTime += timeSlice;
		
		Fade(m_Widgets.m_wBlackOverlay, false, FADE_TIME_BLACK_OVERLAY, timeSlice);
	}	

	//---------------------------------------------------------------------------------------------
	void OnHide()
	{
		SaveLoadingTime(m_fLoadingTime);
	}
	
	//---------------------------------------------------------------------------------------------
	static void ResetLoadingTime()
	{
		GameSessionStorage.s_Data["m_fLoadingTime"] = "0";
	}	
	
	//---------------------------------------------------------------------------------------------
	void SaveLoadingTime(float loadingTime)
	{
		GameSessionStorage.s_Data["m_fLoadingTime"] = loadingTime.ToString();
	}	

	//---------------------------------------------------------------------------------------------
	float GetLoadingTime()
	{
		// Get count
		string sLoadingTime = GameSessionStorage.s_Data["m_fLoadingTime"];
		float fLoadingTime = 0;
		
		// Setup number
		if (sLoadingTime.IsEmpty())
		{
			GameSessionStorage.s_Data["m_fLoadingTime"] = "0";
		}	
		else 
		{
			fLoadingTime = sLoadingTime.ToFloat();
		}

		return fLoadingTime;
	}	
				
	//---------------------------------------------------------------------------------------------
	protected void Fade(Widget w, bool show, float length, float timeslice)
	{
		if (length <= 0)
		{
			w.SetOpacity(show);
			return;		
		}
		
		float opacityCurrent = w.GetOpacity();
		
		if (opacityCurrent == show)
			return;
		
		float progressDelta = timeslice / length;
		
		if (!show)
			progressDelta = -progressDelta;
		
		float progress = opacityCurrent + progressDelta;
		
		progress = Math.Clamp(progress, 0, 1);
		
		float opacity = Math.Lerp(0, 1, progress);
		w.SetOpacity(opacity);
		w.SetVisible(true);
	}
};
