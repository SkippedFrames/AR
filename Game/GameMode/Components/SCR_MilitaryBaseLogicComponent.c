class SCR_MilitaryBaseLogicComponentClass : ScriptComponentClass
{
};

class SCR_MilitaryBaseLogicComponent : ScriptComponent
{
	protected ref array<SCR_MilitaryBaseComponent> m_aBases = {};

	//------------------------------------------------------------------------------------------------
	void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (m_aBases.Contains(base))
			return;

		m_aBases.Insert(base);
		OnBaseRegistered(base);
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		if (base.GetOwner() == GetOwner())
			return;

		if (m_aBases.Count() == 1)
			OnBaseFactionChanged(base.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_aBases.Contains(base))
			return;

		m_aBases.RemoveItem(base);
		OnBaseUnregistered(base);
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base)
	{
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
	void OnBaseFactionChanged(Faction faction)
	{
		FactionAffiliationComponent factionControl = FactionAffiliationComponent.Cast(GetOwner().FindComponent(FactionAffiliationComponent));

		if (!factionControl || factionControl.GetAffiliatedFaction() == faction)
			return;

		factionControl.SetAffiliatedFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Check for play mode again in case init event was set from outside of this class
		if (!GetGame().InPlayMode())
			return;

		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();

		if (baseManager)
			baseManager.RegisterLogicComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryBaseLogicComponent()
	{
		SCR_MilitaryBaseManager.UnregisterLogicComponentStatic(this);

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (base)
				base.UnregisterLogicComponent(this);
		}
	}
};
