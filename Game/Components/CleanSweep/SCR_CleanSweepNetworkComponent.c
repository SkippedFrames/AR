[EntityEditorProps(category: "GameScripted/CleanSweep", description: "A character clean sweep component for comunication from client to server.", color: "0 0 255 255")]
class SCR_CleanSweepNetworkComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CleanSweepNetworkComponent : ScriptComponent
{

	//------------------------------------------------------------------------------------------------
	void CommitSuicide()
	{
		Rpc(RPC_CommitSuicide);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAreaSelectionScreen()
	{
		Rpc(RPC_ShowAreaSelectionScreen);
	}
	
	//------------------------------------------------------------------------------------------------
	void ChooseArea(int areaID, bool swapSides)
	{
		Rpc(RPC_ChooseArea, areaID, swapSides);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControlledEntityOrigin(vector origin)
	{
		Rpc(RPC_SetControlledEntityOrigin, origin);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_CommitSuicide()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		GenericEntity controlledEntity = GenericEntity.Cast(playerController.GetMainEntity());
		if (!controlledEntity)
			return;
		
		CharacterControllerComponent controllerComponent = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		if (!controllerComponent)
			return;
		
		controllerComponent.ForceDeath();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_SetControlledEntityOrigin(vector origin)
	{
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return;
		
		controlledEntity.SetOrigin(origin);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_ShowAreaSelectionScreen()
	{
		SCR_GameModeCleanSweep cleanSweep = SCR_GameModeCleanSweep.Cast(GetGame().GetGameMode());
		if (!cleanSweep)
			return;
		
		cleanSweep.ShowAreaSelectionUI();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_ChooseArea(int areaID, bool swapSides)
	{
		SCR_GameModeCleanSweep cleanSweep = SCR_GameModeCleanSweep.Cast(GetGame().GetGameMode());
		if (!cleanSweep)
			return;
		
		cleanSweep.SetSwapSides(swapSides);
		cleanSweep.SetArea(areaID);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CleanSweepNetworkComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CleanSweepNetworkComponent()
	{
	}

};
