//------------------------------------------------------------------------------------------------
class SCR_PlayMenuTileComponent : SCR_TileBaseComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Is this a big vertical tile? E.g. will use hi-res thumb picture..")]
	protected bool m_bBigTile;

	[Attribute("480", UIWidgets.Slider, "Estimated width of grid thumb picture, in a reference resolution.", "320 1920 1")]
	protected float m_fThumbnailWidth;

	MissionWorkshopItem m_Item;
	ref SCR_MissionHeader m_Header;
	string m_sScenarioPath;

	Widget m_wContentGroup;
	Widget m_wFeatured;
	TextWidget m_wName;
	TextWidget m_wDescription;
	Widget m_wRecentlyPlayed;
	TextWidget m_wRecentlyPlayedText;

	// Mouse interact buttons
	Widget m_wMouseInteractButtons;
	Widget m_wPlay;
	Widget m_wContinue;
	Widget m_wRestart;
	Widget m_wHost;
	Widget m_wFindServer;

	bool m_bIsMouseInteraction;

	// Button components
	SCR_ModularButtonComponent m_Play;
	SCR_ModularButtonComponent m_Continue;
	SCR_ModularButtonComponent m_FindServer;
	SCR_ModularButtonComponent m_Host;
	SCR_ModularButtonComponent m_Restart;

	// Script invokers for the mouse interact buttons
	ref ScriptInvoker m_OnPlay = new ScriptInvoker();
	ref ScriptInvoker m_OnContinue = new ScriptInvoker();
	ref ScriptInvoker m_OnRestart = new ScriptInvoker();
	ref ScriptInvoker m_OnHost = new ScriptInvoker();
	ref ScriptInvoker m_OnFindServer = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wFeatured = w.FindAnyWidget("Featured");
		m_wContentGroup = w.FindAnyWidget("ContentGroup");

		m_wRecentlyPlayed = w.FindAnyWidget("RecentlyPlayed");
		m_wRecentlyPlayedText = TextWidget.Cast(m_wRecentlyPlayed.FindAnyWidget("Label"));

		m_wMouseInteractButtons = w.FindAnyWidget("MouseInteractButtons");
		m_wPlay = w.FindAnyWidget("Play");
		m_wContinue = w.FindAnyWidget("Continue");
		m_wRestart = w.FindAnyWidget("Restart");
		m_wHost = w.FindAnyWidget("Host");
		m_wFindServer = w.FindAnyWidget("FindServer");

		m_Play = SCR_ModularButtonComponent.FindComponent(m_wPlay);
		if (m_Play)
			m_Play.m_OnClicked.Insert(OnPlay);

		m_Continue = SCR_ModularButtonComponent.FindComponent(m_wContinue);
		if (m_Continue)
			m_Continue.m_OnClicked.Insert(OnContinue);

		m_Restart = SCR_ModularButtonComponent.FindComponent(m_wRestart);
		if (m_Restart)
			m_Restart.m_OnClicked.Insert(OnRestart);

		m_Host = SCR_ModularButtonComponent.FindComponent(m_wHost);
		if (m_Host)
			m_Host.m_OnClicked.Insert(OnHost);

		m_FindServer = SCR_ModularButtonComponent.FindComponent(m_wFindServer);
		if (m_FindServer)
			m_FindServer.m_OnClicked.Insert(OnFindServer);

		if (GetGame().InPlayMode())
			Enable(false);

		InputManager inputManager = GetGame().GetInputManager();

		if (inputManager)
			m_bIsMouseInteraction = (inputManager.GetLastUsedInputDevice() == EInputDeviceType.MOUSE);

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
		
		UpdateConnectionButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		//PrintFormat("[OnFocus] %1 | %2 | mouse interaction: %3", this, m_wName.GetText(), m_bIsMouseInteraction);

		super.OnFocus(w, x, y);
		m_wMouseInteractButtons.SetVisible(/*m_bIsMouseInteraction*/ true);
		UpdateRestartButtonColor(true);
		//m_wDescription.SetVisible(true);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		//PrintFormat("[OnFocusLost] %1 | %2 | mouse interaction: %3", this, m_wName.GetText(), m_bIsMouseInteraction);

		super.OnFocusLost(w, x, y);
		m_wMouseInteractButtons.SetVisible(false);
		UpdateRestartButtonColor(false);
		//m_wDescription.SetVisible(false);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRestartButtonColor(bool focused)
	{
		Color color = UIColors.IDLE_DISABLED;
		if (focused)
			color = UIColors.NEUTRAL_ACTIVE_STANDBY;

		SCR_ButtonEffectColor colorEffect = SCR_ButtonEffectColor.Cast(m_Restart.FindEffect("IconColor"));
		if (!colorEffect)
			return;
		
		colorEffect.m_cDefault = color;
		m_Restart.InvokeAllEnabledEffects(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateConnectionButtons()
	{
		if (m_Host)
			m_Host.SetEnabled(SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable());
		
		if (m_FindServer)
			m_FindServer.SetEnabled(SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable());
	}
	
	//------------------------------------------------------------------------------------------------
	void Setup(notnull MissionWorkshopItem item, EPlayMenuContentType contentType)
	{
		m_Item = item;

		// Init header only if scenario is NOT coming from an addon
		if (!item.GetOwner())
			m_Header = SCR_MissionHeader.GetMissionHeader(item);

		m_wName.SetText(item.Name());
		m_wDescription.SetText(item.Description());

		bool canContinue = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);

		m_wPlay.SetVisible(!canContinue);
		m_wContinue.SetVisible(canContinue);
		m_wRestart.SetVisible(canContinue);
		UpdateRestartButtonColor(false);
		m_wHost.SetVisible(!GetGame().IsPlatformGameConsole() && item.GetPlayerCount() > 1);
		m_wFindServer.SetVisible(item.GetPlayerCount() > 1);

		// Set image through SCR_ButtonImageComponent
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			ResourceName texture = GetTexture();

			//PrintFormat("%1 | texture: %2", item.Name(), texture);

			if (!texture.IsEmpty())
				comp.SetImage(texture, item.GetOwner() != null);
		}

		//DEBUG
		/*
		string savefile_item = SCR_SaveLoadComponent.GetSaveFileName(item);
		Print(savefile_item);
		string savefile_header = SCR_SaveLoadComponent.GetSaveFileName(m_Header);
		Print(savefile_header);
		*/

		m_wFeatured.SetVisible(contentType == EPlayMenuContentType.FEATURED);
		m_wRecentlyPlayed.SetVisible(contentType == EPlayMenuContentType.RECENT);

		if (contentType == EPlayMenuContentType.RECENT)
		{
			int timeSinceLastPlayedSeconds = m_Item.GetTimeSinceLastPlay();
			string sLastPlayed = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);

			m_wRecentlyPlayedText.SetText(sLastPlayed);
		}

		m_wMouseInteractButtons.SetVisible(false);

		Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected void Enable(bool enable = true)
	{
		m_wRoot.SetEnabled(enable);
		m_wContentGroup.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetTexture()
	{
		if (m_bBigTile && m_Header && !m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;

		if (m_Header)
			return m_Header.m_sLoadingScreen;

		if (m_Item)
		{
			BackendImage image = m_Item.Thumbnail();

			// DEBUG problems with scenario addon thumbs
			/*
			array<ImageScale> scales = {};
			int i = image.GetScales(scales);
			*/

			// Get optimal width for the thumb
			float width = g_Game.GetWorkspace().DPIScale(m_fThumbnailWidth);
			ImageScale scale = image.GetLocalScale((int)width);

			if (!scale)
				return "";

			string path = scale.Path();

			return path;
		}

		return "";
	}

	// React on switching between input methods
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		//PrintFormat("OnInputDeviceUserChanged | %1 -> %2", oldDevice, newDevice);

		if (newDevice == EInputDeviceType.TRACK_IR || newDevice == EInputDeviceType.JOYSTICK)
			return;

		m_bIsMouseInteraction = (newDevice == EInputDeviceType.MOUSE);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlay()
	{
		m_OnPlay.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnContinue()
	{
		m_OnContinue.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnRestart()
	{
		m_OnRestart.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnHost()
	{
		m_OnHost.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnFindServer()
	{
		m_OnFindServer.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateConnectionButtons();
	}
}
