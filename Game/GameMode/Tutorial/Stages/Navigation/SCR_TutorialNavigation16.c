[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation16 : SCR_BaseCampaignTutorialArlandStage
{	
	bool m_bPositionFound;
	int m_iPlacedMarkers;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapMarkersUI mapMarkersUI = SCR_MapMarkersUI.Cast(mapEnt.GetMapUIComponent(SCR_MapMarkersUI));
		if (!mapMarkersUI)
			return;
		
		mapMarkersUI.GetOnCustomMarkerPlaced().Insert(MarkerPlaced);
		
		UpdateHint("beginning")
	}
	//------------------------------------------------------------------------------------------------
	void UpdateHint(string text)
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), text));
	}
	
	//------------------------------------------------------------------------------------------------
	void MarkerPlaced(int posX, int posY, bool isPublic)
	{
		vector position = m_Player.GetOrigin();
		
		position[0] = position[0] - posX;
		position[2] = position[2] - posY;
		
		m_iPlacedMarkers++;
		
		if (m_iPlacedMarkers == 5)
		{
			UpdateHint("5");
			m_TutorialComponent.ShowMapDescriptor("LighthousePos", true);
		}
		
		if (m_iPlacedMarkers == 10)
		{
			UpdateHint("10");
			m_TutorialComponent.ShowMapDescriptor("ChurchPos", true);
		}
		
		if (m_iPlacedMarkers == 15)
		{
			UpdateHint("15");
			m_TutorialComponent.ShowMapDescriptor("TowerPos", true);
		}
		
		if (m_iPlacedMarkers == 20)
		{
			UpdateHint("20");
			m_TutorialComponent.ShowMapDescriptor("NavigationPos", true);
			
			m_fDelay = 7;
			m_bPositionFound = true;
		}
				
		if (position[0] > 150 || position[0] < -150)
			return;
			
		if (position[2] > 150 || position[2] < -150)
			return;
		
		m_bPositionFound = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		return m_bPositionFound;
	}
};
