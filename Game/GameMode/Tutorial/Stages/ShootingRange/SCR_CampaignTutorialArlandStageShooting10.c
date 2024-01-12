[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting10 : SCR_BaseCampaignTutorialArlandStage
{
	protected bool m_bHasGrenade;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_GRENADE_PICK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (storageManComp)	
		{
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		}
		
		IEntity ammobox = GetGame().GetWorld().FindEntityByName("Ammobox3");
		
		if (ammobox)
			GetWaypoint().SetOrigin(ammobox.GetOrigin());
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		m_TutorialComponent.SetWaypointMiscImage("AMMO", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bHasGrenade;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		WeaponComponent wepComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
		
		if (!wepComp)
			return false;
		 
		if (wepComp.GetWeaponType() == EWeaponType.WT_FRAGGRENADE)
			m_bHasGrenade = true;
		
		return true;
	}
};
