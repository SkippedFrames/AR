class SCR_CampaignGrabRadioUserAction : ScriptedUserAction
{
	protected IEntity m_Entity;
	protected SCR_ChimeraCharacter m_Player;
	protected SCR_CampaignMobileAssemblyComponent m_AssemblyComponent;
	protected DamageManagerComponent m_DamageManagerComponent;
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!pOwnerEntity)
			return;
		
		m_Entity = pOwnerEntity;
		m_AssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignMobileAssemblyComponent));
		IEntity truck = Vehicle.Cast(m_Entity.GetParent());
		
		if (!truck)
		{
			BaseWorld world = GetGame().GetWorld();
			
			if (world)
				world.QueryEntitiesBySphere(pOwnerEntity.GetOrigin(), SCR_CampaignReconfigureHQRadioUserAction.MAX_BASE_DISTANCE, IsParentBase, null, EQueryEntitiesFlags.ALL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	DamageManagerComponent GetDamageManagerComponent()
	{
		if (m_DamageManagerComponent)
			return m_DamageManagerComponent;
		
		IEntity truck = Vehicle.Cast(m_Entity.GetParent());
		
		if (!truck)
			return null;
		
		m_DamageManagerComponent = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
		
		return m_DamageManagerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsParentBase(IEntity ent)
	{
		m_Base = SCR_CampaignMilitaryBaseComponent.Cast(ent.FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		return (m_Base == null);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		if (!m_Player)
			return false;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(m_Player.GetFaction());
		
		if (!faction)
			return false;
		
		outName = "#AR-Campaign_Action_RequestRadio-UC";
		ActionNameParams[0] = (campaign.GetMaxRespawnRadios() - faction.GetActiveRespawnRadios()).ToString();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		if (campaign.GetMaxRespawnRadios() <= 0)
			return false;
		
		m_Player = SCR_ChimeraCharacter.Cast(user);
		
		if (!m_Player)
			return false;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(m_Player.GetFaction());
		
		if (!faction)
			return false;
		
		if (m_AssemblyComponent && faction != m_AssemblyComponent.GetParentFaction())
			return false;
		
		if (GetDamageManagerComponent() && GetDamageManagerComponent().GetState() == EDamageState.DESTROYED)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(m_Player.GetFaction());
		
		if (faction.GetActiveRespawnRadios() >= campaign.GetMaxRespawnRadios())
			return false;
		
		if (m_Base && faction != m_Base.GetFaction())
		{
			SetCannotPerformReason("#AR-Campaign_Action_WrongBase-UC");
			return false;
		}
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(user.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return false;
		
		IEntity backpack = loadoutStorage.GetClothFromArea(LoadoutBackpackArea);
		
		if (backpack)
		{
			BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(backpack.FindComponent(BaseLoadoutClothComponent));
				
			if (loadoutCloth && loadoutCloth.GetAreaType().IsInherited(LoadoutBackpackArea) && backpack.FindComponent(SCR_RadioComponent))
				SetCannotPerformReason("#AR-Campaign_Action_RespawnRadioEquipped-UC");
			else
				SetCannotPerformReason("#AR-Campaign_Action_BackpackEquipped-UC");
			
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.AddRadio();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};
