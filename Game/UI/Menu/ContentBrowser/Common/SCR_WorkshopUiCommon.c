/*
Common functions related to Workshop UI.
*/

class SCR_WorkshopUiCommon
{
	const float IMAGE_SIZE_RATIO = 16.0 / 9.0;	

	static const ResourceName DIALOGS_CONFIG = "{26F075E5D30629E5}Configs/ContentBrowser/ContentBrowserDialogs.conf";
	
	// Time interval which is considered 'instant' enough that we don't need to show the loading overlay
	static const float NO_LOADING_OVERLAY_DURATION_MS = 500;
	
	// Default addon thumbnail if it wasn't resolved
	static const ResourceName ADDON_DEFAULT_THUMBNAIL = "{04EB797EBF59CDEF}UI/Textures/Workshop/AddonThumbnails/workshop_defaultFallback_UI.edds";
	static const ResourceName SCENARIO_SP_DEFAULT_THUMBNAIL = "{17D65C6D78C7722C}UI/Textures/Workshop/AddonThumbnails/workshop_scenarios_UI.edds";
	static const ResourceName SCENARIO_MP_DEFAULT_THUMBNAIL = "{62A03BAAAED612E8}UI/Textures/Workshop/AddonThumbnails/workshop_mpScenarios_UI.edds";
	
	// Map which maps addon tags to icon names
	static const ResourceName ADDON_TYPE_FILTER_CATEGORY_CONFIG = "{A557E41062372854}Configs/ContentBrowser/Filters/category_type.conf";
	static ResourceName s_sAddonTagImageSet;
	static ref map<string, string> s_sAddonTagImageMap;
	
	// Map which maps addon tags to default pictures
	static ref map<string, ResourceName> s_sAddonTagDefaultThumbnailMap;
	
	static const string WIDGET_LIST = "AddonList";
	
	
	//------------------------------------------------------------------------------------------------
	// P U B L I C   M E T H O D S 
	//------------------------------------------------------------------------------------------------
	
	
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Call this somewhere once, preferably at start of the game.
	static void OnGameStart()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnAddonsChecked.Remove(SCR_WorkshopUiCommon.Callback_OnAddonsChecked);
		mgr.m_OnAddonsChecked.Insert(SCR_WorkshopUiCommon.Callback_OnAddonsChecked);
		mgr.CheckAddons();
		
