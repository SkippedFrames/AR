[BaseContainerProps()]
class SCR_AIDangerReaction_DoorMovement : SCR_AIDangerReaction
{
	static const float AGENT_DEFAULT_RADIUS = 0.3;
	static const float AGENT_DEFAULT_HEIGHT = 1.8;
	static const float REACTION_DIST_SQ = 25.0;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity door = dangerEvent.GetObject();
		vector agentPos = utility.GetOrigin();
		vector doorPos = door.GetOrigin();
		
		//Initial filter for hearing distance (TODO: optimization spatial grid)
		float distSq = vector.DistanceSq(agentPos, doorPos);
		if (distSq >= REACTION_DIST_SQ)
			return false;
		
		vector min, max;
		float distToDoorY = doorPos[1] - agentPos[1];
		//If agent above door higher than the door height, skip 
		if (-distToDoorY > AGENT_DEFAULT_HEIGHT)
			return false;
			
		door.GetBounds(min, max);
		vector doorExtent = max - min;
		//If agent bellow door and dist is higher than agent height, skip
		if (distToDoorY > doorExtent[1])
			return false;
		
		float distToDoorXZ = vector.DistanceXZ(doorPos, agentPos) - AGENT_DEFAULT_RADIUS;
		doorExtent[1] = 0.0;
		float doorLength = doorExtent.Length();
		//If the agent isn't in XZ range, skip
		if (distToDoorXZ > doorLength)
			return false;
	
		//Ensure the necessary entities and components are valid
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		if (!agent)
			return false;
		
		//Turn around if the instigator of door is an enemy,
		IEntity actionStarter = dangerEvent.GetVictim();
		if (agent.IsEnemy(actionStarter))
		{
			utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, null, agentPos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, 0, null, AGENT_DEFAULT_RADIUS));
			utility.m_LookAction.LookAt(actionStarter.GetOrigin(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
			
		ChimeraCharacter character = ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (character && character.IsInVehicle())
			return false;
		
		DoorComponent doorComponent = DoorComponent.Cast(door.FindComponent(DoorComponent));
		if (!doorComponent)
			return false;
	
		vector doorNormal = doorComponent.GetDoorNormal();
		float distanceToMove = Math.Clamp((doorLength - distToDoorXZ) * 2.0, 2.0 * AGENT_DEFAULT_RADIUS, doorLength);
		vector agentDirection = agentPos - doorPos;
		agentDirection.Normalize();
		vector movePos;
		//Adjust the doorNormal to lead the agent away from the door
		if (vector.DistanceSq(agentDirection, doorNormal) <  vector.DistanceSq(agentDirection, -doorNormal))
			movePos = agentPos + doorNormal * distanceToMove;
		else
			movePos = agentPos - doorNormal * distanceToMove;
		
		//Step away and free door space
		utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, null, movePos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, 0, null, AGENT_DEFAULT_RADIUS));
		return true;
	}
};
