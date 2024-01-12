//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingBuildUserAction : SCR_ScriptedUserAction
{
	protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;
	protected IEntity m_User;
	protected SCR_GadgetManagerComponent m_GadgetManager;
	
	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_LayoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingLayoutComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.AddBuildingValue(GetBuildingToolValue(pUserEntity), pOwnerEntity);
		ProcesXPreward();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		
		if (!ShouldPerformPerFrame())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			charController.TryUseItemOverrideParams(GetBuildingTool(pUserEntity), false, true, itemActionId, 1, 0, int.MAX, 0, 0, false, null);
		}

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
	override void OnConfirmed(IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
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
	//~ If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!LoopActionUpdate(timeSlice))
			return; 
		
		PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means player has a building tool ready (but also when he is running around with building tool hiden)
	override bool CanBeShownScript(IEntity user)
	{
		m_User = user;	
		return m_LayoutComponent && m_LayoutComponent.HasBuildingPreview();
	}
	
	//------------------------------------------------------------------------------------------------
	// The user action can be performed, when player has a building tool in hands.
	override bool CanBePerformedScript(IEntity user)
	{	
		if (!m_GadgetManager)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewGadgetManager);
			
			return false;
		}
		
		SCR_GadgetComponent gadgetComponent = m_GadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return false;
		
		
		return (gadgetComponent.GetMode() == EGadgetMode.IN_HAND);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stops player animation
	protected void CancelPlayerAnimation(IEntity pUserEntity)
	{
		if (!pUserEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		if (!pAnimationComponent)
			return;

		CharacterCommandHandlerComponent handlerComponent = pAnimationComponent.GetCommandHandler();
		if (!handlerComponent)
			return;

		handlerComponent.FinishItemUse();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_LayoutComponent || !m_User)
			return false;

		float progressPercentage = GetBuildingToolValue(m_User) / (m_LayoutComponent.GetToBuildValue() * 0.01);
		float buildPercentage = m_LayoutComponent.GetCurrentBuildValue() / (m_LayoutComponent.GetToBuildValue() * 0.01);

		if (progressPercentage != (int)progressPercentage)
			ActionNameParams[0] = progressPercentage.ToString(lenDec: 1);
		else
			ActionNameParams[0] = progressPercentage.ToString();

		if (buildPercentage != (int)buildPercentage)
			ActionNameParams[1] = buildPercentage.ToString(lenDec: 1);
		else
			ActionNameParams[1] = buildPercentage.ToString();

		return super.GetActionNameScript(outName);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets a new gadget manager. Controlled by an event when the controlled entity has changed.
	void SetNewGadgetManager(IEntity from, IEntity to)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
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
	// Gets the "building tool value" of building tool. It means how many points are added by one usage of the building tool
	int GetBuildingToolValue(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return 0;

		SCR_CampaignBuildingGadgetToolComponent gadgetComponent = SCR_CampaignBuildingGadgetToolComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return 0;

		return gadgetComponent.GetToolConstructionValue();
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcesXPreward()
	{		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_CampaignBuildingManagerComponent campaignBuildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		campaignBuildingManagerComponent.ProcesXPreward();
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
	void ~SCR_CampaignBuildingBuildUserAction()
	{
		CancelPlayerAnimation(m_User);
	}
}
