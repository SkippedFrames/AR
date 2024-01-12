class SCR_AIThrowGrenadeToBehavior : SCR_AIBehaviorBase
{
	//Target Position of the behavior
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.TARGETPOSITION_PORT);
#ifdef AI_DEBUG
	//Diagnostic visualization
	static ref array<ref Shape> m_aDbgShapes;
#endif
	private static vector CHARACTER_HEIGHT_OFFSET = {0, 1.6, 0};
	
	void SCR_AIThrowGrenadeToBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		//m_Target.Init(this, target);
		m_vPosition.Init(this, pos);
	
		if (!utility)
			return;
		
		m_bAllowLook = false;
		SetPriority(PRIORITY_BEHAVIOR_THROW_GRENADE);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Throw_Grenade.bt";
#ifdef AI_DEBUG 
		m_aDbgShapes = new array<ref Shape>;
#endif
	}
	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef AI_DEBUG 
		m_aDbgShapes.Clear();
#endif
	}
	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		if (GetActionState() == EAIActionState.COMPLETED || GetActionState() == EAIActionState.FAILED)
			return;
		super.OnActionFailed();
	}
	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " throwing grenade to " + m_vPosition.ValueToString();
	}
	//----------------------------------------------------------------------------------
	void UpdatePositionInfo(vector pos)
	{
		m_vPosition.m_Value = pos;
	}
	//----------------------------------------------------------------------------------
	/*
		Traces a certain distance of the grenade's movement
		Returns true if the throw doesn't hit anything in the given distance
		Returns false if the throw is obstructed
	
		INPUT 
			Start position, target Position, distance 
			Optional: Thrower entity, Target Entity 
	*/
	static bool TraceForGrenadeThrow(vector vStartPosition, vector vTargetPosition, float fTravelDistance, IEntity owner = null, IEntity target = null) 
	{
		bool found = false;
		vector direction = vTargetPosition - vStartPosition;
		direction[1] = 0.0;
		direction.Normalize();
		
		TraceParam traceparams = TraceParam();
		traceparams.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		traceparams.LayerMask = EPhysicsLayerDefs.Projectile;
		ref array<IEntity> excludeArray = {owner, target};
		traceparams.ExcludeArray = excludeArray;
		traceparams.Start = vStartPosition + CHARACTER_HEIGHT_OFFSET;
		traceparams.End = traceparams.Start + fTravelDistance * direction;
		
		float hit;
		BaseWorld world = owner.GetWorld();
		hit = world.TraceMove(traceparams, null);
		found = (hit == 1);
#ifdef AI_DEBUG
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_GRENADE_TRACE))
		{
			int shapeFlags = ShapeFlags.VISIBLE|ShapeFlags.NOZBUFFER;
			vector p[2];
			if (!found)
			{	//Setup Diagnostic visualization, draw green how much is clear, the rest in red
				vector traversal = traceparams.End - traceparams.Start;
				p[0] = traceparams.Start;
				p[1] = traceparams.Start + hit * traversal;
				Shape valid = Shape.CreateLines(ARGBF(1.0, 0.0, 1.0, 0.0), shapeFlags, p, 2);
				m_aDbgShapes.Insert(valid);
				
				p[0] = traceparams.Start + hit * traversal;
				p[1] = traceparams.End;
				Shape invalid = Shape.CreateLines(ARGBF(1.0, 1.0, 0.0, 0.0), shapeFlags, p, 2);
				m_aDbgShapes.Insert(invalid);
			}
			else
			{
				p[0] = traceparams.Start;
				p[1] = traceparams.End;
				Shape valid = Shape.CreateLines(ARGBF(1.0, 0.0, 1.0, 0.0), shapeFlags, p, 2);
				m_aDbgShapes.Insert(valid);
			}
		}
#endif
		return found;
	}
};
