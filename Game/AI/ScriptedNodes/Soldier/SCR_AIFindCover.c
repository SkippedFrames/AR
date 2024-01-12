class SCR_AIFindCover : AITaskScripted
{
	private static const string PORT_POSITION = "Position";
  	private static const string PORT_STANCE = "Stance";
  	private static const string PORT_DANGER_POS = "DangerPosition";
	private static const string PORT_ENEMY = "Enemy";
	private static const string PORT_IN_COVER = "InCover";
	
	private static const float PROJECTION_DISTANCE = 9;
	private static const vector IN_COVER_OFFSET = Vector(0, 0, 3); // When in cover, we will offset our pos by this vector
	private static const float GROUND_HEIGHT = 0.3;
	private static const vector KNEEL_OFFSET = Vector(0, 0.65, 0);
	private static const vector STAND_OFFSET = Vector(0, 1.1, 0);
	
	private static vector offsets[3] = {
		Vector(0, GROUND_HEIGHT, PROJECTION_DISTANCE), 
		Vector(PROJECTION_DISTANCE, GROUND_HEIGHT, PROJECTION_DISTANCE), 
		Vector(-PROJECTION_DISTANCE, GROUND_HEIGHT, PROJECTION_DISTANCE)
	};
		

#ifdef WORKBENCH
	private ref array<ref Shape> m_DebugShapes = {};
	
	private static const ShapeFlags m_SphereFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
	private static const ShapeFlags m_LineFlags = ShapeFlags.NOOUTLINE|ShapeFlags.TRANSP;
	private static const float DEBUGSPHERE_RADIUS = 0.1;
#endif	
	
	
	private PerceptionComponent m_PerceptionComponent;
	private AIPathfindingComponent m_PathfindingComponent;
	
	private IEntity m_Enemy;
	private vector m_DangerPosition;
	private BaseWorld m_World;
	private ref TraceParam m_TraceParams;

	//------------------------------------------------------------------------------------------------	
	override static protected bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION,
		PORT_STANCE
	};
	override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENEMY,
		PORT_DANGER_POS,
		PORT_IN_COVER
	};	
	override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
#ifdef WORKBENCH	
	// Draw debug trace check, sphere is red when there is direct sight to target. Green when obstructed by smthing.
	//------------------------------------------------------------------------------------------------
	private void DrawDebugTrace(float hit)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
		{
			int color = COLOR_RED;
			if (hit != 1)
				color = COLOR_GREEN; 

			m_DebugShapes.Insert(Shape.CreateSphere(color, m_SphereFlags, m_TraceParams.Start, DEBUGSPHERE_RADIUS));
			
			vector p[2];
			p[0] = m_TraceParams.Start;
			p[1] = vector.Direction(m_TraceParams.Start, m_TraceParams.End)*hit + m_TraceParams.Start;
			
			m_DebugShapes.Insert(Shape.CreateLines(COLOR_RED, m_LineFlags, p, 2));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		ClearDebug();
	}
	
	//------------------------------------------------------------------------------------------------
	private void ClearDebug()
	{
		foreach (auto s : m_DebugShapes)
		{
			s = null;
		}
		m_DebugShapes.Clear();
	}
