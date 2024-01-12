// This component handles server entry and visiualization of server data
//
//------------------------------------------------------------------------------------------------

class SCR_ServerBrowserEntryComponent : SCR_ListMenuEntryComponent
{
	// Attributes
	[Attribute("999")]
	protected int m_iPingLimit;
	
	[Attribute("download")]
	protected string m_sTooltipDownloadIcon;
	
	[Attribute("1.75")]
	protected float m_fTooltipDownloadIconScale;

	// Const
	protected const string LAYOUT_CONTENT = "HorizontalLayout";
	protected const string LAYOUT_LOADING = "Loading";
	
	protected const string BUTTON_FAVORITE = "FavButton";
	protected const string BUTTON_JOIN = "JoinButton";
	protected const string BUTTON_PASSWORD = "PasswordButton";
	
	protected const string ICON_WARNING = "VersionWarningIcon";
	protected const string ICON_UNJOINABLE = "JoinWarningIcon";
	protected const string ICON_MODDED = "ImageModded";
	protected const string ICON_PING = "ImgPing";
	
	protected const string FRAME_NAME = "FrameName";
	protected const string FRAME_SCENARIO = "FrameScenario";
	
	protected const string TEXT_CELL = "Content";
	
	// Base
	protected Room m_RoomInfo;
	protected SCR_EServerEntryProperty m_iProperties;

	// Ping
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref ServerBrowserEntryProperty> m_aPingStates;

	// Backrounds and wrappers
	protected Widget m_wHorizontalContent;
	protected Widget m_wLoading;

	// Favorite widgets and behavior
	protected Widget m_wUnjoinableIcon;

	protected ImageWidget m_wImgPing;
	protected int m_iHighestPing = 0;

	protected Widget m_wImageModded;
	protected Widget m_wJoinButton;
	protected Widget m_wPasswordButton;

	protected SCR_RoomModsManager m_ModsManager;
	protected string m_sPatchSize;
	protected bool m_bIsPatchSizeLoaded;

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		if (!GetGame().InPlayMode())
			return;

		// Get wrappers
		m_wHorizontalContent = m_wRoot.FindAnyWidget(LAYOUT_CONTENT);
		m_wLoading = m_wRoot.FindAnyWidget(LAYOUT_LOADING);

		// Setup favorite button
		Widget favoriteButton = w.FindAnyWidget(BUTTON_FAVORITE);
		if (favoriteButton)
		{
			m_FavComponent = SCR_ModularButtonComponent.Cast(favoriteButton.FindHandler(SCR_ModularButtonComponent));

			m_wVersionWarningIcon = w.FindAnyWidget(ICON_WARNING);
			m_wUnjoinableIcon = w.FindAnyWidget(ICON_UNJOINABLE);
		}

		// Property images and buttons
		m_wImageModded = w.FindAnyWidget(ICON_MODDED);
		m_wJoinButton = w.FindAnyWidget(BUTTON_JOIN);
		m_wPasswordButton = w.FindAnyWidget(BUTTON_PASSWORD);

		// Mouse interaction buttons
		if (m_FavComponent)
			m_aMouseButtons.Insert(m_FavComponent);

		SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.FindComponent(m_wJoinButton);
		if (buttonComp)
		{
			buttonComp.m_OnClicked.Insert(OnJoinInteractionButtonClicked);
			m_aMouseButtons.Insert(buttonComp);
		}

		buttonComp = SCR_ModularButtonComponent.FindComponent(m_wPasswordButton);
		if (buttonComp)
		{
			buttonComp.m_OnClicked.Insert(OnJoinInteractionButtonClicked);
			m_aMouseButtons.Insert(buttonComp);
		}

		// Ping
		m_wImgPing = ImageWidget.Cast(w.FindAnyWidget(ICON_PING));

		// Name
		SCR_HorizontalScrollAnimationComponent scrollComp;
		Widget frameName = m_wRoot.FindAnyWidget(FRAME_NAME);
		if (frameName)
		{
			scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(frameName.FindHandler(SCR_HorizontalScrollAnimationComponent));
			if (scrollComp)
				m_aScrollAnimations.Insert(scrollComp);
		}

		// Scenario
		Widget frameScenario = m_wRoot.FindAnyWidget(FRAME_SCENARIO);
		if (frameScenario)
		{
			scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(frameScenario.FindHandler(SCR_HorizontalScrollAnimationComponent));
			if (scrollComp)
				m_aScrollAnimations.Insert(scrollComp);
		}

		// Loading
		m_wLoading.SetVisible(false);

		// Get highest ping
		for (int i = 0, count = m_aPingStates.Count(); i < count; i++)
		{
			int ping = m_aPingStates[i].m_sValue.ToInt();

			if (m_iHighestPing < ping)
				m_iHighestPing = ping;
		}

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		super.OnTooltipShow(tooltipClass, tooltipWidget, hoverWidget, preset, tag);

