class SCR_ServerSaveRequestCallback: BackendCallback
{
	static const string SESSION_SAVE_NAME = "SCR_SaveFileManager_SessionSave";
	
	protected string m_sFileName;
	protected ref SCR_UploadSaveCallback_PageParams m_PageParams;
	protected ref SCR_ServerSaveUploadCallback m_UploadCallback;
	
	protected bool m_bHasData = false;
	
	override void OnSuccess(int code)
	{
		PrintFormat("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", code);
		
		if (code == 0)
			return;
		
		m_bHasData = code == EBackendRequest.EBREQ_WORKSHOP_GetAsset;
		
		WorldSaveItem item;
		WorldSaveManifest manifest = new WorldSaveManifest();
		
		bool isNew = GetGame().GetBackendApi().GetWorldSaveApi().GetTotalItemCount() == 0;
		if (!isNew)
		{
			//--- Use existing Workshop item
			array<WorldSaveItem> items = {};
			GetGame().GetBackendApi().GetWorldSaveApi().GetPageItems(items);
			item = items[0];
			
			if (m_bHasData)
				item.FillManifest(manifest);
			else
			{
				item.AskDetail(this);
				return;
			}		
		}
		
		//--- Define manifest params
		manifest.m_sName = SESSION_SAVE_NAME;
		manifest.m_sSummary = m_sFileName;
		manifest.m_aFileNames = {m_sFileName};
		
		//--- Create new save from manifest
		if (isNew)
		{
			item = GetGame().GetBackendApi().GetWorldSaveApi().CreateLocalWorldSave(manifest);
		}
		
		//--- Upload file
		m_UploadCallback = new SCR_ServerSaveUploadCallback();
		GetGame().GetBackendApi().GetWorldSaveApi().UploadWorldSave(manifest, m_UploadCallback, item);
	}
	override void OnError(int code, int restCode, int apiCode)
	{
		PrintFormat("[SCR_ServerSaveRequestCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnTimeout()
	{
		Print("[SCR_ServerSaveRequestCallback] OnTimeout");
	}
	void SCR_ServerSaveRequestCallback(string fileName)
	{
		m_sFileName = fileName;
		
		m_PageParams = new SCR_UploadSaveCallback_PageParams();
		m_PageParams.limit = 1;
		GetGame().GetBackendApi().GetWorldSaveApi().RequestPage(this, m_PageParams, false);
	}
};
class SCR_UploadSaveCallback_PageParams: PageParams
{
	override void OnPack()
	{
		StoreBoolean("owned", true);
		StoreString("search", SCR_ServerSaveRequestCallback.SESSION_SAVE_NAME);
	}
};

class SCR_ServerSaveUploadCallback: BackendCallback
{
	override void OnSuccess(int code)
	{
		PrintFormat("[SCR_ServerSaveUploadCallback] OnSuccess(): code=%1", code);
		
		BaseChatComponent chatComponent = BaseChatComponent.Cast(GetGame().GetPlayerController().FindComponent(BaseChatComponent));
		if (chatComponent)
			chatComponent.SendMessage(string.Format("#load %1", SCR_ServerSaveRequestCallback.SESSION_SAVE_NAME), 0);
	}
	override void OnError(int code, int restCode, int apiCode)
	{
		PrintFormat("[SCR_ServerSaveUploadCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnTimeout()
	{
		Print("[SCR_ServerSaveUploadCallback] OnTimeout");
	}
}
