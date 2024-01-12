[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage3 : SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CampaignMilitaryBaseComponent m_BaseComp;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILD_SERVICE");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_fConditionCheckPeriod = 1;
		
		SCR_HintManagerComponent.GetInstance().GetOnHintShow().Remove(CheckGMHint);
		SCR_HintManagerComponent.GetInstance().GetOnHintShow().Insert(CheckGMHint);
		
		OverrideGMHint();
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
		
		IEntity baseEnt = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		if (!baseEnt)
			return;
		
		m_BaseComp = SCR_CampaignMilitaryBaseComponent.Cast(baseEnt.FindComponent(SCR_CampaignMilitaryBaseComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!IsBuildingModeOpen())
		{
			m_TutorialComponent.SetStage(SCR_ECampaignTutorialArlandStage.CONFLICT_BUILDING_ENTER_BUILDING);
			m_TutorialComponent.SetActiveStage(m_TutorialComponent.GetActiveStage()-1);
			SCR_EntityHelper.DeleteEntityAndChildren(this);
		}
		
		if (m_BaseComp && m_BaseComp.GetSupplies() < SCR_BuildingTutorialStage2.DESIRED_SUPPLIES)
			m_BaseComp.AddSupplies(SCR_BuildingTutorialStage2.DESIRED_SUPPLIES - m_BaseComp.GetSupplies());
		
		return m_TutorialComponent.FindBuiltComposition("{543C31BC05F032B6}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_VehicleMaintenance_S_US_01.et");
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckGMHint(SCR_HintUIInfo info, bool isSilent)
	{
		if (!info)
			return;
		
		if (info.GetType() != EHint.UNDEFINED)
		{
			OverrideGMHint();
			GetGame().GetCallqueue().CallLater(OverrideGMHint, 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OverrideGMHint()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingTutorialStage3()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		
		if (SCR_HintManagerComponent.GetInstance())
			SCR_HintManagerComponent.GetInstance().GetOnHintShow().Remove(CheckGMHint);
	}
};
