[EntityEditorProps(category: "GameScripted/Physics", description: "Water Physics component")]
class SCR_WaterPhysicsComponentClass: ScriptComponentClass
{
	[Attribute("0 0 0", UIWidgets.Auto, "Debug point info", category: "Debug")]
	protected ref PointInfo m_vDebugPoint;
};

//------------------------------------------------------------------------------------------------
//! SCR_WaterPhysicsComponent Class
//------------------------------------------------------------------------------------------------
class SCR_WaterPhysicsComponent : ScriptComponent
{
	const bool ENABLE_WATERCURRENT = true;
	const bool ENABLE_FRICTION = true;
	const bool ENABLE_BUOYANCY = true;
	const bool ENABLE_BUOYANCYPOINTS = true;
	const bool ENABLE_IMPACTSLOW = false; // TODO: Improve impact slow
	
	const float DENSITY = 1;
	
	[Attribute("-0.5", UIWidgets.Slider, "Buoyancy of the object. Use higher values to adjust for volume (-1 = sinks, 0 = neutral, 1 = floats)", "-1 1 0.01", category: "Water Physics")]
	protected float m_fBuoyancy;
	[Attribute("0.8", UIWidgets.Slider, "Buoyancy force application point distance scale", "0 5 0.01", category: "Water Physics")]
	protected float m_fBuoyancyApplyDistScale;
	[Attribute("0", UIWidgets.Slider, "Buoyancy force depth boost for submerged points", "-100 100 0.1", category: "Water Physics")]
	protected float m_fBuoyancyDepthOffset;
	[Attribute("1 1 1", UIWidgets.EditBox, "Scaling of linear water friction forces in model space (right/up/forward)", category: "Water Physics")]
	protected vector m_vHydrodynamicScaleLinear;
	[Attribute("1 1 1", UIWidgets.EditBox, "Scaling of angular water friction forces in model space (pitch/yaw/roll)", category: "Water Physics")]
	protected vector m_vHydrodynamicScaleAngular;
	[Attribute("0.8 0.7 0.9", UIWidgets.EditBox, "Scaling of the automated bounding box buoyancy (right/up/forward)", category: "Water Physics")]
	protected vector m_vBoundingBoxScale;
	[Attribute("0 0 0", UIWidgets.EditBox, "Offset of the automated bounding box buoyancy (right/up/forward)", category: "Water Physics")]
	protected vector m_vBoundingBoxOffset;
	[Attribute("1", UIWidgets.CheckBox, "Use bounding box wall centers as buyoancy points", category: "Water Physics")]
	protected bool m_bUseBoundingBox;
	
	[Attribute("0", UIWidgets.CheckBox, "Draw debug", category: "Debug")]
	protected bool m_DrawDebug;
	
	protected ref array<vector> m_aBuoyancyPoints = {};
	
	protected bool m_bInWater;
	protected bool m_bCenterOfMassInWater;
	protected int m_iNextWaterPoint;
	
	protected float m_fAveragedRadius = 1; // Stores the averaged radius of the model
	protected float m_fSmallObjectScale = 0; // Stores a scale (0 - 1) value for small object physics application
	
	// Stores last height of waves (for underwater current simulation), set from the water zone
	float m_fLastWaveHeight;
	
