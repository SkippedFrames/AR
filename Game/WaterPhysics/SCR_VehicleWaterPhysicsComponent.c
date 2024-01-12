[EntityEditorProps(category: "GameScripted/Physics", description: "Vehicle Water Physics component")]
class SCR_VehicleWaterPhysicsComponentClass: SCR_WaterPhysicsComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_VehicleWaterPhysicsComponent Class
//------------------------------------------------------------------------------------------------
class SCR_VehicleWaterPhysicsComponent : SCR_WaterPhysicsComponent
{
	[Attribute("", UIWidgets.Object, "Water jet positions in model space", category: "Water Control")]
	ref array<vector> m_aThrustPoints;
	[Attribute("0", UIWidgets.Slider, "Forward thrust of the vehicle in water (in m/s^2)", "0 1000 0.01", category: "Water Control")]
	float m_fThrustForward;
	[Attribute("0", UIWidgets.Slider, "Reverse thrust of the vehicle in water (in m/s^2)", "0 1000 0.01", category: "Water Control")]
	float m_fThrustReverse;
	[Attribute("0", UIWidgets.Slider, "Steering thrust of the vehicle in water (in deg/s^2), this is applied even on the spot", "0 360 0.01", category: "Water Control")]
	float m_fThrustSteering;
	[Attribute("1", UIWidgets.Slider, "Vehicle forward/backward speed to steering thrust (in m/s to deg/s^2)", "0 36000 0.01", category: "Water Control")]
	float m_fSpeedToSteering;
	[Attribute("0.04", UIWidgets.Slider, "How much buoyancy the vehicle loses per second of being in the water down to a minimum of -1", "0 10 0.01", category: "Water Control")]
	float m_fBuoyancyLoss;
	[Attribute("0.08", UIWidgets.Slider, "How much buoyancy the vehicle loses per second of being in the water down to a minimum of -1 - when vehicle hull is destroyed", "0 10 0.01", category: "Water Control")]
	float m_fBuoyancyLossDestroyed;
	[Attribute("0.1", UIWidgets.Slider, "How much buoyancy the vehicle gains per second of being out of the water up to maximum of initial buoyancy value", "0 10 0.01", category: "Water Control")]
	float m_fBuoyancyGain;
	[Attribute("0.4", UIWidgets.Slider, "Health at which buyoancy loss starts", "0 1 0.01", category: "Water Control")]
	float m_fBuoyancyLossThreshold;
	
	const protected float MIN_THROTTLE = 0.04;
	const protected float MIN_BRAKE = 0.04;
	const protected int REVERSE_GEAR = 0;
	
	protected bool m_bIsLeaking;
	protected int m_iBuoyancySignalIdx;
	protected SignalsManagerComponent m_pSignalsManagerComponent;
	protected VehicleWheeledSimulation_SA m_pSimulation_SA;
	protected VehicleWheeledSimulation m_pSimulation;
	
	// Server Authoritative vehicles informing
	protected NwkCarMovementComponent m_pNwkCarMovementComponent;
	protected RplComponent m_pRplComponent;
	
