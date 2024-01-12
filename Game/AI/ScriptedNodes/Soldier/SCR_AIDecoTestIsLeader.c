class SCR_AIDecoTestIsLeader : DecoratorTestScripted
{	
	override protected bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIGroup group = AIGroup.Cast(agent.GetParentGroup());
		
		if (!group)
			return false;
		
		return group.GetLeaderAgent() == agent;
	}
};
