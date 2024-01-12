#include "scripts/Game/config.c"
#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Campaign", description: "Handles client > server communication in Campaign. Should be attached to PlayerController.", color: "0 0 255 255")]
class SCR_CampaignNetworkComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Used to identify various notifications for client
enum ECampaignClientNotificationID
{
	VEHICLE_SPAWNED,
	NO_SPACE,
	OUT_OF_STOCK,
	SUPPLIES_LOADED,
	SUPPLIES_UNLOADED,
	RESPAWN
};

//------------------------------------------------------------------------------------------------
//! Takes care of Campaign-specific server <> client communication and requests
class SCR_CampaignNetworkComponent : ScriptComponent
{
	// Member variables 
	protected SCR_PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	protected bool m_bFirstSpawn = true;

	protected vector m_vLastLoadedAt;
	protected vector m_vLastUnloadedAt;

	protected float m_fNoRewardSupplies;

	static const float FULL_SUPPLY_TRUCK_AMOUNT = 600.0;
	static const int SUPPLY_DELIVERY_THRESHOLD_SQ = 100 * 100;

	//********************************//
	//RUNTIME SYNCHED MEMBER VARIABLES//
	//********************************//
	
	[RplProp(condition: RplCondition.OwnerOnly)]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fLastAssetRequestTimestamp = -int.MAX;
	#else
	protected WorldTimestamp m_fLastAssetRequestTimestamp;
	#endif
	[RplProp(condition: RplCondition.OwnerOnly)]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fLastHQRadioMessageTimestamp;
	#else
	protected WorldTimestamp m_fLastHQRadioMessageTimestamp;
	#endif
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignNetworkComponent GetCampaignNetworkComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		
		if (!playerController)
			return null;
		
		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		return networkComponent;
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	//! Getter for request cooldown
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetLastRequestTimestamp()
	#else
	WorldTimestamp GetLastRequestTimestamp()
	#endif
	{
		return m_fLastAssetRequestTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for request cooldown
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetLastRequestTimestamp(float timestamp)
	#else
	void SetLastRequestTimestamp(WorldTimestamp timestamp)
	#endif
	{
		if (IsProxy())
			return;
		
		m_fLastAssetRequestTimestamp = timestamp;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Repair destroyed mandatory part of composition
	void RepairComposition(int index, int repairCost, int destructibleID, SCR_SiteSlotEntity slotEnt, notnull SCR_CampaignMilitaryBaseComponent base)
	{
#ifdef ENABLE_BASE_DESTRUCTION
		Rpc(RpcAsk_RepairComposition, index, repairCost, destructibleID, Replication.FindId(slotEnt), Replication.FindId(base));
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to load supplies at a base
	//! \param truckID Unique ID identifying the supply truck
	//! \param player Player trying to unload supplies
	//! \param base Base at which the supplies are being unloaded
	void LoadSupplies(RplId suppliesID, IEntity player, SCR_CampaignMilitaryBaseComponent base, int amount = 0)
	{
		if (!player || !base)
			return;
		
		RplId baseID = Replication.FindId(base);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_LoadSupplies, suppliesID, playerID, baseID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadSuppliesStandalone(RplId suppliesID, IEntity player, SCR_CampaignSuppliesComponent depot, int amount = 0)
	{
		if (!player || !depot)
			return;
		
		RplId depotID = Replication.FindId(depot);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_LoadSuppliesStandalone, suppliesID, playerID, depotID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	void StartLoading(RplId suppliesID, int supplies, bool IsUnloading = false)
	{
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SuppliesLoadingStarted, suppliesID, playerID, supplies, IsUnloading);
	}
	
	//------------------------------------------------------------------------------------------------
	void StopLoading(RplId suppliesID, bool IsUnloading = false)
	{
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SuppliesLoadingCanceled, suppliesID, playerID, IsUnloading);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to unload supplies at a base
	//! \param truckID Unique ID identifying the supply truck
	//! \param player Player trying to unload supplies
	//! \param base Base at which the supplies are being unloaded
	void UnloadSupplies(RplId suppliesID, IEntity player, SCR_CampaignMilitaryBaseComponent base, int amount = 0)
	{
		if (!player || !base)
			return;
		
		RplId baseID = Replication.FindId(base);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_UnloadSupplies, suppliesID, playerID, baseID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRadio()
	{
		Rpc(RpcAsk_AddRadio, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddRadio(int playerID)
	{
		PlayerManager pMan = GetGame().GetPlayerManager();
		
		if (!pMan)
			return;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pMan.GetPlayerControlledEntity(playerID));
		
		if (!player)
			return;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();	
		spawnParams.TransformMode = ETransformMode.WORLD;
		player.GetWorldTransform(spawnParams.Transform);
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(player.GetFaction());
		
		if (!faction)
			return;
		
		Resource res = Resource.Load(faction.GetRadioPrefab());
		
		if (!res)
			return;
		
		IEntity radio = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(),spawnParams);
		
		if (!radio)
			return;
		
		RplComponent rplC = RplComponent.Cast(radio.FindComponent(RplComponent));
		
		if (!rplC)
			return;
		
		Rpc(RpcDo_AddRadio, rplC.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_AddRadio(RplId ID)
	{
		GetGame().GetCallqueue().CallLater(FindRadioDelayed, 10, true, ID)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindRadioDelayed(RplId ID)
	{
		RplComponent rplC = RplComponent.Cast(Replication.FindItem(ID));
		
		if (!rplC)
			return;
		
		GetGame().GetCallqueue().Remove(FindRadioDelayed);
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		if (!player)
			return;
		
		IEntity radio = rplC.GetEntity();
		
		if (!radio)
			return;
		
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!inventory)
			return;
			
		inventory.InsertItem(radio);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeployMobileAsembly(notnull SCR_CampaignMobileAssemblyComponent comp, bool deploy)
	{
		Rpc(RpcAsk_DeployMobileAsembly, Replication.FindId(comp), deploy, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeployMobileAsembly(RplId assemblyComponentID, bool deploy, int playerID)
	{
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(assemblyComponentID));
		
		if (!comp)
			return;
		
		float depth;
		
		if (SCR_WorldTools.IsObjectUnderwater(comp.GetOwner(), vector.Zero, -1, depth) && depth > SCR_CampaignMobileAssemblyComponent.MAX_WATER_DEPTH)
			return;
		
		if (comp.IsDeployed() == deploy)
			return;
		
		if (deploy)
			comp.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED, playerID);
		else
			comp.Deploy(SCR_EMobileAssemblyStatus.DISMANTLED, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to capture a base (change its owner)
	//! \param base Base to be captured
	void CaptureBase(SCR_CampaignMilitaryBaseComponent base)
	{
		if (!base)
			return;
		
		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		int factionID = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);
		int playerID = m_PlayerController.GetPlayerId();
		IEntity player = m_PlayerController.GetControlledEntity();
		
		if (!player)
			return;
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		
		if (!comp)
			return;
		
		if (comp.IsDead())
			return;
		
		Rpc(RpcAsk_CaptureBase, Replication.FindId(base), factionID, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void CaptureBaseGM(SCR_CampaignMilitaryBaseComponent base, int factionIndex)
	{
		if (!base)
			return;
		
		Rpc(RpcAsk_CaptureBase, Replication.FindId(base), factionIndex, SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX);
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleBaseCapture(notnull SCR_CampaignMilitaryBaseComponent base, bool isBeingCaptured)
	{
		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		int factionID = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);
		int playerID = m_PlayerController.GetPlayerId();
		
		if (isBeingCaptured)
			Rpc(RpcAsk_CaptureBaseBegin, Replication.FindId(base), factionID, playerID);
		else
			Rpc(RpcAsk_CaptureBaseEnd, Replication.FindId(base));
	}
	
	//***********//
	//RPC METHODS//
	//***********//
	
#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	//! Repair damaged entity in composition
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RepairComposition(int index, int repairCost, int destructibleID, RplId slotID, RplId baseID)
	{
		if (index == -1 || repairCost == -1 || destructibleID == -1)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_DestructionMultiPhaseComponent destructibleComp = SCR_DestructionMultiPhaseComponent.Cast(destructionManager.FindDynamicallySpawnedDestructibleByIndex(destructibleID, index));
		if (!destructibleComp)
			return;
				
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		if (!base)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
			
		IEntity composition = IEntity.Cast(SCR_EntityHelper.GetMainParent(destructibleComp.GetOwner()));
		if (!composition)
			return;
		
		SCR_CampaignServiceEntityComponent serviceEntityComp = SCR_CampaignServiceEntityComponent.Cast(destructibleComp.GetOwner().FindComponent(SCR_CampaignServiceEntityComponent));
		if (!serviceEntityComp)
			return; 
		
		SCR_CampaignServiceCompositionComponent serviceCompositionComp = SCR_CampaignServiceCompositionComponent.Cast(composition.FindComponent(SCR_CampaignServiceCompositionComponent));
		if (!serviceCompositionComp)
			return;	
		
		// Check if the composition entity belong to is disabled or not. If so, increase number of spawn tickets again.
		/*if (!serviceCompositionComp.IsServiceOperable())
			campaign.OnStructureChanged(base, SCR_SiteSlotEntity.Cast(Replication.FindItem(slotID)), base.GetServiceByLabel(serviceCompositionComp.GetCompositionType()), true);*/
		
		// Repair entity
		serviceEntityComp.RepairEntity();
		
		// Supply in base are reduced (cost of repair)
		base.AddSupplies(-repairCost);
		
		// Update map UI
		if (RplSession.Mode() != RplMode.Dedicated)
			base.GetMapDescriptor().HandleMapInfo();
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	void AddSuppliesFromContextMenu(notnull SCR_CampaignMilitaryBaseComponent base, int suppliesCnt)
	{
		RplId baseID = Replication.FindId(base);
		
		if (!baseID.IsValid())
			return;
		
		Rpc(RpcAsk_AddSuppliesFromContextMenu, baseID, suppliesCnt);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddSuppliesFromContextMenu(RplId baseID, int suppliesCnt)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		if (base.GetSupplies() >= base.GetSuppliesMax())
			return;
		
		base.AddSupplies(Math.Min(suppliesCnt, base.GetSuppliesMax() - base.GetSupplies()));
	}
	//------------------------------------------------------------------------------------------------
	// Sends player notification to players inside Vehicle
	// parameters: entity of vehicle, int ID of player to notify about, optional second parameter for another int number
	protected void SendToVehicleOccupants(ENotification messageID, IEntity vehicleEntity, int playerID, int number = 0)
	{
		array<IEntity> occupants = {}; 
		IEntity parentVehicle = vehicleEntity.GetParent();
		
		//Gettings players from inside of vehicle. Condition allows spawning only Cargo.
		SCR_BaseCompartmentManagerComponent comp = SCR_BaseCompartmentManagerComponent.Cast(parentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		
		if(!parentVehicle)
			return;
		
		comp = SCR_BaseCompartmentManagerComponent.Cast(parentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		comp.GetOccupants(occupants);
		
		if(occupants.IsEmpty())
			return;
		
		foreach (IEntity occupant : occupants)
		{
			int occupantID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occupant);
			if(number != 0)
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID, number);
			else
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Handles start of loading/unloading supplies
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SuppliesLoadingStarted(RplId suppliesID, int playerID, int supplies, bool IsUnloading)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));
		array<IEntity> occupants = {}; //new array<IEntity>;
		
		if(!suppliesComponent)
			return;
		
		//Gettings vehicle entity from cargo
		IEntity vehicleEntity = suppliesComponent.GetOwner();
		
		if(!IsUnloading)
		{
			//suppliesComponent.SetSupplyLoadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER, vehicleEntity, playerID, supplies);
		}
		else
		{
			//suppliesComponent.SetSupplyUnloadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER, vehicleEntity, playerID, supplies);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Handles canceling of loading/unloading supplies
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SuppliesLoadingCanceled(RplId suppliesID, int playerID, bool IsUnloading)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));
		
		if(!suppliesComponent)
			return;
		
		//Gettings vehicle entity from cargo
		IEntity vehicleEntity = suppliesComponent.GetOwner();
		
		if(!IsUnloading)
		{
			//suppliesComponent.DeleteSupplyLoadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_STOPPED, vehicleEntity, playerID);
		}
		else
		{
			//suppliesComponent.DeleteSupplyUnloadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER_STOPPED, vehicleEntity, playerID);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load supplies at a base
	//! \param vehicleID Vehicle entity ID
	//! \param playerID player entity ID
	//! \param baseID base entity ID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LoadSupplies(RplId suppliesID, int playerID, RplId baseID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;
		
		if (amount > base.GetSupplies())
			return;
		
		/*if (suppliesComponent.GetSupplies() == suppliesComponent.GetSuppliesMax())
			return;*/
		
		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(player);
		
		if (!playerFaction || playerFaction != base.GetFaction())
			return;
		
		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
		if (!baseSuppliesComponent)
			return;
		
		/*float distSq = Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2);
		vector vehPos = box.GetOrigin();
		
		if (vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
			return;
		
		if (vector.DistanceSq(vehPos, base.GetOwner().GetOrigin()) > distSq)
		{
			SCR_ServicePointComponent service = base.GetServiceByType(SCR_EServicePointType.SUPPLY_DEPOT);
			if (!service)
				return;
			
			if (vector.DistanceSq(vehPos, service.GetOwner().GetOrigin()) > distSq)
				return;
		}
		
		// Validity check passed, perform action
		int finalAmount = Math.Min(suppliesComponent.GetSuppliesMax() - suppliesComponent.GetSupplies(), amount);
		suppliesComponent.AddSupplies(finalAmount);
		base.AddSupplies(-finalAmount);
		suppliesComponent.SetLastLoadedAt(base);
		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_LOADED, (float)finalAmount, base.GetCallsign());
		suppliesComponent.DeleteSupplyLoadingPlayer(playerID);*/
		
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_FINISHED, suppliesComponent.GetOwner(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LoadSuppliesStandalone(RplId suppliesID, int playerID, RplId depotID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;
		
		SCR_CampaignSuppliesComponent depot = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(depotID));
		
		if (!depot)
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;
		
		/*if (amount > depot.GetSupplies())
			return;
		
		if (suppliesComponent.GetSupplies() == suppliesComponent.GetSuppliesMax())
			return;
		
		float distSq = Math.Pow(depot.GetOperationalRadius(), 2);
		vector vehPos = box.GetOrigin();
		
		if (vector.DistanceSq(vehPos, depot.GetOwner().GetOrigin()) > distSq || vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
			return;
		
		// Validity check passed, perform action
		int finalAmount = Math.Min(suppliesComponent.GetSuppliesMax() - suppliesComponent.GetSupplies(), amount);
		suppliesComponent.AddSupplies(finalAmount);
		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_LOADED, (float)finalAmount, -1);
		suppliesComponent.DeleteSupplyLoadingPlayer(playerID);*/
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_FINISHED, suppliesComponent.GetOwner(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unload supplies at a base
	//! \param vehicleID Vehicle entity ID
	//! \param playerID Vehicle entity ID
	//! \param baseID Vehicle entity ID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UnloadSupplies(RplId suppliesID, int playerID, RplId baseID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;
		
		/*if (amount > suppliesComponent.GetSupplies())
			return;
		
		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(player);
		SCR_CampaignFaction owningFaction = base.GetCampaignFaction();
		
		if (!playerFaction || playerFaction != owningFaction)
			return;
		
		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
		if (!baseSuppliesComponent)
			return;

		float distSq = Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2);
		vector vehPos = box.GetOrigin();
		
		if (vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
			return;
		
		if (vector.DistanceSq(vehPos, base.GetOwner().GetOrigin()) > distSq)
		{
			SCR_ServicePointComponent service = base.GetServiceByType(SCR_EServicePointType.SUPPLY_DEPOT);
			if (!service)
				return;
			
			if (vector.DistanceSq(vehPos, service.GetOwner().GetOrigin()) > distSq)
				return;
		}
		
		// Validity check passed, perform action
		int suppliesCur = base.GetSupplies();
		int suppliesMax = base.GetSuppliesMax();
		int suppliesCnt = Math.Min(amount, suppliesMax - suppliesCur);
		float rewardMultiplier = suppliesCnt / suppliesComponent.GetSuppliesMax();
		suppliesComponent.AddSupplies(-suppliesCnt);
		base.AddSupplies(suppliesCnt);
		suppliesComponent.SetLastUnloadedAt(base);
		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_UNLOADED, (float)suppliesCnt, base.GetCallsign());
		
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		
		// Award XP unless the truck was just loaded in this base
		// ... or if it was both loaded and unloaded in the previous base
		// (handled in suppliesComponent)
		if (compXP && suppliesComponent.AwardXP())
			compXP.AwardXP(playerID, SCR_EXPRewards.SUPPLIES_DELIVERED, rewardMultiplier);
		
		SendPlayerMessage(SCR_ERadioMsg.SUPPLIES, base.GetCallsign(), public: false);
		suppliesComponent.DeleteSupplyUnloadingPlayer(playerID);*/
		
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER_FINISHED, suppliesComponent.GetOwner(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBaseBegin(RplId baseID, int factionIndex, int playerID)
	{
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(factionIndex);
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		base.BeginCapture(faction, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBaseEnd(RplId baseID)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		base.EndCapture();
	}
	
	//------------------------------------------------------------------------------------------------
	void SendPlayerMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int calledID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM, bool checkHQReached = false)
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!fManager)
			return;
		
		if (!m_PlayerController)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity player = m_PlayerController.GetMainEntity();
		
		if (!player)
			return;

		int companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller;
		
		if (!callsignManager.GetEntityCallsignIndexes(player, companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller))
    		return;
		
		SCR_GadgetManagerComponent gadgetMan = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));
		
		if (!gadgetMan)
			return;
		
		IEntity radioEnt = gadgetMan.GetGadgetByType(EGadgetType.RADIO);
		
		if (!radioEnt)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEnt.FindComponent(BaseRadioComponent));
		
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;

		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		
		int factionId = fManager.GetFactionIndex(fManager.GetPlayerFaction(m_PlayerController.GetPlayerId()));
		
		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg;
		msg.SetRadioMsg(msgType);
		msg.SetFactionId(factionId);
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCallerCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(m_PlayerController.GetPlayerId());
		msg.SetEncryptionKey(radio.GetEncryptionKey());
		
		int companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled;
		
		if (called && callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled))
			msg.SetCalledCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled);

		Rpc(RpcDo_PlayRadioMsg, msgType, factionId, baseCallsign, CompressCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller), CompressCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled), param, msg.GetSeed(), 1.0);
		
		if (public)
			transmitter.BeginTransmission(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetLastHQRadioMessageTimestamp(float time)
	#else
	void SetLastHQRadioMessageTimestamp(WorldTimestamp time)
	#endif
	{
		m_fLastHQRadioMessageTimestamp = time;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int CompressCallsign(int company, int platoon, int squad)
	{
		return (company * 10000) + (platoon * 100) + squad;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DecompressCallsign(int callsign, out int company, out int platoon, out int squad)
	{
		company = Math.Floor(callsign / 10000);
		callsign = callsign - (company * 10000);
		
		platoon = Math.Floor(callsign / 100);
		callsign = callsign - (platoon * 100);
		
		squad = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capture a base (change its owner)
	//! \param baseID Base entity ID
	//! \param factionIndex Index of new faction to own the base
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBase(RplId baseID, int factionIndex, int playerID)
	{
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(factionIndex);
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		if (base.BeginCapture(faction, playerID))
			base.SetFaction(faction);
		
		if (base.GetType() == SCR_ECampaignBaseType.RELAY && playerID != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
			SendPlayerMessage(SCR_ERadioMsg.RELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedback(int msgID)
	{
		PlayerFeedbackImpl(msgID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackValueBase(int msgID, float value, int baseID)
	{
		// Short delay so replicated values have time to catch up on client's machine
		GetGame().GetCallqueue().CallLater(PlayerFeedbackImpl, SCR_GameModeCampaign.MINIMUM_DELAY, false, msgID, value, -1, baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackBase(int msgID, int baseID)
	{
		PlayerFeedbackImpl(msgID, 0, -1, baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	protected void PlayerFeedbackImpl(int msgID, float value = 0, int assetID = -1, int baseID = -1)
	{
		LocalizedString msg;
		LocalizedString msg2;
		int duration = 2;
		int prio = -1;
		string msg1param1;
		string msg2param1;
		string msg2param2;
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		SCR_CampaignMilitaryBaseComponent base = campaign.GetBaseManager().FindBaseByCallsign(baseID);
		SCR_CampaignFeedbackComponent manager = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!campaign)
			return;
		
		switch (msgID)
		{	
			case ECampaignClientNotificationID.SUPPLIES_LOADED:
			{
				msg = "#AR-Campaign_SuppliesLoaded-UC";
				duration = 6;
				prio = SCR_ECampaignPopupPriority.SUPPLIES_HANDLED;
				msg1param1 = value.ToString();
				
				if (base)
				{
					msg2 = "#AR-Campaign_SuppliesAmountInfo-UC";
					msg2param1 = base.GetBaseName();
					msg2param2 = base.GetSupplies().ToString();
				}
				
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_LOADSUPPLIES);
				break;
			};
			case ECampaignClientNotificationID.SUPPLIES_UNLOADED:
			{
				msg = "#AR-Campaign_SuppliesUnloaded-UC";
				duration = 6;
				prio = SCR_ECampaignPopupPriority.SUPPLIES_HANDLED;
				msg1param1 = value.ToString();
				
				if (base)
				{
					msg2 = "#AR-Campaign_SuppliesAmountInfo-UC";
					msg2param1 = base.GetBaseName();
					msg2param2 = base.GetSupplies().ToString();
				}
				
				if (!campaign.IsTutorial())
				{
					if (manager)
						manager.ShowHint(EHint.CONFLICT_BUILDING);
				}
				
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_UNLOADSUPPLIES);
				break;
			};
			case ECampaignClientNotificationID.RESPAWN:
			{
				//manager.SetIsPlayerInRadioRange(true);
				if (manager)
					manager.OnRespawn();
				
				if (!base)
					return;

				msg = base.GetBaseNameUpperCase();
				ChimeraWorld world = m_PlayerController.GetWorld();
				TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
				
				if (timeManager)
				{
					int hours;
					int minutes;
					int seconds;
					timeManager.GetHoursMinutesSeconds(hours, minutes, seconds);
					string strHours = hours.ToString();
					
					if (hours > 0 && hours < 10)
						strHours = "0" + strHours;
					
					string strMinutes = minutes.ToString();
					
					if (minutes < 10)
						strMinutes = "0" + strMinutes;
					
					msg = string.Format("%1, %2:%3", msg, strHours, strMinutes);
				}
				
				msg2 = SCR_BaseTask.TASK_HINT_TEXT;
				msg2param1 = SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT;
				duration = 5;
				prio = SCR_ECampaignPopupPriority.RESPAWN;
				
				if (m_bFirstSpawn)
				{
					m_bFirstSpawn = false;
					duration = 120;
				}
				
				break;
			}; 			
			default: {return;};
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(msg, duration, msg2, param1: msg1param1, text2param1: msg2param1, text2param2: msg2param2);
	}
	
	//------------------------------------------------------------------------------------------------
	void SendVehicleSpawnHint(int hintID)
	{
		Rpc(RpcDo_VehicleSpawnHint, hintID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_VehicleSpawnHint(int hintID)
	{
		SCR_CampaignFeedbackComponent feedbackComponent = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!feedbackComponent)
			return;
		
		feedbackComponent.ShowHint(hintID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RespawnLocationPopup(int baseID)
	{
		Rpc(RpcDo_PlayerFeedbackBase, ECampaignClientNotificationID.RESPAWN, baseID);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerAliveStateChanged(bool alive)
	{
		Rpc(RpcDo_PlayerEnableShowingSpawnPosition, alive);	
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.Cast(GetOwner().FindComponent(SCR_CampaignFeedbackComponent));

		if (!comp)
			return;

		IEntity player = m_PlayerController.GetControlledEntity();
		
		if (!player)
			return;

		EventHandlerManagerComponent eventHandlerManagerComponent = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));

		if (!eventHandlerManagerComponent)
			return;

		if (alive)
		{
			comp.OnConsciousnessChanged(true);
			eventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", comp, comp.OnConsciousnessChanged);
		}
		else
		{
			eventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", comp, comp.OnConsciousnessChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_PlayerEnableShowingSpawnPosition(bool enable)
	{
		// Adds function to queue. Sometimes, game is not fast enough to spawn player entity
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
			GetGame().GetCallqueue().CallLater(comp.EnablePlayerSpawnHint, 100, true, enable);
		
		if (!enable)
			comp.PauseHintQueue();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleRadioRespawnTimer(RplId spawnPointId)
	{
		SCR_PlayerRadioSpawnPointCampaign spawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(SCR_SpawnPoint.GetSpawnPointByRplId(spawnPointId));
		
		if (!spawnpoint)
			return;

		Rpc(RpcAsk_HandleRadioRespawnTimer, spawnPointId);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_HandleRadioRespawnTimer(RplId selectedSpawnPointId)
	{
		if (!m_PlayerController)
			return;
		
		SCR_PlayerRadioSpawnPointCampaign spawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(SCR_SpawnPoint.GetSpawnPointByRplId(selectedSpawnPointId));
		
		if (!spawnpoint)
			return;
		
		IEntity operator = GetGame().GetPlayerManager().GetPlayerControlledEntity(spawnpoint.GetPlayerID());
		
		if (!operator)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		
		if (!fManager)
			return;
		
		SCR_TimedSpawnPointComponent timer = SCR_TimedSpawnPointComponent.Cast(campaign.FindComponent(SCR_TimedSpawnPointComponent));
		
		if (!timer)	
			return;
		
		int playerId = m_PlayerController.GetPlayerId();
		float suicidePenalty;
		
		SCR_CampaignClientData data = campaign.GetClientData(playerId);
		
		if (data)
			suicidePenalty = data.GetRespawnPenalty();
		
		timer.SetRespawnTime(playerId, fManager.GetRankRadioRespawnCooldown(SCR_CharacterRankComponent.GetCharacterRank(operator)) + suicidePenalty);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayRadioMsg(SCR_ERadioMsg msg, int FactionId, int baseCallsign, int callerCallsignCompany, int callerCallsignPlatoon, int callerCallsignSquad, int calledCallsignCompany, int calledCallsignPlatoon, int calledCallsignSquad, bool isPublic, int param, float seed, float quality, int playerID)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		int playerCallsignCompany, playerCallsignPlatoon, playerCallsignSquad, playerCallsignCharacter;
		callsignManager.GetEntityCallsignIndexes(m_PlayerController.GetMainEntity(), playerCallsignCompany, playerCallsignPlatoon, playerCallsignSquad, playerCallsignCharacter);
		
		if (isPublic || playerID == m_PlayerController.GetPlayerId())
			Rpc(RpcDo_PlayRadioMsg, msg, FactionId, baseCallsign, CompressCallsign(callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad), CompressCallsign(calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad), param, seed, quality);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_PlayRadioMsg(SCR_ERadioMsg msg, int FactionId, int baseCallsign, int callerCallsign, int calledCallsign, int param, float seed, float quality)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		int callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad;
		DecompressCallsign(callerCallsign, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad);
		DecompressCallsign(calledCallsign, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad);
		
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		comp.PlayRadioMsg(msg, FactionId, baseCallsign, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad, param, seed, quality);

		/*if (checkHQReached)
			GetGame().GetCallqueue().CallLater(CheckHQReached, 7000)*/
	}
		
	//------------------------------------------------------------------------------------------------
	void CheckHQReached()
	{
		/*#ifndef AR_CAMPAIGN_TIMESTAMP
		if (m_fLastHQRadioMessageTimestamp < (Replication.Time() - 8000))
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fLastHQRadioMessageTimestamp.PlusMilliseconds(8000).Less(world.GetServerTimestamp()))
		#endif
			return;*/
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSuppliesInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (!playerController || !resourceComponentFrom || !resourceComponentTo)
			return;

		IEntity player = playerController.GetControlledEntity();

		if (!player)
			return;

		vector pos = player.GetOrigin();

		switch (interactionType)
		{
			case EResourcePlayerInteractionType.VEHICLE_LOAD:
			{
				OnSuppliesLoaded(pos, resourceValue);
				break;
			}
			
			case EResourcePlayerInteractionType.VEHICLE_UNLOAD:
			{
				OnSuppliesUnloaded(pos, resourceValue, playerController.GetPlayerId());
				break;
			}
			
			case EResourcePlayerInteractionType.INVENTORY_SPLIT:
			{
				Vehicle vehicleFrom = Vehicle.Cast(SCR_EntityHelper.GetMainParent(resourceComponentFrom.GetOwner(), true));
				Vehicle vehicleTo = Vehicle.Cast(SCR_EntityHelper.GetMainParent(resourceComponentTo.GetOwner(), true));
				
				// Ignore vehicle to vehicle transfers
				if (vehicleFrom && vehicleTo)
					break;
				
				if (vehicleFrom)
					OnSuppliesUnloaded(pos, resourceValue, playerController.GetPlayerId());
				else
					OnSuppliesLoaded(pos, resourceValue);
				
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesLoaded(vector position, float amount)
	{
		if (m_vLastLoadedAt == vector.Zero || vector.DistanceSqXZ(m_vLastLoadedAt, position) <= SUPPLY_DELIVERY_THRESHOLD_SQ)
			m_fNoRewardSupplies += amount;		// Player is loading at the same spot as before
		else
			m_fNoRewardSupplies = amount;		// Player is loading somewhere new
		
		m_vLastLoadedAt = position;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesUnloaded(vector position, float amount, int playerId)
	{
		bool firstUnload = m_vLastUnloadedAt == vector.Zero;
		bool notLoadedYet = m_vLastLoadedAt == vector.Zero;
		bool travelled = vector.DistanceSqXZ(m_vLastLoadedAt, position) > SUPPLY_DELIVERY_THRESHOLD_SQ;

		if (firstUnload || vector.DistanceSqXZ(m_vLastUnloadedAt, position) <= SUPPLY_DELIVERY_THRESHOLD_SQ)
		{
			// Player is unloading at the same spot as before
			// In this case do nothing here
		}
		else if (!notLoadedYet && travelled)
		{
			// Player is unloading at a new spot
			// They previously loaded supplies somewhere else
			// Let them have the full reward
			m_fNoRewardSupplies = 0;
		}

		if (firstUnload && travelled)
			m_fNoRewardSupplies = 0;

		m_vLastUnloadedAt = position;

		// Player is unloading without loading first, no XP
		if (notLoadedYet)
			return;

		// Make sure XP is awarded only for supplies which did not originate at this spot
		// So players can't load and unload supplies at the same base and generate infinite XP
		float difference = amount - m_fNoRewardSupplies;
		m_fNoRewardSupplies -= amount;
		m_fNoRewardSupplies = Math.Max(m_fNoRewardSupplies, 0);

		if (difference <= 0)
			return;

		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (compXP)
			compXP.AwardXP(playerId, SCR_EXPRewards.SUPPLIES_DELIVERED, Math.Min(difference, amount) / FULL_SUPPLY_TRUCK_AMOUNT);
	}

	//------------------------------------------------------------------------------------------------
	void ResetSavedSupplies()
	{
		m_vLastLoadedAt = vector.Zero;
		m_vLastUnloadedAt = vector.Zero;
		m_fNoRewardSupplies = 0;
	}

	//------------------------------------------------------------------------------------------------
	// Init
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
		
		if (!m_PlayerController)
		{
			Print("SCR_CampaignNetworkComponent must be attached to PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (m_PlayerController.GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
			SCR_SpawnPointRequestUIComponent.SGetOnSpawnPointSelected().Insert(HandleRadioRespawnTimer);

		if (IsProxy())
			return;

		SCR_ResourcePlayerControllerInventoryComponent comp = SCR_ResourcePlayerControllerInventoryComponent.Cast(m_PlayerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

		if (comp)
			comp.GetOnPlayerInteraction().Insert(OnPlayerSuppliesInteraction);
	}
	
	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignNetworkComponent()
	{
		SCR_SpawnPointRequestUIComponent.SGetOnSpawnPointSelected().Remove(HandleRadioRespawnTimer);
	}
};
