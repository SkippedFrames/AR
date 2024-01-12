[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage96Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage96 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 5;
		m_fDuration = 15;
		
		string hintString = "Look around the vehicle's interiour. Notice that there are two buttons to the left of the steering wheel, one being the starter, and other being the lights. You can turn on the car this way." + CreateString("#AR-Keybind_Movement", "CarThrust", "CarBrake") + CreateString("#AR-Keybind_Steer", "CarSteering");
	
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", m_fDuration, isTimerVisible:true);
	}
};
