enum EResourcePlayerInteractionType
{
	VEHICLE_LOAD,
	VEHICLE_UNLOAD,
	STORAGE,
	INVENTORY_SPLIT
}

//------------------------------------------------------------------------------------------------
void ScriptInvoker_ResourceOnPlayerInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue);
typedef func ScriptInvokerActiveWidgetInteractionFunc;
typedef ScriptInvokerBase<ScriptInvokerActiveWidgetInteractionFunc> ScriptInvokerResourceOnPlayerInteraction;

[ComponentEditorProps(category: "GameScripted/Resources", description: "")]
class SCR_ResourcePlayerControllerInventoryComponentClass : ScriptComponentClass
{	
}

typedef SCR_ResourceSystemSubscriptionHandle<SCR_ResourcePlayerControllerInventoryComponent> SCR_SCR_ResourcePlayerControllerInventoryComponentSubscriptionHandle;

class SCR_ResourcePlayerControllerInventoryComponent : ScriptComponent
{
	protected ref ScriptInvokerResourceOnPlayerInteraction m_OnPlayerInteractionInvoker;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerResourceOnPlayerInteraction GetOnPlayerInteraction()
	{
		if (!m_OnPlayerInteractionInvoker)
			m_OnPlayerInteractionInvoker = new ScriptInvokerResourceOnPlayerInteraction();
		
		return m_OnPlayerInteractionInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceActor TryGetGenerationActor(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType, out float currentResourceValue, out float maxResourceValue)
	{
		SCR_ResourceContainer containerFrom = resourceComponent.GetContainer(resourceType);
		
		if (containerFrom)
		{
			currentResourceValue	= containerFrom.GetResourceValue();
			maxResourceValue		= containerFrom.GetMaxResourceValue();
			
			return containerFrom;
		}
		
		SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		
		if (generator)
		{
			currentResourceValue	= generator.GetAggregatedResourceValue();
			maxResourceValue		= generator.GetAggregatedMaxResourceValue();
			
			return generator;
		}
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		
		if (generator)
		{
			currentResourceValue	= generator.GetAggregatedResourceValue();
			maxResourceValue		= generator.GetAggregatedMaxResourceValue();
			
			return generator;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceActor TryGetConsumptionActor(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType, out float currentResourceValue, out float maxResourceValue)
	{
		SCR_ResourceContainer containerFrom = resourceComponent.GetContainer(resourceType);
		
		if (containerFrom)
		{
			currentResourceValue	= containerFrom.GetResourceValue();
			maxResourceValue		= containerFrom.GetMaxResourceValue();
			
			return containerFrom;
		}
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		
		if (consumer)
		{
			currentResourceValue	= consumer.GetAggregatedResourceValue();
			maxResourceValue		= consumer.GetAggregatedMaxResourceValue();
			
			return consumer;
		}
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		
		if (consumer)
		{
			currentResourceValue	= consumer.GetAggregatedResourceValue();
			maxResourceValue		= consumer.GetAggregatedMaxResourceValue();
			
			return consumer;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TryPerformResourceConsumption(notnull SCR_ResourceActor actor, EResourceType resourceType, float resourceValue, bool ignoreOnEmptyBehavior = false)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
		{
			EResourceContainerOnEmptyBehavior emptyBehavior = container.GetOnEmptyBehavior();
			SCR_ResourceEncapsulator encapsulator = container.GetResourceEncapsulator();
			
			if (ignoreOnEmptyBehavior)
				container.SetOnEmptyBehavior(EResourceContainerOnEmptyBehavior.NONE);
			
			if (encapsulator)
				encapsulator.RequestConsumtion(resourceValue);
			else
				container.DecreaseResourceValue(resourceValue);
			
			if (ignoreOnEmptyBehavior)
				container.SetOnEmptyBehavior(emptyBehavior);
			
			return true;
		}
		
		SCR_ResourceConsumer consumer = SCR_ResourceConsumer.Cast(actor);
		
		if (consumer)
		{
			consumer.RequestConsumtion(resourceValue);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TryPerformResourceGeneration(notnull SCR_ResourceActor actor, EResourceType resourceType, float resourceValue)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
		{
			SCR_ResourceEncapsulator encapsulator = container.GetResourceEncapsulator();
			
			if (encapsulator)
				encapsulator.RequestGeneration(resourceValue);
			else
				container.IncreaseResourceValue(resourceValue);
			
			return true;
		}
		
		SCR_ResourceGenerator generator = SCR_ResourceGenerator.Cast(actor);
		
		if (generator)
		{
			generator.RequestGeneration(resourceValue);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSubscription(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		Rpc(RpcAsk_RequestSubscription, resourceComponentRplId, interactorType.ToString(), resourceType, resourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestUnsubscription(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		Rpc(RpcAsk_RequestUnsubscription, resourceComponentRplId, interactorType.ToString(), resourceType, resourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestSubscription(RplId resourceComponentRplId, string interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		if (!resourceComponentRplId.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		
		if (!resourceComponent)
			return;

		SCR_ResourceInteractor interactor;
				
		if (interactorType == "SCR_ResourceGenerator")
			interactor = resourceComponent.GetGenerator(resourceIdentifier, resourceType);
		else if (interactorType == "SCR_ResourceConsumer")
			interactor = resourceComponent.GetConsumer(resourceIdentifier, resourceType);
		else
			return;
		
		GetGame().GetResourceSystemSubscriptionManager().SubscribeListener(Replication.FindId(this), interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestUnsubscription(RplId resourceComponentRplId, string interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		if (!resourceComponentRplId.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		
		if (!resourceComponent)
			return;

		SCR_ResourceInteractor interactor;
		
		if (interactorType == "SCR_ResourceGenerator")
			interactor = resourceComponent.GetGenerator(resourceIdentifier, resourceType);
		else if (interactorType == "SCR_ResourceConsumer")
			interactor = resourceComponent.GetConsumer(resourceIdentifier, resourceType);
		else
			return;
		
		GetGame().GetResourceSystemSubscriptionManager().UnsubscribeListener(Replication.FindId(this), interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcAsk_OnPlayerInteraction(EResourcePlayerInteractionType interactionType, RplId rplIdResourceComponentFrom, RplId rplIdResourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (!rplIdResourceComponentFrom.IsValid())
			return;
		
		PlayerController playerController = PlayerController.Cast(GetOwner());
		
		if (!playerController)
			return;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentFrom));
		
		if (!resourceComponentFrom)
			return;
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentTo));
		
		OnPlayerInteraction(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ArsenalRequestItem(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, ResourceName resourceNameItem, EResourceType resourceType)
	{
		if (!rplIdInventoryManager.IsValid())
			return;
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(rplIdInventoryManager));
		
		if (!inventoryManagerComponent)
			return;
		
		
		if (!rplIdStorageComponent.IsValid())
			return;
		
		BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(Replication.FindItem(rplIdStorageComponent));
		
		if (!storageComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
		if (!consumer)
			return;
		
		float resourceCost = 0;
		
		//~ Get item cost
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (entityCatalogManager)
		{
			IEntity resourcesOwner = resourceComponent.GetOwner();
			if (!resourcesOwner)
				return;
			
			SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(resourcesOwner.FindComponent(SCR_ArsenalComponent));
			SCR_Faction faction;
			if (arsenalComponent)
				faction = arsenalComponent.GetAssignedFaction();
			
			SCR_EntityCatalogEntry entry;
			
			if (faction)
				 entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resourceNameItem, faction);
			else 
				entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, resourceNameItem);
			
			if (entry)
			{
				SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
				if (data)
					resourceCost = data.GetSupplyCost();
			}
		}
		
		SCR_ResourceConsumtionResponse response = consumer.RequestConsumtion(resourceCost * consumer.GetBuyMultiplier());
		
		if (response.GetReason() == EResourceReason.SUFFICIENT)
			inventoryManagerComponent.TrySpawnPrefabToStorage(resourceNameItem, storageComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ArsenalRefundItem(RplId rplIdResourceComponent, RplId rplIdInventoryItem, EResourceType resourceType)
	{
		if (!rplIdInventoryItem.IsValid())
			return;
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(Replication.FindItem(rplIdInventoryItem));
		
		if (!inventoryItemComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		SCR_ResourceGenerator generator	= resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, resourceType);
		if (!generator)
			return;
		
		IEntity inventoryItemEntity	= inventoryItemComponent.GetOwner();
		if (!inventoryItemEntity)
			return;
		
		float resourceCost = 0;
		
		//~ Get item cost
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (entityCatalogManager)
		{
			IEntity resourcesOwner = resourceComponent.GetOwner();
			if (!resourcesOwner)
				return;
			
			SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(resourcesOwner.FindComponent(SCR_ArsenalComponent));
			SCR_Faction faction;
			if (arsenalComponent)
				faction = arsenalComponent.GetAssignedFaction();
			
			SCR_EntityCatalogEntry entry;
			
			ResourceName resourceNameItem = inventoryItemEntity.GetPrefabData().GetPrefabName();
			
			if (faction)
				 entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resourceNameItem, faction);
			else 
				entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, resourceNameItem);
			
			if (!entry)
				return;

			SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
			if (data)
				resourceCost = data.GetSupplyCost();
		}
		
		SCR_ResourceGenerationResponse response = generator.RequestAvailability(resourceCost * generator.GetResourceMultiplier());
		if (response.GetReason() != EResourceReason.SUFFICIENT)
			return;
		
		IEntity parentEntity = inventoryItemEntity.GetParent();
		SCR_InventoryStorageManagerComponent inventoryManagerComponent;
		
		if (parentEntity)
			inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(parentEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (inventoryManagerComponent && !inventoryManagerComponent.TryDeleteItem(inventoryItemEntity))
			return;
		else if (!inventoryManagerComponent)
			RplComponent.DeleteRplEntity(inventoryItemEntity, false);
		
		generator.RequestGeneration(resourceCost * generator.GetResourceMultiplier());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_MergeContainerWithContainer(RplId rplIdFrom, RplId rplIdTo, EResourceType resourceType)
	{
		if (!rplIdFrom.IsValid() || !rplIdTo.IsValid())
			return;
		
		SCR_ResourceComponent componentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdFrom));
		
		if (!componentFrom)
			return;
		
		SCR_ResourceComponent componentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdTo));
		
		if (!componentTo)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(componentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorTo = TryGetGenerationActor(componentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		float resourceUsed = Math.Min(resourceValueCurrentFrom, resourceValueMaxTo - resourceValueCurrentTo);
		
		if(TryPerformResourceConsumption(actorFrom, resourceType, resourceUsed) && TryPerformResourceGeneration(actorTo, resourceType, resourceUsed))
			OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_MergeContainerWithContainerPartial(RplId rplIdFrom, RplId rplIdTo, EResourceType resourceType, float requestedResources)
	{
		if (!rplIdFrom.IsValid() || !rplIdTo.IsValid())
			return;
		
		SCR_ResourceComponent componentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdFrom));
		
		if (!componentFrom)
			return;
		
		SCR_ResourceComponent componentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdTo));
		
		if (!componentTo)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(componentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorTo = TryGetGenerationActor(componentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		float resourceUsed = Math.Min(Math.Min(resourceValueCurrentFrom, resourceValueMaxTo - resourceValueCurrentTo), requestedResources);
		
		if(TryPerformResourceConsumption(actorFrom, resourceType, resourceUsed) && TryPerformResourceGeneration(actorTo, resourceType, resourceUsed))
			OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_CreatePhysicalContainerWithContainer(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, EResourceType resourceType, float requestedResources)
	{
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		
		if (!resourceComponentFrom)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(resourceComponentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorConsumptionTo, actorGenerationTo;
		
		if (!rplIdInventoryManager.IsValid())
		{
			SCR_EntityCatalog resourceContainerCatalog	= SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
			array<SCR_EntityCatalogEntry> entries		= {};
			array<SCR_BaseEntityCatalogData> data		= {};
			SCR_ResourceContainerItemData datum;
			float resourceUsed;
			int selectedEntryIdx;
			
			resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
			
			for (selectedEntryIdx = data.Count() - 1; selectedEntryIdx >= 0; --selectedEntryIdx)
			{
				datum			= SCR_ResourceContainerItemData.Cast(data[selectedEntryIdx]);
				resourceUsed	= datum.GetMaxResourceValue();
				
				if (resourceUsed >= requestedResources)
					break;
			}
			
			if (selectedEntryIdx < 0)
				selectedEntryIdx = 0;
			
			resourceUsed = Math.Min(resourceUsed, requestedResources);
			
			if (!TryPerformResourceConsumption(actorFrom, resourceType, resourceUsed))
				return;
			
			RandomGenerator randGenerator	= new RandomGenerator();
			vector center					= resourceComponentFrom.GetOwner().GetOrigin();
			vector position					= vector.Up * center[1] + randGenerator.GenerateRandomPointInRadius(0.0, 2.5, center);
			TraceParam param				= new TraceParam();
			param.Start						= position + "0.0 10.0 0.0";
			param.End						= position;
			param.Flags						= TraceFlags.WORLD | TraceFlags.ENTS;
			param.LayerMask					= EPhysicsLayerDefs.Projectile;
			
			float traced = GetGame().GetWorld().TraceMove(param, null);
			
			if (traced < 1)
				position = (param.End - param.Start) * traced + param.Start;
			
			EntitySpawnParams spawnParams				= new EntitySpawnParams();
			spawnParams.TransformMode					= ETransformMode.WORLD;
			spawnParams.Transform[3]					= position;
			IEntity newStorageEntity					= GetGame().SpawnEntityPrefab(Resource.Load(entries[selectedEntryIdx].GetPrefab()), GetGame().GetWorld(), spawnParams);
			SCR_ResourceComponent resourceComponentTo	= SCR_ResourceComponent.FindResourceComponent(newStorageEntity);
			actorConsumptionTo							= TryGetConsumptionActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
			actorGenerationTo							= TryGetGenerationActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
			
			TryPerformResourceConsumption(actorConsumptionTo, resourceType, resourceValueCurrentTo, true);
			
			if (TryPerformResourceGeneration(actorGenerationTo, resourceType, resourceUsed))
			{
				OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, resourceComponentFrom, resourceComponentTo, resourceType, resourceUsed);
				
				return;
			}
			
			delete newStorageEntity;
			
			return;
		}
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(rplIdInventoryManager));
		
		if (!inventoryManagerComponent)
			return;
		
		if (!rplIdStorageComponent.IsValid())
			return;
		
		BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(Replication.FindItem(rplIdStorageComponent));
		
		if (!storageComponent)
			return;
		
		IEntity localEntity							= null;
		IEntity resourcesOwner						= resourceComponentFrom.GetOwner();
		SCR_EntityCatalogEntry selectedEntry		= null;
		int selectedEntryIdx						= -1;
		SCR_EntityCatalog resourceContainerCatalog	= SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
		array<SCR_EntityCatalogEntry> entries		= {};
		array<SCR_BaseEntityCatalogData> data		= {};
		float maxResourceValueTo					= 0.0;
		
		resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
		
		foreach (int idx, SCR_EntityCatalogEntry entry: entries)
		{
			localEntity = GetGame().SpawnEntityPrefabLocal(Resource.Load(entry.GetPrefab()));
			
			if (inventoryManagerComponent.CanInsertItemInStorage(localEntity, storageComponent))
			{
				selectedEntry		= entry;
				selectedEntryIdx	= idx;
				
				delete localEntity;
				
				break;
			}
			
			delete localEntity;
		}
		
		if (!selectedEntry)
			return;
		
		SCR_ResourceContainerItemData datum		= SCR_ResourceContainerItemData.Cast(data[selectedEntryIdx]);
		float maxStoredResources				= Math.Min(resourceValueCurrentFrom, datum.GetMaxResourceValue());
		float resourceUsed						= Math.Min(requestedResources, maxStoredResources);
		
		if (!TryPerformResourceConsumption(actorFrom, resourceType, resourceUsed))
			return;
		
		EntitySpawnParams spawnParams	= new EntitySpawnParams();
		spawnParams.TransformMode		= ETransformMode.WORLD;
		
		inventoryManagerComponent.GetOwner().GetTransform(spawnParams.Transform);
		
		IEntity newStorageEntity = GetGame().SpawnEntityPrefab(Resource.Load(selectedEntry.GetPrefab()), GetGame().GetWorld(), spawnParams);
		
		if (!newStorageEntity)
			return;
		
		SCR_ResourceComponent resourceComponentTo	= SCR_ResourceComponent.FindResourceComponent(newStorageEntity);
		actorConsumptionTo							= TryGetConsumptionActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		actorGenerationTo							= TryGetGenerationActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		TryPerformResourceConsumption(actorConsumptionTo, resourceType, resourceValueCurrentTo, true);
		
		if (!TryPerformResourceGeneration(actorGenerationTo, resourceType, resourceUsed))
		{
			delete newStorageEntity;
			
			return;
		}
		
		inventoryManagerComponent.TryInsertItemInStorage(newStorageEntity, storageComponent);
		OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, resourceComponentFrom, resourceComponentTo, resourceType, resourceUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerInteraction(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		IEntity owner = GetOwner();
		PlayerController playerController = PlayerController.Cast(owner);
		
		if (!playerController)
			return;
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!playerController)
			return;
		
		if (rplComponent && !rplComponent.IsProxy() && !rplComponent.IsOwner())
			Rpc(RpcAsk_OnPlayerInteraction, interactionType, Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), resourceType, resourceValue);
		
		GetOnPlayerInteraction().Invoke(interactionType, owner, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
	}
	
	/*!
	Called when Entity is being to be destroyed (deleted) or component to be deleted (see Game::DeleteScriptComponent).
	\param owner Entity which owns the component
	*/
	override event protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GetGame().GetResourceSystemSubscriptionManager().UnsubscribeListenerCompletely(Replication.FindId(this));
	}
}
