class SCR_CampaignBuildingDisassemblyUserAction : ScriptedUserAction
{	
	protected SCR_CampaignBuildingCompositionComponent m_CompositionComponent;
	protected SCR_EditableEntityComponent m_EditableEntity;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected ref array<SCR_EditableVehicleComponent> m_EditableVehicle = {};
	protected bool m_bCompositionSpawned;
	protected bool m_bTurretCollected;
	// Temporary solution how to prevent disassembly of HQ in Conflict.
	protected bool m_DoNotDisassemble;
	protected IEntity m_RootEntity;
	protected IEntity m_User;
	
	protected SCR_GadgetManagerComponent m_GadgetManager;

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RootEntity = pOwnerEntity.GetRootParent();
				
		m_CompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(m_RootEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		m_EditableEntity = SCR_EditableEntityComponent.Cast(m_RootEntity.FindComponent(SCR_EditableEntityComponent));
		
		SetEditorManager();
		
		if (m_CompositionComponent && GetOwner() == GetOwner().GetRootParent())
			m_CompositionComponent.GetOnCompositionSpawned().Insert(OnCompositionSpawned);
		
		// Temporary solution how to prevent disassembly of HQ in Conflict.
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!campaignGameMode || !m_EditableEntity)
			return;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(m_EditableEntity.GetInfo(GetOwner()));
		if (!editableEntityUIInfo)
			return;
		
		array<EEditableEntityLabel> entityLabels = {};
		editableEntityUIInfo.GetEntityLabels(entityLabels);
		if (entityLabels.Contains(EEditableEntityLabel.SERVICE_HQ))
			m_DoNotDisassemble = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			charController.TryUseItemOverrideParams(GetBuildingTool(pUserEntity), false, true, itemActionId, 2, 0, int.MAX, 0, 0, false, null);
		}
		
		m_User = pUserEntity;
		
		super.OnActionStart(pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			CharacterCommandHandlerComponent cmdHandler = CharacterCommandHandlerComponent.Cast(pAnimationComponent.GetCommandHandler());
			if (cmdHandler)
				cmdHandler.FinishItemUse();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CancelPlayerAnimation(IEntity entity)
	{
		if (!entity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			CharacterCommandHandlerComponent cmdHandler = CharacterCommandHandlerComponent.Cast(pAnimationComponent.GetCommandHandler());
			cmdHandler.FinishItemUse();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{			
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent || !m_RootEntity)
			return;
		
		if (HasCompositionService())
			TryToSendNotification(pOwnerEntity, pUserEntity, networkComponent);
		
		networkComponent.DeleteCompositionByUserAction(m_RootEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	void TryToSendNotification(IEntity pOwnerEntity, IEntity pUserEntity, notnull SCR_CampaignBuildingNetworkComponent networkComponent)
	{
		if (!m_CompositionComponent)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
			
		IEntity provider = m_CompositionComponent.GetProviderEntity();
		if (!provider)
			return;
			
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
			
		array<SCR_MilitaryBaseComponent> bases = {};
		providerComponent.GetBases(bases);
		if (bases.IsEmpty())
			return;
			
		int callsign = bases[0].GetCallsign();
		if (callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return;
		
		Faction baseFaction = bases[0].GetFaction();
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
		if (playerFaction != baseFaction)
			return;

		networkComponent.SendDeleteNotification(m_EditableEntity.GetOwner(), playerId, callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the editable entity component has a service trait set.
	bool HasCompositionService()
	{
		if (!m_EditableEntity)
			return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(m_EditableEntity.GetInfo());
		return editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.TRAIT_SERVICE);
	}
		
	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means player has a building tool.
	override bool CanBeShownScript(IEntity user)
	{									
		if (m_DoNotDisassemble)
			return false;
		
		if (!m_GadgetManager)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewGadgetManager);
			
			return false;
		}
					
		if (!SCR_CampaignBuildingGadgetToolComponent.Cast(m_GadgetManager.GetHeldGadgetComponent()))
			return false;
		
		// The user action is on entity with composition component, show it if the composition is spawned.
		if (GetOwner() == GetOwner().GetRootParent())
			return m_bCompositionSpawned;
		
		return GetOwner().FindComponent(SCR_CampaignBuildingLayoutComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets a new gadget manager. Controlled by an event when the controlled entity has changed.
	void SetNewGadgetManager(IEntity from, IEntity to)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{ 			
		if (m_bCompositionSpawned && !m_bTurretCollected)
			GetAllTurretsInComposition();
		
		// If the editor manager doesn't exists, try to get one and set as for an example when connecting to a server with build compositions, the editor manager doesn't exist when the user action inicialized.
		if (!m_EditorManager)
			SetEditorManager();
		
		if (!m_EditorManager || m_EditorManager.IsOpened())
			return false;
				
		array<CompartmentAccessComponent> crewCompartmentAccess = {};
		
		foreach (SCR_EditableVehicleComponent editableVehicle : m_EditableVehicle)
		{
			if (editableVehicle.GetCrew(crewCompartmentAccess, false) != 0)
				return false;
		}
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (!gadgetManager)
			return false;
		
		SCR_GadgetComponent gadgetComponent = gadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return false;
		
		return (gadgetComponent.GetMode() == EGadgetMode.IN_HAND);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		m_EditorManager = core.GetEditorManager();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the hierarchy of the composition for any turret and make a list of them.
	void GetAllTurretsInComposition()
	{
		m_bTurretCollected = true;
		set<SCR_EditableEntityComponent> editableEntities = new set<SCR_EditableEntityComponent>();
		m_EditableEntity.GetChildren(editableEntities);
				
		foreach (SCR_EditableEntityComponent ent : editableEntities)
		{
			SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(ent);
			if (editableVehicle)
				m_EditableVehicle.Insert(editableVehicle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get call once the composition is fully spawned
	void OnCompositionSpawned(bool compositionSpawned)
	{
		m_bCompositionSpawned = compositionSpawned;
		if (m_CompositionComponent)
			m_CompositionComponent.GetOnCompositionSpawned().Remove(OnCompositionSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get building tool entity
	IEntity GetBuildingTool(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return null;
		
		return gadgetManager.GetHeldGadget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingDisassemblyUserAction()
	{
		CancelPlayerAnimation(m_User);
	}
}