		InitAddonTagMaps();
	}
	
	
	
	//---------------------------------------------------------------------------------------------
	//! Tries to open Workshop UI, if not possible due to privileges, negotiates them.
	static void TryOpenWorkshop()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (mgr.GetUgcPrivilege())
			ContentBrowserUI.Create();
		else
		{
			mgr.m_OnUgcPrivilegeResult.Insert(OnTryOpenWorkshopUgcPrivilegeResult);
			mgr.NegotiateUgcPrivilegeAsync();
		}
	}
	
	protected static void OnTryOpenWorkshopUgcPrivilegeResult(bool result)
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnUgcPrivilegeResult.Remove(OnTryOpenWorkshopUgcPrivilegeResult);
		if (result)
			ContentBrowserUI.Create();
	}
	
	
		
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
	
	//---------------------------------------------------------------------------------------------
	//! Returns image associated with given Workshop tag
	static void GetTagImage(string tag, out ResourceName imageSet, out string image)
	{
		if (!s_sAddonTagImageSet)
		{
			imageSet = string.Empty;
			image = string.Empty;
			return;
		}
		
		tag.ToLower();
		string _image = s_sAddonTagImageMap.Get(tag);
		image = _image;
		imageSet = s_sAddonTagImageSet;
	}
	
	
	//---------------------------------------------------------------------------------------------
	static ResourceName GetDefaultAddonThumbnail(notnull SCR_WorkshopItem item)
	{
		array<WorkshopTag> tagObjs = {};
		
		WorkshopItem internalItem = item.GetWorkshopItem();
		
		if (!internalItem || !s_sAddonTagDefaultThumbnailMap)
			return ADDON_DEFAULT_THUMBNAIL;
		
		// Iterate all tags, return first found default iamge
		internalItem.GetTags(tagObjs);
		foreach (WorkshopTag tagObj : tagObjs)
		{
			string tagStr = tagObj.Name();
			tagStr.ToLower();
			
			if (s_sAddonTagDefaultThumbnailMap.Contains(tagStr))
				return s_sAddonTagDefaultThumbnailMap.Get(tagStr);
		}

		return ADDON_DEFAULT_THUMBNAIL;
	}
	
	
	//------------------------------------------------------------------------------------------------
	static bool GetConnectionState()
	{	
		auto backend = GetGame().GetBackendApi();
		bool connected = backend.IsActive() && backend.IsAuthenticated();
		return connected;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns prefered image width for grid screen.
	static int GetPreferedTileImageWidth()
	{
		const float scale = 260.0/1920.0; // Estimated tile width is ~260 px
		float w, h;
		GetGame().GetWorkspace().GetScreenSize(w, h);
		int widthInt = scale*w;
		return widthInt;
	}
	
	
	//------------------------------------------------------------------------------------------------
	// S C E N A R I O   S T A R T I N G
	
	
	//------------------------------------------------------------------------------------------------
	//! Starts the mission or opens a confirmation dialog if it can't be immediately started
	static void TryPlayScenario(MissionWorkshopItem scenario)
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		if (nTotal > 0)
			new SCR_StartScenarioWhileDownloadingDialog(scenario, false);
		else
			scenario.Play();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hosts the scenario or opens a confirmation dialog if it can't be immediately started - TODO remove?
	static void TryHostScenario(MissionWorkshopItem scenario)
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		if (nTotal > 0)
			new SCR_StartScenarioWhileDownloadingDialog(scenario, true);
		else	
		{
			ref DSConfig config = new DSConfig;
			ref DSGameConfig game = new DSGameConfig;
			config.game = game;
			game.scenarioId = scenario.Id();
			//game.maxPlayers = scenario.GetPlayerCount();
			game.name = scenario.Name() + System.GetMachineName();
			WorkshopItem hostedMod = scenario.GetOwner();
			if (hostedMod)
				game.hostedScenarioModId = hostedMod.Id();
			
			ref array<WorkshopItem> offlineMods = new array<WorkshopItem>;
			ref array<ref DSMod> modIds = new array<ref DSMod>;
			GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(offlineMods);
			foreach(auto mod : offlineMods)
			{
				ref DSMod modData = new DSMod;
				modData.modId = mod.Id();
				modData.name = mod.Name();
				modData.version = mod.GetActiveRevision().GetVersion();
				modIds.Insert(modData);
			}
			config.game.mods = modIds;
			
			scenario.Host(config);
		}
	}
	
	
	
	
	
	//------------------------------------------------------------------------------------------------
	// A D D O N   S T A T E S   A N D   A C T I O N S
	
	//---------------------------------------------------------------------------------------------
	//! Toggles addon enabled state. Must be called from non-toggleable buttons.
	//! Might show confirmation dialog.
	static void OnEnableAddonButton(SCR_WorkshopItem item)
	{	
		if (!item)
			return;
		
		if (!item.GetOffline() || item.GetRestricted())
			return;
		
		bool newEnabled = !item.GetEnabled();
		SetAddonEnabled(item, newEnabled);
	}
	
	
	//---------------------------------------------------------------------------------------------
	//! Sets addon enabled state from a toggleable button.
	//! Might show confirmation dialog.
	static void OnEnableAddonToggleButton(SCR_WorkshopItem item, SCR_ModularButtonComponent buttonComp)
	{
		if (!item)
			return;
		
		if (!item.GetOffline() || item.GetRestricted())
			return;
		
		// Button was clicked and it is now toggled or not, and this directly sets addon state
		bool newEnabled = buttonComp.GetToggled();
		SetAddonEnabled(item, newEnabled);
	}
	
	//---------------------------------------------------------------------------------------------
	//! Shows a confirmation dialog when unsubscribing an addon.
	static void OnUnsubscribeAddonButton(SCR_WorkshopItem item)
	{
		// Check if some addons depend on this
		array<ref SCR_WorkshopItem> offlineAndDependent = SCR_AddonManager.SelectItemsAnd(
			item.GetDependentAddons(),
			EWorkshopItemQuery.OFFLINE);
		
		if (!offlineAndDependent.IsEmpty())
			SCR_DeleteAddonDependentAddonsDetected.Create(offlineAndDependent, item);	// Some addons depend on this, show a special confirmation
		else
			SCR_DeleteAddonDialog.CreateDeleteAddon(item);	// Show a standard confirmation for addon deletion
	}
	
	
	//---------------------------------------------------------------------------------------------
	//! Returns short description of highest priority problem of an item according to GetHighestPriorityProblem() value.
	static bool GetHighestPriorityProblemDescription(SCR_WorkshopItem item, out string descriptionShort, out bool critical)
	{
		// Not downloaded yet - no problem
		if (!item.GetOffline())
			return false;
		
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();
		
		switch (problem)
		{
			case EWorkshopItemProblem.DEPENDENCY_MISSING:
				descriptionShort = "#AR-Workshop_State_MissingDependencies";
				critical = true;
				return true;
			
			case EWorkshopItemProblem.DEPENDENCY_DISABLED:
				descriptionShort = "#AR-Workshop_State_MustEnableDependencies";
				critical = true;
				return true;
			
			case EWorkshopItemProblem.UPDATE_AVAILABLE:
				descriptionShort = "#AR-Workshop_State_UpdateAvailable";
				critical = false;
				return true;
			
			case EWorkshopItemProblem.DEPENDENCY_OUTDATED:
				descriptionShort = "#AR-Workshop_State_DependencyUpdateAvailable";
				critical = false;
				return true;
			
			default:
				descriptionShort = string.Empty;
				critical = false;
				return false;
		}
		
		descriptionShort = string.Empty;
		critical = false;
		
		return false;
	}
	
	
	
	//---------------------------------------------------------------------------------------------
	//! Returns name of suggested primary action
	static string GetPrimaryActionName(SCR_WorkshopItem item)
	{	
		bool downloading = item.GetDownloadAction() != null || item.GetDependencyCompositeAction() != null;
		
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();
		
		if (!downloading)
		{
			if (!item.GetOffline())
				return "#AR-Workshop_ButtonDownload";
			else
			{
				switch (problem)
				{
					case EWorkshopItemProblem.DEPENDENCY_MISSING:	return "#AR-Workshop_ButtonDownloadDependencies";
					case EWorkshopItemProblem.DEPENDENCY_DISABLED:	return "#AR-Workshop_ButtonEnableDependencies";
					case EWorkshopItemProblem.UPDATE_AVAILABLE:		return "#AR-Workshop_ButtonUpdate";
					case EWorkshopItemProblem.DEPENDENCY_OUTDATED:	return "#AR-Workshop_ButtonUpdateDependencies";
					default:										return string.Empty;
				}
			}
		}
		else
		{
			// Downloading
			return "#AR-Workshop_ButtonCancel";
		}
		
		return string.Empty;
	}
	
	
	
	//---------------------------------------------------------------------------------------------
	//! Returns string such as "Banned" or "Reported" for a restricted item. Otherwise returns empty string.
	static string GetRestrictedAddonStateText(SCR_WorkshopItem item)
	{
		if (!item.GetRestricted())
			return string.Empty;
		
		if (item.GetReportedByMe())
			return "#AR-Workshop_State_Reported";
		else if (item.GetBlocked())
			return "#AR-Workshop_State_Banned";
		else
			return "#AR-Workshop_State_Restricted";
	}
	
	protected static ref ScriptInvoker<> m_OnDownloadConfirmDisplayed = new ScriptInvoker<>();
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnDownloadConfirmDisplayed()
	{
		return m_OnDownloadConfirmDisplayed;
	}
	
	//---------------------------------------------------------------------------------------------
	//! Performs the primary suggested action
	//! You must store the dlRequest somewhere (preferably in UI), in case that the download will be started.
	static void ExecutePrimaryAction(notnull SCR_WorkshopItem item, out SCR_WorkshopDownloadSequence dlRequest)
	{
		bool downloading = item.GetDownloadAction() != null || item.GetDependencyCompositeAction() != null;
		
		// Paused -- todo
		
		// Perform action according to problem
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();
		
		if (!downloading)
		{
			if (!item.GetOffline())
			{
				dlRequest = SCR_WorkshopDownloadSequence.Create(item, item.GetLatestRevision(), dlRequest);
				dlRequest.GetOnDownloadConfirmDisplayed().Remove(OnDownloadConfirmDisplayed);
				dlRequest.GetOnDownloadConfirmDisplayed().Insert(OnDownloadConfirmDisplayed);
				return;
			}
			else
			{
				switch (problem)
				{
					// Start download/update of whatever is possible to download or update
					case EWorkshopItemProblem.DEPENDENCY_MISSING:
					case EWorkshopItemProblem.UPDATE_AVAILABLE:
					case EWorkshopItemProblem.DEPENDENCY_OUTDATED:
					{
						dlRequest = SCR_WorkshopDownloadSequence.Create(item, item.GetLatestRevision(), dlRequest);
						dlRequest.GetOnDownloadConfirmDisplayed().Remove(OnDownloadConfirmDisplayed);
						dlRequest.GetOnDownloadConfirmDisplayed().Insert(OnDownloadConfirmDisplayed);
						return;
					}
					
					case EWorkshopItemProblem.DEPENDENCY_DISABLED:
					{
						item.SetDependenciesEnabled(true);
						return;
					}
				}
			}
		}
		else
		{
			// Aggregate all actions of this item and create a dialog to confirm disabling all current downloads
			SCR_WorkshopItemActionDownload actionThisItem = item.GetDownloadAction();
			SCR_WorkshopItemActionComposite actionDependenciesThisItem = item.GetDependencyCompositeAction();
			
			array<ref SCR_WorkshopItemActionDownload> actionsToCancel = {};
			
			if (actionThisItem)
				actionsToCancel.Insert(actionThisItem);
			
			if (actionDependenciesThisItem)
			{
				array<ref SCR_WorkshopItemAction> dependencyActions = actionDependenciesThisItem.GetActions();
				foreach (auto a : dependencyActions)
				{
					SCR_WorkshopItemActionDownload downloadAction = SCR_WorkshopItemActionDownload.Cast(a);
					if (downloadAction)
						actionsToCancel.Insert(downloadAction);
				}
			}
			
			// Create a dialog
			new SCR_CancelDownloadConfirmationDialog(actionsToCancel);
			
			return;
		}
		
		return;
	}
	
	//---------------------------------------------------------------------------------------------
	protected static void OnDownloadConfirmDisplayed(SCR_DownloadSequence downloadSequence, SCR_DownloadConfirmationDialog confirmDialog)
	{
		if (m_OnDownloadConfirmDisplayed)
				m_OnDownloadConfirmDisplayed.Invoke(downloadSequence, confirmDialog);
	}
	
	//---------------------------------------------------------------------------------------------
	//! Updates icon and text with according to current state of addon
	static void UpdateAddonStateIconAndText(SCR_WorkshopItem item, SCR_ContentBrowser_ColorScheme colorScheme, ImageWidget wImage, TextWidget wText)
	{
		// Issues / problems
		bool imageVisible = false;
		bool textVisible = false;
		const ResourceName stateImageset = "{9DA6A249AD4D71C4}UI/Textures/Icons/icons_wrapperUI-48.imageset";
		
		string image;
		string stateText;
		Color color = colorScheme.m_Moderate;
		
		bool downloading = item.GetDownloadAction() != null || item.GetDependencyCompositeAction() != null;
		
		if (downloading)
		{
			image = "downloading";
			stateText = "#AR-Workshop_ButtonDownloading";
			color = colorScheme.m_Moderate;
			imageVisible = true;
			textVisible = true;
		}
		else
		{
			
			EWorkshopItemProblem problem = item.GetHighestPriorityProblem();
			if (problem == EWorkshopItemProblem.UPDATE_AVAILABLE)
			{
				image = "download";
				color = colorScheme.m_Moderate;
				textVisible = false;
				imageVisible = true;
			}
			else
			{
				bool problemCritical;
				bool problemExists = SCR_WorkshopUiCommon.GetHighestPriorityProblemDescription(item, stateText, problemCritical);
				if (problemExists)
				{
					image = "warning";
					color = colorScheme.m_Moderate;
					if (problemCritical)
						color = colorScheme.m_Critical;
					imageVisible = true;
					textVisible = true;
				}
				else
				{
					image = "check";
					imageVisible = item.GetOffline();
					textVisible = false;
					color = colorScheme.m_Good;
				}
			}
		}
		
		wText.SetText(stateText);
		wText.SetColor(color);
		wText.SetVisible(textVisible);
		if (!image.IsEmpty())
			wImage.LoadImageFromSet(0, stateImageset, image);		
		wImage.SetVisible(imageVisible);
		wImage.SetColor(color);
	}
	
	
	
	
	
	
	//------------------------------------------------------------------------------------------------
	// S T R I N G   F O R M A T T I N G 
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Formats addon version. For example "1.2.3" -> "v. 1.2.3".
	static string FormatVersion(string version)
	{
		return string.Format("v. %1", version);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Converts report type enum to stringtable entry
	static string GetReportTypeString(EWorkshopReportType eReportType)
	{
		const int REPORT_REASONS_COUNT = EWorkshopReportType.EWREPORT_OTHER + 1;
		const string REPORT_REASONS[REPORT_REASONS_COUNT] = 
		{
			"#AR-Workshop_Report_InappropiateContent",
			"#AR-Workshop_Report_OffensiveLanguage",
			"#AR-Workshop_Report_Missleading_NonFunctional",
			"#AR-Workshop_Report_Spam",
			"#AR-Workshop_Report_Scam",
			"#AR-Workshop_Report_Malicious",
			"#AR-Workshop_Report_IPInfringement",
			"#AR-Workshop_Report_Other"
		};
		
		int reportType = eReportType;
		
		if (reportType < 0 || reportType >= REPORT_REASONS_COUNT)
			return string.Empty;
		
		return REPORT_REASONS[reportType];
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// P R O T E C T E D   M E T H O D S 
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	//! Callback from addon manager when addons are checked
	protected static void Callback_OnAddonsChecked()
	{
		array<string> bannedAddons = {};
		GetGame().GetBackendApi().GetWorkshop().GetBannedItems(bannedAddons);
		
		if (bannedAddons.IsEmpty())
			return;
		
		new SCR_BannedAddonsDetectedDialog(bannedAddons);
	}
	
	//---------------------------------------------------------------------------------------------
	//! Internal method to handle addon enabling. Might show a dialog or enable some other addons(dependnecies) if needed.
	protected static void SetAddonEnabled(SCR_WorkshopItem item, bool newEnabled)
	{
		item.SetEnabled(newEnabled);
		
		if (newEnabled)
		{
			// Auto enable dependencies too
			item.SetDependenciesEnabled(true);
		}
		else
		{
			// Check if some addons depend on this
			array<ref SCR_WorkshopItem> offlineAndEnabled = SCR_AddonManager.SelectItemsAnd(
				item.GetDependentAddons(),
				EWorkshopItemQuery.OFFLINE | EWorkshopItemQuery.ENABLED);
			
			if (!offlineAndEnabled.IsEmpty())
			{
				SCR_DisableDependentAddonsDialog.CreateDisableDependentAddons(offlineAndEnabled, item);
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected static void InitAddonTagMaps()
	{
		s_sAddonTagImageMap = new map<string, string>;
		s_sAddonTagDefaultThumbnailMap = new map<string, ResourceName>;
		
		// Load addon type config
		SCR_FilterCategory cat = SCR_ConfigHelperT<SCR_FilterCategory>.GetConfigObject(ADDON_TYPE_FILTER_CATEGORY_CONFIG);
		if (!cat)
			return;

		s_sAddonTagImageSet = cat.m_sFilterImageSet;

		foreach (SCR_FilterEntry filter : cat.GetFilters())
		{
			string internalName = filter.m_sInternalName;
			internalName.ToLower();
			s_sAddonTagImageMap.Insert(internalName, filter.m_sImageName);
			
			SCR_ContentBrowserFilterTag filterTag = SCR_ContentBrowserFilterTag.Cast(filter);
			if (filterTag)
			{
				if (!filterTag.m_sDefaultThumbnail.IsEmpty())
					s_sAddonTagDefaultThumbnailMap.Insert(internalName, filterTag.m_sDefaultThumbnail);
			}
		}
	}
};


//------------------------------------------------------------------------------------------------
//! Confirmation dialog for unsubscribing an addon.
//! On confirm it deletes local data and unsubscribes the addon.
class SCR_DeleteAddonDialog : SCR_ConfigurableDialogUi
{
	protected ref SCR_WorkshopItem m_Item;
	
	
	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDialog CreateUnsubscribeAddon(SCR_WorkshopItem item)
	{
		return new SCR_DeleteAddonDialog(item, "unsubscribe");
	}
	
	
	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDialog CreateDeleteAddon(SCR_WorkshopItem item)
	{
		return new SCR_DeleteAddonDialog(item, "delete");
	}
	
	
	//------------------------------------------------------------------------------------------------
	private void SCR_DeleteAddonDialog(SCR_WorkshopItem item, string preset)
	{
		m_Item = item;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, preset, this);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Clear preset name
		if (m_Item.GetEnabled())
			SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
		
		if (m_Item.GetSubscribed())
			m_Item.SetSubscribed(false);
		
		m_Item.DeleteLocally();
		
		this.Close();
	}
};

//------------------------------------------------------------------------------------------------
//! Dialog to cancel downloads
class SCR_CancelDownloadConfirmationDialog : SCR_ConfigurableDialogUi
{
	ref array<ref SCR_WorkshopItemActionDownload> m_aActions;

	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{1C5D2CC10D7A1BC3}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineNonInteractive.layout";
	
	//------------------------------------------------------------------------------------------------
	void SCR_CancelDownloadConfirmationDialog(array<ref SCR_WorkshopItemActionDownload> actionsToCancel)
	{
		m_aActions = {};
		
		foreach (SCR_WorkshopItemActionDownload action : actionsToCancel)
		{
			m_aActions.Insert(action);
		}
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, "cancel_download_confirmation", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot(GetRootWidget()).FindAnyWidget(SCR_WorkshopUiCommon.WIDGET_LIST));
		
		// Create widgets for downloads
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, layout);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForCancelDownloadAction(action);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			action.Cancel();
		}
		this.Close();
	}
};


