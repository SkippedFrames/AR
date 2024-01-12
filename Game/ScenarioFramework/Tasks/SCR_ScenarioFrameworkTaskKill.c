//------------------------------------------------------------------------------------------------
class SCR_TaskKillClass: SCR_ScenarioFrameworkTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskKill : SCR_ScenarioFrameworkTask
{	
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset || !m_SupportEntity)
			return;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Asset.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_SupportEntity.FinishTask(this);				
	}
	
	//------------------------------------------------------------------------------------------------
	override void RehookTaskSubject(IEntity object)
	{
		if (!object)
			return;
		
		m_Asset = object;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Asset.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskKillSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Kill support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		if (!m_Asset)
			return;		
			
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Asset.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
	}
}
