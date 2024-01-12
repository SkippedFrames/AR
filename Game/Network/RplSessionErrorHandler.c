//-----------------------------------------------------------------------------
class RplSessionErrorHandler : RplSessionCallbacks
{
	protected const string ERROR_GROUP = "REPLICATION";
	
	//-----------------------------------------------------------------------------
	void RplSessionErrorHandler()
	{
		RplSession.RegisterCallbacks(this);
	}
	
	//-----------------------------------------------------------------------------
	override void EOnFailed(string msg)
	{
		ref ServerBrowserMenuUI serverBrowser = ServerBrowserMenuUI.Cast(
		GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.ServerBrowserMenu));
		
		if (serverBrowser)
		{
			serverBrowser.SetErrorMessage(msg, ERROR_GROUP);
		}
	}

};