#endif
	
	//------------------------------------------------------------------------------------------------		
	bool IsCover(IEntity ownerEntity, vector posGround, vector end, out ECharacterStance stance)
	{
		if (!m_TraceParams)
		{
			m_TraceParams = new TraceParam();		
			m_TraceParams.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
			m_TraceParams.LayerMask =  EPhysicsLayerDefs.Projectile;
		}
		
		m_TraceParams.Exclude = m_Enemy;
		ref array<IEntity> excludeArray = {ownerEntity};
		m_TraceParams.ExcludeArray = excludeArray;
		m_TraceParams.Start = posGround;
		m_TraceParams.End = end;
		
		float hit;
		hit = m_World.TraceMove(m_TraceParams, null);
#ifdef WORKBENCH
		DrawDebugTrace(hit);
#endif	
		if (hit == 1)
			return false;

		m_TraceParams.Start = posGround + KNEEL_OFFSET;
		hit = m_World.TraceMove(m_TraceParams, null);
#ifdef WORKBENCH
		DrawDebugTrace(hit);
#endif
		if (hit == 1)
		{
			stance = ECharacterStance.CROUCH;			
			return true;
		}
		
		m_TraceParams.Start = posGround + STAND_OFFSET;
		hit = m_World.TraceMove(m_TraceParams, null);
#ifdef WORKBENCH
		DrawDebugTrace(hit);
#endif
		if (hit == 1)
		{
			stance = ECharacterStance.STAND;
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_PathfindingComponent)
		{
			m_PathfindingComponent = AIPathfindingComponent.Cast( owner.FindComponent(AIPathfindingComponent));
			
			if (!m_PathfindingComponent)
				NodeError(this, owner, "Missing pathfinding component");
		}
		
		m_World = owner.GetWorld(); 
		bool directionProvided = false;
		
		if (!GetVariableIn(PORT_ENEMY,m_Enemy))
		{
			if (!GetVariableIn(PORT_DANGER_POS,m_DangerPosition))
				return ENodeResult.FAIL;
			else
				directionProvided = true;
		}
		
		ChimeraCharacter enemyCharacter = ChimeraCharacter.Cast(m_Enemy);
		if (!enemyCharacter && !directionProvided)
			return ENodeResult.RUNNING;
		
#ifdef WORKBENCH		
		ClearDebug();
#endif
		
		bool inCoverNow;
		if(!GetVariableIn(PORT_IN_COVER, inCoverNow))
			inCoverNow = false;
		
		IEntity ownerEntity = owner.GetControlledEntity();
		vector enemyAimPos;
		if (!directionProvided)
			enemyAimPos = enemyCharacter.AimingPosition();
		else 
			enemyAimPos = m_DangerPosition;

		
		vector traceOrigin;
		if (inCoverNow)
			traceOrigin = ownerEntity.CoordToParent(IN_COVER_OFFSET);
		else
			traceOrigin = ownerEntity.GetOrigin();
		ECharacterStance stance;
		
		vector hitNavmeshPos;
		bool coverFound;		
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
			m_DebugShapes.Insert(Shape.CreateSphere(Color.PINK, m_SphereFlags, traceOrigin, DEBUGSPHERE_RADIUS));
#endif
		
		foreach (vector offsetLocal : offsets)
		{
			vector traceEndWorld = ownerEntity.CoordToParent(offsetLocal);
			bool holeInNavmesh = !m_PathfindingComponent.RayTrace(traceOrigin, traceEndWorld, hitNavmeshPos);
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
			{
				//Shows blue sphere shows to where was tried cast on navmesh. Blue sphere. 
				int color = COLOR_BLUE;
				if (holeInNavmesh)
					color = COLOR_BLUE_A;
				m_DebugShapes.Insert(Shape.CreateSphere(color, m_SphereFlags, traceEndWorld, DEBUGSPHERE_RADIUS));
			}
#endif
			
			if (!holeInNavmesh)
				continue;
				
			coverFound = IsCover(ownerEntity, hitNavmeshPos, enemyAimPos, stance);
			if (coverFound)
					break;
		}
		
		if (!coverFound)
			return ENodeResult.FAIL; // we did not find cover, we should give up and leave it to other alternatives (e.g. side steppping,...)
		
		SetVariableOut(PORT_STANCE, stance);
		SetVariableOut(PORT_POSITION, hitNavmeshPos);
		
		return ENodeResult.SUCCESS;
	}
	
	override string GetOnHoverDescription()
	{
		return "Provide enemy entity or danger position of attack and node returns cover spot and stance";	
	}
};
