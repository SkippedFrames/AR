[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotClearAreaClass : SCR_ScenarioFrameworkSlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_ScenarioFrameworkSlotClearArea : SCR_ScenarioFrameworkSlotTask
{
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned()
	{
		m_bInitiated = true;
		if (m_ParentLayer)
		{
			m_ParentLayer.GetOnAllChildrenSpawned().Insert(AfterParentChildrenSpawned);
			m_ParentLayer.CheckAllChildrenSpawned(this);
		}

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AfterParentChildrenSpawned()
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		foreach(SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}
		
		if (m_ParentLayer)
			m_ParentLayer.GetOnAllChildrenSpawned().Remove(AfterParentChildrenSpawned);
	}
}
