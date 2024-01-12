[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Restore editor modes when world changes in multiplayer.")]
class SCR_RestoreEditorModesComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RestoreEditorModesComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "When enabled, the system will remember and restore also modes of limited editor managers (e.g., those with only PHOTO mode, not EDIT).")]
	protected bool m_bRestoreLimitedEditors;
	
	protected string GAME_SESSION_STORAGE_MODES = "SCR_RestoreEditorModesComponent_ModesMap";
	protected string DELIMITER_PLAYERS = " ";
	protected string DELIMITER_VALUES = ":";
	
	protected ref map<string, EEditorMode> m_mModes = new map<string, EEditorMode>();
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerCreatedServer(SCR_EditorManagerEntity editorManager)
	{
		int playerID = editorManager.GetPlayerID();
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerID);
		EEditorMode modes;
		if (m_mModes.Find(playerUID, modes))
		{
			PrintFormat("SCR_RestoreEditorModesComponent: Restoring editor modes %1 for playerId=%2 ('%3')", SCR_Enum.FlagsToString(EEditorMode, modes), playerID, GetGame().GetPlayerManager().GetPlayerName(playerID));
			editorManager.AddEditorModes(EEditorModeAccess.BASE, modes);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (!Replication.IsRunning() || !Replication.IsServer())
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		BackendApi backendApi = GetGame().GetBackendApi();
		
		string modes;
		array<SCR_EditorManagerEntity> managers = {};
		core.GetEditorEntities(managers);
		foreach (SCR_EditorManagerEntity editorManager: managers)
		{
			if (m_bRestoreLimitedEditors || !editorManager.IsLimited())
				modes += backendApi.GetPlayerIdentityId(editorManager.GetPlayerID()) + DELIMITER_VALUES + editorManager.GetEditorModes() + DELIMITER_PLAYERS;
		}
		
		GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_MODES, modes);
		
		core.Event_OnEditorManagerCreatedServer.Remove(OnEditorManagerCreatedServer);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsRunning() || !Replication.IsServer())
			return;
		
		string modesMap;
		if (!GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_MODES, modesMap))
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		array<string> players = {};
		array<string> values = {};
		modesMap.Split(DELIMITER_PLAYERS, players, true);
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			players[i].Split(DELIMITER_VALUES, values, false);
			m_mModes.Insert(values[0], values[1].ToInt());
		}
		
		GameSessionStorage.s_Data.Remove(GAME_SESSION_STORAGE_MODES);
		
		core.Event_OnEditorManagerCreatedServer.Insert(OnEditorManagerCreatedServer);
	}
}
