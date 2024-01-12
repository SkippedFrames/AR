class SCR_FuelHitZone : SCR_DestructibleHitzone
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Auto, desc: "Fuel Tank ID", category: "Fuel Damage Config")]
	protected int m_iFuelTankID;

	[Attribute(EVehicleHitZoneGroup.FUEL_TANKS.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;

	protected SCR_FuelNode m_FuelTank;

	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		SetFuelNodeID(m_iFuelTankID);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateFuelTankState()
	{
		if (!m_FuelTank)
			return;

		EDamageState state = GetDamageState();
		float health = GetDamageStateThreshold(state);
		m_FuelTank.SetHealth(health);
	}

	//------------------------------------------------------------------------------------------------
	SCR_FuelNode GetFuelTank()
	{
		return m_FuelTank;
	}

	//------------------------------------------------------------------------------------------------
	int GetFuelTankID()
	{
		return m_iFuelTankID;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign hitzone to a simulation fuel tank
	void SetFuelNodeID(int fuelNodeID)
	{
		m_iFuelTankID = fuelNodeID;

		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return;

		SCR_FuelNode scrFuelNode;
		array<BaseFuelNode> fuelNodes = {};
		fuelManager.GetFuelNodesList(fuelNodes);
		foreach (BaseFuelNode fuelNode : fuelNodes)
		{
			scrFuelNode = SCR_FuelNode.Cast(fuelNode);
			if (scrFuelNode && scrFuelNode.GetFuelTankID() == m_iFuelTankID)
			{
				m_FuelTank = scrFuelNode;
				break;
			}
		}

		UpdateFuelTankState();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();

		UpdateFuelTankState();
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	override float GetSecondaryExplosionScale()
	{
		// Truck addon fuel cargo that does not have its own health
		float damage;
		if (GetBaseDamageMultiplier() == 0 && GetHitZoneContainer().GetDefaultHitZone() == this)
			damage = 1;
		else
		 	damage = 1 - GetDamageStateThreshold(GetDamageState());

		// TODO: Curve based on fuel to air ratio
		return m_FuelTank.GetFuel() * damage;
	}
}
