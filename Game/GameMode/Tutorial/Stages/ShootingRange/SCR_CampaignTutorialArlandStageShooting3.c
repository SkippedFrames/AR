[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting3 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponManager)
			return false;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrent();
		
		if (!weaponComponent)
			return false;
		
		BaseMagazineComponent mag = weaponComponent.GetCurrentMagazine();
		
		if (mag)
			return (mag.GetAmmoCount() != 0);
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};
