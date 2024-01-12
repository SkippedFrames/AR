[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableDeployableSpawnPointsEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		BaseGameMode gameMode = BaseGameMode.Cast(item);
		if (!gameMode)
			return null;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(playerSpawnPointManager.IsDeployingSpawnPointsEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gameMode = BaseGameMode.Cast(item);
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return;
			
		playerSpawnPointManager.EnableDeployableSpawnPoints(var.GetBool(), playerID);
	}
}