//------------------------------------------------------------------------------------------------
//! Dialog to cancel downloads during to server joining
class SCR_ServerJoinDownloadsConfirmationDialog : SCR_ConfigurableDialogUi
{
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aActions = {};	
	protected ref array<SCR_DownloadManager_AddonDownloadLine> m_aLineCompents = {};
	
	protected static const string TAG_ALL = "server_download_all_cancel";
	protected static const string TAG_REQUIRED = "server_download_required";
	protected static const string TAG_UNRELATED = "server_download_unrelated_cancel";
	
	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{1C5D2CC10D7A1BC3}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineNonInteractive.layout";
	
	//------------------------------------------------------------------------------------------------
	static SCR_ServerJoinDownloadsConfirmationDialog Create(array<ref SCR_WorkshopItemActionDownload> actions, SCR_EJoinDownloadsConfirmationDialogType type)
	{
		string tag;
		
		switch (type)
		{
			case SCR_EJoinDownloadsConfirmationDialogType.ALL:
				tag = TAG_ALL;
				break;
		
			case SCR_EJoinDownloadsConfirmationDialogType.REQUIRED:
				tag = TAG_REQUIRED;
				break;
			
			case SCR_EJoinDownloadsConfirmationDialogType.UNRELATED:
				tag = TAG_UNRELATED;
				break;
		}
		
		return new SCR_ServerJoinDownloadsConfirmationDialog(tag, actions);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ServerJoinDownloadsConfirmationDialog(string tag, array<ref SCR_WorkshopItemActionDownload> actions)
	{
		foreach (SCR_WorkshopItemActionDownload action : actions)
		{
			m_aActions.Insert(action);
		}
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, tag, this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{	
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot(GetRootWidget()).FindAnyWidget(SCR_WorkshopUiCommon.WIDGET_LIST));
		
		// Create widgets for downloads
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, layout);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForServerDownloadAction(action);
			
			m_aLineCompents.Insert(comp);
		}
		
