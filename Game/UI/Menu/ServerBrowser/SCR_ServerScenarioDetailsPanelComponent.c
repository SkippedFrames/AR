/*
Scenaraio panel with additional mods data.
Used mainly in server browser for displaying server(room) scenario and it's mods data
*/

class SCR_ServerScenarioDetailsPanelComponent : SCR_ScenarioDetailsPanelComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected string m_sDefaultScenarioImg;
	
	// Widget Names 
	const string WIDGET_IMG_NAME = "m_NameImg";
	const string WIDGET_MODS_OVERLAY = "m_AddonSizeOverlay";
	const string WIDGET_MODS_COUNT = "m_TxtModsCount";
	const string WIDGET_MODS_LOADING = "LoadingMods";
	const string WIDGET_MODS_ICON = "AddonSizeIcon";
	protected string WIDGET_MODS_LOCKED_ICON = "AddonsLockedIcon";
	
	const string WIDGET_BACKEND_IMAGE = "m_BackendImage";
	
	// Icons names 
	const string ICON_MODS_DONWLOADED = "check";
	const string ICON_MODS_MISSING = "download";
	
	// Localized strings 
	const string STR_VERSION_MISMATCH = "#AR-ServerBrowser_JoinModVersionMissmatch";
	const string STR_UNJOINABLE = "#AR-ServerBrowser_NoneServers";
	const string STR_MODS_MISSING = "#AR-ServerBrowser_JoinModMissing";
	const string STR_MODS_DONWLOADED = "#AR-Workshop_ButtonUpToDate";
	
	const string STR_DEFAULT_NAME = "#AR-MainMenu_Multiplayer_Name";
	const string STR_DEFAULT_DESCRIPTION = "#AR-MainMenu_Multiplayer_Description";
	const string STR_DEFAULT_VERSION = "v0.0.0.0";
	
	static string STR_MOD_HINT = "#AR-ServerBrowser_ClickServer";
	
	// Mods Widgets 
	protected ImageWidget m_wImgModsIcon = null;
	protected ImageWidget m_wImgModsLocked;
	protected ImageWidget m_wImgName = null;
	protected TextWidget m_wTxtModsCount = null;
	protected SCR_LoadingOverlay m_ModsLoading = null;
	
	protected SCR_ScenarioBackendImageComponent m_ScenarioBackendImage;
	
	// Server additional data 
	protected Room m_Room = null;
	protected SCR_RoomModsManager m_ModsManager = null;
	protected bool m_bHideScenarioImg = false; 
	
	//------------------------------------------------------------------------------------------------
	// Override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		super.HandlerAttached(w);
		
		// Name image 
		m_wImgName = ImageWidget.Cast(w.FindAnyWidget(WIDGET_IMG_NAME));
		
		// Get additional 
		m_wTxtModsCount = TextWidget.Cast(w.FindAnyWidget(WIDGET_MODS_COUNT));
		m_wImgModsIcon = ImageWidget.Cast(w.FindAnyWidget(WIDGET_MODS_ICON));
		m_wImgModsLocked = ImageWidget.Cast(w.FindAnyWidget(WIDGET_MODS_LOCKED_ICON));
		
		Widget wModsLoading = w.FindAnyWidget(WIDGET_MODS_LOADING);
		if (wModsLoading)
			m_ModsLoading = SCR_LoadingOverlay.Cast(wModsLoading.FindHandler(SCR_LoadingOverlay));
		
		// Backend image 
		Widget wScenarioBackendImage = w.FindAnyWidget(WIDGET_BACKEND_IMAGE);
		if (wScenarioBackendImage)
			m_ScenarioBackendImage = SCR_ScenarioBackendImageComponent.Cast(wScenarioBackendImage.FindHandler(SCR_ScenarioBackendImageComponent));
		
		// Setup widgets 
		SetDefaultScenario("");
		HideModsWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetScenario(MissionWorkshopItem scenario)
	{
		m_ScenarioBackendImage.GetEventOnImageSelected().Clear();
		m_ScenarioBackendImage.GetEventOnImageSelected().Insert(OnScenarioBackendImageSelected);
		
		DisplayRoomDataScenario(m_Room);
		
		// Update scenario
		m_Scenario = scenario;	
		if (scenario)
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(scenario.GetHeader());
			m_Header = header;
		}
		else
			m_Header = null;
		
		UpdateAllWidgets();
		
		// Set room version
		string strVersion = STR_DEFAULT_VERSION;
		
		if (!m_Room.Joinable())
			strVersion = STR_UNJOINABLE + "!";

		else if (m_Room.GameVersion())
		{
			strVersion = "v" + m_Room.GameVersion();
		
			if (m_Room.GameVersion() != GetGame().GetBuildVersion())
				strVersion += " - " + STR_VERSION_MISMATCH + "!";
		}
		
		m_Widgets.m_AuthorNameText.SetText(strVersion);
		
		m_wImgName.SetVisible(false);
		m_Widgets.m_TypeOverlay.SetVisible(true);
		m_Widgets.m_TypeText.SetText(m_Room.Name());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnScenarioBackendImageSelected()
	{
		m_ScenarioBackendImage.GetEventOnImageSelected().Remove(OnScenarioBackendImageSelected);
		m_Widgets.m_LoadingOverlay.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHideScenarioImg(bool hideContent) { m_bHideScenarioImg = hideContent; }
	
	//------------------------------------------------------------------------------------------------
	//! Change image behavior 
	override protected void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		if (m_bHideScenarioImg)
			m_Widgets.m_Image.LoadImageTexture(0, m_sDefaultScenarioImg, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// public functions 
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Find default scenario content by given tag 
	//! Should display fallback text when no scenario is selected for various reasons 
	void SetDefaultScenario(string contentTag)
	{
		// Get content 
		DetailsPanelContentPreset content = FallbackContentByTag(contentTag);
		if (!content)
		{
			#ifdef SB_DEBUG
			Print("[SCR_ServerScenarioDeatilsPanelComponent] No fallback content was found!");
			#endif
			return;	
		}

		// Set fallback image 
		if (!content.m_sImage.IsEmpty())
			m_Widgets.m_Image.LoadImageTexture(0, content.m_sImage);
		else
		{
			if (m_FallbackContent && m_FallbackContent.m_DefaultContent)
			{
				if (!m_FallbackContent.m_DefaultContent.m_sImage.IsEmpty())
					m_Widgets.m_Image.LoadImageTexture(0, m_FallbackContent.m_DefaultContent.m_sImage);
			}
		}
		
		// Correct image size 
		int sx, sy;
		m_Widgets.m_Image.GetImageSize(0, sx, sy);
		m_Widgets.m_Image.SetSize(sx, sy);
		m_Widgets.m_LoadingOverlay.SetVisible(false);
		
		// Title 
		string title = content.m_sTitle;
		//title.ToUpper();
		m_Widgets.m_NameText.SetText(title);
		if (!m_IconImageSet.IsEmpty() && !content.m_sTitleImageName.IsEmpty())
		{
			m_wImgName.SetVisible(true);
			m_wImgName.LoadImageFromSet(0, m_IconImageSet, content.m_sTitleImageName);
			m_wImgName.SetColor(content.m_sTitleImageColor);
		}
		
		// Description 
		m_Widgets.m_DescriptionText.SetText(content.m_sDescription);
		
		// Show default description 
		m_Widgets.m_Image.SetVisible(true);
		m_Widgets.m_NameText.SetVisible(true);
		m_Widgets.m_DescriptionText.SetVisible(true);
		
		m_Widgets.m_AuthorNameText.SetVisible(false);
		m_Widgets.m_TypeOverlay.SetVisible(false);
		if(m_Widgets.m_ImageCrossplayOverlay)
			m_Widgets.m_ImageCrossplayOverlay.SetVisible(false);
		
		HideModsWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void DisplayRoomData(Room room, bool showMods = true)
	{
		DisplayRoomDataScenario(room);
		m_Widgets.m_TopSize.SetOpacity(1);
		
		// Display mods loading 
		HideModsWidgets();
		
		bool show = room.IsModded() && showMods;
		
		m_ModsLoading.SetShown(show);
		m_wImgModsLocked.SetVisible(false);
		
		if (!room.IsModded())
			return;
		
		// Loading for public modded server
		if (!room.PasswordProtected())
		{
			m_ModsLoading.SetShown(show);
		}
		else
		{
			// Warning for protected modded server
			if (!room.IsAuthorized())
			{
				m_ModsLoading.SetShown(false);
		
				m_wImgModsIcon.SetVisible(true);
				m_wImgModsLocked.SetVisible(true);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------
	protected void DisplayRoomDataScenario(Room room)
	{
		m_Room = room;
		
		// Check room 
		if (!m_Room)
			return;
		
		// Check widgets 
		if (!m_Widgets.m_AuthorNameText || !m_Widgets.m_TypeText)
			return;
		
		m_Widgets.m_NameText.SetVisible(true);
		m_Widgets.m_AuthorNameText.SetVisible(true);
		m_Widgets.m_TypeOverlay.SetVisible(true);

		// Set room version
		string strVersion = STR_DEFAULT_VERSION;
		if (room.GameVersion())
			strVersion = "v" + room.GameVersion();
		
		// Scenario 
		string scenarioDescription = "";
		if (m_Scenario)
			scenarioDescription	= m_Scenario.Description();
		
		// Set content 
		string title = room.ScenarioName();
		//title.ToUpper();
		m_Widgets.m_NameText.SetText(title);
		m_Widgets.m_DescriptionText.SetText(scenarioDescription);
		m_Widgets.m_AuthorNameText.SetText(strVersion);
		m_Widgets.m_TypeText.SetText(room.Name());
		
		// Version match 
		if (room.GameVersion() == GetGame().GetBuildVersion() && room.Joinable())
		{
			m_Widgets.m_AuthorNameText.SetColor(m_ColorScheme.m_Moderate);
		}
		else
		{
			m_Widgets.m_AuthorNameText.SetColor(m_ColorScheme.m_Critical);
			
			string text = m_Widgets.m_AuthorNameText.GetText();
			if(room.Joinable())
				text += " - " + STR_VERSION_MISMATCH + "!";
			
			m_Widgets.m_AuthorNameText.SetText(text);
		}
		
		m_Widgets.m_LoadingOverlay.SetVisible(true);
		
		if(m_Widgets.m_ImageCrossplayOverlay)
			m_Widgets.m_ImageCrossplayOverlay.SetVisible(m_Room.IsCrossPlatform());
	}
	
	//-----------------------------------------------------------------------------------
	void DisplayDefaultScenarioImage()
	{
		if (m_ScenarioBackendImage)
			m_ScenarioBackendImage.SetScenarioAndImage(null, null);
		
		m_Widgets.m_LoadingOverlay.SetVisible(false);
	}

	//-----------------------------------------------------------------------------------
	//! Hide all widgets displaying info about mods
	//! Used mostly when server is not modded or not selected 
	protected void HideModsWidgets()
	{	
		m_wTxtModsCount.SetVisible(false);
		m_wImgModsIcon.SetVisible(false);
		m_Widgets.m_AddonSizeText.SetVisible(false);
		m_Widgets.m_AddonStateText.SetVisible(false);
		m_Widgets.m_AddonStateIcon.SetVisible(false);
		
		m_ModsLoading.SetShown(false);
	}
	
	//-----------------------------------------------------------------------------------
	//! Set mods to display in panel
	void DisplayMods()
	{
		//m_aMods = mods;
		
		// Check widgets  
		if (!m_wTxtModsCount || !m_Widgets.m_AddonSizeText || !m_Widgets.m_AddonStateText || !m_Widgets.m_AddonStateIcon)
			return;
		
		// Get values 
		array<ref SCR_WorkshopItem> allMods = m_ModsManager.GetRoomItemsScripted();
		
		int count = allMods.Count();
		m_ModsLoading.SetShown(false);
		
		// Show UGC restricted message
		if (count > 0 && !SCR_AddonManager.GetInstance().GetUgcPrivilege())
		{
			m_Widgets.m_AddonSizeText.SetVisible(true);
			m_Widgets.m_AddonSizeText.SetText("#AR-ServerBrowser_ContentNotAllowed");
			m_Widgets.m_AddonSizeText.SetColor(m_ColorScheme.m_Critical); 
			return;
		}
		
		// Hide mods widget if no mods 
		bool show = (count != 0);
		
		m_wImgModsIcon.SetVisible(show);
		m_wTxtModsCount.SetVisible(show);
		m_Widgets.m_AddonSizeText.SetVisible(show);
		m_Widgets.m_AddonStateText.SetVisible(show);
		m_Widgets.m_AddonStateIcon.SetVisible(show);
		
		if (!show)
			return;
		
		string totalSize = m_ModsManager.GetModListSizeString(allMods);
		
		// Set Text 
		m_wTxtModsCount.SetText(count.ToString());
		m_Widgets.m_AddonSizeText.SetText(totalSize);
		m_Widgets.m_AddonSizeText.SetColor(UIColors.LIGHT_GREY);
		
		// Set updated message as default 
		m_Widgets.m_AddonStateText.SetText(STR_MODS_DONWLOADED);
		m_Widgets.m_AddonStateText.SetColor(m_ColorScheme.m_Good);
		m_Widgets.m_AddonStateIcon.LoadImageFromSet(0, m_IconImageSet, ICON_MODS_DONWLOADED);
		m_Widgets.m_AddonStateIcon.SetColor(m_ColorScheme.m_Good);
		
		// Check mods to update size 
		array<ref SCR_WorkshopItem> toUpdateMods = m_ModsManager.GetRoomItemsToUpdate();
		
		// Check to update mods count 
		if (toUpdateMods.IsEmpty())
			return;
		
		// Change text if needs update
		string toUpdateSize = m_ModsManager.GetModListPatchSizeString(toUpdateMods);
		
		m_Widgets.m_AddonStateText.SetText(toUpdateSize);
		m_Widgets.m_AddonStateText.SetColor(m_ColorScheme.m_Download);
		m_Widgets.m_AddonStateIcon.LoadImageFromSet(0, m_IconImageSet, ICON_MODS_MISSING);
		m_Widgets.m_AddonStateIcon.SetColor(m_ColorScheme.m_Download); 
	}
	
	//-----------------------------------------------------------------------------------
	//! Quick display mods count
	void DisplayModsCount(int count)
	{
		// Show if modded 
		bool show = (count != 0);
		
		// Set widgets 
		m_wTxtModsCount.SetVisible(true);
		m_wImgModsIcon.SetVisible(true);
		m_Widgets.m_AddonSizeText.SetVisible(false);
		
		m_wTxtModsCount.SetText(count.ToString());
	}
	
	//-----------------------------------------------------------------------------------
	void SetModsManager(SCR_RoomModsManager manager) { m_ModsManager = manager; }
};
