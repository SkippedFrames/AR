enum SCR_ERotorDamageState: EDamageState
{
	MINOR = 3,
	MAJOR = 4,
	CRITICAL = 5
}

class SCR_RotorHitZone : SCR_DestructibleHitzone
{
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Auto, desc: "Rotor ID", category: "Rotor Damage")]
	protected int m_iRotorId;

	[Attribute(desc: "Rotor collision detection root point", category: "Rotor Damage")]
	protected ref PointInfo m_RotorPointInfo;

	[Attribute(uiwidget: UIWidgets.Slider, desc: "Rotor radius measured up from root in Rotor Info space\n[m]", params: "0.5 50 0.01", category: "Helicopter Damage")]
	protected float m_fRotorRadius;

	[Attribute(uiwidget: UIWidgets.Slider, desc: "Rotor thickness measured up from root in Rotor Info space\n[m]", params: "0.1 10 0.01", category: "Helicopter Damage")]
	protected float m_fRotorThickness;

	[Attribute(uiwidget: UIWidgets.Slider, desc: "Average blade width measured up from side to side at the tip of the blade\n[m]", params: "0.1 1000 0.01", category: "Helicopter Damage")]
	protected float m_fBladeWidth;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Amount of blades on this rotor", category: "Helicopter Damage")]
	protected int m_iBladeCount;

	[Attribute(defvalue: "0.9", uiwidget: UIWidgets.Auto, desc: "Efficiency with minor damage state", category: "Rotor Damage")]
	protected float m_fEfficiencyMinor;

	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.Auto, desc: "Efficiency with major damage state", category: "Rotor Damage")]
	protected float m_fEfficiencyMajor;

	[Attribute(defvalue: "0.6", uiwidget: UIWidgets.Auto, desc: "Efficiency with critical damage state", category: "Rotor Damage")]
	protected float m_fEfficiencyCritical;
	
	[Attribute(EVehicleHitZoneGroup.DRIVE_TRAIN.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;

	protected VehicleHelicopterSimulation m_Simulation;

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		if (m_RotorPointInfo)
			m_RotorPointInfo.Init(pOwnerEntity);

		SetRotorIndex(m_iRotorId);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculates the amount of damage a hitzone will receive.
	\param damageType - damage type
	\param rawDamage - incoming damage, without any modifiers taken into account
	\param hitEntity - damaged entity
	\param struckHitZone - hitzone to damage
	\param damageSource - projectile
	\param damageSourceGunner - damage source instigator
	\param damageSourceParent - damage source parent entity (soldier, vehicle)
	\param hitMaterial - hit surface physics material
	\param colliderID - collider ID - if it exists
	\param hitTransform - hit position, direction and normal
	\param impactVelocity - projectile velocity in time of impact
	\param nodeID - bone index in mesh obj
	\param isDOT - true if this is a calculation for DamageOverTime
	*/
	override float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, notnull Instigator instigator, const GameMaterial hitMaterial, int colliderID, inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		if (!isDOT && damageType == EDamageType.KINETIC && m_RotorPointInfo && Math.RandomFloat01() > RotorHitChance(hitTransform))
			return 0;

		return super.ComputeEffectiveDamage(damageType, rawDamage, hitEntity, struckHitZone, damageSource, instigator, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates the chance the rotor was hit by a bullet between 0 and 1
	float RotorHitChance(vector hitTransform[3])
	{
		if (!m_Simulation || m_Simulation.RotorGetState(GetRotorIndex()) != RotorState.SPINNING)
			return 1;

		if (m_fBladeWidth <= 0 || m_iBladeCount < 1)
			return 1;

		vector transform[4];
		m_RotorPointInfo.GetTransform(transform);
		vector relativePosition = hitTransform[0].InvMultiply4(transform);

		float distanceFromAxis = vector.DistanceXZ(relativePosition, vector.Zero);
		if (distanceFromAxis == 0)
			return 1;

		return m_fBladeWidth * m_iBladeCount / (Math.PI2 * distanceFromAxis);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();

		UpdateRotorState();
	}
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	PointInfo GetPointInfo()
	{
		return m_RotorPointInfo;
	}

	//------------------------------------------------------------------------------------------------
	int GetRotorIndex()
	{
		return m_iRotorId;
	}

	//------------------------------------------------------------------------------------------------
	//! Register rotor hitzone with damage manager
	void SetRotorIndex(int id)
	{
		m_iRotorId = id;
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;

		// Vehicle.GetDamageManager() may not return damage manager during hitzone init
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (damageManager)
			damageManager.RegisterVehicleHitZone(this);

		m_Simulation = VehicleHelicopterSimulation.Cast(vehicle.FindComponent(VehicleHelicopterSimulation));
	}

	//------------------------------------------------------------------------------------------------
	void UpdateRotorState()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.GetDamageManager());
		if (damageManager)
			damageManager.UpdateVehicleState();
	}

	//------------------------------------------------------------------------------------------------
	//! Set wheel parameters when damage exceeds thresholds
	float GetEfficiency()
	{
		float efficiency = 1;

		SCR_ERotorDamageState state = GetDamageState();
		if (state == SCR_ERotorDamageState.DESTROYED)
			efficiency = 0;
		else if (state == SCR_ERotorDamageState.CRITICAL)
			efficiency = m_fEfficiencyCritical;
		else if (state == SCR_ERotorDamageState.MAJOR)
			efficiency = m_fEfficiencyMajor;
		else if (state == SCR_ERotorDamageState.MINOR)
			efficiency = m_fEfficiencyMinor;

		return efficiency;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if rotor is spinning
	bool IsSpinning()
	{
		return m_Simulation && m_Simulation.RotorGetState(m_iRotorId) == RotorState.SPINNING;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if world position is within rotor disc
	bool HasCollision(vector worldPosition)
	{
		if (!m_RotorPointInfo)
			return false;

		if (m_fRotorRadius <= 0)
			return false;

		if (m_fRotorThickness <= 0)
			return false;

		if (m_Simulation && m_Simulation.RotorGetState(m_iRotorId) != RotorState.SPINNING)
			return false;

		vector transform[4];
		m_RotorPointInfo.GetTransform(transform);
		vector relativePosition = worldPosition.InvMultiply4(transform);

		// Check if position is below rotor or above rotor thickness
		if (m_fRotorThickness > 0 && relativePosition[1] < 0 || relativePosition[1] > m_fRotorThickness)
			return false;

		// Check if position is inside rotor cylinder
		return vector.DistanceSqXZ(relativePosition, vector.Zero) <= m_fRotorRadius * m_fRotorRadius;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Draw helper visualization
	override void DrawDebug()
	{
		if (!m_RotorPointInfo)
			return;

		if (m_fRotorRadius <= 0)
			return;

		if (m_fRotorThickness <= 0)
			return;

		vector transform[4];
		m_RotorPointInfo.GetWorldTransform(transform);
		vector transformTop[4] = transform;
		transformTop[3] = transformTop[3] + transformTop[1] * m_fRotorThickness;

		ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;

		//! Cyan: thickness of the rotor
		Shape.CreateArrow(transform[3], transformTop[3], 0, Color.DARK_MAGENTA, shapeFlags);

		//! Red: lower plane of the rotor
		SCR_Shape.DrawCircle(transform, m_fRotorRadius, Color.DARK_RED, COLOR_RED_A, shapeFlags);
		Shape.CreateArrow(transform[3] - transform[0] * m_fRotorRadius, transform[3], 0, Color.DARK_RED, shapeFlags);
		Shape.CreateArrow(transform[3] + transform[0] * m_fRotorRadius, transform[3], 0, Color.DARK_RED, shapeFlags);
		Shape.CreateArrow(transform[3] - transform[2] * m_fRotorRadius, transform[3], 0, Color.DARK_RED, shapeFlags);
		Shape.CreateArrow(transform[3] + transform[2] * m_fRotorRadius, transform[3], 0, Color.DARK_RED, shapeFlags);

		//! Blue: upper plane of the rotor
		SCR_Shape.DrawCircle(transformTop, m_fRotorRadius, Color.DARK_BLUE, COLOR_BLUE_A, shapeFlags);
		Shape.CreateArrow(transformTop[3] - transform[0] * m_fRotorRadius, transformTop[3], 0, Color.DARK_BLUE, shapeFlags);
		Shape.CreateArrow(transformTop[3] + transform[0] * m_fRotorRadius, transformTop[3], 0, Color.DARK_BLUE, shapeFlags);
		Shape.CreateArrow(transformTop[3] - transform[2] * m_fRotorRadius, transformTop[3], 0, Color.DARK_BLUE, shapeFlags);
		Shape.CreateArrow(transformTop[3] + transform[2] * m_fRotorRadius, transformTop[3], 0, Color.DARK_BLUE, shapeFlags);
	}
#endif
}
