[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeCombatOpsManagerClass : SCR_GameModeSFManagerClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------

class SCR_GameModeCombatOpsManager : SCR_GameModeSFManager
{	
	override void LoadHeaderSettings()
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (!header)
			return;
		
		SCR_MissionHeaderCombatOps headerCombatOps = SCR_MissionHeaderCombatOps.Cast(header);
		if (!headerCombatOps)
			return;
		
		if (headerCombatOps.m_iMaxNumberOfTasks != -1)
			m_iMaxNumberOfTasks = headerCombatOps.m_iMaxNumberOfTasks;
		
		if (!headerCombatOps.m_aTaskTypesAvailable.IsEmpty())
		{
			m_aTaskTypesAvailable.Clear();
			foreach (SCR_ScenarioFrameworkTaskType taskType : headerCombatOps.m_aTaskTypesAvailable)
			{
				m_aTaskTypesAvailable.Insert(taskType);
			}
		}
	};
}
