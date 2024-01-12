[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage95Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage95 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_GETINHMW");
		m_fWaypointCompletionRadius = 10;
		
		string hintString = "First, approach the vehicle on the Driver side and get in. You should be able to see an interaction on the door." + CreateString("Get in vehicle","CharacterAction") + CreateString("#AR-KeybindEditor_MultiSelection","SelectAction");
	
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
