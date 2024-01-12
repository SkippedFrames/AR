[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting13 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WEAPON_PICK_MG");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		IEntity gun = GetGame().GetWorld().FindEntityByName("MG_M249");
				
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
				return (weaponManager.GetCurrent().GetWeaponType() == EWeaponType.WT_MACHINEGUN);
			else
				return false;
		else
			return true;
	}
};
