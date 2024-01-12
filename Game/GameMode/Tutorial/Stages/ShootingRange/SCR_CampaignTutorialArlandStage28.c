[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage28Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage28 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_0");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Turrets" + CreateString("#AR-Editor_CommandAction_AIWaypoint_GetInNearest_Name","CharacterAction") , duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterInVehicle(ECompartmentType.Turret);
	}
};
