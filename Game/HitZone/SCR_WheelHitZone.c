enum EWheelDamageState: EDamageState
{
	PUNCTURED = 3
};

class SCR_WheelHitZone : SCR_DestructibleHitzone
{
	[Attribute( defvalue: "-1", uiwidget: UIWidgets.Auto, desc: "Wheel ID", category: "Wheel Damage")]
	protected int m_iWheelId;
	
	[Attribute( defvalue: "0.9", uiwidget: UIWidgets.Auto, desc: "Radius multiplier for a damaged wheel.", category: "Wheel Damage")]
	protected float m_fDamagedRadiusScale;
	[Attribute( defvalue: "0.4", uiwidget: UIWidgets.Auto, desc: "Longitudinal friction multiplier for a damaged wheel.", category: "Wheel Damage")]
	protected float m_fDamagedLongitudinalFrictionScale;
	[Attribute( defvalue: "0.4", uiwidget: UIWidgets.Auto, desc: "Lateral friction multiplier for a damaged wheel.", category: "Wheel Damage")]
	protected float m_fDamagedLateralFrictionScale;
	[Attribute( defvalue: "1.5", uiwidget: UIWidgets.Auto, desc: "Roughness increase for a damaged wheel.", category: "Wheel Damage")]
	protected float m_fDamagedRoughnessIncrease;
	[Attribute( defvalue: "0.1", uiwidget: UIWidgets.Auto, desc: "Drag of a damaged wheel.", category: "Wheel Damage")]
	protected float m_fDamagedDrag;
	
	[Attribute( defvalue: "0.8", uiwidget: UIWidgets.Auto, desc: "Radius multiplier for a destroyed wheel.", category: "Wheel Damage")]
	protected float m_fDestroyedRadiusScale;
	[Attribute( defvalue: "0.2", uiwidget: UIWidgets.Auto, desc: "Longitudinal friction multiplier for a destroyed wheel.", category: "Wheel Damage")]
	protected float m_fDestroyedLongitudinalFrictionScale;
	[Attribute( defvalue: "0.2", uiwidget: UIWidgets.Auto, desc: "Lateral friction multiplier for a destroyed wheel.", category: "Wheel Damage")]
	protected float m_fDestroyedLateralFrictionScale;
	[Attribute( defvalue: "3.0", uiwidget: UIWidgets.Auto, desc: "Roughness increase for a destroyed wheel.", category: "Wheel Damage")]
	protected float m_fDestroyedRoughnessIncrease;
	[Attribute( defvalue: "1.0", uiwidget: UIWidgets.Auto, desc: "Drag of a destroyed wheel.", category: "Wheel Damage")]
	protected float m_fDestroyedDrag;
	
