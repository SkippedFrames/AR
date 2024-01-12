[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting17Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting17 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WEAPON_PICK_SNIPER");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		IEntity gun = GetGame().GetWorld().FindEntityByName("Sniper");
				
		if (gun)
			GetWaypoint().SetOrigin(gun.GetOrigin());
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));	
		m_TutorialComponent.SetWaypointMiscImage("GUNREADY", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (weaponManager)
			if (weaponManager.GetCurrent())
				return (weaponManager.GetCurrent().GetWeaponType() == EWeaponType.WT_SNIPERRIFLE);
			else
				return false;
		else
			return true;
	}
};
