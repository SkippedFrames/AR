//------------------------------------------------------------------------------------------------	
class SCR_ScriptPlatformRequestCallback : ScriptPlatformRequestCallback
{
	ref ScriptInvoker m_OnResult = new ScriptInvoker; // (UserPrivilege privilege, UserPrivilegeResult result)
	
	
	//------------------------------------------------------------------------------------------------
	override void OnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		m_OnResult.Invoke(privilege, result);
	}	
};


//------------------------------------------------------------------------------------------------	
class SCR_OnlineServiceBackendCallbacks : BackendCallback
{
	ref ScriptInvoker m_OnDownloading = new ref ScriptInvoker();
	ref ScriptInvoker m_OnSuccess = new ref ScriptInvoker();
	ref ScriptInvoker m_OnItem = new ref ScriptInvoker();
	ref ScriptInvoker m_OnDownloadSuccess = new ref ScriptInvoker();
	ref ScriptInvoker m_OnUnsubscribe = new ref ScriptInvoker();
	ref ScriptInvoker m_OnScenarios = new ref ScriptInvoker();
	ref ScriptInvoker m_OnImage = new ref ScriptInvoker();
	ref ScriptInvoker m_OnDependencies = new ref ScriptInvoker();
	
	ref ScriptInvoker m_OnError = new ref ScriptInvoker();
	ref ScriptInvoker m_OnTimeout = new ref ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_OnlineServiceBackendCallbacks()
	{
		m_OnDownloading = null;
		m_OnSuccess = null;
		m_OnItem = null;
		m_OnDownloadSuccess = null;
		m_OnUnsubscribe = null;
		m_OnDependencies = null;
		m_OnImage = null;
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		switch (code)
		{
			case EBackendRequest.EBREQ_WORKSHOP_GetDownloadURL:
				m_OnDownloading.Invoke();
			break;
			 	
			case EBackendRequest.EBREQ_WORKSHOP_GetAsset:
				m_OnItem.Invoke(code);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_DownloadAsset:
				m_OnDownloadSuccess.Invoke(code);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_DeleteSubscriptions:
				m_OnUnsubscribe.Invoke();
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_GetDependencyTree:
				m_OnDependencies.Invoke(code);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_DownloadImage:
				m_OnImage.Invoke(code);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_GetAssetScenarios:
				m_OnScenarios.Invoke(code);
			break;
			
			
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError( EBackendError code, int restCode, int apiCode )
	{
		Print("[BackendCallback] OnError: "+ g_Game.GetBackendApi().GetErrorCode(code) + " RestCode: " + restCode + " ApiCode: "+apiCode);
		m_OnError.Invoke(code);
	}
	
	override void OnTimeout()
	{
		Print("[BackendCallback] OnTimeout");
		m_OnTimeout.Invoke();
	}
};


////////////////////////////////////////////////////////////
// Callbacks for specific workshop requests
////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
class SCR_WorkshopCallbackBase : BackendCallback
{
	// Script invokers for generic results
	ref ScriptInvoker m_OnError = new ref ScriptInvoker;
	ref ScriptInvoker m_OnTimeout = new ref ScriptInvoker;
	ref ScriptInvoker m_OnSuccess = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	void SCR_WorkshopCallbackBase()
	{
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 new", this), LogLevel.DEBUG);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_WorkshopCallbackBase()
	{
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 Delete", this), LogLevel.DEBUG);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess( int code )
	{
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 OnSuccess: %2", this, typename.EnumToString(EBackendRequest, code)), LogLevel.DEBUG);
		#endif
		m_OnSuccess.Invoke(this, code);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(EBackendError code, int restCode, int apiCode )
	{
		Print(string.Format("%1 OnError: %2 %3 %4", this, typename.EnumToString(EBackendError, code), restCode, apiCode), LogLevel.ERROR);
		m_OnError.Invoke(this, code, restCode, apiCode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		Print(string.Format("%1 OnTimeout", this), LogLevel.ERROR);
		m_OnTimeout.Invoke(this);
	}
};


//-----------------------------------------------------------------------------------------------
class SCR_WorkshopItemCallback_LoadDependencies : SCR_WorkshopCallbackBase
{
	
};

//-----------------------------------------------------------------------------------------------
class SCR_WorkshopItemCallback_LoadScenarios : SCR_WorkshopCallbackBase
{
	
};


//-----------------------------------------------------------------------------------------------
// WorkshopApi.RequestPage
class SCR_WorkshopApiCallback_RequestPage : SCR_WorkshopCallbackBase
{
	int m_iPageId;
	
	ref ScriptInvoker m_OnDownloadImage = new ref ScriptInvoker;
	ref ScriptInvoker m_OnGetAssets = new ref ScriptInvoker;

		
	void SCR_WorkshopApiCallback_RequestPage(int pageId)
	{
		m_iPageId = pageId;
		
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 New: page: %2", this, this.m_iPageId), LogLevel.DEBUG);
		#endif
	}
	
	void ~SCR_WorkshopApiCallback_RequestPage(ContentBrowserUI contentBrowser, int tabId, int pageId)
	{
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 Delete: page: %2", this, this.m_iPageId), LogLevel.DEBUG);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);
		
		switch (code)
		{
			case EBackendRequest.EBREQ_WORKSHOP_GetAssetList:
				m_OnGetAssets.Invoke(this);
				break;
			
			case EBackendRequest.EBREQ_WORKSHOP_DownloadImage:
				m_OnDownloadImage.Invoke(this);
				break;
		}
	}
};



//-----------------------------------------------------------------------------------------------
// WorkshopItem.AskDetails
class SCR_WorkshopItemCallback_AskDetails : SCR_WorkshopCallbackBase
{
	WorkshopItem m_Item;
	ref ScriptInvoker m_OnGetAsset = new ref ScriptInvoker;
	ref ScriptInvoker m_OnGetDependencyTree = new ref ScriptInvoker;
	ref ScriptInvoker m_OnDownloadImage = new ref ScriptInvoker;
	ref ScriptInvoker m_OnGetAssetScenarios = new ref ScriptInvoker; // outdated
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemCallback_AskDetails(WorkshopItem item)
	{
		m_Item = item;
	}
	
	//-----------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);
		
		switch (code)
		{		 	
			case EBackendRequest.EBREQ_WORKSHOP_GetAsset: //
				m_OnGetAsset.Invoke(this);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_GetDependencyTree: //
				m_OnGetDependencyTree.Invoke(this);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_DownloadImage: //
				m_OnDownloadImage.Invoke(this);
			break;
			
			case EBackendRequest.EBREQ_WORKSHOP_GetAssetScenarios: //
				m_OnGetAssetScenarios.Invoke(this);
			break;
		}
	}
};



//-----------------------------------------------------------------------------------------------
// WorkshopItem.Rate, WorkshopItem.ResetRating
class SCR_WorkshopItemCallback_Rate : SCR_WorkshopCallbackBase
{
	
};

//-----------------------------------------------------------------------------------------------
// WorkshopItem.LoadMyReport
class SCR_WorkshopItemCallback_LoadMyReport : SCR_WorkshopCallbackBase
{
};

//-----------------------------------------------------------------------------------------------
// ImageScale.Download
class SCR_WorkshopItemCallback_DownloadImage : SCR_WorkshopCallbackBase
{
	ImageScale m_Scale;
};
