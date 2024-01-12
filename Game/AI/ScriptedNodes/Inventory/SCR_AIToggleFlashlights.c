class SCR_AIToggleFlashlights : AITaskScripted
{
	[Attribute("true", UIWidgets.CheckBox, "Set flashlights active or inactive?")]
	protected bool m_bEnable;
	
	protected SCR_GadgetManagerComponent m_GadgetManager;
	
	//------------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Sets all flashlights attached to vest enabled or disabled. Returns failure if no suitable flashlight found.";
	}
	
	
	//------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(owner.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return string.Format("Flashlights Enabled: %1", m_bEnable);
	}
	
	//------------------------------------------------------------------------------------------------------
	static ENodeResult ToggleFlashlights(notnull SCR_GadgetManagerComponent gadgetManager, bool newState)
	{
		array<SCR_GadgetComponent> gadgets = gadgetManager.GetGadgetsByType(EGadgetType.FLASHLIGHT);
		
		if (!gadgets)
			return ENodeResult.FAIL;
		
		if (gadgets.IsEmpty())
			return ENodeResult.FAIL;
		
		bool success = false;
		foreach (SCR_GadgetComponent gadget : gadgets)
		{
			InventoryItemComponent invComp = InventoryItemComponent.Cast(gadget.GetOwner().FindComponent(InventoryItemComponent));
			
			if (!invComp)
				continue;
			
			EquipmentStorageSlot slot = EquipmentStorageSlot.Cast(invComp.GetParentSlot());
			
			// Don't toggle gadgets not in slot
			if (!slot)
				continue;
			
			bool occluded = slot.IsOccluded();
			
			gadget.ToggleActive(newState && !occluded);
			success |= true;
		}
			
		if (success)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------------
	static protected override bool VisibleInPalette()
	{
		return false;
	}
};



class SCR_AIToggleFlashlightsOnSimulate : SCR_AIToggleFlashlights
{
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_GadgetManager)
			return ENodeResult.FAIL;
		
		return ToggleFlashlights(m_GadgetManager, m_bEnable);
	};
	
	//------------------------------------------------------------------------------------------------------
	override static protected bool VisibleInPalette()
	{
		return true;
	}
};



class SCR_AIToggleFlashlightsOnAbort : SCR_AIToggleFlashlights
{
	//------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_GadgetManager)
			return;
		
		ToggleFlashlights(m_GadgetManager, m_bEnable);
	}
	
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------------
	override static protected bool VisibleInPalette()
	{
		return true;
	}
};
