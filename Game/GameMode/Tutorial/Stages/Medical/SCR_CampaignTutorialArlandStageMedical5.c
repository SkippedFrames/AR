[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical5Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical5: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		
		RegisterWaypoint("WP_MEDICAL_3");
	}
	
};
