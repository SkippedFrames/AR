[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_ResupplySupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
};

class SCR_ResupplySupportStationComponent : SCR_BaseSupportStationComponent
{
	[Attribute("10", desc: "Fallback item supply cost. If for some reason the item that was supplied had no cost or could not be found then the fallback cost is used",  category: "Resupply Support Station", params: "1 inf 1")]
	protected int m_iFallbackItemSupplyCost;
	
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		m_EntityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		super.DelayedInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool InitValidSetup()
	{
		if (!m_EntityCatalogManager)
		{
			Print("'SCR_ResupplySupportStationComponent' needs a entity catalog manager!", LogLevel.ERROR);
			return false;
		}
		
		//~ Resupply ammo does not support range as only players can resupply themselves or others at the moment
		if (UsesRange())
		{
			Print("'SCR_ResupplySupportStationComponent' does not support range. Make sure m_fRange is set to -1", LogLevel.ERROR);
			return false;
		}
		
		return super.InitValidSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyCost)
	{
		if (!SCR_BaseResupplySupportStationAction.Cast(action))
		{
			Debug.Error2("SCR_ResupplySupportStationComponent", "'IsValid' fails as the resupply support station is executed with a non SCR_BaseResupplySupportStationAction action! This is will break the support station!");
			return false;
		}
		
		return super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyCost);
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.RESUPPLY_AMMO;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyCostAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!IsUsingSupplies())
			return 0;
		
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_EntityCatalogEntry catalogEntry = m_EntityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, resupplyAction.GetItemToResupply(), GetFaction());
		if (!catalogEntry)
		{
			Print("'SCR_ResupplySupportStationComponent' could not find SCR_EntityCatalogEntry for '" + resupplyAction.GetItemToResupply() + "' so will use fallback cost!", LogLevel.WARNING);
			return Math.ClampInt(m_iFallbackItemSupplyCost + m_iBaseSupplyCostOnUse, 0, int.MAX);
		}
		
		//~ Could not find arsenal data use fallback cost
		SCR_ArsenalItem arsenalData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!arsenalData)
		{
			Print("'SCR_ResupplySupportStationComponent' could not find SCR_ArsenalItem '" + resupplyAction.GetItemToResupply() + "' so will use fallback cost!", LogLevel.WARNING);
			return Math.ClampInt(m_iFallbackItemSupplyCost + m_iBaseSupplyCostOnUse, 0, int.MAX);
		}
		
		return Math.ClampInt(arsenalData.GetSupplyCost() + m_iBaseSupplyCostOnUse, 0, int.MAX);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent inventoryManager = resupplyAction.GetTargetInventory();
		if (!inventoryManager)
			return;
		
		//~ Consume supplies
		if (IsUsingSupplies())
		{
			if (!OnConsumeSuppliesServer(GetSupplyCostAction(actionOwner, actionUser, action)))
				return;
		}
		
		map<ResourceName, int> itemsToResupply = new map<ResourceName, int>;
		itemsToResupply.Insert(resupplyAction.GetItemToResupply(), 1);
		inventoryManager.ResupplyMagazines(itemsToResupply);
		
		super.OnExecutedServer(actionOwner, actionUser, action);
	}	
	
	//------------------------------------------------------------------------------------------------
	//~ Called by OnExecuteBroadcast and is executed both on server and on client
	//~ playerId can be -1 if the user was not a player
	protected override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		if (!actionUser || !actionOwner)
			return;
		
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent targetInventory = resupplyAction.GetTargetInventory();
		if (!targetInventory)
			return;
		
		//~ Player resupplied themselves
		if (targetInventory.GetOwner() == actionUser)
		{
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionOwner);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionUser);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse());
			}
		}
		//~ Player resupplied other	
		else 
		{			
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionUser, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionUser);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionOwner);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
				{
					RplId ownerId;
					RplId userId;
					FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse(), userId);
				}
			}
		}				
	}
};
