[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical4: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		
		RegisterWaypoint("WP_MEDICAL_2");
	}
};
