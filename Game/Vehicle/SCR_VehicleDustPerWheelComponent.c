// Spawns dust particle effect that drags with the vehicle during ride

[ComponentEditorProps(category: "GameScripted/Test", description:"SCR_VehicleDustPerWheel")]
class SCR_VehicleDustPerWheelClass : ScriptGameComponentClass
{
	[Attribute("0", UIWidgets.Auto, "Vehicle index in ParticleEffectInfo")]
	int m_iVehicleIndex;

	[Attribute("", UIWidgets.Auto, "Simulation IDs of wheels to apply particle effects to")]
	ref array<int> m_aWheels;

	[Attribute("10", UIWidgets.Slider, "Minimal speed for 0% effect intensity interpolation\n[km/h]", "0 300 1")]
	float m_fDustStartSpeed;

	[Attribute("80", UIWidgets.Slider, "Maximal speed for 100% effect intensity interpolation\n[km/h]", "0 300 1")]
	float m_fDustTopSpeed;

	[Attribute("250", UIWidgets.Slider, "Maximal distance at which the effect is visible\n[m]", "0 2000 1")]
	float m_fMaxDistanceVisible;
	
	float m_fMaxDistanceVisibleSqr;
	
	override static array<typename> Requires(IEntityComponentSource src)
	{
		array<typename> requires = new array<typename>;
		
		requires.Insert(RplComponent);
		
		return requires;
	}
};

//! Vehicle dust per wheel data;
class VehicleDust
{
	vector m_vLocalDustPos;
	bool m_bWheelHasContact;
	int m_iLastSwap;
	GameMaterial m_Material;
	ParticleEffectEntity m_pParticleEffectEntity;
};

class SCR_VehicleDustPerWheel : ScriptGameComponent
{
	protected static const float 				UPDATE_TIME = 1.0 / 30.0;
	static const int							UPDATE_TIMEOUT = 1000; //Minimal delay between two particle swaps.

	protected VehicleWheeledSimulation			m_Simulation;
	protected VehicleWheeledSimulation_SA		m_Simulation_SA;
	protected Physics							m_Physics;
	protected NwkMovementComponent				m_NwkMovementComponent;
	protected SCR_VehicleDustPerWheelClass		m_ComponentData;
	protected RplComponent						m_RplComponent;
	protected ref array<ref VehicleDust>		m_aVehicleDusts = {};
	protected float 							m_fUpdateTime;
	protected float								m_fTime;
	protected IEntity							m_pOwner;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_pOwner = owner;
		
		if (System.IsConsoleApp())
		{
			Deactivate(m_pOwner);
			return;
		}

		m_ComponentData = SCR_VehicleDustPerWheelClass.Cast(GetComponentData(m_pOwner));
		if (!m_ComponentData || m_ComponentData.m_fDustStartSpeed > m_ComponentData.m_fDustTopSpeed || m_ComponentData.m_fMaxDistanceVisible <= 0)
		{
			Deactivate(m_pOwner);
			return;
		}
		
		if (!m_ComponentData.m_fMaxDistanceVisibleSqr)
		{
			m_ComponentData.m_fMaxDistanceVisibleSqr = m_ComponentData.m_fMaxDistanceVisible * m_ComponentData.m_fMaxDistanceVisible;
		}
		
		m_NwkMovementComponent = NwkMovementComponent.Cast(m_pOwner.FindComponent(NwkMovementComponent));
		
		m_Physics = m_pOwner.GetPhysics();
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if(GetGame().GetIsClientAuthority())
		{
			m_Simulation = VehicleWheeledSimulation.Cast(m_pOwner.FindComponent(VehicleWheeledSimulation));
			if (!m_Simulation || !m_Simulation.IsValid())
			{
				Deactivate(m_pOwner);
				return;
			}
		}
		else
		{
			m_Simulation_SA = VehicleWheeledSimulation_SA.Cast(m_pOwner.FindComponent(VehicleWheeledSimulation_SA));
			if (!m_Simulation_SA || !m_Simulation_SA.IsValid())
			{
				Deactivate(m_pOwner);
				return;
			}
		}

