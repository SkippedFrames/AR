[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_RewindComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RewindComponent : SCR_BaseGameModeComponent
{
	[Attribute(ESaveType.EDITOR.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eSaveType;
	
	[Attribute(desc: "Name of the save file. Optional, leave empty to use default.")]
	protected string m_sCustomName;
	
	//---------------------------------------------------------------------------------------
	/*!
	\return Local instance of this component
	*/
	static SCR_RewindComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return SCR_RewindComponent.Cast(gameMode.FindComponent(SCR_RewindComponent));
		else
			return null;
	}

	//---------------------------------------------------------------------------------------
	/*!
	\return Check if a rewind point exists for this session.
	*/
	bool HasRewindPoint()
	{
		return Replication.IsServer() && GetGame().GetSaveManager().FileExists(m_eSaveType, m_sCustomName);
	}

	//---------------------------------------------------------------------------------------
	/*!
	Create a rewind point for this session.
	*/
	void CreateRewindPoint()
	{
		if (Replication.IsServer() && !HasRewindPoint())
			GetGame().GetSaveManager().Save(m_eSaveType, m_sCustomName);
	}
	
	//---------------------------------------------------------------------------------------
	/*!
	Remove existing rewind point for this session.
	*/
	void DeleteRewindPoint()
	{
		if (Replication.IsServer())
			GetGame().GetSaveManager().Delete(m_eSaveType, m_sCustomName);
	}

	//---------------------------------------------------------------------------------------
	/*!
	Rewind to existing rewing point of this session.
	*/
	void Rewind()
	{
		if (Replication.IsServer())
			GetGame().GetSaveManager().RestartAndLoad(m_eSaveType, m_sCustomName);
	}
	
	//---------------------------------------------------------------------------------------
	protected void OnSaved(ESaveType type, string fileName)
	{
		if (type == ESaveType.USER)
			GetGame().GetCallqueue().CallLater(DeleteRewindPoint, 1); //--- Cannot call instantly, session storage is still locked by saving a file
	}
	
	//---------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!SCR_Global.IsEditMode(owner))
			GetGame().GetSaveManager().GetOnSaved().Insert(OnSaved);
	}
};

class SCR_RewindDialog: SCR_ConfigurableDialogUi
{
	protected ESaveType m_eSaveType;
	protected string m_sCustomName;
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{		
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		rewindManager.Rewind();
	}
	
	//---------------------------------------------------------------------------------------------
	void SCR_RewindDialog(ESaveType saveType, string customName)
	{
		m_eSaveType = saveType;
		m_sCustomName = customName;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "rewind", this);
	}
};
