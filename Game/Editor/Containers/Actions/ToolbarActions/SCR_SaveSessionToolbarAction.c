[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SaveSessionToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the operation will always bring up a save dialog.")]
	protected bool m_bSaveAs;
	
	[Attribute(ESaveType.USER.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eSaveType;
	
	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		//--- The action opens local UI
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//--- Disallow on client, or in MP for "Save" version ("Save As" is allowed in MP)
		if (!Replication.IsServer() || (!m_bSaveAs && Replication.IsRunning()))
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
		if (m_bSaveAs || !GetGame().GetSaveManager().OverrideCurrentSave(m_eSaveType))
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.EditorSaveDialog);
	}
};
