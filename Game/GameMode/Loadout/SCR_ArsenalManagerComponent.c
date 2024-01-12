[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ArsenalManagerComponentClass: SCR_BaseGameModeComponentClass
{
	static override array<typename> Requires(IEntityComponentSource src)
	{
		array<typename> requires = new array<typename>;
		
		requires.Insert(SerializerInventoryStorageManagerComponent);
		
		return requires;
	}
};

//~ Scriptinvokers
void SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged(int playerId, bool hasValidLoadout);
typedef func SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged;

class SCR_ArsenalManagerComponent : SCR_BaseGameModeComponent
{
	protected static SCR_ArsenalManagerComponent s_Instance;
	
	[Attribute("{27F28CF7C6698FF8}Configs/Arsenal/ArsenalSaveTypeInfoHolder.conf", desc: "Holds a list of save types than can be used for arsenals. Any new arsenal save type should be added to the config to allow it to be set by Editor", params: "conf class=SCR_ArsenalSaveTypeInfoHolder")]
	protected ResourceName m_sArsenalSaveTypeInfoHolder;
	
	[Attribute("{183361B6DA2C304F}Configs/Arsenal/ArsenalLoadoutSaveBlacklists.conf", desc: "This is server only, A blacklist of entities that are not allowed to be saved at arsenals if the blacklist the item is in is enabled. Can be null.",params: "conf class=SCR_LoadoutSaveBlackListHolder")]
	protected ResourceName m_sLoadoutSaveBlackListHolder;
	
	//=== Authority
	protected ref map<int, string> m_aPlayerLoadouts = new map<int, string>();
	
	//=== Broadcast
	protected ref ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> m_OnPlayerLoadoutUpdated = new ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged>();
	
	protected ref SCR_ArsenalSaveTypeInfoHolder m_ArsenalSaveTypeInfoHolder;
	protected ref SCR_LoadoutSaveBlackListHolder m_LoadoutSaveBlackListHolder;
	
	protected bool m_bLocalPlayerLoadoutAvailable;
	ref SCR_PlayerLoadoutData m_bLocalPlayerLoadoutData;
	
	//------------------------------------------------------------------------------------------------
	static bool GetArsenalManager(out SCR_ArsenalManagerComponent arsenalManager)
	{
		arsenalManager = s_Instance;
		return s_Instance != null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get Save type info holder
	*/
	SCR_ArsenalSaveTypeInfoHolder GetArsenalSaveTypeInfoHolder()
	{
		return m_ArsenalSaveTypeInfoHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get Loadout save blacklist holder
	*/
	SCR_LoadoutSaveBlackListHolder GetLoadoutSaveBlackListHolder()
	{
		return m_LoadoutSaveBlackListHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerChangedFaction(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliation, Faction faction)
	{
		if (!m_aPlayerLoadouts.Contains(playerId))
			return;
		
		//~ Remove loadout from map on server and inform players
		m_aPlayerLoadouts.Remove(playerId);
		DoPlayerClearHasLoadout(playerId);
		Rpc(DoPlayerClearHasLoadout, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Update BlackList active states (Server only)
	\param orderedActiveStates An ordered list of all blacklists and their new active state
	\param playerChangedSetting optional Player ID that changed the setting to send notification to game masters that the setting was changed
	*/
	void SetLoadoutBlackListActiveStates(notnull array<bool> orderedActiveStates, bool clearExistingLoadouts, int editorPlayerIdClearedLoadout = -1)
	{
		if (!GetGameMode().IsMaster() || !m_LoadoutSaveBlackListHolder)
			return;
		
		if (editorPlayerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_LOADOUT_SAVE_BLACKLIST, editorPlayerIdClearedLoadout);
		
		m_LoadoutSaveBlackListHolder.SetOrderedBlackListsActive(orderedActiveStates);
		
		//~ Clear existing loadouts
		if (clearExistingLoadouts)
		{
			//~ Clear local loadout
			Rpc(RPC_OnPlayerLoadoutCleared, editorPlayerIdClearedLoadout);
			RPC_OnPlayerLoadoutCleared(editorPlayerIdClearedLoadout);
			
			//~ Clear existing loadouts on server
			m_aPlayerLoadouts.Clear();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_OnPlayerLoadoutCleared(int playerIdClearedLoadout)
	{
		if (!m_bLocalPlayerLoadoutAvailable)
			return;
		
		//~ No local loadout availible anymore
		m_bLocalPlayerLoadoutAvailable = false;
		
		//~ Player loadout was cleared. This can happen when respawn menu is open and needs to be refreshed
		m_OnPlayerLoadoutUpdated.Invoke(SCR_PlayerController.GetLocalPlayerId(), false);
		
		//~ Send notification to player to inform them their loadout was cleared
		if (playerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED_BY_EDITOR);
		else 
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetLocalPlayerLoadoutAvailable()
	{
		return m_bLocalPlayerLoadoutAvailable;
	}
	
	//------------------------------------------------------------------------------------------------
	 ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> GetOnLoadoutUpdated()
	{
		return m_OnPlayerLoadoutUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	//=== Authority
	bool GetPlayerArsenalLoadout(int playerId, out string jsonCharacter)
	{
		return m_aPlayerLoadouts.Find(playerId, jsonCharacter) && jsonCharacter != string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanSaveLoadout(int playerId, notnull GameEntity characterEntity, FactionAffiliationComponent playerFactionAffiliation, SCR_ArsenalComponent arsenalComponent, bool sendNotificationOnFailed)
	{
		//~ Always allow saving if no arsenal component or no restrictions
		if (!arsenalComponent || (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.NO_RESTRICTIONS && (!m_LoadoutSaveBlackListHolder || m_LoadoutSaveBlackListHolder.GetBlackListsCount() == 0)))
			return true;
		
		SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(characterEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!characterInventory)
		{
			Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but character has no inventory manager!", LogLevel.ERROR);
			
			if (sendNotificationOnFailed)
				SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
			
			return false;
		}
			
		SCR_EntityCatalog itemCatalog;
		SCR_ArsenalInventoryStorageManagerComponent arsenalInventory;
		
		//~ Check either faction or arsenal inventory if item is in it
		if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY || arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.IN_ARSENAL_ITEMS_ONLY)
		{			
			if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY)
			{				
				//~ Player has no faction affiliation comp
				if (!playerFactionAffiliation)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no faction affiliation component, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
				//~ Player has no faction
				if (!playerFaction)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no SCR_Faction, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				itemCatalog = playerFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
				if (!itemCatalog)
				{
					Print(string.Format("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player faction '%1' has no ITEM catalog!", playerFaction.GetFactionKey()), LogLevel.ERROR);
					
					if (sendNotificationOnFailed)
						SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
					
					return false;
				}
			}
			else 
			{
				arsenalInventory = SCR_ArsenalInventoryStorageManagerComponent.Cast(arsenalComponent.GetArsenalInventoryComponent());
				if (!arsenalInventory)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' and arsenal check type is 'IN_ARSENAL_ITEMS_ONLY' but arsenal has no SCR_ArsenalInventoryStorageManagerComponent, so saving is simply allowed", LogLevel.WARNING);
					return true;
				}
			}
		}

		set<string> checkedEntities = new set<string>();
		array<IEntity> allPlayerItems = {};
		characterInventory.GetAllRootItems(allPlayerItems);
		
		EntityPrefabData prefabData;
		RplComponent itemRplComponent;
		string resourceName;
		
		int invalidItemCount = 0;
		
		foreach (IEntity item : allPlayerItems)
		{			
			prefabData = item.GetPrefabData();
			
			//~ Ignore if no prefab data
			if (!prefabData)
				continue;
			
			//~ Ignore if no prefab resource name
			resourceName = prefabData.GetPrefabName();
			if (resourceName.IsEmpty())
				continue;
			
			//~ Do not check the same item twice
			if (checkedEntities.Contains(resourceName))
				continue;
				
			checkedEntities.Insert(resourceName);
			
			//~ Check if item is blacklisted if there is a blacklist
			if (m_LoadoutSaveBlackListHolder && m_LoadoutSaveBlackListHolder.GetBlackListsCount() != 0)
			{
				//~ Item is blackListed so do not allow saving
				if (m_LoadoutSaveBlackListHolder.IsPrefabBlacklisted(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_ITEM_BLACKLISTED, itemRplComponent.Id());
					}
				
					invalidItemCount++;
					continue;
				}
			}
			
			//~ Check arsenal inventory
			if (arsenalInventory)
			{
				//~ Not in inventory of arsenal so cannot save
				if (!arsenalInventory.IsPrefabInArsenalStorage(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_IN_ARSENAL, itemRplComponent.Id());
					}
						
					invalidItemCount++;
					continue;
				}
			}
			//~ Check item catalog
			else if (itemCatalog)
			{
				//~ Item not in faction catalog so cannot save
				if (!itemCatalog.GetEntryWithPrefab(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_FACTION, itemRplComponent.Id());
					}
						
					invalidItemCount++;
					continue;
				}
			}
		}
		
		//~ Saving failed because of invalid items
		if (invalidItemCount > 0 && sendNotificationOnFailed)
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED_INVALID_ITEMS, invalidItemCount);
		
		return invalidItemCount == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//=== Authority
	void SetPlayerArsenalLoadout(int playerId, GameEntity characterEntity, SCR_ArsenalComponent arsenalComponent)
	{
		//~ If Not Authority return
		if (!GetGameMode().IsMaster())
			return;
		
		if (playerId <= 0)
			return;
		
		if (!characterEntity)
		{
			DoSetPlayerLoadout(playerId, string.Empty, characterEntity);
			return;
		}
		
		SCR_PlayerController clientPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!clientPlayerController || clientPlayerController.IsPossessing())
			return;
		
		string factionKey = SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTIONKEY_NONE;
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			factionKey = factionAffiliation.GetAffiliatedFaction().GetFactionKey();
		
		if (!CanSaveLoadout(playerId, characterEntity, factionAffiliation, arsenalComponent, true))
			return;			
		
		SCR_JsonSaveContext context = new SCR_JsonSaveContext();
		if (!context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTION_KEY, factionKey) || !context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY, characterEntity))
			return;
		
		DoSetPlayerLoadout(playerId, context.ExportToString(), characterEntity);
	}
	
	protected SCR_PlayerLoadoutData GetPlayerLoadoutData(GameEntity characterEntity)
	{
		SCR_PlayerLoadoutData loadoutData();
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(characterEntity.FindComponent(EquipedLoadoutStorageComponent));
		if (loadoutStorage)
		{
			int slotsCount = loadoutStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = loadoutStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty())
					continue;
				
				SCR_ClothingLoadoutData clothingData();
				clothingData.SlotIdx = i;
				clothingData.ClothingPrefab = prefabName;
				
				loadoutData.Clothings.Insert(clothingData);
			}
		}
		
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(characterEntity.FindComponent(EquipedWeaponStorageComponent));
		if (weaponStorage)
		{
			int slotsCount = weaponStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = weaponStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty())
					continue;
				
				SCR_WeaponLoadoutData weaponData();
				weaponData.SlotIdx = i;
				weaponData.WeaponPrefab = prefabName;
				
				loadoutData.Weapons.Insert(weaponData);
			}
		}
		
		return loadoutData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DoSetPlayerLoadout(int playerId, string loadoutString, GameEntity characterEntity)
	{
		bool loadoutValid = !loadoutString.IsEmpty();
		bool loadoutChanged = loadoutValid && loadoutString != m_aPlayerLoadouts.Get(playerId);
		
		m_aPlayerLoadouts.Set(playerId, loadoutString);
		
		if (loadoutValid && loadoutChanged)
		{
			SCR_PlayerLoadoutData loadoutData = GetPlayerLoadoutData(characterEntity);
			DoSendPlayerLoadout(playerId, loadoutData);
			Rpc(DoSendPlayerLoadout, playerId, loadoutData);
		}
		
		DoSetPlayerHasLoadout(playerId, loadoutValid, loadoutChanged);
		Rpc(DoSetPlayerHasLoadout, playerId, loadoutValid, loadoutChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSetPlayerHasLoadout(int playerId, bool loadoutValid, bool loadoutChanged)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (m_bLocalPlayerLoadoutAvailable != loadoutValid || loadoutChanged)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED);
			else 
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_NOT_SAVED_UNCHANGED);
			
			m_bLocalPlayerLoadoutAvailable = loadoutValid;
		}
		m_OnPlayerLoadoutUpdated.Invoke(playerId, loadoutValid);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSendPlayerLoadout(int playerId, SCR_PlayerLoadoutData loadoutData)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			m_bLocalPlayerLoadoutData = loadoutData;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoPlayerClearHasLoadout(int playerId)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
			m_bLocalPlayerLoadoutAvailable = false;

		m_OnPlayerLoadoutUpdated.Invoke(playerId, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Init item black list (Server only). Call one frame later to make sure catalogs are initalized
		if (m_LoadoutSaveBlackListHolder)
			GetGame().GetCallqueue().CallLater(m_LoadoutSaveBlackListHolder.Init);
		
		if (!GetGameMode().IsMaster())
			return; 
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionChanged_S().Insert(OnPlayerChangedFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		if (SCR_Global.IsEditMode() || s_Instance)
			return;

		s_Instance = SCR_ArsenalManagerComponent.Cast(GetGameMode().FindComponent(SCR_ArsenalManagerComponent));
		SetEventMask(owner, EntityEvent.INIT);
		
		//~ Get config for saveType holder. This is used by arsenals in the world
		m_ArsenalSaveTypeInfoHolder = SCR_ArsenalSaveTypeInfoHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sArsenalSaveTypeInfoHolder, true));
		if (!m_ArsenalSaveTypeInfoHolder)
			Print("'SCR_ArsenalManagerComponent' failed to load Arsenal Save Type Holder config!", LogLevel.ERROR);
		
		//~ Get loadout save blacklist config, Can be null (Server Only)
		if (GetGameMode().IsMaster())
			m_LoadoutSaveBlackListHolder = SCR_LoadoutSaveBlackListHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sLoadoutSaveBlackListHolder, false));		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !GetGameMode() || !GetGameMode().IsMaster())
			return; 
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionChanged_S().Remove(OnPlayerChangedFaction);
	}
};
