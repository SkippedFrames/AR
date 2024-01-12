//-------------------------------------------------------------------------
class SCR_MenuLoadingComponent
{
	protected static ClientLobbyApi m_Lobby; 
	
	protected static bool m_bInitialLoad = true;
	//protected static ChimeraMenuPreset m_LastMenu = -1;
	
	protected static DialogUI m_DialogToDisplay = null;

	// Invoker  
	static ref ScriptInvoker m_OnMenuOpening = new ref ScriptInvoker;
	
	//static ref ScriptInvoker m_OnOpeningServerBrowser = new ref ScriptInvoker;
	
	//-------------------------------------------------------------------------
	//! Load previously active menu instead of main menu screen
	static bool LoadLastMenu()
	{
		// Special cases 
		if (JoinInvite())
			return false;
		
		string lastMenu = GameSessionStorage.s_Data["m_LastMenu"];
		
		// No menu to load 
		if (lastMenu == "")
			return false;
		
		// Load right menu 
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(lastMenu.ToInt());
		
		// Specific behavior 
		/*switch (m_LastMenu)
		{
			// Default 
			default:
			break;
		}*/
		
		m_OnMenuOpening.Invoke(lastMenu.ToInt());
		
		m_bInitialLoad = false;
		
		// Menu available
		return true;
	}

	
	//-------------------------------------------------------------------------
	static void SaveLastMenu(ChimeraMenuPreset menuPreset) 
	{ 
		GameSessionStorage.s_Data["m_LastMenu"] = menuPreset.ToString();
		//m_LastMenu = menuPreset;
	}
	
	//-------------------------------------------------------------------------
	static void ClearLastMenu() 
	{
		 GameSessionStorage.s_Data["m_LastMenu"] = ""; 
	}
	
	//-------------------------------------------------------------------------
	//! Start joining to inveted room 
	protected static bool JoinInvite()
	{
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();
		
		// Check invited rooom
		Room invited = lobby.GetInviteRoom();
		if (!invited)
			return false;
		
		#ifdef SB_DEBUG
		Print("[SCR_MenuLoadingComponent] Client is invited to room: " + invited + ", name: " + invited.Name());
		#endif
		
		// Open server browser 
		ServerBrowserMenuUI serverBrowser = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (serverBrowser)
		{
			serverBrowser.JoinProcess_Init(invited);
			return true;
		}
		
		return false;
	}

	//-------------------------------------------------------------------------
	void SCR_MenuLoadingComponent()
	{
		
	}
	
	//-------------------------------------------------------------------------
	void ~SCR_MenuLoadingComponent()
	{
		
	}
};
