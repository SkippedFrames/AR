[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict editor component to handle building mode placing", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingEditorComponentClass : SCR_PlacingEditorComponentClass
{
	[Attribute(params: "et", desc: "List of prefabs that are ignored for clipping check.")]
	protected ref array<ResourceName> m_aClippingCheckIgnoredPrefabs;

	bool ContainPrefab(ResourceName res)
	{
		if (m_aClippingCheckIgnoredPrefabs.Contains(res))
			return true;

		return false;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingPlacingEditorComponent : SCR_PlacingEditorComponent
{
	[Attribute("{C7EE4C198B641E21}Sounds/Editor/BaseBuilding/Editor_BaseBuilding.acp", UIWidgets.ResourceNamePicker, "Sound project file to be used for building", "acp")]
	protected ResourceName m_sSoundFile;

	protected bool m_bCanBeCreated = true;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_FreeRoamBuildingClientTriggerEntity m_AreaTrigger;
	protected IEntity m_Provider;
	protected ECantBuildNotificationType m_eBlockingReason;
	protected SCR_EditablePreviewEntity m_PreviewEnt;

	//------------------------------------------------------------------------------------------------
	//! Return the base which belongs to a provider (if any)
	protected bool GetProviderBase(out SCR_MilitaryBaseComponent base)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return false;

		base = SCR_MilitaryBaseComponent.Cast(m_Provider.FindComponent(SCR_MilitaryBaseComponent));
		if (!base)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the preview of prefab to build is created.
	protected void OnPreviewCreated(SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		SCR_CampaignBuildingPlacingObstructionEditorComponent obstructionComponent = SCR_CampaignBuildingPlacingObstructionEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingObstructionEditorComponent, true, true));
		if (!obstructionComponent)
			return;

		obstructionComponent.OnPreviewCreated(previewEnt);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the initial state of the preview can / can't be created (based on the place where player place the preview into the world.)
	protected void SetInitialCanBeCreatedState(notnull SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		float distance = vector.DistanceSq(m_AreaTrigger.GetOrigin(), previewEnt.GetOrigin());
		if (distance < m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius())
			m_bCanBeCreated = true;

		m_bCanBeCreated = false;
		m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		return;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBeforeEntityCreatedServer(ResourceName prefab)
	{
		// Get SCR_CampaignBuildingManagerComponent (on Game mode) here and set temporary owner.
		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent || !m_CampaignBuildingComponent)
			return;

		buildingManagerComponent.SetTemporaryProvider(m_CampaignBuildingComponent.GetProviderEntity());
	}

	//------------------------------------------------------------------------------------------------
	override void OnEntityCreatedServer(array<SCR_EditableEntityComponent> entities)
	{
		if (!m_CampaignBuildingComponent)
			return;

		int count = entities.Count();
		for (int i = 0; i < count; i++)
		{
			IEntity entityOwner = entities[i].GetOwnerScripted();
			if (!entityOwner)
				continue;

			SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entityOwner.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (!compositionComponent)
				continue;

			SetProviderAndBuilder(compositionComponent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set the provider and builder entity to composition. Provider is the entity to which a composition belong to. For example a base, supply truck... Builder is a player who build it.
	protected void SetProviderAndBuilder(notnull SCR_CampaignBuildingCompositionComponent compositionComponent)
	{
		IEntity provider = m_CampaignBuildingComponent.GetProviderEntity();
		if (!provider)
			return;

		SCR_EditorManagerEntity managerEnt = GetManager();
			if (!managerEnt)
				return;

		int id = managerEnt.GetPlayerID();

		compositionComponent.SetBuilderId(id);
		compositionComponent.SetProviderEntity(provider);

		// Don't set up this hook if the provider is a base. We don't want to change the ownership here
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			return;

		SCR_EditorModeEntity ent = SCR_EditorModeEntity.Cast(GetOwner());
		compositionComponent.SetClearProviderEvent(ent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlaceEntityServer(int prefabID, SCR_EditableEntityComponent entity)
	{
		vector position = entity.GetOwner().GetOrigin();
		Rpc(PlaySoundEvent, position, m_sSoundFile);
		PlaySoundEvent(position, m_sSoundFile);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void PlaySoundEvent(vector pos, string soundEvent)
	{
		vector transform[4];
		transform[3] = pos;

		if (soundEvent && pos)
			AudioSystem.PlayEvent(soundEvent, SCR_SoundEvent.SOUND_BUILD, transform, new array<string>, new array<float>);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void CreateEntityServer(SCR_EditorPreviewParams params, RplId prefabID, int playerID, int entityIndex, bool isQueue, array<RplId> recipientIds)
	{
		// Cancel spawning of the composition instantly and spawn a composition layout instead.
		SCR_EditorLinkComponent.IgnoreSpawning(true);

		// Cancel a services registration to base - register once the final structure is erected.
		SCR_ServicePointComponent.SpawnAsOffline(true);
		
		super.CreateEntityServer(params, prefabID, playerID, entityIndex, isQueue, recipientIds);
	}

	//------------------------------------------------------------------------------------------------
	// Check if the preview is outisde of the building radius. if preview doesn't exist return false - preview doesn't exist on server and the same CanCreateEntity is used on server too.
	bool IsPreviewOutOfRange()
	{
		if (!m_AreaTrigger || !m_PreviewEnt)
			return false;

		return (vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), m_PreviewEnt.GetOrigin()) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius());
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
	override bool CanCreateEntity(out ENotification outNotification = -1, inout SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		SCR_CampaignBuildingPlacingObstructionEditorComponent obstructionComponent = SCR_CampaignBuildingPlacingObstructionEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingObstructionEditorComponent, true, true));
		if (!obstructionComponent)
		{
			outNotification = ENotification.EDITOR_PLACING_BLOCKED;
			return false;
		}

		if (obstructionComponent.IsPreviewOutOfRange(outNotification))
		{
			previewStateToShow = SCR_EPreviewState.BLOCKED;
			return false;
		}

		return obstructionComponent.CanCreate(outNotification, previewStateToShow);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		InitVariables();
		SetInitialEvent();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivateServer()
	{
		InitVariables();
		GetOnPlaceEntityServer().Insert(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		super.EOnEditorOpen();
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		super.EOnEditorOpenServer();
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	private void SetInitialEvent()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));
		if (!m_PreviewManager)
			return;

		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	private void InitVariables()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		m_Provider = m_CampaignBuildingComponent.GetProviderEntity();
		m_AreaTrigger = SCR_FreeRoamBuildingClientTriggerEntity.Cast(m_CampaignBuildingComponent.GetTrigger());
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDeactivateServer()
	{
		GetOnPlaceEntityServer().Remove(OnPlaceEntityServer);
	}
}

enum ECantBuildNotificationType
{
	OUT_OF_AREA = 0,
	BLOCKED = 1,
}
