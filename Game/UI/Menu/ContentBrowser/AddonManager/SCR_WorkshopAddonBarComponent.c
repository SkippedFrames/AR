class SCR_WorkshopAddonBarComponent : SCR_ScriptedWidgetComponent
{
	protected const ResourceName ADDON_BAR_ICON_LAYOUT = "{9B80BD4A534C651C}UI/layouts/Menus/ContentBrowser/AddonManager/AddonBar/AddonBarIcon.layout";

	protected ref SCR_WorkshopAddonBarWidgets m_Widgets = new SCR_WorkshopAddonBarWidgets();

	[Attribute("20", UIWidgets.EditBox, desc: "After how many character should be display preset name cut")]
	protected int m_iPresetNameCap;

	protected static SCR_ConfigurableDialogUi m_FailDialog;

	protected ref SCR_AddonPatchSizeLoader m_Loader = new SCR_AddonPatchSizeLoader();
	SCR_LoadingOverlayDialog m_LoadingOverlay;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_Widgets.Init(w);

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (!mgr)
			return;

		mgr.m_OnAddonsEnabledChanged.Insert(Callback_OnAddonsEnabledChanged);
		mgr.GetPresetStorage().GetEventOnUsedPresetChanged().Insert(OnUsedPresetChanged);

		OnUsedPresetChanged(mgr.GetPresetStorage().GetUsedPreset());

		UpdateAllWidgets();

		m_Widgets.m_PresetsButtonComponent.GetButton().m_OnClicked.Insert(OnPresetsButtonClicked);
		m_Widgets.m_UpdateButtonComponent.GetButton().m_OnClicked.Insert(OnUpdateButtonClicked);

		OnFrame();
		
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

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();

		if (mgr)
			SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(Callback_OnAddonsEnabledChanged);

		ChimeraMenuBase menu = ChimeraMenuBase.GetOwnerMenu(w);
		if (menu)
			menu.m_OnUpdate.Remove(OnFrame);
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Remove(OnMenuDisabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnAddonsEnabledChanged()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Show actual preset name on preset name change
	protected void OnUsedPresetChanged(string name)
	{
		// Cap
		if (!name.IsEmpty() && name.Length() > m_iPresetNameCap)
			name = name.Substring(0, m_iPresetNameCap) + "...";

		m_Widgets.m_PresetsButtonComponent.SetLabelText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();

		if (!mgr)
			return;

		// Mod count text
		int nAddonsEnabled = SCR_AddonManager.CountItemsBasic(SCR_AddonManager.GetInstance().GetOfflineAddons(), EWorkshopItemQuery.ENABLED);
		m_Widgets.m_PresetsButtonComponent.SetVisible(nAddonsEnabled > 0);
		m_Widgets.m_PresetsButtonComponent.SetCountText(nAddonsEnabled.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPresetsButtonClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.AddonsToolsMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdateButtonClicked()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);

		// Load patch sizes for latest revision
		m_Loader = new SCR_AddonPatchSizeLoader();

		foreach (SCR_WorkshopItem item : addonsOutdated)
		{
			Revision rev = item.GetLatestRevision();
			item.SetItemTargetRevision(rev);
			m_Loader.InsertItem(item);
		}

		m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();

		m_Loader.GetOnAllPatchSizeLoaded().Insert(OnUpdatePatchSizeLoaded);
		m_Loader.LoadPatchSizes();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdatePatchSizeLoaded(SCR_AddonPatchSizeLoader loader, bool allLoaded)
	{
		// Cleanup
		m_Loader.GetOnAllPatchSizeLoaded().Remove(OnUpdatePatchSizeLoaded);
		m_LoadingOverlay.Close();

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);

		// Open download confirmation dialog
		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions = {};
		foreach (SCR_WorkshopItem item : addonsOutdated)
		{
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>(item, item.GetLatestRevision()));

		}

		SCR_AddonUpdateConfirmationDialog.CreateForUpdates(addonsAndVersions, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (!mgr)
			return;

		int nOutdated = mgr.GetCountAddonsOutdated();

		m_Widgets.m_UpdateButtonComponent.SetVisible(nOutdated > 0);

		if (nOutdated > 0)
		{
			m_Widgets.m_UpdateButtonComponent.SetCountText(nOutdated.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayFailDialog()
	{
		// Check if dialog is not already opened
		if (m_FailDialog)
			return;

		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> failed = {};
		array<ref SCR_WorkshopItemActionDownload> failedActions = SCR_DownloadManager.GetInstance().GetFailedDownloads();

		foreach (SCR_WorkshopItemActionDownload action : failedActions)
		{
			SCR_WorkshopItem item = action.m_Wrapper;
			Revision version = item.GetDependency().GetRevision();

			failed.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>>(item, version));
		}

		// Setup dialog
		SCR_DownloadFailDialog dialog = SCR_DownloadFailDialog.CreateFailedAddonsDialog(failed, false);
		if (!dialog)
			return;

		m_FailDialog = dialog;
		m_FailDialog.m_OnConfirm.Insert(Callback_OnFailDialogConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnFailDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		// Clear dialog
		m_FailDialog.m_OnConfirm.Clear();
		m_FailDialog.Close();
		m_FailDialog = null;
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuEnabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			menu.m_OnUpdate.Remove(OnFrame);
			menu.m_OnUpdate.Insert(OnFrame);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuDisabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
			menu.m_OnUpdate.Remove(OnFrame);
	}
}
