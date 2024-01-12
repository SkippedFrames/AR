[EntityEditorProps(category: "GameScripted/Building", description: "Handles client > server communication in Building. Should be attached to PlayerController.")]
class SCR_CampaignBuildingNetworkComponentClass : ScriptComponentClass
{
}

class SCR_CampaignBuildingNetworkComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	void AddBuildingValue(int buildingValue, notnull IEntity compositionOutline)
	{
		RplComponent comp = RplComponent.Cast(compositionOutline.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_AddBuildingValue, buildingValue, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEditorMode(int playerID, IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_RemoveEditorMode, playerID, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	void RequestEnterBuildingMode(IEntity provider, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		RplComponent providerRplComp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!providerRplComp)
			return;

		Rpc(RpcAsk_RequestEnterBuildingMode, providerRplComp.Id(), playerID, UserActionActivationOnly, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete composition by a tool
	void DeleteCompositionByUserAction(notnull IEntity composition)
	{
		RplComponent comp = RplComponent.Cast(composition.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_DeleteCompositionByUserAction, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//! Send a notification about deleted composition
	void SendDeleteNotification(notnull IEntity composition, int playerId, int callsign)
	{
		RplComponent comp = RplComponent.Cast(composition.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_SendDeleteNotification, comp.Id(), playerId, callsign);
	}

	//------------------------------------------------------------------------------------------------
	//! Add a building value to a composition outline. Once the define value is reach, composition will be spawned.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddBuildingValue(int buildingValue, RplId compId)
	{
		IEntity compositionOutline = GetProviderFormRplId(compId);
		if (!compositionOutline)
			return;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(compositionOutline.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return;

		layoutComponent.AddBuildingValue(buildingValue);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete given player building mode.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveEditorMode(int playerID, RplId compId)
	{
		IEntity provider = GetProviderFormRplId(compId);
		if (!provider)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		bool isActiveUser = buildingManagerComponent.RemovePlayerIdFromProvider(playerID, providerComponent);
		buildingManagerComponent.RemoveProvider(playerID, providerComponent, isActiveUser);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a trigger as a child of the provider entity.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestEnterBuildingMode(RplId rplProviderId, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		IEntity provider = GetProviderFormRplId(rplProviderId);
		if (!provider)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		buildingManagerComponent.GetEditorMode(playerID, provider, UserActionActivationOnly, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete composition, executed from user action
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeleteCompositionByUserAction(RplId rplCompositionId)
	{
		IEntity composition = GetProviderFormRplId(rplCompositionId);
		if (!composition)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetRootParent().FindComponent(SCR_EditableEntityComponent));
		if (editableEntity)
			editableEntity.Delete();
		else
			RplComponent.DeleteRplEntity(composition, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Sent delete notification.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SendDeleteNotification(RplId rplCompositionId, int playerId, int callsign)
	{
		IEntity composition = GetProviderFormRplId(rplCompositionId);
		if (!composition)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));
		if (!faction)
			return;

		SCR_NotificationsComponent.SendToFaction(faction, true, ENotification.EDITOR_SERVICE_DISASSEMBLED, playerId, rplCompositionId, callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increase XP
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddXPReward(int playerId)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!compXP)
			return;
		
		compXP.AwardXP(playerId, SCR_EXPRewards.FREE_ROAM_BUILDING_BUILT);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddXPReward(int playerId)
	{
		Rpc(RpcAsk_AddXPReward, playerId);
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetProviderFormRplId(RplId rplProviderId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplProviderId));
		if (!rplComp)
			return null;

		return rplComp.GetEntity();
	}
}
