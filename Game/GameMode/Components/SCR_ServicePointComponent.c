class SCR_ServicePointComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_ServicePointComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute(SCR_EServicePointType.SUPPLY_DEPOT.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EServicePointType))]
	protected SCR_EServicePointType m_eType;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.SearchComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eBuildingLabel;

	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of entity which will hold the UI data in case this entity gets streamed out.", "et")]
	protected ResourceName m_sDelegatePrefab;

	// This var will later represent a service status - functional, broken or any other...
	[RplProp(onRplName: "ServiceStateChanged")]
	protected SCR_EServicePointStatus m_eServiceState = SCR_EServicePointStatus.OFFLINE;

	protected SCR_FactionAffiliationComponent m_FactionControl;
	protected static bool s_bSpawnAsOffline;
	protected ref ScriptInvoker m_OnServiceStateChanged;
	protected SCR_ServicePointDelegateComponent m_Delegate;

	//------------------------------------------------------------------------------------------------
	ResourceName GetDelegatePrefab()
	{
		return m_sDelegatePrefab;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnServiceStateChanged()
	{
		if (!m_OnServiceStateChanged)
			m_OnServiceStateChanged = new ScriptInvoker();

		return m_OnServiceStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetType()
	{
		return m_eType;
	}

	//------------------------------------------------------------------------------------------------
	EEditableEntityLabel GetLabel()
	{
		return m_eBuildingLabel;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EServicePointStatus GetServiceState()
	{
		return m_eServiceState;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the current operational state of the service
	void SetServiceState(SCR_EServicePointStatus state)
	{
		m_eServiceState = state;
		if (m_OnServiceStateChanged)
			m_OnServiceStateChanged.Invoke(state, this);

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Called from RplProp event
	void ServiceStateChanged()
	{
		if (m_OnServiceStateChanged)
			m_OnServiceStateChanged.Invoke(m_eServiceState, this);
	}

	//------------------------------------------------------------------------------------------------
	void SetDelegate(notnull SCR_ServicePointDelegateComponent delegate)
	{
		m_Delegate = delegate;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base)
	{
		if (m_OnServiceStateChanged)
			m_OnServiceStateChanged.Invoke(SCR_EServicePointStatus.DELETED, this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
	}

	//------------------------------------------------------------------------------------------------
	void RegisterService()
	{
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
		if (!baseManager)
			return;

		baseManager.RegisterLogicComponent(this);

		if (m_FactionControl)
		{
			Faction faction = m_FactionControl.GetAffiliatedFaction();
			if (!faction)
				faction = m_FactionControl.GetDefaultAffiliatedFaction();

			OnFactionChanged(null, null, faction);
			m_FactionControl.GetOnFactionChanged().Insert(OnFactionChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The service will not be registered on init. For an example for Free Roam building - the registration will happen once the service is fully build.
	static void SpawnAsOffline(bool val)
	{
		s_bSpawnAsOffline = val;
	}

	//------------------------------------------------------------------------------------------------
	//! Switch service to online status.
	void SetServiceOnline()
	{
		SetServiceState(SCR_EServicePointStatus.ONLINE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_FactionControl = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		super.OnPostInit(owner);
		RegisterService();

		if (s_bSpawnAsOffline)
		{
			s_bSpawnAsOffline = false;

			SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true).FindComponent(SCR_EditorLinkComponent));
			if (linkComponent)
				linkComponent.GetOnLinkedEntitiesSpawned().Insert(SetServiceOnline);

			return;
		}

		SetServiceState(SCR_EServicePointStatus.ONLINE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (m_Delegate)
			RplComponent.DeleteRplEntity(m_Delegate.GetOwner(), false);
	}
}

enum SCR_EServicePointType
{
	ARMORY,
	LIGHT_VEHICLE_DEPOT,
	HEAVY_VEHICLE_DEPOT,
	FIELD_HOSPITAL,
	BARRACKS,
	RADIO_ANTENNA,
	HELIPAD,
	FUEL_DEPOT,
	REPAIR_DEPOT,
	SUPPLY_DEPOT,
}

enum SCR_EServicePointStatus
{
	OFFLINE,
	ONLINE,
	UNDER_CONSTRUCTION,
	BROKEN,
	DELETED
}
