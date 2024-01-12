[EntityEditorProps(category: "GameScripted/Campaign", description: "Component to be used with Conflict barrack compositions, handling unit spawning.", color: "0 0 255 255")]
class SCR_CampaignBarracksComponentClass: SCR_BarracksComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBarracksComponent: SCR_BarracksComponent
{
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected int m_iDeadOnDespawn;
	protected bool m_bInitialized;
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	[Attribute("25", UIWidgets.Auto, "How much it cost to spawn one soldier?", category: "Campaign")]
	protected int m_iUnitCost;
	
	bool IsInitialized()
	{
		return m_bInitialized;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSuppliesComponent(SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SuppliesComponent = supplyComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DespawnGroup(SCR_AIBarracksGroup grp)
	{
		SCR_AIGroup group = grp.GetGroup();
		if (group)
			m_iDeadOnDespawn = grp.GetGroupSize() - group.GetAgentsCount();
		
		super.DespawnGroup(grp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SpawnUnit(ResourceName unitResource, SCR_AIBarracksGroup barrackGrp)
	{
		if (!m_SuppliesComponent)
			return;
		
		/*if (m_SuppliesComponent.GetSupplies() < m_iUnitCost)
			return;
		
		if (!barrackGrp.GetIsDespawned())
			m_SuppliesComponent.AddSupplies(-m_iUnitCost);
		else if (m_iDeadOnDespawn > 0)
		{
			m_iDeadOnDespawn--;
			m_SuppliesComponent.AddSupplies(-m_iUnitCost);
		}*/
		
		super.SpawnUnit(unitResource, barrackGrp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGroupSpawned(notnull SCR_AIGroup grp)
	{
		super.OnGroupSpawned(grp);
		
		SCR_AIGroupUtilityComponent comp = SCR_AIGroupUtilityComponent.Cast(grp.FindComponent(SCR_AIGroupUtilityComponent));
		
		if (!comp)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		
		if (!baseManager)	
			return;
		
		ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered> onEnemyDetected = comp.m_Perception.GetOnEnemyDetectedFiltered();
		
		if (!onEnemyDetected)
			return;
		
		onEnemyDetected.Insert(baseManager.OnEnemyDetectedByDefenders);
		
		if (m_Base)
			m_Base.SetDefendersGroup(grp);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeBarrack()
	{
		if (IsProxy())
			return;
		
		if (!m_SuppliesComponent)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign || campaign.IsTutorial())
			return;
		
		//Initializes handling of units. If there is not enough supplies, it will use invoker to check when there is change in them
		m_bInitialized = true;
		/*if (m_SuppliesComponent.GetSupplies() > m_iUnitCost)
			InitHandler();
		else
			m_SuppliesComponent.m_OnSuppliesChanged.Insert(InitHandler);*/
	}
	
	//------------------------------------------------------------------------------------------------
	void StopHandler()
	{
		m_bInitialized = false;
		GetGame().GetCallqueue().Remove(HandleGroups);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitHandler()
	{
		if (m_SuppliesComponent)
		{
			GetGame().GetCallqueue().CallLater(HandleGroups, GROUP_HANDLER_DELAY, true);
			m_SuppliesComponent.m_OnSuppliesChanged.Remove(InitHandler);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBase(SCR_CampaignMilitaryBaseComponent base)
	{
		m_Base = base;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
	}
}
