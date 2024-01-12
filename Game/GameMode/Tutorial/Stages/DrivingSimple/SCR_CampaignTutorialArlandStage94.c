[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage94Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage94 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 9;
		string hintString = "Welcome to the Driving school. Here you will learn the fundementals of interacting with vehicles.";
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", m_fDuration, isTimerVisible:true);
	}
};
