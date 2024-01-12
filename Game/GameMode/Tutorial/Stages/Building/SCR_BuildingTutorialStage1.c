[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		IEntity base = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		if (!base)
			return;

		m_TutorialComponent.SpawnAsset("BuildingSupplyTruck", "{3F2AA823B6C65E1E}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_MERDC.et");
		m_TutorialComponent.SpawnAsset("Barricade", "{4885DEA01D687DB3}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_Bunker_S_USSR_01.et");
		m_TutorialComponent.ResetStage_CampaignBuilding();
	}
	
	
};
