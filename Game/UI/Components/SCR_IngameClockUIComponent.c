/**
Gets the Ingame time and displays it on an UI text element.
*/
class SCR_IngameClockUIComponent : MenuRootSubComponent 
{			
	[Attribute("Time", desc: "Clock needs at least the Time icon, time multiplier or Time Text to function")]
	protected string m_sTimeTextName;
	
	[Attribute("DayTime_Icon", desc: "Clock needs at least the Time icon, time multiplier or Time Text to function")]
	protected string m_sTimeIconName;
	
	[Attribute("Time_Multiplier", desc: "Clock needs at least the Time icon, time multiplier or Time Text to function")]
	protected string m_sTimeMultiplierName;
	
	[Attribute("#AR-ValueUnit_Short_Multiplier", uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sTimeMultiplierFormat;
	
	[Attribute("#AR-Weather_Forecast_NoTimeProgression", uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sNoTimeProgression;
	
	[Attribute("1", desc: "Time until the UI will update in seconds")]
	protected float m_fUpdateFreq;
	
	[Attribute("1", desc: "If true will hide the time multiplier if one (and assigned)")]
	protected bool m_bHideTimeMultiplierIfOne;
	
	[Attribute(desc: "If the current time of day is in this Phase show moonphase instead of daytimeIcon", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDayTimeEnums))]
	protected ref array<EDayTimeEnums> m_aShowMoonphaseAtDayTimes;

	//~ References
	protected TextWidget m_TimeText;
	protected ImageWidget m_TimeIcon;
	protected TextWidget m_TimeMultiplier;
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManagerEntity;
	

	//======================== CLOCK UPDATE ========================\\
	protected void UpdateIngameClock()
	{
		//Get time
		if (m_TimeText)
		{
			int hour, minute, sec;
			m_TimeAndWeatherManagerEntity.GetHoursMinutesSeconds(hour, minute, sec);
			m_TimeText.SetTextFormat(SCR_FormatHelper.GetTimeFormattingHoursMinutes(hour, minute));
		}
		
		if (m_TimeIcon)
		{			
			SCR_UIInfo dayTimeUIInfo;
			EDayTimeEnums dayTimePhase = m_TimeAndWeatherManagerEntity.GetCurrentDayTimeUIInfoAndPhase(dayTimeUIInfo);
			
			if (m_aShowMoonphaseAtDayTimes.Contains(dayTimePhase))
			{
				SCR_MoonPhaseUIInfo moonPhase = m_TimeAndWeatherManagerEntity.GetCurrentMoonPhaseInfoForDate();
				m_TimeIcon.SetRotation(moonPhase.GetMoonphaseImageRotation());
				moonPhase.SetIconTo(m_TimeIcon);
			}
			else 
			{
				m_TimeIcon.SetRotation(0);
				dayTimeUIInfo.SetIconTo(m_TimeIcon);
			}
		}
		
		if (m_TimeMultiplier)
		{
			if (m_TimeAndWeatherManagerEntity.GetIsDayAutoAdvanced())
			{
				float timeMultiplier = 86400 / m_TimeAndWeatherManagerEntity.GetDayDuration();	
				int TimeMultiplierInt = timeMultiplier;
				
				bool visible = !(m_bHideTimeMultiplierIfOne && (TimeMultiplierInt == 1 && (timeMultiplier - TimeMultiplierInt) < 0.1));
				m_TimeMultiplier.SetVisible(visible);
				
				if (visible)
				{
					if ((timeMultiplier - TimeMultiplierInt) != 0)
						m_TimeMultiplier.SetTextFormat(m_sTimeMultiplierFormat, timeMultiplier.ToString(-1, 1));
					else 
						m_TimeMultiplier.SetTextFormat(m_sTimeMultiplierFormat, TimeMultiplierInt);
				}
			}
			else 
			{
				m_TimeMultiplier.SetText(m_sNoTimeProgression);
				m_TimeMultiplier.SetVisible(true);
			}
		}
			
	}
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//On Init
	override void HandlerAttachedScripted(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_TimeText = TextWidget.Cast(w.FindAnyWidget(m_sTimeTextName));
		m_TimeIcon = ImageWidget.Cast(w.FindAnyWidget(m_sTimeIconName));
		m_TimeMultiplier = TextWidget.Cast(w.FindAnyWidget(m_sTimeMultiplierName));
		
		if (!m_TimeText && !m_TimeIcon && !m_TimeMultiplier)
		{
			Print("'SCR_IngameClockUIComponent' is has no time time widgets and therefore won't function", LogLevel.ERROR);
			return;
		}		

		ChimeraWorld world = GetGame().GetWorld();
		m_TimeAndWeatherManagerEntity = world.GetTimeAndWeatherManager();
		if (!m_TimeAndWeatherManagerEntity)
		{
			Print("'SCR_IngameClockUIComponent' could not find the TimeAndWeatherManager", LogLevel.ERROR);
			return;
		}
		
		UpdateIngameClock();
		GetGame().GetCallqueue().CallLater(UpdateIngameClock, m_fUpdateFreq * 1000, true);
	}
	
	
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		if (m_TimeAndWeatherManagerEntity)
			GetGame().GetCallqueue().Remove(UpdateIngameClock);
		
	}
};
