class SCR_AISetPosition : AITaskScripted
{
	protected static const string TARGET_INFO_PORT = "TargetInfo";
	
	protected static const string ENTITY_PORT = "EntitytIn";
	protected static const string AGENT_PORT = "AgentIn";
	protected static const string TARGET_PORT = "DestinationIn";
		
	protected ref TStringArray s_aVarsIn = {
		ENTITY_PORT,
		AGENT_PORT,
		TARGET_PORT,
	};
	
	CharacterControllerComponent m_charContr;
	
	//------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription() { return "Sets (teleports) entity to target, that can be vector, smartAction or vehicle compartment"; };
	
	//------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity entityToTeleport;
		vector positionToTeleport;
		
		if(!GetVariableIn(ENTITY_PORT, entityToTeleport))
		{
			AIAgent agent;
			
			if (!GetVariableIn(AGENT_PORT, agent))
				return NodeError(this, owner, "Did not provided entity to teleport!");
			entityToTeleport = agent.GetControlledEntity();
		}
		
		if (!entityToTeleport)
			return NodeError(this, owner, "Entity is null!");
		
		if (GetVariableType(true,TARGET_PORT) == vector)
		{
			GetVariableIn(TARGET_PORT, positionToTeleport);
		}
		else if (GetVariableType(true,TARGET_PORT) == SCR_AISmartActionComponent)
		{
			SCR_AISmartActionComponent SAComponent;
			vector originOfObject;
			
			GetVariableIn(TARGET_PORT, SAComponent);
			originOfObject = SAComponent.GetOwner().GetOrigin();
			positionToTeleport = originOfObject + SAComponent.GetActionOffset();
		}
		else if (GetVariableType(true,TARGET_PORT).IsInherited(BaseCompartmentSlot))
		{
			BaseCompartmentSlot compartmentSlot;
			CompartmentAccessComponent CAComponent = CompartmentAccessComponent.Cast(entityToTeleport.FindComponent(CompartmentAccessComponent));
			IEntity vehicle;
			
			GetVariableIn(TARGET_PORT, compartmentSlot);
			vehicle = compartmentSlot.GetOwner();
			if (!CAComponent || !vehicle)
				return NodeError(this, owner, "Enity to teleport does not have CompartmentAccessComponent or target is not an Entity!");
			if (!CAComponent.MoveInVehicle(vehicle, compartmentSlot))
			{
#ifdef AI_DEBUG
				Print("Teleport to vehicle was not successful!", LogLevel.VERBOSE);
#endif
				if (!m_charContr)
				{
					m_charContr = CharacterControllerComponent.Cast(entityToTeleport.FindComponent(CharacterControllerComponent));
					if (!m_charContr)
						return NodeError(this, owner, "Enity to teleport does not have CharacterControllerComponent!");
				}
				
				if (m_charContr.IsSwimming())
					return ENodeResult.FAIL;
				else if (m_charContr.IsFalling())
					return ENodeResult.RUNNING;
				else
					return ENodeResult.FAIL;
			}
			return ENodeResult.SUCCESS;
		}
		
		BaseGameEntity gEntity = BaseGameEntity.Cast(entityToTeleport);
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = positionToTeleport;
		
		gEntity.Teleport(mat); // teleporting entity
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	override bool CanReturnRunning()
	{
		return true;
	}
}