		string message = tooltipClass.GetDefaultMessage();

		switch (tag)
		{
			case "VersionMismatch":
				SCR_VersionMismatchTooltipComponent comp = SCR_VersionMismatchTooltipComponent.FindComponent(tooltipWidget);
				if (comp && m_RoomInfo)
					comp.SetWrongVersionMessage(m_RoomInfo.GameVersion());
				break;

			case "Join":
				if (m_bIsPatchSizeLoaded)
					message = string.Format(message, GetDownloadSizeMessage());

				tooltipClass.SetMessage(message);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		// Password and Play buttons
		if (m_wJoinButton)
			m_wJoinButton.SetVisible(m_bMouseButtonsEnabled && m_bFocused && !(m_iProperties & SCR_EServerEntryProperty.PASSWORD_PROTECTED) && !(m_iProperties & SCR_EServerEntryProperty.UNJOINABLE));

		super.UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_ModsManager)
		{
			m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
			m_bIsPatchSizeLoaded = false;
		}
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnJoinInteractionButtonClicked()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("");
	}

	//------------------------------------------------------------------------------------------------
	//! Set text in cell by it's widget name
	protected void SetCellText(string cellName, string str)
	{
		Widget wCell = m_wRoot.FindAnyWidget(cellName);
		if (!wCell)
			return;

		TextWidget wText = TextWidget.Cast(wCell.FindAnyWidget(TEXT_CELL));
		if (wText)
			wText.SetText(str);
	}

	//------------------------------------------------------------------------------------------------
	//! Display number of current ping and add icon and color
	protected void DisplayPing(int ping)
	{
		//TODO: the ping threshold are manually set in the layout. This values should be unified with the server browser threshold check and filters .conf 4F6F41C387ADC14E
		
		float lastHighest = m_iHighestPing + 1;
		ServerBrowserEntryProperty displayState;

		// No ping state
		if (ping == 0)
		{
			if (m_wImgPing)
				m_wImgPing.SetVisible(false);

			return;
		}

		// Go through available icons
		foreach (ServerBrowserEntryProperty pingState : m_aPingStates)
		{
			float pingFromStr = pingState.m_sValue.ToFloat();

			// Find smallest ping
			if (ping < pingFromStr && pingFromStr < lastHighest)
			{
				displayState = pingState;
				lastHighest = pingFromStr;
			}
		}

		// Set ping text
		string strPing = Math.Floor(ping).ToString();

		// Over limit
		if (ping > m_iHighestPing || ping < 0)
			displayState = m_aPingStates[m_aPingStates.Count() - 1];

		if (ping > m_iPingLimit)
			strPing = m_iPingLimit.ToString() + "#ENF-ComboModifier";

		SetCellText("Ping", strPing);

		// Set ping icon
		if (displayState && m_wImgPing)
		{
			m_wImgPing.SetVisible(true);
			m_wImgPing.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, displayState.m_sImageName);
			m_wImgPing.SetColor(displayState.m_Color);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Check properties and assign state of room
	protected void CheckRoomProperties()
	{
		m_iProperties = 0;

		// Client versions missmatch
		bool wrongVersion = m_RoomInfo.GameVersion() != GetGame().GetBuildVersion();
		bool restrictedUGC = m_RoomInfo.IsModded() && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		if (wrongVersion || restrictedUGC)
			m_iProperties |= SCR_EServerEntryProperty.VERSION_MISMATCH;

		// Locked with password
		if (m_RoomInfo.PasswordProtected())
			m_iProperties |= SCR_EServerEntryProperty.PASSWORD_PROTECTED;

		// Crossplay
		if (m_RoomInfo.IsCrossPlatform())
			m_iProperties |= SCR_EServerEntryProperty.CROSS_PLATFORM;

		// LAN - TODO@wernerjak - add local network check
		// m_iProperties |= SCR_EServerEntryProperty.LAN;

		// MODDED
		if (m_RoomInfo.IsModded())
			m_iProperties |= SCR_EServerEntryProperty.MODDED;

		// UNJOINABLE
		if (!m_RoomInfo.Joinable())
			m_iProperties |= SCR_EServerEntryProperty.UNJOINABLE;
	}

	//------------------------------------------------------------------------------------------------
	// Create property images based on room setup
	protected void DisplayServerProperties()
	{
		// Check states
		if (m_wImageModded)
			m_wImageModded.SetVisible(m_iProperties & SCR_EServerEntryProperty.MODDED);

		if (m_wPasswordButton)
			m_wPasswordButton.SetVisible(m_iProperties & SCR_EServerEntryProperty.PASSWORD_PROTECTED && !(m_iProperties & SCR_EServerEntryProperty.UNJOINABLE));

		// Turn favorites button into warning or unjoinable icon
		if (!m_FavComponent || !m_wVersionWarningIcon || !m_wUnjoinableIcon)
			return;

		bool versionMismatch = m_iProperties & SCR_EServerEntryProperty.VERSION_MISMATCH;
		bool unjoinable = m_iProperties & SCR_EServerEntryProperty.UNJOINABLE;

		m_FavComponent.SetEnabled(!versionMismatch && !unjoinable);

		m_wVersionWarningIcon.SetVisible(versionMismatch && !unjoinable);
		m_wUnjoinableIcon.SetVisible(unjoinable);

		m_bDisabled = versionMismatch || unjoinable;
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDownloadSizeMessage()
	{
		if (!m_ModsManager || !(m_iProperties & SCR_EServerEntryProperty.MODDED) || m_sPatchSize.IsEmpty())
			return string.Empty;

		string icon, color;
		
		icon = string.Format("<image set='%1' name='%2' scale='%3'/>", UIConstants.ICONS_IMAGE_SET, m_sTooltipDownloadIcon, m_fTooltipDownloadIconScale.ToString());
		return string.Format("  [%1%2 ]", icon, m_sPatchSize);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerDetailModsLoaded()
	{
		if (!m_ModsManager || !(m_iProperties & SCR_EServerEntryProperty.MODDED))
			return;
		
		array<ref SCR_WorkshopItem> toUpdateMods = m_ModsManager.GetRoomItemsToUpdate();
		if (!toUpdateMods.IsEmpty())
			m_sPatchSize = m_ModsManager.GetModListPatchSizeString(toUpdateMods);
		else
			m_sPatchSize = string.Empty;
		
		m_bIsPatchSizeLoaded = true;
		UpdateTooltipJoinDownloadSizeMessage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTooltipJoinDownloadSizeMessage()
	{
		if (!m_CurrentTooltip || !m_CurrentTooltip.IsVisible())
			return;
		
		switch (m_CurrentTooltip.GetTag())
		{
			case "Join":
				string message = m_CurrentTooltip.GetDefaultMessage();
				message = string.Format(message, GetDownloadSizeMessage());
				m_CurrentTooltip.SetMessage(message);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Set room and display room info in entry
	void SetRoomInfo(Room room)
	{
		m_RoomInfo = room;

		// Visualize as empty?
		if (!m_RoomInfo)
			return;

		// Setup room properties
		CheckRoomProperties();
		DisplayServerProperties();

		// Name
		SetCellText("Name", m_RoomInfo.Name());

		// Scenario
		SetCellText("Scenario", m_RoomInfo.ScenarioName());

		// Player count
		string playerCount = m_RoomInfo.PlayerCount().ToString();
		string playerCountMax = m_RoomInfo.PlayerLimit().ToString();

		SetCellText("Players", playerCount + "/" + playerCountMax);

		// Favorite
		if (m_FavComponent)
			SetFavorite(m_RoomInfo.IsFavorite());

		// Ping
		DisplayPing(room.GetPing());
	}

	//------------------------------------------------------------------------------------------------
	//! Set button visuals and behavior
	void EmptyVisuals(bool enable)
	{
		m_wHorizontalContent.SetVisible(!enable);
		m_wLoading.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	void SetModsManager(SCR_RoomModsManager modsManager)
	{
		m_ModsManager = modsManager;
		
		if (m_ModsManager)
		{
			m_ModsManager.GetOnGetAllDependencies().Insert(OnServerDetailModsLoaded);
			m_bIsPatchSizeLoaded = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	Room GetRoomInfo()
	{
		return m_RoomInfo;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsModded()
	{
		return m_iProperties & SCR_EServerEntryProperty.MODDED;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsEnabled(out bool versionMismatch, out bool unjoinable)
	{
		versionMismatch = m_iProperties & SCR_EServerEntryProperty.VERSION_MISMATCH;
		unjoinable = m_iProperties & SCR_EServerEntryProperty.UNJOINABLE;
		return !versionMismatch && !unjoinable;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class ServerBrowserEntryProperty
{
	[Attribute("1", UIWidgets.ComboBox, "", category:"Selector", ParamEnumArray.FromEnum(SCR_EServerEntryProperty))]
	SCR_EServerEntryProperty m_iPropertyState;

	[Attribute("", UIWidgets.EditBox, desc: "Image name to load")]
	string m_sImageName;

	[Attribute("", UIWidgets.EditBox, desc: "String value of property")]
	string m_sValue;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_Color;
}

//------------------------------------------------------------------------------------------------
enum SCR_EServerEntryProperty
{
	VERSION_MISMATCH = 1<<0,
	PASSWORD_PROTECTED = 1<<1,
	CROSS_PLATFORM = 1<<2,
	LAN = 1<<3,
	MODDED = 1<<4,
	UNJOINABLE = 1<<5
}