		m_fUpdateTime = UPDATE_TIME;
		m_fTime = 0.0;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void OnDelete(IEntity owner)
	{
		DisconnectFromVehiclesDustSystem();
		
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnTicksOnRemoteProxy()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		ConnectToVehiclesDustSystem();

		int count;
		if(GetGame().GetIsClientAuthority())
		 	count = Math.Min(m_Simulation.WheelCount(), m_ComponentData.m_aWheels.Count());
		else
			count = Math.Min(m_Simulation_SA.WheelCount(), m_ComponentData.m_aWheels.Count());

		m_aVehicleDusts.Resize(count);
		for (int i = 0; i < count; ++i)
			m_aVehicleDusts[i] = new VehicleDust();

#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST, "", "Show dust materials", "Vehicles");
#endif
	}
	
	protected void ConnectToVehiclesDustSystem()
	{
		World world = GetOwner().GetWorld();
		VehiclesDustSystem updateSystem = VehiclesDustSystem.Cast(world.FindSystem(VehiclesDustSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	protected void DisconnectFromVehiclesDustSystem()
	{
		World world = GetOwner().GetWorld();
		VehiclesDustSystem updateSystem = VehiclesDustSystem.Cast(world.FindSystem(VehiclesDustSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		if (!m_RplComponent.IsProxy() || m_RplComponent.IsOwner())
		{
			if ((!m_Physics || !m_Physics.IsActive()))
				return;
		}
		else if (m_NwkMovementComponent && !m_NwkMovementComponent.IsInterpolating())
		{
			return;
		}
		
		if(GetGame().GetIsClientAuthority())
		{
			if (m_Simulation.GetSpeedKmh() < m_ComponentData.m_fDustStartSpeed)
				m_fUpdateTime = UPDATE_TIME;
			else
				m_fUpdateTime = -1.0;
		}
		else
		{
			if (m_Simulation_SA.GetSpeedKmh() < m_ComponentData.m_fDustStartSpeed)
				m_fUpdateTime = UPDATE_TIME;
			else
				m_fUpdateTime = -1.0;
		}
		
		m_fTime += timeSlice;
		if (m_fUpdateTime < 0.0 || m_fTime >= m_fUpdateTime)
		{
			m_fTime = 0.0;
			
			UpdateBatch();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBatch()
	{
		vector camMat[4];
		m_pOwner.GetWorld().GetCurrentCamera(camMat);
		
		float distanceFromCamera = vector.DistanceSq(m_pOwner.GetOrigin(), camMat[3]);
		float speed;
		if(GetGame().GetIsClientAuthority())
			speed = m_Simulation.GetSpeedKmh();
		else
			speed = m_Simulation_SA.GetSpeedKmh();
		
		foreach (int index, VehicleDust vehicleDust : m_aVehicleDusts)
		{
			UpdateEffect(vehicleDust, index, speed, distanceFromCamera);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected TraceParam TraceWheelContact(int index, out vector position)
	{
		int wheelIdx = m_ComponentData.m_aWheels[index];

		vector worldTransform[4];
		m_pOwner.GetWorldTransform(worldTransform);

		float radius;

		if(GetGame().GetIsClientAuthority())
		 	radius = m_Simulation.WheelGetRadiusState(wheelIdx);
		else
		 	radius = m_Simulation_SA.WheelGetRadiusState(wheelIdx);

		// Physics are offset by center of mass
		Physics physics = m_pOwner.GetPhysics();
		vector centerOfMass = physics.GetCenterOfMass();

		TraceParam trace = new TraceParam();
		if(GetGame().GetIsClientAuthority())
		{
			trace.Start = (m_Simulation.WheelGetPosition(wheelIdx, 1.0) + centerOfMass).Multiply4(worldTransform);
			trace.End = (m_Simulation.WheelGetPosition(wheelIdx, 0.0) + centerOfMass).Multiply4(worldTransform) + worldTransform[1] * -radius;
		}
		else
		{
			trace.Start = (m_Simulation_SA.WheelGetPosition(wheelIdx, 1.0) + centerOfMass).Multiply4(worldTransform);
			trace.End = (m_Simulation_SA.WheelGetPosition(wheelIdx, 0.0) + centerOfMass).Multiply4(worldTransform) + worldTransform[1] * -radius;
		}
		
		trace.Flags = TraceFlags.WORLD;
		trace.LayerMask = EPhysicsLayerDefs.VehicleCast;

		float hit = m_pOwner.GetWorld().TraceMove(trace, null);

		vector remotePosition = trace.Start + (trace.End - trace.Start) * hit;
		position = remotePosition.InvMultiply4(worldTransform);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
		{
			vector p[2];
			p[0] = trace.Start;
			p[1] = trace.End;
			Shape.CreateLines(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, p, 2);
			Shape.CreateSphere(Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, remotePosition, 0.05);
		}
#endif

		return trace;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateEffect(VehicleDust vehicleDust, int index, float speed, float distanceFromCamera)
	{
		int wheelIdx = m_ComponentData.m_aWheels[index];
		
		if (speed < m_ComponentData.m_fDustStartSpeed || distanceFromCamera >= m_ComponentData.m_fMaxDistanceVisibleSqr)
		{
			vehicleDust.m_Material = null;
			if (vehicleDust.m_pParticleEffectEntity)
			{
				vehicleDust.m_pParticleEffectEntity.StopEmission();
				UpdatePosition(vehicleDust, wheelIdx);
			}
			return;
		}
		
		int ticks = System.GetTickCount();
		int dif = Math.AbsInt(ticks - vehicleDust.m_iLastSwap);

		if (vehicleDust.m_pParticleEffectEntity && dif < UPDATE_TIMEOUT)
		{
			UpdateCurrent(vehicleDust, speed, wheelIdx);
			return;
		}

		GameMaterial newMaterial;
		bool wheelHasContact;
		if (m_RplComponent && m_RplComponent.IsRemoteProxy())
		{
			vector position;
			TraceParam trace = TraceWheelContact(index, position);
			if (trace)
			{
				newMaterial = trace.SurfaceProps;
				wheelHasContact = trace.TraceEnt != null;
				vehicleDust.m_vLocalDustPos = position;
			}
		}
		else
		{
			if(GetGame().GetIsClientAuthority())
			{
				if (m_Simulation.WheelGetContactLiquidState(wheelIdx) > 0)
					newMaterial = m_Simulation.WheelGetContactLiquidMaterial(wheelIdx);
				else
					newMaterial = m_Simulation.WheelGetContactMaterial(wheelIdx);

				wheelHasContact = m_Simulation.WheelHasContact(wheelIdx);
			}
			else
			{
				if (m_Simulation_SA.WheelGetContactLiquidState(wheelIdx) > 0)
					newMaterial = m_Simulation_SA.WheelGetContactLiquidMaterial(wheelIdx);
				else
					newMaterial = m_Simulation_SA.WheelGetContactMaterial(wheelIdx);

				wheelHasContact = m_Simulation_SA.WheelHasContact(wheelIdx);
			}
		}

		vehicleDust.m_bWheelHasContact = wheelHasContact;

		GameMaterial currentMaterial = vehicleDust.m_Material;
		ResourceName newResource;

		if (newMaterial && newMaterial != currentMaterial)	//different materials -> check resource
		{
			ParticleEffectInfo effectInfo = newMaterial.GetParticleEffectInfo();
			if (effectInfo)
				newResource = effectInfo.GetVehicleDustResource(m_ComponentData.m_iVehicleIndex);
		}
		
		//update current effect
		if (currentMaterial && vehicleDust.m_pParticleEffectEntity && (newMaterial == currentMaterial || (currentMaterial.GetParticleEffectInfo() && currentMaterial.GetParticleEffectInfo().GetResourceName() == newResource)))
		{
			if (newMaterial != currentMaterial)	//different GameMats, but resources are the same
			{
				vehicleDust.m_Material = newMaterial;
			}
			
			UpdateCurrent(vehicleDust, speed, wheelIdx);
			return;
		}
		
		vehicleDust.m_Material = newMaterial;
		if (vehicleDust.m_pParticleEffectEntity) // stop old effect
		{
			vehicleDust.m_pParticleEffectEntity.StopEmission();
			vehicleDust.m_pParticleEffectEntity = null;
		}
		
		//Create new effect
		if (newResource && newResource.Length() > 0)
		{
			ParticleEffectEntitySpawnParams spawnParams();
			spawnParams.TargetWorld = GetOwner().GetWorld();
			spawnParams.Parent = GetOwner();
			vehicleDust.m_pParticleEffectEntity = ParticleEffectEntity.SpawnParticleEffect(newResource, spawnParams);
			vehicleDust.m_iLastSwap = ticks;
		}

		UpdateCurrent(vehicleDust, speed, wheelIdx);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateCurrent(VehicleDust vehicleDust, float speed, int wheelIdx)
	{
		if (!vehicleDust.m_pParticleEffectEntity)
			return;
		
		UpdatePosition(vehicleDust, wheelIdx);
		UpdateVehicleDustEffect(vehicleDust, speed, wheelIdx);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePosition(VehicleDust vehicleDust, int wheelIdx)
	{
		vector position;
		if (m_RplComponent && m_RplComponent.IsRemoteProxy())
		{
			vector worldTransform[4];
			GetOwner().GetWorldTransform(worldTransform);
			position = vehicleDust.m_vLocalDustPos.Multiply4(worldTransform);
		}
		else
		{
			if(GetGame().GetIsClientAuthority())
			{
				if(!m_Simulation.WheelHasContact(wheelIdx))
					return;

				position = m_Simulation.WheelGetContactPosition(wheelIdx);
			}
			else
			{
				if(!m_Simulation_SA.WheelHasContact(wheelIdx))
					return;

				position = m_Simulation_SA.WheelGetContactPosition(wheelIdx);
			}
		}

		vehicleDust.m_pParticleEffectEntity.SetOrigin(position);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleDustEffect(VehicleDust vehicleDust, float speed, int wheelIdx)
	{
		float endSpeed = m_ComponentData.m_fDustTopSpeed;
		float speedCoef = 0;
		float birthCoef = 0;
		float gravityCoef = 0;

		float longitudinalSlip = 0; 
		float lateralSlip = 0;

		if(GetGame().GetIsClientAuthority())
		{
			longitudinalSlip = m_Simulation.WheelGetLongitudinalSlip(wheelIdx);
			lateralSlip = m_Simulation.WheelGetLateralSlip(wheelIdx);
		}
		else
		{
			longitudinalSlip = m_Simulation_SA.WheelGetLongitudinalSlip(wheelIdx);
			lateralSlip = m_Simulation_SA.WheelGetLateralSlip(wheelIdx);
		}
		
		float effectiveSlip = Math.Clamp(longitudinalSlip + lateralSlip, 0, 1);

		if (vehicleDust.m_bWheelHasContact)
		{
			speedCoef = Math.AbsFloat(0.5 + speed * 0.5 / endSpeed + effectiveSlip);
			birthCoef = Math.AbsFloat(0.5 + speed * 0.5 / endSpeed + effectiveSlip * 2);
			gravityCoef = Math.AbsFloat(0.8 + speed * 0.2 / endSpeed);
		}

		Particles particles = vehicleDust.m_pParticleEffectEntity.GetParticles();
		particles.MultParam(-1, EmitterParam.BIRTH_RATE, birthCoef);
		particles.MultParam(-1, EmitterParam.GRAVITY_SCALE_RND, gravityCoef);
		particles.MultParam(-1, EmitterParam.VELOCITY, speedCoef);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, speedCoef);

#ifdef ENABLE_DIAG
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
			return;

		if (!vehicleDust.m_bWheelHasContact)
			return;

		if (!vehicleDust.m_Material)
			return;

		vector effectPosition = vehicleDust.m_pParticleEffectEntity.GetWorldTransformAxis(3);
		DebugTextWorldSpace.Create(GetOwner().GetWorld(), vehicleDust.m_Material.GetName() + "\nlongSlip: " + longitudinalSlip.ToString(-1, 3) + " latSlip: " + lateralSlip.ToString(-1, 3), DebugTextFlags.CENTER | DebugTextFlags.ONCE, effectPosition[0], effectPosition[1] - 1, effectPosition[2]);
#endif
	}
};
