[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage27Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage27 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_0");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition3" + CreateString("#AR-Keybind_Movement", "CarThrust", "CarBrake") + CreateString("#AR-Keybind_Steer", "CarSteering"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
