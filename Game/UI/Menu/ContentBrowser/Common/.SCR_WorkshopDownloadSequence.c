//! Helper class to manage starts of new downloads from different menus.
//! We can start a download from many menus. The download process requests dependency list,
//! then resolves which dependencies must be downloaded and either starts the downloads or 
//! shows a confirmation dialog.
/*
class SCR_WorkshopDownloadSequence
{	
	protected ref SCR_WorkshopItem m_Item;
	bool m_bSubscribeToAddons; // When true, we will subscribe automatically when download is started
	
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	protected ref array<ref SCR_WorkshopItem> m_aDependencies;
	protected bool m_bAllDependencyDetailsLoaded = false;
	protected bool m_bWaitingResponse;	// Set to false when data fetching is complete
	protected bool m_bCanceled;	// When true, finishing data reception will not trigger further downloading or other actions.
	protected bool m_bFailed;	// True when failed due to timeour or error
	
	//------------------------------------------------------------------------------------------------
	//! Tries to create a new request if previous doesn't exist or finished. Otherwise returns the previous request.
	static SCR_WorkshopDownloadSequence TryCreate(SCR_WorkshopItem item, bool subscribe, SCR_WorkshopDownloadSequence previous)
	{
		if (previous)
		{
			if (previous.m_bWaitingResponse)
				return previous;
		}
		
		SCR_WorkshopDownloadSequence newRequest = new SCR_WorkshopDownloadSequence(item, subscribe);
		return newRequest;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cancels this download request.
	void Cancel()
	{
		m_bCanceled = true;
		m_bWaitingResponse = false;
	}
	
	//------------------------------------------------------------------------------------------------
	private void SCR_WorkshopDownloadSequence(SCR_WorkshopItem item, bool subscribe, bool showLoadingOverlay = true)
	{
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("SCR_WorkshopDownloadSequence: New for: %1", item.GetName()));
		#endif
		
		m_bWaitingResponse = true;
		m_bCanceled = false;
		
		// Show loading overlay, but now now, wait until some time passes and we don't receive requested data.
		if (showLoadingOverlay)
			GetGame().GetCallqueue().CallLater(CreateLoadingOverlay, SCR_WorkshopUiCommon.NO_LOADING_OVERLAY_DURATION_MS, false);
		
		// Subscribe to dependency load event of item and load details (and dependencies)
		m_bSubscribeToAddons = subscribe;
		m_Item = item;
		m_Item.m_OnDependenciesLoaded.Insert(Callback_OnDependenciesLoaded);
		
		m_Item.m_OnGetAsset.Insert(Callback_OnAddonGetAsset);
		m_Item.m_OnError.Insert(Callback_OnError);
		m_Item.m_OnTimeout.Insert(Callback_OnTimeout);
		m_Item.LoadDetails();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_WorkshopDownloadSequence()
	{
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("SCR_WorkshopDownloadSequence: Delete for: %1", m_Item.GetName()));
		#endif
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		// Unsubscribe from item's events
		m_Item.m_OnDependenciesLoaded.Remove(Callback_OnDependenciesLoaded);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateLoadingOverlay()
	{
		// Show only if we are still waiting for data
		if (m_bWaitingResponse)
		{
			m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
			m_LoadingOverlay.m_OnCloseStarted.Insert(Cancel); // Cancel this when the loading overlay close is initiated by user
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_WorkshopItem.m_OnDependenciesLoaded
	protected void Callback_OnDependenciesLoaded(SCR_WorkshopItem item)
	{	
		// Unsubscribe - this callback is not needed any more
		m_Item.m_OnDependenciesLoaded.Remove(Callback_OnDependenciesLoaded);
		
		m_aDependencies = m_Item.GetLatestDependencies();
		
		if (item.GetRestricted())
			OnAddonRestricted();
		else if (m_aDependencies.IsEmpty())
			OnAllDependenciesDetailsLoaded();
		else
		{
			// Load details of all dependencies
			foreach (SCR_WorkshopItem dep : m_aDependencies)
			{
				dep.m_OnGetAsset.Insert(Callback_OnDependencyDetailsLoaded);
				dep.m_OnTimeout.Insert(Callback_OnTimeout);
				dep.LoadDetails();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnAddonGetAsset(SCR_WorkshopItem item)
	{
		if (!item.GetRequestFailed())
			return;

		GetGame().GetCallqueue().Remove(CreateLoadingOverlay);
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		//SCR_CommonDialogs.CreateRequestErrorDialog();
		HandleError();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnError(SCR_WorkshopItem item)
	{
		Debug.Error("Item error when starting SCR_WorkshopDownloadSequence");
		
		HandleError();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnTimeout(SCR_WorkshopItem item)
	{
		HandleError();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unified behavior for error and timeout
	protected void HandleError()
	{
		if (m_bWaitingResponse || !m_bFailed)
		{
			if (m_LoadingOverlay)
				m_LoadingOverlay.CloseAnimated();
			
			//SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateTimeoutTryAgainCancelDialog();
			//dialog.m_OnConfirm.Insert(OnCofirmDownloadErrorDialog);
			
			// Add action to downloads
			SCR_WorkshopItemActionDownload action = new SCR_WorkshopItemActionDownload(m_Item, latestVersion: true);
			SCR_DownloadManager.GetInstance().AddDownloadManagerEntry(m_Item, action);
			action.Activate();
			
			if (!GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.DownloadManagerDialog))
			{
				GetGame().GetCallqueue().CallLater(SetupAddonFail, 0, false, action);
			}
		}
			
		m_bWaitingResponse = false;
		m_bFailed = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupAddonFail(SCR_WorkshopItemActionDownload action)
	{
		SCR_DownloadManager_Dialog.Create();
		action.ForceFail();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Retry download
	protected void OnCofirmDownloadErrorDialog()
	{
		m_bWaitingResponse = true;
		m_bCanceled = false;
		m_bFailed = false;
		
		CreateLoadingOverlay();
		
		m_Item.m_OnGetAsset.Remove(Callback_OnAddonGetAsset);
		m_Item.m_OnError.Remove(Callback_OnError);
		m_Item.m_OnTimeout.Remove(Callback_OnTimeout);
		
		m_Item.m_OnGetAsset.Insert(Callback_OnAddonGetAsset);
		m_Item.m_OnError.Insert(Callback_OnError);
		m_Item.m_OnTimeout.Insert(Callback_OnTimeout);
		m_Item.LoadDetails();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnDependencyDetailsLoaded(SCR_WorkshopItem item)
	{
		item.m_OnGetAsset.Remove(Callback_OnDependencyDetailsLoaded);
		
		if (m_bAllDependencyDetailsLoaded)
			return;
		
		int nDetailsNotLoaded;
		foreach (auto i : m_aDependencies)
		{
			if (!i.GetDetailsLoaded())
				nDetailsNotLoaded++;
		}
		
		if (nDetailsNotLoaded == 0 && !m_bAllDependencyDetailsLoaded)
		{
			m_bAllDependencyDetailsLoaded = true;
			OnAllDependenciesDetailsLoaded();
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called when finally all the details of all dependencies are loaded
	protected void OnAllDependenciesDetailsLoaded()
	{		
		m_bWaitingResponse = false;
		
		// Close the loading overlay
		if (m_LoadingOverlay)
			m_LoadingOverlay.CloseAnimated(false);
		
		// Bail if this was canceled or failed
		if (m_bFailed || m_bCanceled)
			return;
		
		// Check if any dependencies are restricted
		int nRestricted = SCR_AddonManager.CountItemsBasic(m_aDependencies, EWorkshopItemQuery.RESTRICTED, true);
		if (nRestricted > 0)
		{
			auto restrictedDependencies = SCR_AddonManager.SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.RESTRICTED);
			SCR_AddonListDialog addonsDialog = SCR_AddonListDialog.CreateRestrictedAddonsDownload(restrictedDependencies);
		
			// Handle cancel reports done 
			SCR_ReportedAddonsDialog reportedDialog = SCR_ReportedAddonsDialog.Cast(addonsDialog);
			if (reportedDialog)
				reportedDialog.GetOnAllReportsCanceled().Insert(OnAllReportsCanceled);
			
			return;
		}
		
		
		
		// Select addons for download and calculate total size
		
		bool downloadMainItem = false;
		
		SCR_WorkshopItem item = m_Item;
		array<ref SCR_WorkshopItem> dependencies = m_aDependencies;
		
		array<ref SCR_WorkshopItem> dependenciesToLoad = new array<ref SCR_WorkshopItem>;
		
		SCR_DownloadManager.SelectAddonsForLatestDownload(dependencies, dependenciesToLoad);
		
		float totalDownloadSize = SCR_DownloadManager.GetTotalSizeBytes(dependenciesToLoad);
		
		if (SCR_DownloadManager.IsLatestDownloadRequired(item))
		{
			totalDownloadSize += item.GetSizeBytes();
			downloadMainItem = true;
		}
		
		int depCount = dependenciesToLoad.Count();
			
		// If size above threshold, open a confirmation dialog
		float smallDownloadThreshold = ContentBrowserUI.GetSmallDownloadThreshold();
		bool showConfirmation = !dependencies.IsEmpty(); // Show confirmation only if there are dependencies
		//showConfirmation = true; // debug always show confirmation
		if (showConfirmation)
		{
			auto dlg = SCR_DownloadConfirmationDialog.CreateForAddonAndDependencies(item, downloadMainItem, dependenciesToLoad, m_bSubscribeToAddons);
		}
		else
		{
			// Start download immediately
			SCR_DownloadManager.GetInstance().DownloadLatestWithDependencies(m_Item, downloadMainItem, dependenciesToLoad);
			
			// Subscribe to items
			if (m_bSubscribeToAddons)
			{
				if (m_Item)
					m_Item.SetSubscribed(true);
				
				foreach (auto i : dependenciesToLoad)
					i.SetSubscribed(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when all reports from dialog are cancled to clear invoker actions and display download dialog 
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
		OnAllDependenciesDetailsLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the main addon is restricted
	protected void OnAddonRestricted()
	{
		m_bWaitingResponse = false;
		
		// Close the loading overlay
		if (m_LoadingOverlay)
			m_LoadingOverlay.CloseAnimated(false);
		
		SCR_WorkshopUiCommon.CreateDialog("error_addon_blocked");
	}
};
