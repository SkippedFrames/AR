class SCR_AIDecoTestIsOutVehicleCondition : DecoratorTestScripted
{
	// this tests AIWaypoint waypoint completion condition: either characters are out of vehicles 
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIWaypoint waypoint = AIWaypoint.Cast(controlled);
		if (!waypoint)
			return false;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group)
		{
			Debug.Error("Running on AIAgent that is not a SCR_AIGroup group!");
			return false;
		}
		
		array<AIAgent> agents = {};		
		
		EAIWaypointCompletionType completionType = waypoint.GetCompletionType();		
		
		switch (completionType)
		{
			case EAIWaypointCompletionType.All :
			{
				group.GetAgents(agents);				
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (character && character.IsInVehicle())
					{
						return false;						
						break;
					}
				}
				return true;
			}
			case EAIWaypointCompletionType.Leader :
			{
				ChimeraCharacter character = ChimeraCharacter.Cast(group.GetLeaderEntity());
				if (!character)
					return false;
				return !character.IsInVehicle(); // leader is outside the vehicle --> condition is true
			}
			case EAIWaypointCompletionType.Any :
			{
				group.GetAgents(agents);
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (!character) // same logic as leader's
						return false;
					if (character.!IsInVehicle())
						return true;					
				}
				return false;
			}
		}
		return false;
	}	
};