		super.OnMenuOpen(preset);
	}
};


//------------------------------------------------------------------------------------------------
//! Dialog which lists names of all banned addons
class SCR_BannedAddonsDetectedDialog : SCR_ConfigurableDialogUi
{
	ref array<string> m_aItemNames = {};
	
	//------------------------------------------------------------------------------------------------
	void SCR_BannedAddonsDetectedDialog(array<string> items)
	{	
		m_aItemNames.Copy(items);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, "banned_addons_detected", this);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		string message = "#AR-Workshop_Baned_Item";
		for (int i = 0; i < m_aItemNames.Count(); i++)
			message = message + "\n- " + m_aItemNames[i];

		this.SetMessage(message);
	}
};

//------------------------------------------------------------------------------------------------
//! Shows a list of addons and some text.
class SCR_AddonListDialog : SCR_ConfigurableDialogUi
{
	ref array<ref SCR_WorkshopItem> m_aItems = {};
	protected ref array<SCR_DownloadManager_AddonDownloadLine> m_aDownloadLines = {};
	
	protected ResourceName ADDON_LINE_LAYOUT = "{BB5AEDDA3C4134FD}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineConfirmation.layout";
	
	//------------------------------------------------------------------------------------------------
	//! !!! Don't use the constructor! Use the Create... methods instead.
	void SCR_AddonListDialog(array<ref SCR_WorkshopItem> items, string preset)
	{
		foreach (auto i : items)
			m_aItems.Insert(i);
		
		if (!preset.IsEmpty())
			SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, preset, this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot(GetRootWidget()).FindAnyWidget(SCR_WorkshopUiCommon.WIDGET_LIST));
		
