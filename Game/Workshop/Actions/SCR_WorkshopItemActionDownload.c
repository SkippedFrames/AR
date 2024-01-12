//! Action for downloading a specific revision of a mod

void ScriptInvoker_ActionDownloadProgress(SCR_WorkshopItemActionDownload action, int progressSize);
typedef func ScriptInvoker_ActionDownloadProgress;

void ScriptInvoker_ActionDownloadFullStorage(SCR_WorkshopItemActionDownload action, float size);
typedef func ScriptInvoker_ActionDownloadFullStorage;

class SCR_WorkshopItemActionDownload : SCR_WorkshopItemAction
{
	protected ref Revision m_StartVersion;		// Version which was when the download was started
	protected ref Revision m_TargetRevision;
	
	protected bool m_bTargetVersionLatest;	// When true, means we want to download latest version
	
	protected ref SCR_WorkshopCallbackBase m_Callback;
	
	protected float m_fSizeBytes; // Total size of this download.
	
	// True if this action is waiting for details to start the download
	protected bool m_bWaitingForRevisions;
	
	// True when the actual download has been started
	protected bool m_bDownloadStarted;
	
	protected float m_fProgress;
	protected float m_fDownloadedSize;
	
	// State of enabled state when this action was activated
	protected bool m_bAddonEnabledAtStart;
	
	// Pause and resume handling
	protected ref SCR_BackendCallback m_PauseDownloadCallback = new SCR_BackendCallback();
	protected bool m_bRunningImmediate;		// Immediate change what is requested
	protected bool m_bRunningAsync;	// Changes after reponse
	
	protected ref ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress> m_OnDownloadProgress;
	protected ref ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage> m_OnFullStorageError;
	
	//-----------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress> GetOnDownloadProgress()
	{
		if (!m_OnDownloadProgress)
			m_OnDownloadProgress = new ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress>();
		
		return m_OnDownloadProgress;
	}
	