	protected float m_fBuoyancyInitial;
	protected float m_fBuoyancyLossCurrent;
	
#ifndef DISABLE_WATERPHYSICS
	//------------------------------------------------------------------------------------------------
	override void SimulateWaterPhysics(float timeSlice)
	{
		super.SimulateWaterPhysics(timeSlice);
		
		// Buoyancy changes
		if (!m_bIsLeaking)
		{
			if (GetCenterOfMassInWater())
			{
				// Buoyancy loss
				if (m_fBuoyancyLossCurrent > 0 && m_fBuoyancy > -1)
				{
					m_bIsLeaking = true;
					ConnectToVehicleWaterPhysicsSystem();
				}
			}
			else
			{
				// Buoyancy gain
				if (m_fBuoyancyGain > 0 && m_fBuoyancy < m_fBuoyancyInitial)
				{
					m_bIsLeaking = true;
					ConnectToVehicleWaterPhysicsSystem();
				}
			}
		}
		
		if (!GetInWater())
			return;
		
		IEntity owner = GetOwner();
		
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		// Thrust
		float enginePower;
		float thrust;
		float rudder;
		
		if(GetGame().GetIsClientAuthority())
		{
			if (m_pSimulation && m_pSimulation.EngineIsOn())
			{
				enginePower = Math.InverseLerp(0, m_pSimulation.EngineGetRPMPeakPower(), m_pSimulation.EngineGetRPM());
				
				if (m_pSimulation.GetThrottle() > MIN_THROTTLE)
					thrust = Math.Clamp(enginePower, 0, 1);
				else if (m_pSimulation.GetBrake() > MIN_BRAKE)
					thrust = -1;
				
				// Reverse gear control
				// TODO Take driving assist mode in account
				if (m_pSimulation.GetGear() == REVERSE_GEAR)
					thrust *= -1;
			}
			
			if (m_pSimulation)
				rudder = -m_pSimulation.GetSteering();
		}
		else
		{
			if (m_pSimulation_SA && m_pSimulation_SA.EngineIsOn())
			{
				enginePower = Math.InverseLerp(0, m_pSimulation_SA.EngineGetRPMPeakPower(), m_pSimulation_SA.EngineGetRPM());
				
				if (m_pSimulation_SA.GetThrottle() > MIN_THROTTLE)
					thrust = Math.Clamp(enginePower, 0, 1);
				else if (m_pSimulation_SA.GetBrake() > MIN_BRAKE)
					thrust = -1;
				
				// Reverse gear control
				// TODO Take driving assist mode in account
				if (m_pSimulation_SA.GetGear() == REVERSE_GEAR)
					thrust *= -1;
			}
			
			if (m_pSimulation_SA)
				rudder = -m_pSimulation_SA.GetSteering();
		}		
		
		float thrustVal = m_fThrustForward;
		if (thrust < 0) // Reversing
		{
			rudder *= -1;
			thrustVal = m_fThrustReverse;
		}
		thrustVal *= thrust;
		
		vector mat[4];
		owner.GetTransform(mat);
		
		if (m_aThrustPoints.IsEmpty())
			m_aThrustPoints.Insert(physics.GetCenterOfMass());
		
		float jetPower = 1/m_aThrustPoints.Count();
	
		int jetsInWater;
		
		// Movement
		if (thrustVal != 0)
		{
			float depth;
			foreach (vector thrustPoint: m_aThrustPoints)
			{
				// Check if point is underwater
				SCR_WorldTools.IsObjectUnderwater(owner, thrustPoint, -1, depth);
				if (depth > 0)
				{
					jetsInWater++;
					physics.ApplyImpulseAt(owner.CoordToParent(thrustPoint), mat[2] * physics.GetMass() * thrustVal * jetPower * timeSlice);
				}
			}
		}
		
		// Steering
		if (rudder != 0 && (m_fThrustSteering != 0 || m_fSpeedToSteering != 0))
		{
			float torqueVal = m_fThrustSteering * -rudder * enginePower * jetPower * jetsInWater;
			if (m_fSpeedToSteering != 0)
			{
				float forwardSpeed = physics.GetVelocity().InvMultiply3(mat)[2];
				torqueVal += Math.AbsFloat(forwardSpeed) * m_fSpeedToSteering * -rudder;
			}
			if (torqueVal != 0)
				physics.SetAngularVelocity(mat[1] * torqueVal * timeSlice * Math.DEG2RAD + physics.GetAngularVelocity());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFixed(float timeSlice)
	{
		float buoyancySignalValue;
		
		if (GetCenterOfMassInWater())
		{
			// Buoyancy gain loss
			m_fBuoyancy = Math.Clamp(m_fBuoyancy - m_fBuoyancyLossCurrent * timeSlice, -1, m_fBuoyancyInitial);
			buoyancySignalValue = m_fBuoyancy + 1;
			
			// All the air has leaked
			if (m_fBuoyancy <= -1)
			{
				m_fBuoyancy = -1;
				DisconnectFromVehicleWaterPhysicsSystem();
				m_bIsLeaking = false;
				buoyancySignalValue = 0;
			}
		}
		else if (m_fBuoyancyGain > 0)
		{
			// Buoyancy gain
			m_fBuoyancy = Math.Clamp(m_fBuoyancy + m_fBuoyancyGain * timeSlice, -1, m_fBuoyancyInitial);
			buoyancySignalValue = m_fBuoyancy - m_fBuoyancyInitial;
			
			// Buoyancy back to normal and owner did not have on frame initially, so disable frame event
			if (m_fBuoyancy >= m_fBuoyancyInitial)
			{
				m_fBuoyancy = m_fBuoyancyInitial;
				DisconnectFromVehicleWaterPhysicsSystem();
				m_bIsLeaking = false;
				buoyancySignalValue = 0;
			}
		}
		
		// Buoyancy signal
		// If > 0, than sinking
		// If < 0, than leaking	
		if (m_pSignalsManagerComponent)
			m_pSignalsManagerComponent.SetSignalValue(m_iBuoyancySignalIdx, buoyancySignalValue);
	}
	
	protected void ConnectToVehicleWaterPhysicsSystem()
	{
		World world = GetOwner().GetWorld();
		VehicleWaterPhysicsSystem updateSystem = VehicleWaterPhysicsSystem.Cast(world.FindSystem(VehicleWaterPhysicsSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	protected void DisconnectFromVehicleWaterPhysicsSystem()
	{
		World world = GetOwner().GetWorld();
		VehicleWaterPhysicsSystem updateSystem = VehicleWaterPhysicsSystem.Cast(world.FindSystem(VehicleWaterPhysicsSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Lose buoyancy if health is below threshold
	void SetHealth(float health)
	{
		if (m_fBuoyancyLossThreshold > 0)
			health = Math.InverseLerp(0, m_fBuoyancyLossThreshold, health);
		
		health = Math.Clamp(health, 0, 1);
		m_fBuoyancyLossCurrent = Math.Lerp(m_fBuoyancyLossDestroyed, m_fBuoyancyLoss, health);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_fBuoyancyInitial = m_fBuoyancy;
		m_fBuoyancyLossCurrent = m_fBuoyancyLoss;
	
		if(GetGame().GetIsClientAuthority())
		{
			m_pSimulation = VehicleWheeledSimulation.Cast(owner.FindComponent(VehicleWheeledSimulation));
			if (m_pSimulation && !m_pSimulation.IsValid())
				m_pSimulation = null;
		}
		else
		{
			m_pSimulation_SA = VehicleWheeledSimulation_SA.Cast(owner.FindComponent(VehicleWheeledSimulation_SA));
			if (m_pSimulation_SA && !m_pSimulation_SA.IsValid())
				m_pSimulation_SA = null;
		}
		
		
		m_pSignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		
		if (m_pSignalsManagerComponent)
			m_iBuoyancySignalIdx = m_pSignalsManagerComponent.AddOrFindMPSignal("Buoyancy", 0.1, 1);
		
		if(!GetGame().GetIsClientAuthority())
		{
			m_pNwkCarMovementComponent = NwkCarMovementComponent.Cast(owner.FindComponent(NwkCarMovementComponent));
			m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromVehicleWaterPhysicsSystem();
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEnterWater()
	{
		if(!GetGame().GetIsClientAuthority() && m_pNwkCarMovementComponent && m_pRplComponent)
		{
			if(!m_pRplComponent.IsProxy())
				m_pNwkCarMovementComponent.SetAllowance(false, 2, 0.1, 1, 1);
			else if(m_pRplComponent.IsProxy() && m_pRplComponent.IsOwner())
				m_pNwkCarMovementComponent.SetPrediction(false);
		}
	}
	
	override void OnExitWater()
	{
		if(!GetGame().GetIsClientAuthority() && m_pNwkCarMovementComponent  && m_pRplComponent)
		{
			if(!m_pRplComponent.IsProxy())
				m_pNwkCarMovementComponent.SetAllowance(true, 300, 0.003, 0.25, 0.3);
			else if(m_pRplComponent.IsProxy() && m_pRplComponent.IsOwner())
				m_pNwkCarMovementComponent.SetPrediction(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when Item is initialized from replication stream.
	Carries the data from Master.
	*/
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadHalf(m_fBuoyancy);
		reader.ReadHalf(m_fBuoyancyInitial);
		reader.ReadHalf(m_fBuoyancyLossCurrent);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when Item is getting replicated from Master to Slave connection.
	The data will be delivered to Slave using RplInit method.
	*/
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteHalf(m_fBuoyancy);
		writer.WriteHalf(m_fBuoyancyInitial);
		writer.WriteHalf(m_fBuoyancyLossCurrent);
		
		return true;
	}
#endif
};
