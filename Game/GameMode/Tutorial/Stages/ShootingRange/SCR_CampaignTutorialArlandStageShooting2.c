[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting2 : SCR_BaseCampaignTutorialArlandStage
{
	protected bool m_bHasM16Mag;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_MAGAZINE_PICK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (storageManComp)	
		{
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		}
		
		IEntity ammobox = GetGame().GetWorld().FindEntityByName("Ammobox");
		
		if (ammobox)
			GetWaypoint().SetOrigin(ammobox.GetOrigin());
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));	
		m_TutorialComponent.SetWaypointMiscImage("AMMO", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bHasM16Mag;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
		
		if (!magComp)
			return false;
		 
		BaseMagazineWell baseMagwell;
		baseMagwell = magComp.GetMagazineWell();
		
		if (!baseMagwell)
			return false;
		
		MagazineWellStanag556 m16mag = MagazineWellStanag556.Cast(baseMagwell);
		
		if (!m16mag)	
			return false;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
		
		m_bHasM16Mag = true;
		return true;
	}
};