	//-----------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage> GetOnFullStorageError()
	{
		if (!m_OnFullStorageError)
			m_OnFullStorageError = new ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage>();
		
		return m_OnFullStorageError;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemActionDownload(SCR_WorkshopItem wrapper, bool latestVersion, Revision revision = null)
	{
		m_bTargetVersionLatest = latestVersion;
		m_TargetRevision = revision;
		m_StartVersion = wrapper.GetCurrentLocalRevision();

		m_fSizeBytes = m_Wrapper.GetTargetRevisionPatchSize();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the target version.
	//! !! If you were targeting latest version, but the actual download has not started because it
	//! is waiting for more data from backend, then this will return an empty string until the
	//! data is fetched from backend.
	Revision GetTargetRevision()
	{
		return m_TargetRevision;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the version which was at the moment when the download was started
	Revision GetStartRevision()
	{
		return m_StartVersion;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetTargetedAtLatestVersion()
	{
		return m_bTargetVersionLatest;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when this is an update, false when regular download.
	bool GetUpdate()
	{
		return m_StartVersion;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns current download progress if active or paused, 1 if complete, otherwise 0.
	float GetProgress()
	{
		if (IsCompleted())
			return 1.0;
		return m_fProgress;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns download size. Might return 0 until the actiion is activated.
	float GetSizeBytes()
	{
		return m_fSizeBytes;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{	
		m_bRunningImmediate = true;
		m_bRunningAsync = true;
		
		m_fDownloadedSize = 0;
		
		// Check if addon was enabled, when action is finished it will be restored
		m_bAddonEnabledAtStart = m_Wrapper.GetEnabled();
		
		// If we have versions already, we start downloading immediately
		// Otherwise we load details and start waiting
		return StartDownloadOrLoadDetails();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnCancel()
	{
		bool success = m_Wrapper.Internal_CancelDownload();

		return success && !IsFailed();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetRunningAsync()
	{
		return m_bRunningAsync;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsRunningAsyncSolved()
	{
		return m_bRunningImmediate && m_bRunningAsync;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsPauseAsyncSolved()
	{
		return !m_bRunningImmediate && !m_bRunningAsync;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnPause()
	{
		// Check
		WorkshopItem item = m_Wrapper.GetWorkshopItem();
		if (!item)
			return false;
		
		// Setup result
		if (m_bDownloadStarted)
		{
			m_bRunningImmediate = false;
			
			m_PauseDownloadCallback.GetEventOnResponse().Insert(OnPauseResponse);
			item.PauseDownload(m_PauseDownloadCallback);
		}
		
		// Result
		return !IsFailed();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPauseResponse(SCR_BackendCallback callback)
	{
		callback.GetEventOnResponse().Remove(OnPauseResponse);
		
		// Fail
		if (callback.GetResponseType() != EBackendCallbackResponse.SUCCESS)
		{
			// Get back to previsius reqeusted state 
			m_bRunningImmediate = true;
			return;
		}
		
		// Success
		m_bRunningAsync = false;
		InvokeOnChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnResume()
	{
		// Check
		WorkshopItem item = m_Wrapper.GetWorkshopItem();
		if (!item)
			return false;
		
		// Setup result
		if (m_bDownloadStarted)
		{
			m_bRunningImmediate = true;
			
			//m_PauseDownloadCallback.GetEventOnResponse().Insert(OnResumeResponse);
			
			m_bRunningAsync = true;
			item.ResumeDownload(null);
			InvokeOnChanged();
			
			return true;
		}
		
		// If download didn't actually start, we must not resume but start a download
		return StartDownloadOrLoadDetails();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnResumeResponse(SCR_BackendCallback callback)
	{
		callback.GetEventOnResponse().Remove(OnResumeResponse);
		
		// Fail
		if (callback.GetResponseType() != EBackendCallbackResponse.SUCCESS)
		{
			// Get back to previsius reqeusted state 
			m_bRunningImmediate = false;
			return;
		}
		
		// Success
		m_bRunningAsync = true;
		
		InvokeOnChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		// Restore 'enabled' state
		m_Wrapper.SetEnabled(m_bAddonEnabledAtStart);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! We can't start download before details are loaded, because details have the revisions.
	//! Starts download immediately or loads details of workshop item.
	protected bool StartDownloadOrLoadDetails()
	{
		#ifdef WORKSHOP_DEBUG
		_print("StartDownloadOrLoadDetails()");
		#endif
		
		#ifdef WORKSHOP_DEBUG
		_print("StartDownloadOrLoadDetails(): Revisions are loaded, starting the download");
		#endif
		
		// Instant fail if mod is restricted
		// We know that it's restricted only after we have loaded the details
		if (m_Wrapper.GetRestricted())
		{
			Fail();
			return false;
		}
		
		m_Callback = new SCR_WorkshopCallbackBase();
		m_Callback.m_OnError.Insert(Callback_OnError);
		m_Callback.m_OnTimeout.Insert(Callback_OnTimeout);
		
		if (m_bTargetVersionLatest)
		{
			m_TargetRevision = m_Wrapper.GetLatestRevision();
			#ifdef WORKSHOP_DEBUG
			_print("StartDownloadOrLoadDetails(): Download of latest version was requested");
			#endif
		}
		
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("StartDownloadOrLoadDetails(): Target version: %1", m_TargetRevision.GetVersion()));
		#endif
			
		bool success = m_Wrapper.Internal_StartDownload(m_TargetRevision, m_Callback);
		
		m_bWaitingForRevisions = false;
		m_bDownloadStarted = true;
		
		return success && !IsFailed(); // The workshop API might have called the Error handler by now
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Internal_Update(float timeSlice)
	{
		if (!m_Wrapper)
			return;
		
		// Check if comms have failed
		if (m_State != STATE_COMPLETED && m_State != STATE_FAILED)
		{
			if (m_Wrapper.GetRequestFailed())
				Fail();
		}
		
		// Complete the action when the addon is at the target revisioon and is downloaded
		if (m_State == STATE_ACTIVE)
		{
			if (m_bWaitingForRevisions)
			{
				// Try to start the download again if we have loaded the details
				if (m_Wrapper.GetRevisionsLoaded())
					StartDownloadOrLoadDetails();
			}
			else
			{
				bool offline = m_Wrapper.GetOffline();
				Revision currentVersion = m_Wrapper.GetCurrentLocalRevision();
				
				//#ifdef WORKSHOP_DEBUG
				//_print(string.Format("Internal_Update: Offline: %1, CurrentVersion: '%2', TargetVersion: '%3'", offline, currentVersion, m_sTargetVersion));
				//#endif
				
				float progress = m_Wrapper.Internal_GetDownloadProgress();
				if (progress > m_fProgress)
				{					
					float currentDownloadSize = m_Wrapper.GetTargetRevisionPatchSize() * progress;
					
					if (m_OnDownloadProgress)
						m_OnDownloadProgress.Invoke(this, currentDownloadSize - m_fDownloadedSize);
					
					m_fProgress = progress;
					m_fDownloadedSize = currentDownloadSize;
					
					InvokeOnChanged();
				}
				
				if (offline && Revision.AreEqual(currentVersion, m_TargetRevision))
				{
					Complete();
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnError(SCR_WorkshopCallbackBase callback, int code, int restCode, int apiCode)
	{
		//if (code != EBackendError.EBERR_INVALID_STATE) // Ignore this one for now // EApiCode.EACODE_ERROR_OK
		
		callback.m_OnError.Remove(Callback_OnError);
		callback.m_OnTimeout.Remove(Callback_OnTimeout);
		
		Fail(code);
		
		// Full storage issues 
		if (code == EBackendError.EBERR_STORAGE_IS_FULL)
		{
			FullStorageFail();
		}
		//data are not available or validation failed
		else if ((restCode >= 400 && restCode < 500) || (code == EBackendError.EBERR_VALIDATION_FAILED))	
		{
			m_Wrapper.DeleteDownloadProgress();
		}
		//else network error
	}
	
	//-----------------------------------------------------------------------------------------------
	void FullStorageFail()
	{
		if (m_OnFullStorageError)
		{	
			float size = m_Wrapper.GetItemTargetRevision().GetTotalSize();
			
			if (size > 0)
				m_OnFullStorageError.Invoke(this, size);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnTimeout(SCR_WorkshopCallbackBase callback)
	{
		Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Try redownload failed addon
	void RetryDownload()
	{
		m_Wrapper.RetryDownload(m_TargetRevision);
	}
	
	//-----------------------------------------------------------------------------------------------
	void ForceFail()
	{
		Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetDownloadSize()
	{
		return m_fDownloadedSize;
	}
};
