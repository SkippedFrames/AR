[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PauseGameTimeToolbarAction : SCR_BaseToggleToolbarAction
{
	protected bool m_bIsGameTimePaused;
	protected ChimeraWorld m_World;
	
	//---------------------------------------------------------------------------------------------
	//--- ToDo: Use event?
	protected void UpdateGameTimePause()
	{
		if (m_bIsGameTimePaused != m_World.IsGameTimePaused())
		{
			m_bIsGameTimePaused = m_World.IsGameTimePaused();
			
			Toggle(!m_bIsGameTimePaused, !m_bIsGameTimePaused);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		return !Replication.IsRunning();
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return !Replication.IsRunning();
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PauseGameTimeEditorComponent pauseManager = SCR_PauseGameTimeEditorComponent.Cast(SCR_PauseGameTimeEditorComponent.GetInstance(SCR_PauseGameTimeEditorComponent));
		if (pauseManager)
			pauseManager.TogglePause();
		else
			m_World.PauseGameTime(!m_World.IsGameTimePaused());
	}
	
	//---------------------------------------------------------------------------------------------
	override void Track()
	{
		m_World = GetGame().GetWorld();
		m_bIsGameTimePaused = m_World.IsGameTimePaused();
		GetGame().GetCallqueue().CallLater(UpdateGameTimePause, 1, true);
		
		Toggle(!m_bIsGameTimePaused, !m_bIsGameTimePaused);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Untrack()
	{
		GetGame().GetCallqueue().Remove(UpdateGameTimePause);
	}
};
