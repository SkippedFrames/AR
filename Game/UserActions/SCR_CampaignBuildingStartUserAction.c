//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingStartUserAction : ScriptedUserAction
{
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected SCR_CampaignBuildingProviderComponent m_ProviderComponent;
	protected Physics m_ProviderPhysics;
	protected RplComponent m_RplComponent;
	protected DamageManagerComponent m_DamageManager;
	protected SCR_CompartmentAccessComponent m_CompartmentAccess;
	
	protected const int PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ = 0;
	
	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		InitializeSuppliesComponent();
		
		m_DamageManager = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
		
		if (GetGame().GetPlayerController())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeSuppliesComponent()
	{
		IEntity mainParent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		m_ProviderPhysics = mainParent.GetPhysics();
		
		// Check if the supplies component is at the owner (supply truck)
		m_ProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingProviderComponent));
		if (m_ProviderComponent)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		m_ProviderComponent.RequestBuildingMode(playerID, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_ProviderComponent || GetUserRank(user) >= m_ProviderComponent.GetAccessRank())
			return true;
		
		FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComp)
			return false;
		
		string rankName;
		SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
		if (faction)
			rankName = faction.GetRankName(m_ProviderComponent.GetAccessRank());
			
		SetCannotPerformReason(rankName);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetUserRank(notnull IEntity user)
	{		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;
		
		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ProviderComponent)
			return false;
		
		if (!m_CompartmentAccess)
		{
			m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(user.FindComponent(SCR_CompartmentAccessComponent));
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewComparmentComponent);
			
			return false;
		}
		
		if (m_CompartmentAccess.IsGettingIn())
			return false;
		
		// Don't quit if the providerPhysics doesn't exist. The provider might not have one.
		if (m_ProviderPhysics)	
		{
			vector velocity = m_ProviderPhysics.GetVelocity();
			if ((velocity.LengthSq()) > PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ)
				return false;
		}
		
		// Don't show the action if player is within any vehicle.		
		ChimeraCharacter char = ChimeraCharacter.Cast(user);
		if (!char || char.IsInVehicle())
			return false;
		
		// No action if the provider is destroyed
		if (m_DamageManager)
		{
			if (m_DamageManager.GetState() == EDamageState.DESTROYED)
				return false;
		}
				
		return m_ProviderComponent.IsPlayerFactionSame(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{	
		if (!m_ResourceComponent)
			m_ResourceComponent = m_ProviderComponent.GetResourceComponent();
		
		if (!m_ResourceComponent 
		||	!m_ResourceConsumer && !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES, m_ResourceConsumer))
			return false;
		
		if (!m_ResourceInventoryPlayerComponentRplId || !m_ResourceInventoryPlayerComponentRplId.IsValid())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		
		if (m_ResourceSubscriptionHandleConsumer)
			m_ResourceSubscriptionHandleConsumer.Poke();
		else
			m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		ActionNameParams[0] = string.ToString(m_ResourceConsumer.GetAggregatedResourceValue());
		outName = ("#AR-Campaign_Action_ShowBuildPreview-UC");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Sets a new compartment component. Controlled by an event when the controlled entity has changed.
	void SetNewComparmentComponent(IEntity from, IEntity to)
	{
		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
	}
	
};
