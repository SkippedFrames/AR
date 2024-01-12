//------------------------------------------------------------------------------------------------
class SCR_AccountWidgetComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(UIColors.GetColorAttribute(UIColors.CONFIRM), UIWidgets.ColorPicker)]
	protected ref Color m_ColorOnline;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_ColorOffline;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_ColorConnecting;

	[Attribute(SCR_ServicesStatusHelper.ICON_CONNECTION)]
	protected string m_sIconOnline;

	[Attribute(SCR_ServicesStatusHelper.ICON_SERVICES_ISSUES)]
	protected string m_sIconOffline;

	[Attribute(SCR_ServicesStatusHelper.ICON_CONNECTION)]
	protected string m_sIconConnecting;
	
	[Attribute("0")]
	protected bool m_bShowOnlineIcon;
	
	[Attribute("Profile")]
	protected string m_sTooltipTag;
	
	[Attribute("#AR-Account_AuthenticationFailed")]
	protected string m_sTooltipMessageOffline;
	
	[Attribute("#AR-Account_Authenticating")]
	protected string m_sTooltipMessageConnecting;
	
	protected SCR_CoreMenuHeaderButtonComponent m_News;
	protected SCR_CoreMenuHeaderButtonComponent m_Career;

	protected SCR_ModularButtonComponent m_Profile;
	protected SCR_DynamicIconComponent m_ProfileStatusIcon;

	protected bool m_bLoggedIn;
	protected BackendApi m_BackendApi;
	
	protected SCR_ScriptedWidgetTooltip m_Tooltip;

	static const int AUTH_CHECK_PERIOD = 1000;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_News = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "NewsButton", w));
		if (m_News)
			m_News.GetButton().m_OnClicked.Insert(OnNews);

		m_Career = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "CareerButton", w));
		if (m_Career)
			m_Career.GetButton().m_OnClicked.Insert(OnCareer);

		Widget profile = w.FindAnyWidget("Profile");
		if (profile)
		{
			m_Profile = SCR_ModularButtonComponent.FindComponent(profile);
			if (m_Profile)
				m_Profile.m_OnClicked.Insert(OnProfile);
			
			m_ProfileStatusIcon = SCR_DynamicIconComponent.FindComponent("DotOverlay", profile);
			if (m_ProfileStatusIcon)
				m_ProfileStatusIcon.SetIconColor(m_ColorOffline);
		}
		
		m_BackendApi = GetGame().GetBackendApi();
		
		UpdateAuthentication();
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Insert(OnMenuDisabled);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Remove(OnMenuDisabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuEnabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			GetGame().GetCallqueue().Remove(UpdateAuthentication);
			GetGame().GetCallqueue().CallLater(UpdateAuthentication, AUTH_CHECK_PERIOD, true);
			
			SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuDisabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			GetGame().GetCallqueue().Remove(UpdateAuthentication);
			
			SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		if (tag == m_sTooltipTag)
		{
			m_Tooltip = tooltipClass;
			UpdateAuthentication();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAuthentication()
	{
		if (!m_BackendApi)
		{
			m_BackendApi = GetGame().GetBackendApi();
			if (!m_BackendApi)
				return;
		}

		if (!m_ProfileStatusIcon)
			return;

		m_ProfileStatusIcon.SetVisibile(true);
		
		Color color = m_ColorOffline;
		string image = m_sIconOffline;
		
		string tooltipMessage = m_sTooltipMessageOffline;
		Color tooltipMessageColor = m_ColorOffline;
		
		if (m_BackendApi.IsAuthInProgress())
		{
			color = m_ColorConnecting;
			image = m_sIconConnecting;
			
			tooltipMessage = m_sTooltipMessageConnecting;
			tooltipMessageColor = m_ColorConnecting;
		}
		
		else if (m_BackendApi.IsAuthenticated())
		{
			color = m_ColorOnline;
			image = m_sIconOnline;
			
			m_ProfileStatusIcon.SetVisibile(m_bShowOnlineIcon);
			
			if (m_Tooltip)
				tooltipMessage = m_Tooltip.GetDefaultMessage();
			
			tooltipMessageColor = Color.White;
		}

		m_ProfileStatusIcon.SetIconColor(color);
		m_ProfileStatusIcon.SetImage(image);
		
		if (m_Tooltip)
		{
			m_Tooltip.SetMessage(tooltipMessage);
			m_Tooltip.SetMessageColor(tooltipMessageColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNews()
	{
		SCR_ProfileSuperMenu menu = SCR_ProfileSuperMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ProfileSuperMenu, 0, true));
		if (menu)
			menu.SetPage(SCR_EProfileSuperMenuTabId.NEWS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCareer()
	{
		//TODO: uncomment once Career Menu is finished
		//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CareerProfileMenu, 0, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnProfile()
	{
		// Show profile screen
		if (m_BackendApi.IsBIAccountLinked())
			SCR_LoginProcessDialogUI.CreateProfileDialog();
		else
			SCR_LoginProcessDialogUI.CreateLoginDialog();
	}
}
