[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_TutorialComponent.SpawnAsset("Navigation_car", "{5674FAEB9AB7BDD0}Prefabs/Vehicles/Wheeled/M998/M998.et");
	}
};
