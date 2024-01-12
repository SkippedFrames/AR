[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotExtractionClass : SCR_ScenarioFrameworkSlotTaskClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotExtraction : SCR_ScenarioFrameworkSlotTask
{
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
        if (m_eActivationType != activation)
			return;
			
		super.Init(area, activation);
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		if (!SCR_PlayersPresentTriggerEntity.Cast(m_Entity))
			return;

		SCR_PlayersPresentTriggerEntity.Cast(m_Entity).SetOwnerFaction(factionManager.GetFactionByKey(m_sFactionKey));
	}
};
