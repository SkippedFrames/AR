class SCR_AIUnEquipItems : AITaskScripted
{
	private IEntity m_OwnerEntity;
	private CharacterControllerComponent controller;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "UnEquip on abort or alternatively on simulate" )]
	private bool m_bKeepRunningUntilAborted;
	private bool m_bHasAborted;

	// -----------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_OwnerEntity = owner.GetControlledEntity();
		if (!m_OwnerEntity)
			NodeError(this, owner, "Missing owner entity!");
	}

	// -----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_bKeepRunningUntilAborted)
		{
			UnEquip();
			return ENodeResult.SUCCESS;
		}

		return ENodeResult.RUNNING;
	}

	// -----------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bKeepRunningUntilAborted)
			UnEquip();
	}

	// -----------------------------------------------------------------------------------------------
	protected void UnEquip()
	{
		if (m_bHasAborted)
			return;
		
		m_bHasAborted = true;
		
		if (!m_OwnerEntity)
			return;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(m_OwnerEntity);
		if (gadgetManager && gadgetManager.GetHeldGadgetComponent())
		{
			gadgetManager.RemoveHeldGadget();
			return;
		}
		
		controller = CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(CharacterControllerComponent));
		if (controller)
			controller.RemoveGadgetFromHand();
	}

	// -----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_bKeepRunningUntilAborted)
			return "Item unequiped when this node is aborted";
		else 
			return "Item unequiped when node starts running";
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}

	// -----------------------------------------------------------------------------------------------
	override bool CanReturnRunning()
	{
		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Scripted Node: UnEquip any generic item or gadget that is currently being held by the character.";
	}
};
