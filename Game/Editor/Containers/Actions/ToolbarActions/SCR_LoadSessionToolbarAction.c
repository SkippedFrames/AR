[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_LoadSessionToolbarAction : SCR_EditorToolbarAction
{
	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		//--- The action opens local UI
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//--- Disallow if editor save struct is not configured
		SCR_SaveLoadComponent saveLoadComponent = SCR_SaveLoadComponent.GetInstance();
		if (!saveLoadComponent || !saveLoadComponent.ContainsStruct(SCR_EditorStruct))
			return false;
		
		if (Replication.IsServer())
		{
			//--- Loading is always available for the host and in singleplayer
			return true;
		}
		else
		{
			//--- Loading is available only if the game is not hosted (the host has exclusive access to that feature, nobody should do it without their knowledge)
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			return gameMode && !gameMode.IsHosted();
		}
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.EditorLoadDialog);
	}
};