		// Create widgets
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT, layout);
			
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForWorkshopItem(item, null, false);
			
			m_aDownloadLines.Insert(comp);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AddonListDialog CreateItemsList(array<ref SCR_WorkshopItem> items, string preset, ResourceName dialogsConfig = "")
	{
		if (dialogsConfig == "")
			dialogsConfig = SCR_WorkshopUiCommon.DIALOGS_CONFIG;
						
		SCR_AddonListDialog addonListDialog = new SCR_AddonListDialog(items, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, addonListDialog);
		return addonListDialog;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog when downloading restricted addons
	static SCR_AddonListDialog CreateRestrictedAddonsDownload(array<ref SCR_WorkshopItem> items)
	{
		return DisplayRestrictedAddonsList(items, "error_restricted_dependencies_download", "error_reported_dependencies_download");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog when trying to connect to a server with restricted addons
	static SCR_AddonListDialog CreateRestrictedAddonsJoinServer(array<ref SCR_WorkshopItem> items)
	{
		return DisplayRestrictedAddonsList(items, "error_restricted_addons_join_server", "error_reported_addons_join_server");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display dialogs for reported and blocked (banned) mods
	static SCR_AddonListDialog DisplayRestrictedAddonsList(array<ref SCR_WorkshopItem> items, string tagBlocked, string tagReported)
	{
		// Banned specific
		array <ref SCR_WorkshopItem> banned = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.BLOCKED); 
		
		if (!banned.IsEmpty())
			return new SCR_AddonListDialog(banned, tagBlocked);
		
		// Reported
		array <ref SCR_WorkshopItem> reported = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.REPORTED_BY_ME | EWorkshopItemQuery.AUTHOR_BLOCKED);
		return new SCR_ReportedAddonsDialog(reported, tagReported);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog when failed to load game with selected mods
	static SCR_AddonListDialog CreateFailedToStartWithMods(array<ref SCR_WorkshopItem> items)
	{
		return new SCR_AddonListDialog(items, "error_failed_to_start_with_mods");
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_DownloadManager_AddonDownloadLine> GetDonwloadLines()
	{
		return m_aDownloadLines;
	}
};


//------------------------------------------------------------------------------------------------
//! When confirmed, provided addons will be enabled or disabled.
class SCR_SetAddonsEnabledDialog : SCR_AddonListDialog
{
	// New value of enabled state of addons.
	protected bool m_bNewEnabledValue;
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		super.OnConfirm();
		
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			if (item.GetOffline())
			{
				item.SetEnabled(m_bNewEnabledValue);
			}
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Dialog which is used when an addon is disabled but some dependent addons depend on it.
class SCR_DisableDependentAddonsDialog : SCR_SetAddonsEnabledDialog
{
	protected ref SCR_WorkshopItem m_Dependency;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DisableDependentAddonsDialog(array<ref SCR_WorkshopItem> items, string preset, SCR_WorkshopItem dependency)
	{
		m_Dependency = dependency;
		m_bNewEnabledValue = false; // When confirmed, addons will be disabled
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DisableDependentAddonsDialog CreateDisableDependentAddons(array<ref SCR_WorkshopItem> items, SCR_WorkshopItem dependency)
	{
		return new SCR_DisableDependentAddonsDialog(items, "disable_dependent_addons", dependency);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Disable the dependency too
		m_Dependency.SetEnabled(false);
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		// Enable back the dependency
		m_Dependency.SetEnabled(true);
		
		super.OnCancel();	
	}
};


//------------------------------------------------------------------------------------------------
//! Dialog which sends a request to delete user's report.
class SCR_CancelMyReportDialog : SCR_ConfigurableDialogUi
{
	// Widget names 
	protected const string WIDGET_TXT_TYPE_MSG = "TxtTypeMsg";
	protected const string WIDGET_TXT_COMMENT = "TxtComment";
	
	// Widgets 
	protected TextWidget m_wTxtTypeMsg;
	protected TextWidget m_wTxtComment;

	protected ref SCR_WorkshopItem m_Item;
	protected bool m_bAuthorReport = false;
	protected ref SCR_WorkshopItemActionCancelReport m_Action;
	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;
	
	//------------------------------------------------------------------------------------------------
	void SCR_CancelMyReportDialog(SCR_WorkshopItem item, bool authorReport = false)
	{
		m_Item = item;
		m_bAuthorReport = authorReport;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, "cancel_report", this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Get reports 
		string reportDescription;
		EWorkshopReportType reportType;
		
		if (!m_bAuthorReport)
			m_Item.GetReport(reportType, reportDescription);
		
		string reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(reportType);
		
		// Find widgets 
		m_wTxtTypeMsg = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_TXT_TYPE_MSG));
		m_wTxtComment = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_TXT_COMMENT));
		
		// Display report details
		if (!m_bAuthorReport)
			SetMessage("#AR-Workshop_CancelReportDescription");
		
		m_wTxtTypeMsg.SetText(reportTypeStr);
		m_wTxtComment.SetText(reportDescription);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
		
		m_Action = m_Item.CancelReport();
		m_Action.m_OnCompleted.Insert(Callback_OnSuccess);
		m_Action.m_OnFailed.Insert(Callback_OnFailed);
		m_Action.Activate();
		
		m_OnConfirm.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnSuccess()
	{
		m_LoadingOverlayDlg.CloseAnimated();
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnFailed()
	{
		m_LoadingOverlayDlg.CloseAnimated();
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionCancelReport GetWorkshopItemAction()
	{
		return m_Action;
	}
};

//------------------------------------------------------------------------------------------------
//! This dialog is shown when we want to delete an addon but some other downloaded addons depend on it.
class SCR_DeleteAddonDependentAddonsDetected : SCR_AddonListDialog
{
	protected ref SCR_WorkshopItem m_Item;	
	
	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDependentAddonsDetected Create(array<ref SCR_WorkshopItem> items, SCR_WorkshopItem item)
	{
		SCR_DeleteAddonDependentAddonsDetected dlg = new SCR_DeleteAddonDependentAddonsDetected(items, "delete_dependent_addons_detected", item);
		return dlg;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SCR_DeleteAddonDependentAddonsDetected(array<ref SCR_WorkshopItem> items, string preset, SCR_WorkshopItem item)
	{
		m_Item = item;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delete the provided addon on confirmation. Disable dependent addons if they are enabled.
	override void OnConfirm()
	{
		if (m_Item.GetSubscribed())
			m_Item.SetSubscribed(false);
		
		m_Item.DeleteLocally();
		
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			if (item.GetEnabled())
				item.SetEnabled(false);
		}
		
		this.Close();
	}	
};

//------------------------------------------------------------------------------------------------
//! Same as SCR_BackendImageComponent, but implements default image based on tag of Workshop Item
class SCR_WorkshopItemBackendImageComponent : SCR_BackendImageComponent
{
	protected ref SCR_WorkshopItem m_Item;
	
	protected override void SetImage(BackendImage image);
	
	protected const string LOADING_BACKGROUND_IMAGE = "{75455009AFED376B}UI/Textures/Workshop/AddonThumbnails/workshop_loading_UI.edds";
	
	//----------------------------------------------------------------------------------
	//! Sets reference to workshop item. Call this, then call SetImage as usual
	void SetWorkshopItemAndImage(SCR_WorkshopItem item, BackendImage image)
	{
		m_Item = item;
		
		super.SetImage(image);
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowDefaultImage()
	{
		ResourceName img;
		if (m_Item)
			img = SCR_WorkshopUiCommon.GetDefaultAddonThumbnail(m_Item);
		else
			img = SCR_WorkshopUiCommon.ADDON_DEFAULT_THUMBNAIL;
		ShowImage(img);
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowLoadingImage(string fallbackImage)
	{
		if (!m_wImage)
			return;
		
		if (fallbackImage.IsEmpty())
		{
			// Custom behavior when there is no fallback image
			// Show custom image
			ShowImage(LOADING_BACKGROUND_IMAGE);
			
			if (m_LoadingOverlay)
				m_LoadingOverlay.SetShown(true);
		}
		else
			super.ShowLoadingImage(fallbackImage); // Default behavior
	}
};

//------------------------------------------------------------------------------------------------
//! Same as SCR_BackendImageComponent, but implements default image based on scenario
class SCR_ScenarioBackendImageComponent : SCR_BackendImageComponent
{
	protected ref MissionWorkshopItem m_Scenario;
	
	protected override void SetImage(BackendImage image);
	
	protected const string LOADING_BACKGROUND_IMAGE = SCR_WorkshopUiCommon.SCENARIO_SP_DEFAULT_THUMBNAIL;
	
	//----------------------------------------------------------------------------------
	//! Sets reference to workshop item. Call this, then call SetImage as usual
	void SetScenarioAndImage(MissionWorkshopItem scenario, BackendImage image)
	{
		m_Scenario = scenario;
		
		super.SetImage(image);
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowDefaultImage()
	{
		// Later we can choose default image based on scenario type, player count...
		ShowImage(SCR_WorkshopUiCommon.SCENARIO_SP_DEFAULT_THUMBNAIL);
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowLoadingImage(string fallbackImage)
	{
		if (!m_wImage)
			return;
		
		if (fallbackImage.IsEmpty())
		{
			// Custom behavior when there is no fallback image
			// Show custom image
			if (m_bShowLoadingImage)
				ShowImage(LOADING_BACKGROUND_IMAGE);
			
			if (m_LoadingOverlay)
				m_LoadingOverlay.SetShown(true);
		}
		else
			super.ShowLoadingImage(fallbackImage); // Default behavior
	}
};
