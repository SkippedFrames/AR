//------------------------------------------------------------------------------------------------
[EntityEditorProps(insertable: false)]
class SCR_MilitaryBaseManagerClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_MilitaryBaseManager : GenericEntity
{
	protected static SCR_MilitaryBaseManager s_Instance = null;

	protected ref array<SCR_MilitaryBaseComponent> m_aBases = {};
	protected ref array<SCR_MilitaryBaseLogicComponent> m_aLogicComponents = {};
	protected ref array<int> m_aAvailableCallsignIds;

	protected ref ScriptInvoker m_OnBaseFactionChanged;
	protected ref ScriptInvoker m_OnLogicRegisteredInBase;
	protected ref ScriptInvoker m_OnServiceRegisteredInBase;
	protected ref ScriptInvoker m_OnServiceUnregisteredInBase;

	//------------------------------------------------------------------------------------------------
	static SCR_MilitaryBaseManager GetInstance(bool createNew = true)
	{
		if (!s_Instance && createNew)
		{
			s_Instance = SCR_MilitaryBaseManager.Cast(GetGame().SpawnEntity(SCR_MilitaryBaseManager, GetGame().GetWorld()));

			if (s_Instance)
				s_Instance.SetupInvokers();
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseFactionChanged(Faction faction, SCR_MilitaryBaseComponent base)
	{
		if (m_OnBaseFactionChanged)
			m_OnBaseFactionChanged.Invoke(base, faction);
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceRegisteredInBase(SCR_ServicePointComponent service, SCR_MilitaryBaseComponent base)
	{
		if (m_OnServiceRegisteredInBase)
			m_OnServiceRegisteredInBase.Invoke(base, service);
	}

	//------------------------------------------------------------------------------------------------
	void OnLogicRegisteredInBase(SCR_MilitaryBaseLogicComponent logic, SCR_MilitaryBaseComponent base)
	{
		if (m_OnLogicRegisteredInBase)
			m_OnLogicRegisteredInBase.Invoke(base, logic);
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceUnregisteredInBase(SCR_ServicePointComponent service, SCR_MilitaryBaseComponent base)
	{
		if (m_OnServiceUnregisteredInBase)
			m_OnServiceUnregisteredInBase.Invoke(base, service);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnBaseFactionChanged()
	{
		if (!m_OnBaseFactionChanged)
			m_OnBaseFactionChanged = new ScriptInvoker();

		return m_OnBaseFactionChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnServiceRegisteredInBase()
	{
		if (!m_OnServiceRegisteredInBase)
			m_OnServiceRegisteredInBase = new ScriptInvoker();

		return m_OnServiceRegisteredInBase;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnLogicRegisteredInBase()
	{
		if (!m_OnLogicRegisteredInBase)
			m_OnLogicRegisteredInBase = new ScriptInvoker();

		return m_OnLogicRegisteredInBase;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnServiceUnregisteredInBase()
	{
		if (!m_OnServiceUnregisteredInBase)
			m_OnServiceUnregisteredInBase = new ScriptInvoker();

		return m_OnServiceUnregisteredInBase;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupInvokers()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

		if (core)
			core.Event_OnEntityTransformChanged.Insert(OnEntityTransformChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! When a base or logic component is moved, recalculate registered members
	protected void OnEntityTransformChanged(notnull SCR_EditableEntityComponent editComponent)
	{
		array<IEntity> queue = {editComponent.GetOwner()};
		SCR_MilitaryBaseComponent baseComponent;
		SCR_MilitaryBaseLogicComponent logicComponent;
		IEntity processedEntity;
		IEntity nextInHierarchy;
		bool checkeEditableEntityComponent;

		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);

			if (!checkeEditableEntityComponent || !SCR_EditableEntityComponent.Cast(processedEntity.FindComponent(SCR_EditableEntityComponent)))
			{
				baseComponent = SCR_MilitaryBaseComponent.Cast(processedEntity.FindComponent(SCR_MilitaryBaseComponent));
				logicComponent = SCR_MilitaryBaseLogicComponent.Cast(processedEntity.FindComponent(SCR_MilitaryBaseLogicComponent));

				if (baseComponent)
					OnBaseTransformChanged(baseComponent);

				if (logicComponent)
					OnLogicTransformChanged(logicComponent);
			}

			nextInHierarchy = processedEntity.GetChildren();

			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Base has been moved, recalculate members in radius
	protected void OnBaseTransformChanged(notnull SCR_MilitaryBaseComponent baseComponent)
	{
		vector position = baseComponent.GetOwner().GetOrigin();
		int distanceLimit = baseComponent.GetRadius() * baseComponent.GetRadius();

		foreach (SCR_MilitaryBaseLogicComponent logicComponent : m_aLogicComponents)
		{
			if (vector.DistanceSqXZ(position, logicComponent.GetOwner().GetOrigin()) > distanceLimit)
			{
				baseComponent.UnregisterLogicComponent(logicComponent);
				logicComponent.UnregisterBase(baseComponent);
			}
			else
			{
				baseComponent.RegisterLogicComponent(logicComponent);
				logicComponent.RegisterBase(baseComponent);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Logic has been moved, recalculate members in radius of bases
	protected void OnLogicTransformChanged(notnull SCR_MilitaryBaseLogicComponent logicComponent)
	{
		vector position = logicComponent.GetOwner().GetOrigin();
		int radius;

		foreach (SCR_MilitaryBaseComponent baseComponent : m_aBases)
		{
			radius = baseComponent.GetRadius();

			if (vector.DistanceSqXZ(position, baseComponent.GetOwner().GetOrigin()) > (radius * radius))
			{
				baseComponent.UnregisterLogicComponent(logicComponent);
				logicComponent.UnregisterBase(baseComponent);
			}
			else
			{
				baseComponent.RegisterLogicComponent(logicComponent);
				logicComponent.RegisterBase(baseComponent);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadAvailableCallsigns()
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		m_aAvailableCallsignIds = {};
		array<Faction> allFactions = {};
		array<SCR_Faction> playableFactions = {};
		fManager.GetFactionsList(allFactions);
		array<int> callsignIndexesSource = {};

		foreach (Faction faction : allFactions)
		{
			SCR_Faction factionCast = SCR_Faction.Cast(faction);

			if (factionCast && factionCast.IsPlayable())
				playableFactions.Insert(factionCast);
		}

		if (playableFactions.IsEmpty())
			return;

		// All playable factions need to have a callsign with the same ID registered in order for it to be used
		// Grab the indexes available from the first valid faction in the list so we can compare it with the other factions
		callsignIndexesSource = playableFactions[0].GetBaseCallsignIndexes();
		bool canAdd;

		foreach (int indexSource : callsignIndexesSource)
		{
			canAdd = true;

			// Ignore the index unless it's registered in all other factions
			foreach (int i, SCR_Faction faction : playableFactions)
			{
				// No need to check it against the original source faction
				if (i == 0)
					continue;

				if (!faction.GetBaseCallsignByIndex(indexSource))
				{
					canAdd = false;
					break;
				}
			}

			if (!canAdd)
				continue;

			m_aAvailableCallsignIds.Insert(indexSource);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		m_aBases.Insert(base);

		vector basePosition = base.GetOwner().GetOrigin();
		int baseRadius = base.GetRadius();
		int distanceLimit = baseRadius * baseRadius;

		if (Replication.IsServer())
		{
			// If not done already, grab all available callsign IDs
			if (!m_aAvailableCallsignIds)
				LoadAvailableCallsigns();

			// Assign random available callsign
			if (m_aAvailableCallsignIds && !m_aAvailableCallsignIds.IsEmpty())
			{
				int i = m_aAvailableCallsignIds.GetRandomIndex();

				base.SetCallsignIndexAutomatic(m_aAvailableCallsignIds[i]);
				m_aAvailableCallsignIds.Remove(i);
			}
		}

		foreach (SCR_MilitaryBaseLogicComponent component : m_aLogicComponents)
		{
			if (vector.DistanceSqXZ(component.GetOwner().GetOrigin(), basePosition) <= distanceLimit)
			{
				base.RegisterLogicComponent(component);
				component.RegisterBase(base);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Static version so we don't create a new instance upon game shutdown, which would cause script leak
	static void UnregisterBaseStatic(notnull SCR_MilitaryBaseComponent base)
	{
		if (s_Instance)
			s_Instance.UnregisterBase(base);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (m_aAvailableCallsignIds)
			m_aAvailableCallsignIds.Insert(base.GetCallsign());

		m_aBases.RemoveItem(base);
	}

	//------------------------------------------------------------------------------------------------
	int GetBases(out array<SCR_MilitaryBaseComponent> bases)
	{
		if (bases)
			return bases.Copy(m_aBases);
		else
			return m_aBases.Count();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		m_aLogicComponents.Insert(component);

		vector position = component.GetOwner().GetOrigin();
		int radius;

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			radius = base.GetRadius();

			if (vector.DistanceSqXZ(base.GetOwner().GetOrigin(), position) <= (radius * radius))
			{
				base.RegisterLogicComponent(component);
				component.RegisterBase(base);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Static version so we don't create a new instance upon game shutdown, which would cause script leak
	static void UnregisterLogicComponentStatic(notnull SCR_MilitaryBaseLogicComponent component)
	{
		if (s_Instance)
			s_Instance.UnregisterLogicComponent(component);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		m_aLogicComponents.RemoveItem(component);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryBaseManager()
	{
		if (s_Instance == this)
			s_Instance = null;

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

		if (core)
			core.Event_OnEntityTransformChanged.Remove(OnEntityTransformChanged);
	}
}
