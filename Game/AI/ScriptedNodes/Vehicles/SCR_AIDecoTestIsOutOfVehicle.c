class SCR_AIDecoTestIsOutOfVehicle : DecoratorTestScripted
{
	CompartmentAccessComponent compAccComp;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIGroup gr = AIGroup.Cast(agent);
		if (gr)
		{
			array<AIAgent> agents = new array<AIAgent>();
			gr.GetAgents(agents);
			
			foreach (AIAgent ag : agents)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(ag.GetControlledEntity());
				if (char)
				{
					if (char.IsInVehicle())
						return false;
				}			
			}
			return true;
		}
		if (!controlled)
			return false;
		
		if (!compAccComp)
			compAccComp = CompartmentAccessComponent.Cast(controlled.FindComponent(CompartmentAccessComponent));
		return !compAccComp.IsInCompartment();		
	}
};
