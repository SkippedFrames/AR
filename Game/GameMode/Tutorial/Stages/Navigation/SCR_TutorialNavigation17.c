[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation17Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation17 : SCR_BaseCampaignTutorialArlandStage
{	
	bool m_bPositionFound;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapMarkersUI mapMarkersUI = SCR_MapMarkersUI.Cast(mapEnt.GetMapUIComponent(SCR_MapMarkersUI));
		if (!mapMarkersUI)
			return;
		
		mapMarkersUI.GetOnCustomMarkerPlaced().Remove(MarkerPlaced);
		mapMarkersUI.GetOnCustomMarkerPlaced().Insert(MarkerPlaced);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	void MarkerPlaced(int posX, int posY, bool isPublic)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("WP_GREENHOUSE");
		if (!ent)
			return;
		
		vector position = ent.GetOrigin();
		
		position[0] = position[0] - posX;
		position[2] = position[2] - posY;
		
		if (position[0] > 50 || position[0] < -50)
			return;
			
		if (position[2] > 50 || position[2] < -50)
			return;
		
		m_bPositionFound = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bPositionFound;
	}
};