	[Attribute(EVehicleHitZoneGroup.WHEELS.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;
		
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		UpdateWheelState();
		UpdateDamageSignal();
	}
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
	
		UpdateWheelState();
		UpdateDamageSignal()
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWheelIndex()
	{
		return m_iWheelId;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWheelIndex(int index)
	{
		m_iWheelId = index;
	
		UpdateWheelState();
		UpdateDamageSignal();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set wheel parameters when damage exceeds thresholds
	void UpdateWheelState()
	{
		if (m_iWheelId == -1)
			return;
		
		IEntity owner = GetOwner();
		
		IEntity parent = SCR_EntityHelper.GetMainParent(owner, true);
		if(GetGame().GetIsClientAuthority())
		{
			VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(parent.FindComponent(VehicleWheeledSimulation));
			if (!simulation || !simulation.IsValid() || m_iWheelId >= simulation.WheelCount())
				return;		
		
			// Update simulation
			float radius				= simulation.WheelGetRadius(m_iWheelId);
			float longitudinalFriction	= simulation.WheelTyreGetLongitudinalFriction(m_iWheelId);
			float lateralFriction		= simulation.WheelTyreGetLateralFriction(m_iWheelId);
			float roughness				= simulation.WheelTyreGetRoughness(m_iWheelId);
			float drag;
			
			// Only run the rest of code if state has changed
			EDamageState state = GetDamageState();
			if (state == EWheelDamageState.DESTROYED)
			{
				radius					*= m_fDestroyedRadiusScale;
				longitudinalFriction	*= m_fDestroyedLongitudinalFrictionScale;
				lateralFriction			*= m_fDestroyedLateralFrictionScale;
				roughness				+= m_fDestroyedRoughnessIncrease;
				drag					=  m_fDestroyedDrag;
			}
			else if (state == EWheelDamageState.PUNCTURED)
			{
				radius					*= m_fDamagedRadiusScale;
				longitudinalFriction	*= m_fDamagedLongitudinalFrictionScale;
				lateralFriction			*= m_fDamagedLateralFrictionScale;
				roughness				+= m_fDamagedRoughnessIncrease;
				drag					=  m_fDamagedDrag;
			}
			
			simulation.WheelSetRadiusState(m_iWheelId, radius);
			simulation.WheelTyreSetLongitudinalFrictionState(m_iWheelId, longitudinalFriction);
			simulation.WheelTyreSetLateralFrictionState(m_iWheelId, lateralFriction);
			simulation.WheelTyreSetRoughnessState(m_iWheelId, roughness);
			simulation.WheelSetRollingDrag(m_iWheelId, drag);
			
			// Need to wake physics up when wheel becomes destroyed
			if (state == EWheelDamageState.PUNCTURED || state == EWheelDamageState.DESTROYED)
				WakeUpPhysics();	
		}
		else
		{
			VehicleWheeledSimulation_SA simulation = VehicleWheeledSimulation_SA.Cast(parent.FindComponent(VehicleWheeledSimulation_SA));
			if (!simulation || !simulation.IsValid() || m_iWheelId >= simulation.WheelCount())
				return;
			
			// Update simulation
			float previousRadius 		= simulation.WheelGetRadiusState(m_iWheelId);
			float radius				= simulation.WheelGetRadius(m_iWheelId);
			float longitudinalFriction	= simulation.WheelTyreGetLongitudinalFriction(m_iWheelId);
			float lateralFriction		= simulation.WheelTyreGetLateralFriction(m_iWheelId);
			float roughness				= simulation.WheelTyreGetRoughness(m_iWheelId);
			float drag;
			
			// Only run the rest of code if state has changed
			EDamageState state = GetDamageState();
			if (state == EWheelDamageState.DESTROYED)
			{
				radius					*= m_fDestroyedRadiusScale;
				longitudinalFriction	*= m_fDestroyedLongitudinalFrictionScale;
				lateralFriction			*= m_fDestroyedLateralFrictionScale;
				roughness				+= m_fDestroyedRoughnessIncrease;
				drag					=  m_fDestroyedDrag;
			}
			else if (state == EWheelDamageState.PUNCTURED)
			{
				radius					*= m_fDamagedRadiusScale;
				longitudinalFriction	*= m_fDamagedLongitudinalFrictionScale;
				lateralFriction			*= m_fDamagedLateralFrictionScale;
				roughness				+= m_fDamagedRoughnessIncrease;
				drag					=  m_fDamagedDrag;
			}
			
			simulation.WheelSetRadiusState(m_iWheelId, radius);
			simulation.WheelTyreSetLongitudinalFrictionState(m_iWheelId, longitudinalFriction);
			simulation.WheelTyreSetLateralFrictionState(m_iWheelId, lateralFriction);
			simulation.WheelTyreSetRoughnessState(m_iWheelId, roughness);
			simulation.WheelSetRollingDrag(m_iWheelId, drag);		
			
			// Need to wake physics up when wheel becomes destroyed
			if (!float.AlmostEqual(radius, previousRadius))
				WakeUpPhysics();	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDamageSignal()
	{		
		if (m_iWheelId == -1)
			return;
		
		IEntity parent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		if (!parent)
			return;
		
		// Set TireDamage signal
		float damageState = GetDamageState();
		
		SignalsManagerComponent signalManager = SignalsManagerComponent.Cast(parent.FindComponent(SignalsManagerComponent));
		if (signalManager)
		{
			int damageSignal = signalManager.AddOrFindSignal("TireDamage" + m_iWheelId.ToString());
			if (damageSignal != -1)
				signalManager.SetSignalValue(damageSignal, damageState);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void PlayDestructionSound(EDamageState damageState)
	{
		EWheelDamageState pDS = GetPreviousDamageState();
		
		if (pDS <= EWheelDamageState.INTERMEDIARY && damageState >= EWheelDamageState.DESTROYED)
		{
			IEntity owner = GetOwner();
			
			IEntity parent = SCR_EntityHelper.GetMainParent(owner, true);
			if (!parent)
				return;
			
			SoundComponent soundComponent = SoundComponent.Cast(parent.FindComponent(SoundComponent));			
			Physics physics = owner.GetPhysics();
			if (!physics || !soundComponent)
				return;
			
			vector offset;
			if (!HasColliderNodes())
			{
				// Use center of mass of whole entity
				offset = physics.GetCenterOfMass();
				offset = owner.CoordToParent(offset);
			}
			else
			{
				// Get collider geometry
				int colliderID = -1;
				array<string> colliderNames = {};
				GetAllColliderNames(colliderNames);
				foreach (string colliderName: colliderNames)
				{
					colliderID = physics.GetGeom(colliderNames[0]);
					if (colliderID != -1)
						break;
				}
				
				if (colliderID == -1)
					return;
				
				offset = physics.GetGeomWorldPosition(colliderID);
			}
			
			offset = parent.CoordToLocal(offset);
			soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_TIRE_PUNCTURE, offset);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Wake physics up
	void WakeUpPhysics()
	{
		IEntity parent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		Physics physics = parent.GetPhysics();
		if (!physics)
			return;
		
		if (!physics.IsDynamic())
			return;
		
		if (physics.IsActive())
			return;
		
		vector centerOfMass = physics.GetCenterOfMass();
		float force = 0.01 * physics.GetMass();
		physics.ApplyImpulseAt(centerOfMass, vector.Up * force);
		physics.ApplyImpulseAt(centerOfMass, vector.Up * -force);
	}
};
