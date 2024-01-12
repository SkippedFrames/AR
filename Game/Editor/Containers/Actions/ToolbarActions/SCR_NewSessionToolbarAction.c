[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_NewSessionToolbarAction: SCR_EditorToolbarAction
{
	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//--- Disabled in MP, SCR_EndGameToolbarAction is used instead
		if (Replication.IsRunning())
			return false;
		
		//--- Disallow if editor save struct is not configured
		SCR_SaveLoadComponent saveLoadComponent = SCR_SaveLoadComponent.GetInstance();
		return saveLoadComponent && saveLoadComponent.ContainsStruct(SCR_EditorStruct);
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		new SCR_NewSessionDialog();
	}
};

class SCR_NewSessionDialog: SCR_ConfigurableDialogUi
{
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		GameStateTransitions.RequestScenarioRestart();
	}
	
	//---------------------------------------------------------------------------------------------
	void SCR_NewSessionDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "session_new", this);
	}
};