	private ref array<Shape> m_aDebugShapes;
	
#ifndef DISABLE_WATERPHYSICS
	//------------------------------------------------------------------------------------------------
	protected void CreateBuoyancyPointsFromBound()
	{
		m_aBuoyancyPoints.Clear();
		
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;
		
		vector comPos = physics.GetCenterOfMass();
		
		vector mins, maxs;
		GetOwner().GetBounds(mins, maxs);
		mins = mins;
		maxs = maxs;
		
		float min_x, min_y, min_z;
		float max_x, max_y, max_z;
		min_x = mins[0] * m_vBoundingBoxScale[0] + m_vBoundingBoxOffset[0];
		min_y = mins[1] * m_vBoundingBoxScale[1] + m_vBoundingBoxOffset[1];
		min_z = mins[2] * m_vBoundingBoxScale[2] + m_vBoundingBoxOffset[2];
		max_x = maxs[0] * m_vBoundingBoxScale[0] + m_vBoundingBoxOffset[0];
		max_y = maxs[1] * m_vBoundingBoxScale[1] + m_vBoundingBoxOffset[1];
		max_z = maxs[2] * m_vBoundingBoxScale[2] + m_vBoundingBoxOffset[2];
		
		// Get 6 points from 3 axes
		m_aBuoyancyPoints.Insert(vector.Right * min_x + Vector(0, comPos[1], comPos[2]));
		m_aBuoyancyPoints.Insert(vector.Right * max_x + Vector(0, comPos[1], comPos[2]));
		m_aBuoyancyPoints.Insert(vector.Up * min_y + Vector(comPos[0], 0, comPos[2]));
		m_aBuoyancyPoints.Insert(vector.Up * max_y + Vector(comPos[0], 0, comPos[2]));
		m_aBuoyancyPoints.Insert(vector.Forward * min_z + Vector(comPos[0], comPos[1], 0));
		m_aBuoyancyPoints.Insert(vector.Forward * max_z + Vector(comPos[0], comPos[1], 0));
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCenterOfMassInWater()
	{
		return m_bCenterOfMassInWater;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetInWater()
	{
		return m_bInWater;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetInWater(bool inWater)
	{
		bool wasInWater = m_bInWater;
		
		if (!inWater)
			m_bCenterOfMassInWater = false;
		
		if (wasInWater == inWater)
			return;
		
		m_bInWater = inWater;
	
		if (inWater)
			OnEnterWater();
		else
			OnExitWater();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnterWater()
	{
		// Impact slow-down
		// Currently WIP, it's not working well.
		if (!ENABLE_IMPACTSLOW)
			return;
		
		IEntity owner = GetOwner();
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		vector velocity = physics.GetVelocity();
		vector direction = velocity.Normalized();
		float speed = velocity.Length();
		float mass = physics.GetMass();
		
		float density = DENSITY;
		
		float objAreaInDir = SCR_Global.GetSurfaceAreaInDir(owner, direction); // vector.Down
		float maxForce = speed * mass * density;
		float impactForce = Math.Clamp(speed * objAreaInDir * 20, 0, maxForce);
		physics.ApplyImpulse(direction * -impactForce);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnExitWater()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SimulateWaterPhysics(float timeSlice)
	{
		if (!ENABLE_BUOYANCY)
		{
			SetInWater(false);
			return;
		}
		
		IEntity owner = GetOwner();
		
		Physics physics = owner.GetPhysics();
		float mass = physics.GetMass();
		
		// Massless object should not float
		if (mass <= 0)
		{
			SetInWater(false);
			return;
		}
		
		vector velocity = physics.GetVelocity();
		vector angularVelocity = physics.GetAngularVelocity();
		
		float speed = velocity.Length();
		float speedUp = velocity[1];
		
		vector matObj[4];
		owner.GetWorldTransform(matObj);
		
		// Ensure the matrix is using 1x scale for forces etc
		float ownerScale = owner.GetScale();
		if (ownerScale != 1)
		{
			for (int a = 0; a < 3; a++)
			{
				vector axis = matObj[a];
				axis.Normalize();
				matObj[a] = axis;
			}
		}
		
		vector centerOfMass = physics.GetCenterOfMass();
		vector centerOfMassW = owner.CoordToParent(centerOfMass); // use center of mass as center of object
		
		float centerOfMassDepth;
		SCR_WorldTools.IsObjectUnderwater(owner, centerOfMass, -1, centerOfMassDepth);
		m_bCenterOfMassInWater = centerOfMassDepth > 0;
		
		// Neutral buoyancy first (counteract gravity)
		vector buoyancyForce = -1 * Physics.VGravity * mass * timeSlice; 
		
		/*
			Scale buoyancy force by adjusted buoyancy value
				0 = No buoyancy, sink with gravity
				1 = Neutral buoyancy, counteract gravity
				2 = Positive buoyancy, floats
		*/
		buoyancyForce = buoyancyForce * (m_fBuoyancy + 1);
		
		float speedUpChange = buoyancyForce[1] / mass;
		float speedUpFinal = speedUp + speedUpChange;
		vector buoyancyForcePart = buoyancyForce * (1 / m_aBuoyancyPoints.Count()); // Scale point-applied buoyancy force by num points in water
		
		//! DEBUG
		if (m_DrawDebug)
		{
			if (!m_aDebugShapes)
				m_aDebugShapes = {};
			
			foreach (Shape shape: m_aDebugShapes)
				shape = null;
			
			m_aDebugShapes.Clear();
			
			if (centerOfMassDepth > 0)
			{
				vector lines[2];
				lines[0] = centerOfMassW;
				lines[1] = centerOfMassW + centerOfMassDepth * vector.Up;
				
				m_aDebugShapes.Insert(Shape.CreateLines(COLOR_YELLOW, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, lines, 2));
				m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_RED, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, centerOfMassW, 0.1));
			}
			else
			{
				m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_GREEN, ShapeFlags.NOOUTLINE|ShapeFlags.TRANSP, centerOfMassW, 0.1));
			}
		}
		
		// Depth offset for buoyancy force calculations
		float mainDepth = centerOfMassDepth + m_fBuoyancyDepthOffset;
		
		bool inWater;
		float depth;
		vector buoyancyPointW;
		
		foreach (vector buoyancyPoint: m_aBuoyancyPoints)
		{
			// Check if point is underwater
			depth = -1;
			SCR_WorldTools.IsObjectUnderwater(owner, buoyancyPoint, -1, depth);
			
			//! DEBUG
			if (m_DrawDebug)
			{
				if (depth > 0)
				{
					vector lines[2];
					lines[0] = owner.CoordToParent(buoyancyPoint);
					lines[1] = owner.CoordToParent(buoyancyPoint) + depth*vector.Up;
					
					m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_RED, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, owner.CoordToParent(buoyancyPoint), 0.05));
					m_aDebugShapes.Insert(Shape.CreateLines(COLOR_YELLOW, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, lines, 2));
				}
				else
				{
					m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_GREEN, ShapeFlags.NOOUTLINE|ShapeFlags.TRANSP, owner.CoordToParent(buoyancyPoint), 0.05));
				}
			}
			
			if (depth > 0)
				inWater = true;
			else
				continue;
			
			depth += m_fBuoyancyDepthOffset;
			
			if (ENABLE_BUOYANCYPOINTS)
			{
				buoyancyPointW = centerOfMass + (buoyancyPoint - centerOfMass) * m_fBuoyancyApplyDistScale;
				buoyancyPointW = buoyancyPointW.Multiply4(matObj);
			}
			else
			{
				buoyancyPointW = centerOfMassW;
			}
			
			// Large object
			float scale = Math.Clamp((depth * depth - speedUpFinal) / (m_fAveragedRadius * 0.5), 0, 1) * (1 - m_fSmallObjectScale);
			
			// Small object
			scale += Math.Clamp((depth + mainDepth - speedUpFinal * 0.1) * 0.5 / (m_fAveragedRadius * 0.5), 0, 1) * m_fSmallObjectScale;
			
			// Apply buoyant forces to each of the points that are underwater
			physics.ApplyImpulseAt(buoyancyPointW, buoyancyForcePart * scale);
		}
		
		// None of the points in water
		if (!inWater)
		{
			SetInWater(false);
			return;
		}
		
		// Pseudo calculation of water terminal velocity
		float termDepthScale = Math.Clamp(mainDepth / m_fAveragedRadius, 0, 1);
		float termVelDepthScale = termDepthScale * termDepthScale * speed / 2;
		termVelDepthScale = Math.Clamp(termVelDepthScale, 0, 100);
		
		angularVelocity = physics.GetAngularVelocity();
		
		float density = DENSITY;
		
		vector forceVec = -velocity * mass * density * termVelDepthScale;
		vector forceAngVec = angularVelocity * density * timeSlice;
		
		if (!ENABLE_FRICTION)
		{
			forceVec = vector.Zero;
			forceAngVec = vector.Zero;
		}
		
		// Get water current forces
		if (ENABLE_WATERCURRENT)
		{
			vector currentVec = vector.Zero; // needs new direction source
			forceVec = forceVec + currentVec;
		}
		
		int axis;
		float axisForce, axisForceMax;
		vector forceMax;
		if (m_vHydrodynamicScaleLinear != vector.One)
		{
			forceMax = -velocity * mass;
			forceMax = forceMax.InvMultiply3(matObj);
			forceVec = forceVec.InvMultiply3(matObj);
			for (axis = 0; axis < 3; axis++)
			{
				axisForceMax = Math.AbsFloat(forceMax[axis]) / timeSlice;
				axisForce = Math.Clamp(forceVec[axis] * m_vHydrodynamicScaleLinear[axis], -axisForceMax, axisForceMax);
				forceVec[axis] = axisForce;
			}
			forceVec = forceVec.Multiply3(matObj);
		}
		if (m_vHydrodynamicScaleAngular != vector.One)
		{
			forceMax = angularVelocity * timeSlice;
			forceMax = forceMax.InvMultiply3(matObj);
			forceAngVec = forceAngVec.InvMultiply3(matObj);
			for (axis = 0; axis < 3; axis++)
			{
				axisForceMax = Math.AbsFloat(forceMax[axis]) / timeSlice;
				axisForce = Math.Clamp(forceAngVec[axis] * m_vHydrodynamicScaleAngular[axis], -axisForceMax, axisForceMax);
				forceAngVec[axis] = axisForce;
			}
			forceAngVec = forceAngVec.Multiply3(matObj);
		}
		
		// Apply terminal velocity and water current forces
		physics.ApplyImpulse(forceVec * timeSlice);
		physics.SetAngularVelocity(angularVelocity - forceAngVec);
		
		SetInWater(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		vector parentSize = SCR_EntityHelper.GetEntitySize(owner);
		m_fAveragedRadius = (Math.AbsFloat(parentSize[0]) + Math.AbsFloat(parentSize[1]) + Math.AbsFloat(parentSize[2])) / 3;
		m_fAveragedRadius = Math.Clamp(m_fAveragedRadius, 0.1, 20);
		
		m_fSmallObjectScale = 1 - Math.Clamp((SCR_EntityHelper.GetEntityRadius(owner) - 1) / 5, 0, 1);
		
		if (m_bUseBoundingBox)
			CreateBuoyancyPointsFromBound();
		
		if (!m_aBuoyancyPoints.IsEmpty())
			SetEventMask(owner, EntityEvent.PHYSICSACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromWaterPhysicsSystem();
		
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (activeState && ENABLE_BUOYANCY)
			ConnectToWaterPhysicsSystem();
		else
			DisconnectFromWaterPhysicsSystem();
	}

	//------------------------------------------------------------------------------------------------
	void Simulate(float timeSlice)
	{
		// Somehow we lost all the reference points
		if (m_aBuoyancyPoints.IsEmpty())
		{
			DisconnectFromWaterPhysicsSystem();
			return;
		}
		
		if (!m_bInWater)
		{
			//! Lazy checks when vehicle is not in water, we should only test one point per cycle
			m_iNextWaterPoint = m_iNextWaterPoint++ % m_aBuoyancyPoints.Count();
			if (!m_aBuoyancyPoints[m_iNextWaterPoint])
				return;
			
			// Check if point is underwater
			if (SCR_WorldTools.IsObjectUnderwater(GetOwner(), m_aBuoyancyPoints[m_iNextWaterPoint], -1))
				SetInWater(true);
		}
		
		//! Full buoyancy simulation. May have to begin the same frame if either of the points has been proven to be underwater
		if (m_bInWater)
			SimulateWaterPhysics(timeSlice);
	}
	
	protected void ConnectToWaterPhysicsSystem()
	{
		World world = GetOwner().GetWorld();
		WaterPhysicsSystem updateSystem = WaterPhysicsSystem.Cast(world.FindSystem(WaterPhysicsSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	protected void DisconnectFromWaterPhysicsSystem()
	{
		World world = GetOwner().GetWorld();
		WaterPhysicsSystem updateSystem = WaterPhysicsSystem.Cast(world.FindSystem(WaterPhysicsSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
#endif
};
