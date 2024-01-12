 class SCR_AIDecoTestDoorIsOpen : DecoratorTestScripted
{
	private DoorComponent m_doorComponent;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		if (controlled)
		{	
			if (!m_doorComponent)
				m_doorComponent = DoorComponent.Cast(controlled.FindComponent(DoorComponent));
			if (!m_doorComponent)
				return false;
			return m_doorComponent.IsOpen();
		}
		return false;
	}
};
