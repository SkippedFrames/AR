[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RewindToolbarAction: SCR_EditorToolbarAction
{
	[Attribute(ESaveType.EDITOR.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eSaveType;
	
	[Attribute()]
	protected string m_sCustomName;
	
	[Attribute(desc: "When enabled, the action will delete the rewind point instead of loading it.")]
	protected bool m_bIsDelete;
	
	protected SCR_RewindComponent m_RewindManager;
	protected SCR_BaseToolbarEditorUIComponent m_Toolbar;
	
	protected void OnSaveChanged(ESaveType type, string fileName)
	{
		if (m_Toolbar && type == ESaveType.EDITOR)
			m_Toolbar.MarkForRefresh();
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (Replication.IsRunning())
			return false;
		
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		return rewindManager && rewindManager.HasRewindPoint();
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (m_bIsDelete)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			rewindManager.DeleteRewindPoint();
			
			SCR_PauseGameTimeEditorComponent pauseManager = SCR_PauseGameTimeEditorComponent.Cast(SCR_PauseGameTimeEditorComponent.GetInstance(SCR_PauseGameTimeEditorComponent));
			if (pauseManager)
				pauseManager.SetPause(true);
		}
		else
		{
			new SCR_RewindDialog(m_eSaveType, m_sCustomName);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnInit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		m_Toolbar = toolbar;
		GetGame().GetSaveManager().GetOnSaved().Insert(OnSaveChanged);
		GetGame().GetSaveManager().GetOnDeleted().Insert(OnSaveChanged);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnExit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		GetGame().GetSaveManager().GetOnSaved().Insert(OnSaveChanged);
		GetGame().GetSaveManager().GetOnDeleted().Insert(OnSaveChanged);
	}
};
