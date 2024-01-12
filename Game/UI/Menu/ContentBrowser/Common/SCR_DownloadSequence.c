//------------------------------------------------------------------------------------------------
//! Helper class to manage starts of new downloads from different menus.
//! We can start a download from many menus. The download process requests dependency list,
//! then resolves which dependencies must be downloaded and either starts the downloads or 
//! shows a confirmation dialog.
/*
class SCR_DownloadSequenceUI
{	
	protected ref SCR_WorkshopItem m_ScriptedItem;
	
	protected ref WorkshopItem m_Item;
	protected ref Revision m_Revision;
	protected ref array<Dependency> m_aDependencies = {}; 
	//protected ref Dependency m_Dependency; 
	
	protected ref SCR_BackendCallback m_DependenciesCallback;
	
	bool m_bSubscribeToAddons; // When true, we will subscribe automatically when download is started
	
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	protected ref array<ref SCR_WorkshopItem> m_aSciptedDependencies;
	protected bool m_bAllDependencyDetailsLoaded = false;
	protected bool m_bWaitingResponse;	// Set to false when data fetching is complete
	protected bool m_bCanceled;	// When true, finishing data reception will not trigger further downloading or other actions.
	protected bool m_bFailed;	// True when failed due to timeour or error

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static SCR_DownloadSequenceUI InitSequence(WorkshopItem item, Revision revision, bool subscribe, out SCR_DownloadSequenceUI sequence)
	{
		// Check previous
		if (sequence)
		{
			return sequence;
		}
		
		// Start new sequence 
		sequence = new SCR_DownloadSequenceUI(item, revision, subscribe);
		return sequence;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cancels this download request.
	void Cancel()
	{
		m_bCanceled = true;
		m_bWaitingResponse = false;
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
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadDependenciesResponse(SCR_BackendCallback callback)
	{
		// Reactions
		switch (callback.GetResponseType())
		{
			case EBackendCallbackResponse.SUCCESS:
			{
				// Get dependencies of current revision 
				m_Revision.GetDependencies(m_aDependencies);
		
				// TODO: Wrapper for checking restriction and state 
				
				
				/*
				if (item.GetRestricted())
					OnAddonRestricted();
				else if (m_aSciptedDependencies.IsEmpty())
					OnAllDependenciesDetailsLoaded();
				else
				{
					// Load details of all dependencies
					
					foreach (SCR_WorkshopItem dep : m_aSciptedDependencies)
					{
						dep.m_OnGetAsset.Insert(Callback_OnDependencyDetailsLoaded);
						dep.m_OnTimeout.Insert(Callback_OnTimeout);
						dep.LoadDetails();
					}
					
					
					//OnAllDependenciesDetailsLoaded();
				}
				*/
			/*	
				OnDependenciesLoaded();
				ShowDependencies();
				
				break;
			}
			
			case EBackendCallbackResponse.ERROR: SCR_CommonDialogs.CreateRequestErrorDialog(); break;
			case EBackendCallbackResponse.TIMEOUT: SCR_CommonDialogs.CreateRequestErrorDialog(); break;
		}
		
		// Cleanup	
		callback.GetEventOnResponse().Remove(OnLoadDependenciesResponse);
		
		GetGame().GetCallqueue().Remove(CreateLoadingOverlay);
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDependenciesLoaded()
	{
		// Check restrictions 
		
		
		// Calculate size delta
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register finished calculatino and wait until all deltas are available
	protected void OnDependencySizeDeltaCalculated()
	{
		// Register done 
		
		// Check all done
	}
	
	//------------------------------------------------------------------------------------------------
	//! On finished calculation display requeired download size
	protected void OnAllDependenciesSizeDeltaCalculated()
	{
		// Sum deltas
		
		// Show dialog
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowDependencies()
	{
		// Check current size 
		for (int i = 0, count = m_aDependencies.Count(); i < count; i++)
		{
			string size = SCR_ByteFormat.GetReadableSize(m_aDependencies[i].TotalFileSize());
			PrintFormat("%1. dep: %2 - %3", i, m_aDependencies[i].GetName(), size);
		}
		
		Print("-------------------------------------------------");
	}
	
	// TODO: Rerporting feature is useful, but might be better to put elsewhere
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when all reports from dialog are cancled to clear invoker actions and display download dialog 
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
		//OnAllDependenciesDetailsLoaded();
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
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	array<Dependency> GetDependencies()
	{
		array<Dependency> dependencies;
		m_aDependencies.Copy(dependencies);
		
		return dependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	// Constructor & destructor
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	// TODO: Check what to do with subscribe
	void SCR_DownloadSequenceUI(WorkshopItem item, Revision revision, bool subscribe, bool showLoadingOverlay = true)
	{
		// Init setup
		m_bWaitingResponse = true;
		m_bCanceled = false;
		
		// Show loading overlay - wait short time, display loading only if request takes longer
		if (showLoadingOverlay)
			GetGame().GetCallqueue().CallLater(CreateLoadingOverlay, SCR_WorkshopUiCommon.NO_LOADING_OVERLAY_DURATION_MS, false);
		
		// Subscribe to dependency load event of item and load details (and dependencies)
		m_bSubscribeToAddons = subscribe;
		
		m_Item = item;
		m_Revision = revision; 
		 
		// Load revision details  
		m_DependenciesCallback = new SCR_BackendCallback();
		m_DependenciesCallback.GetEventOnResponse().Insert(OnLoadDependenciesResponse);
		
		m_Item.LoadDependencies(m_DependenciesCallback, m_Revision);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DownloadSequenceUI()
	{
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("SCR_DownloadSequenceUI: Delete for: %1", m_ScriptedItem.GetName()));
		#endif
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		// Unsubscribe from item's events
		//m_ScriptedItem.m_OnDependenciesLoaded.Remove(Callback_OnDependenciesLoaded);
	}
};
