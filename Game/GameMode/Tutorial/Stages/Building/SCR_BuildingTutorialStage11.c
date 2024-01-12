[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage11Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage11 : SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_ResourceEncapsulator m_SupplyEncapsulator;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		RegisterWaypoint(supplyTruck);
		m_bCheckWaypoint = false;
		
		SetupSupplyConsumer();

		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSupplyConsumer()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		
		if (!supplyTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(supplyTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		EntitySlotInfo slotInfo = slotManager.GetSlotByName("Cargo");
		if (!slotInfo)
			return;
		
		IEntity cargo = slotInfo.GetAttachedEntity();
		if (!cargo)
			return;
		
		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.FindResourceComponent(cargo);
		if (!resourceComp)
			return;
		
		m_SupplyEncapsulator = resourceComp.GetEncapsulator(EResourceType.SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyEncapsulator)
			return false;
		
		return m_SupplyEncapsulator.GetAggregatedResourceValue() == m_SupplyEncapsulator.GetAggregatedMaxResourceValue();
	}
};
