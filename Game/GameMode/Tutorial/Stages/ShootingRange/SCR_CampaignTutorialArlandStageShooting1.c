[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WEAPON_PICK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		IEntity gun = GetGame().GetWorld().FindEntityByName("M16");
				
		if (gun)
			GetWaypoint().SetOrigin(gun.GetOrigin());
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));	
		m_TutorialComponent.SetWaypointMiscImage("GUNLOWER", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (weaponManager)
			if (weaponManager.GetCurrent())
				return (weaponManager.GetCurrent().GetWeaponType() == EWeaponType.WT_RIFLE);
			else
				return false;
		else
			return true;
	}
};
