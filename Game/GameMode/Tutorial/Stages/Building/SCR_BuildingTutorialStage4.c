[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage4 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.FindBuiltComposition(m_TutorialComponent.VEHICLE_MAINTENANCE_PREFAB))
		{
			m_TutorialComponent.SetStage(SCR_ECampaignTutorialArlandStage.CONFLICT_BUILDING_PLACE_SERVICE);
			m_TutorialComponent.SetActiveStage(m_TutorialComponent.GetActiveStage()-2);
			SCR_EntityHelper.DeleteEntityAndChildren(this);
		}
		
		return !IsBuildingModeOpen();
	}
};
