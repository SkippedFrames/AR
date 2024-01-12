[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_BaseFactionTriggerEntityClass: SCR_BaseTriggerEntityClass
{
};
class SCR_BaseFactionTriggerEntity: SCR_BaseTriggerEntity
{
	[Attribute(desc: "Faction which is used for area control calculation.", category: "Faction Trigger")]
	protected FactionKey m_sOwnerFactionKey;
	
	protected Faction m_OwnerFaction;
	
	/*!
	Set owner faction.
	\param faction Owner faction
	*/
	void SetOwnerFaction(Faction faction)
	{
		m_OwnerFaction = faction;
	}
	
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (!m_OwnerFaction || !IsAlive(ent))
			return false;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		return factionAffiliation && factionAffiliation.GetAffiliatedFaction() == m_OwnerFaction;
	}
	override protected void EOnInit(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_sOwnerFactionKey);
	}
	void SCR_BaseFactionTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
};
