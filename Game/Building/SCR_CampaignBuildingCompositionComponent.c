[EntityEditorProps(category: "Game/Building", description: "Component attached to compositions.")]
class SCR_CampaignBuildingCompositionComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingCompositionComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfigurationSpawn;

	[Attribute()]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfigurationDespawn;

	// Could be for example a base to which this composition belongs to.
	protected IEntity m_ProviderEntity;
	protected RplId m_RplCompId;
	private SCR_EditorModeEntity m_EditorModeEntity;
	protected int m_iBuilderId = INVALID_PLAYER_ID;
	protected int m_iCost;
	protected int m_iPrefabId;
	protected bool m_bInteractionLock;

	[RplProp(onRplName: "OnCompositionSpawned")]
	protected bool m_bCompositionIsSpawned;

	protected ref ScriptInvokerVoid m_OnBuilderSet;
	protected ref ScriptInvokerBool m_OnCompositionSpawned;

	protected static const int INVALID_PLAYER_ID = 0;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);

		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(owner.FindComponent(SCR_EditorLinkComponent));
		if (!linkComponent || !linkComponent.IsSpawningIgnored())
		{
			m_bInteractionLock = true;
			return;
		}

		if (IsProxy())
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return;

		ResourceName resName = editableEntity.GetPrefab();
		if (resName.IsEmpty())
			return;

		int prefabId = buildingManagerComponent.GetCompositionId(resName);
		if (prefabId == -1)
			return;

		SpawnCompositionLayout(prefabId, editableEntity);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnCompositionLayout(int prefabId, SCR_EditableEntityComponent entity)
	{
		if (!entity)
			return;

		Resource compositionOutlineResource = Resource.Load(GetOutlineToSpawn(entity));
		if (!compositionOutlineResource.IsValid())
			return;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Parent = entity.GetOwner();

		IEntity compositionLayout = GetGame().SpawnEntityPrefab(compositionOutlineResource, GetGame().GetWorld(), spawnParams);
		if (!compositionLayout)
			return;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(compositionLayout.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return;

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return;

		// If the spawned composition has a service component, mark service as "under construction"
		SCR_ServicePointComponent serviceComponent = SCR_ServicePointComponent.Cast(entity.GetOwner().FindComponent(SCR_ServicePointComponent));
		if (serviceComponent)
			serviceComponent.SetServiceState(SCR_EServicePointStatus.UNDER_CONSTRUCTION);

		layoutComponent.SetPrefabId(prefabId);
		compositionComponent.SetPrefabId(prefabId);
	}

	//------------------------------------------------------------------------------------------------
	//! Search for the outline that is assigned to this composition to be spawned.
	ResourceName GetOutlineToSpawn(notnull SCR_EditableEntityComponent entity)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return string.Empty;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return string.Empty;

		SCR_CampaignBuildingCompositionOutlineManager outlineManager = buildingManagerComponent.GetOutlineManager();
		if (!outlineManager)
			return string.Empty;

		return outlineManager.GetCompositionOutline(entity);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(owner.FindComponent(SCR_EditorLinkComponent));
		if (!linkComponent)
			return;

		SCR_EditableEntityComponent editableEnt = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEnt)
			return;

		SetCompositionCost(editableEnt);

		linkComponent.GetOnLinkedEntitiesSpawned().Insert(SetIsCompositionSpawned);
	}

	//------------------------------------------------------------------------------------------------
	// Set the destroy event to root entity, when all direct child are destructible
	void SetDestroyEvents()
	{
		if (SCR_EntityHelper.GetMainParent(GetOwner(), true) != GetOwner())
			return;

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			// The preview exist at this moment but will be deleted, skip it.
			SCR_BasePreviewEntity previewEnt = SCR_BasePreviewEntity.Cast(child);
			if (previewEnt)
			{
				child = child.GetSibling();
				continue;
			}

			// The layout of the composition wasn't deleted yet in this frame. Skip it.
			SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(child.FindComponent(SCR_CampaignBuildingLayoutComponent));
			if (layoutComponent)
			{
				child = child.GetSibling();
				continue;
			}

			// Look for all childs with destruction component among childs of root entity. If there is at least one entity without this component, we don't have to continue, as the root entity will never stay alone - so there isn't need to delete it.
			SCR_DestructionMultiPhaseComponent destructionComponent = SCR_DestructionMultiPhaseComponent.Cast(child.FindComponent(SCR_DestructionMultiPhaseComponent));
			if (!destructionComponent)
				return;

			child = child.GetSibling();
		}

		SCR_DestructionBaseComponent.GetOnDestructibleDestroyedInvoker().Insert(DestructibleEntityDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	// A destructible entity in composition was destroyed. Evaluate if there is any other left.
	void DestructibleEntityDestroyed(SCR_DestructionMultiPhaseComponent component)
	{
		// Calling one frame later as by the end of this one, the entity that triggered this can still exist.
		GetGame().GetCallqueue().CallLater(CheckExistingChild, 1, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if this entity doesn't have any child. If so, delete it.
	void CheckExistingChild()
	{
		if (SCR_EntityHelper.GetChildrenCount(GetOwner()) == 0)
			SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	// Called when the composition is spawned (gradual building)
	protected void SetIsCompositionSpawned()
	{
		m_bCompositionIsSpawned = true;
		Replication.BumpMe();

		if (m_iBuilderId != INVALID_PLAYER_ID)
			CompositionBuildSound();

		if (!IsProxy())
			SetDestroyEvents();

		if (m_OnCompositionSpawned)
			m_OnCompositionSpawned.Invoke(true);
	}

	//------------------------------------------------------------------------------------------------
	//! An event called on proxi when composition is spawned
	protected void OnCompositionSpawned()
	{
		CompositionBuildSound();

		if (m_OnCompositionSpawned)
			m_OnCompositionSpawned.Invoke(true);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnCompositionSpawned()
	{
		if (!m_OnCompositionSpawned)
			m_OnCompositionSpawned = new ScriptInvokerBool();

		return m_OnCompositionSpawned;
	}

	//------------------------------------------------------------------------------------------------
	// Gets the composition cost from editable entity component.
	protected void SetCompositionCost(notnull SCR_EditableEntityComponent editableEnt)
	{
		array<ref SCR_EntityBudgetValue> outBudgets = {};
		editableEnt.GetEntityBudgetCost(outBudgets, GetOwner());
		foreach (SCR_EntityBudgetValue budgetEnt : outBudgets)
		{
			if (budgetEnt.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
			{
				m_iCost = budgetEnt.GetBudgetValue();
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Set composition ID which is used to identify composition by building mode.
	void SetPrefabId(int prefabId)
	{
		m_iPrefabId = prefabId;
	}

	//------------------------------------------------------------------------------------------------
	int GetCompositionCost()
	{
		return m_iCost;
	}

	//------------------------------------------------------------------------------------------------
	int GetPrefabId()
	{
		return m_iPrefabId;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCompositionSpawned()
	{
		return m_bCompositionIsSpawned;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnBuilderSet()
	{
		if (!m_OnBuilderSet)
			m_OnBuilderSet = new ScriptInvokerVoid();

		return m_OnBuilderSet;
	}

	//------------------------------------------------------------------------------------------------
	void SetProviderEntity(IEntity newOwner)
	{
		m_ProviderEntity = newOwner;
		if (m_OnBuilderSet)
			m_OnBuilderSet.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetProviderEntity()
	{
		return m_ProviderEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove provider entity when the provider is not a base and the building mode was terminated.
	void RemoveProviderEntity()
	{
		m_ProviderEntity = null;
	}

	//------------------------------------------------------------------------------------------------
	void SetBuilderId(int id)
	{
		m_iBuilderId = id;
	}

	//------------------------------------------------------------------------------------------------
	int GetBuilderId()
	{
		return m_iBuilderId;
	}

	//------------------------------------------------------------------------------------------------
	// Set the lock of the composition. When set, the composition can't be moved or deleted from Free Roam mode anymore.
	void SetInteractionLock(bool lockState)
	{
		m_bInteractionLock = lockState;
		AfterLockChanged(lockState);
	}

	//------------------------------------------------------------------------------------------------
	void SetInteractionLockServer(bool lockState)
	{
		SetInteractionLock(lockState);
		Rpc(RpcDo_SetCompositionLock, lockState);
	}

	//------------------------------------------------------------------------------------------------
	bool IsInteractionLocked()
	{
		return m_bInteractionLock;
	}

	//------------------------------------------------------------------------------------------------
	// Run evaluation if the entity still pass the given filter.
	void AfterLockChanged(bool lockState)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!entity)
			return;

		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.ACTIVE);
		if (filter)
			filter.Validate(entity);

		// ignore the following code if the compositino was locked
		if (lockState)
			return;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return;

		SCR_CampaignBuildingTransformingEditorComponent transformComponent = SCR_CampaignBuildingTransformingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingTransformingEditorComponent));
		if (!transformComponent)
			return;

		transformComponent.ReleaseLastRejectedPivot();
	}

	//------------------------------------------------------------------------------------------------
	void SetProviderEntityServer(IEntity newOwner)
	{
		SetProviderEntity(newOwner);

		RplId id = RplId.Invalid();

		if (newOwner)
		{
			RplComponent comp = RplComponent.Cast(newOwner.FindComponent(RplComponent));
			if (comp)
				id = comp.Id();
		}

		Rpc(RpcDo_SetProviderEntity, id);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove provider entity when the provider is not a base and the building mode was terminated. Called on server from invoker
	void RemoveProviderEntityServer()
	{
		RemoveProviderEntity();
		if (m_EditorModeEntity)
			m_EditorModeEntity.GetOnClosedServer().Remove(RemoveProviderEntityServer);
		Rpc(RpcDo_RemoveProviderEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Set an event to remove a provider from composition component when the building mode is terminated.
	void SetClearProviderEvent(notnull SCR_EditorModeEntity ent)
	{
		m_EditorModeEntity = ent;
		ent.GetOnClosedServer().Insert(RemoveProviderEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a provider, loaded from RPL ID
	void SetProviderFromRplID()
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_RplCompId));
		if (!rplComp)
			return;

		SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Remove(SetProviderFromRplID);
		SetProviderEntity(rplComp.GetEntity());
	}

	//------------------------------------------------------------------------------------------------
	//! Play the sound when the composition is fully spawned.
	void CompositionBuildSound()
	{
		if (!m_AudioSourceConfigurationSpawn || !m_AudioSourceConfigurationSpawn.IsValid())
			return;

		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (soundManagerEntity)
			soundManagerEntity.CreateAndPlayAudioSource(GetOwner(), m_AudioSourceConfigurationSpawn);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return (rplComponent && rplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_RemoveProviderEntity()
	{
		RemoveProviderEntity();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetProviderEntity(RplId rplCompId)
	{
		IEntity newOwner = null;

		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (rplComp)
			newOwner = IEntity.Cast(rplComp.GetEntity());

		SetProviderEntity(newOwner);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetCompositionLock(bool lockState)
	{
		SetInteractionLock(lockState);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteBool(m_bInteractionLock);

		if (!m_ProviderEntity)
		{
			writer.WriteBool(0);
			return true;
		}

		RplComponent rplComp = RplComponent.Cast(m_ProviderEntity.FindComponent(RplComponent));
		if (!rplComp)
		{
			writer.WriteBool(0);
			return true;
		}

		writer.WriteBool(1);
		writer.WriteRplId(rplComp.Id());
		writer.WriteInt(m_iBuilderId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadBool(m_bInteractionLock);

		bool providerExist;
		reader.ReadBool(providerExist);
		if (!providerExist)
			return true;

		reader.ReadRplId(m_RplCompId);
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_RplCompId));
		if (!rplComp)
		{
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Insert(SetProviderFromRplID);
			return true;
		}

		reader.ReadInt(m_iBuilderId);
		SetProviderEntity(rplComp.GetEntity());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!owner || !m_AudioSourceConfigurationDespawn || !m_AudioSourceConfigurationDespawn.IsValid())
			return;

		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (soundManagerEntity)
			soundManagerEntity.CreateAndPlayAudioSource(owner, m_AudioSourceConfigurationDespawn);
	}
}
