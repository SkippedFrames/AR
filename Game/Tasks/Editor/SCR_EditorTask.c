[EntityEditorProps(category: "GameScripted/Tasks", description: "Move task.", color: "0 0 255 255")]
class SCR_EditorTaskClass: SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_EditorTask : SCR_BaseTask
{	
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	protected ETaskTextType m_TextType;
	
	[Attribute("0", category: "Editor Task")]
	protected int m_iTextIndex;
	
	[Attribute("0", UIWidgets.ComboBox, "Task completion", "", ParamEnumArray.FromEnum(EEditorTaskCompletionType) )]
	protected EEditorTaskCompletionType m_iTaskCompletionType;
	
	protected LocalizedString m_sLocationName;
	
	/*!
	Set name of location to which this task relates to.
	\param locationName Name of the location
	*/
	void SetLocationName(LocalizedString locationName)
	{
		m_sLocationName = locationName;
	}
	
	/*!
	Get name of location to which this task relates to.
	\return string locationName Name of the location
	*/
	string GetLocationName()
	{
		return m_sLocationName;
	}
	/*!
	Get type of custom texts this task should use.
	\return Type of texts
	*/
	ETaskTextType GetTextType()
	{
		return m_TextType;
	}
	/*!
	Get index of custom text from SCR_TextsTaskManagerComponent.
	\return Index from the array of texts
	*/
	int GetTextIndex()
	{
		return m_iTextIndex;
	}
	/*!
	Set index of custom text from SCR_TextsTaskManagerComponent.
	\param index Index from the array of texts
	*/
	void SetTextIndex(int index)
	{
		m_iTextIndex = index;
	}
	/*!
	Get the task completion type
	\return MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	EEditorTaskCompletionType GetTaskCompletionType()
	{
		return m_iTaskCompletionType;
	}
	/*!
	Set the task completion type
	\param newTaskCompletionType MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	void SetTaskCompletionType(EEditorTaskCompletionType newTaskCompletionType)
	{
		if (m_iTaskCompletionType == newTaskCompletionType)
			return;
		
		m_iTaskCompletionType = newTaskCompletionType;
	}
	/*!
	Get UI info of custom text for this task.
	\return UI info
	*/
	SCR_UIDescription GetInfo()
	{
		SCR_TextsTaskManagerComponent textsComponent = SCR_TextsTaskManagerComponent.GetInstance();
		if (textsComponent)
			return textsComponent.GetText(m_TextType, m_iTextIndex);
		else
			return null;
	}
	override string GetTitle()
	{
		SCR_UIName info = GetInfo();
		if (info)
			return info.GetName();
		else
			return m_sName;
	}
	override void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
	{
		SCR_UIName info = GetInfo();
		if (info)
			textWidget.SetTextFormat(info.GetName(), m_sLocationName);
		else
			textWidget.SetTextFormat(taskText, m_sLocationName);
	}
	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
	{
		SCR_UIDescription info = GetInfo();
		if (info)
			textWidget.SetTextFormat(info.GetDescription(), m_sLocationName);
		else
			textWidget.SetTextFormat(taskText);
	}
	
	protected void PopUpNotification(string prefix, bool alwaysInEditor)
	{
		//--- Get player faction (prioritize respawn faction, because it's defined even when player is waiting for respawn)
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();		
				
		if (!playerFaction)
			playerFaction = SCR_PlayerController.GetLocalMainEntityFaction();
		
		//--- Show notification when player is assigned, of the same faction, or has unlimited editor (i.e., is Game Master)
		if (IsAssignedToLocalPlayer() || playerFaction == GetTargetFaction() || (alwaysInEditor && !SCR_EditorManagerEntity.IsLimitedInstance()))
		{
			//--- SCR_PopUpNotification.GetInstance() is never null, as it creates the instance if it doesn't exist yet
			SCR_PopUpNotification.GetInstance().PopupMsg(prefix + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_sLocationName, sound: SCR_SoundEvent.TASK_SUCCEED);
		}
	}
	override protected void ShowPopUpNotification(string subtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(GetTitle(), text2: subtitle, param1: m_sLocationName);
	}
	
	/*!
	Show notification related to the task state. Delete notification is called by SCR_EditableTaskComponent
	\param taskNotification notification to show
	*/
	void ShowTaskNotification(ENotification taskNotification, bool SendOverNetwork = false)
	{
		SCR_EditableEntityComponent editableTask = SCR_EditableEntityComponent.Cast(FindComponent(SCR_EditableEntityComponent));
		if (!editableTask)
			return;
	
		int taskID = Replication.FindId(editableTask);
		
		Faction faction = GetTargetFaction();
		if (!faction)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		vector position;
	 	editableTask.GetPos(position);
		
		int factionIndex = factionManager.GetFactionIndex(faction);
		
		//Send local GM
		if (!SendOverNetwork)
		{
			if (taskNotification == ENotification.EDITOR_TASK_PLACED)
				GetGame().GetCallqueue().CallLater(DelayedPlacedNotification, 1, false, position, taskID, factionIndex);
			else
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(taskNotification, position, taskID, factionIndex);
		}
		else 
		{
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(taskNotification, position, taskID, factionIndex);
		}
	}
	
	protected override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		//--- Delete the task once it's finished (ToDo: Keep it, but hide it in the editor once completed tasks can be shown in the task list)
		RplComponent rplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if ((!rplComponent || rplComponent.Role() == RplRole.Authority) && (newState == SCR_TaskState.FINISHED || newState == SCR_TaskState.CANCELLED))
		{
			if (GetTaskManager())
				GetTaskManager().DeleteTask(this);
		}
	}
	
	protected void DelayedPlacedNotification(vector position, int taskID, int factionIndex)
	{
		SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_TASK_PLACED, position, taskID, factionIndex);
	}
	
	
	override void Create(bool showMsg = true)
	{
		super.Create(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_AVAILABLE_TEXT, false);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_PLACED);	
	}
	override void Finish(bool showMsg = true)
	{
		super.Finish(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_COMPLETED_TEXT, true);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_COMPLETED);
			
	}
	override void Fail(bool showMsg = true)
	{
		super.Fail(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_FAILED_TEXT, true);
			
		ShowTaskNotification(ENotification.EDITOR_TASK_FAILED);
			
	}
	override void Cancel(bool showMsg = true)
	{
		super.Cancel(showMsg);
		
		if (showMsg)
			PopUpNotification(TASK_CANCELLED_TEXT, true);
		
		ShowTaskNotification(ENotification.EDITOR_TASK_CANCELED);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool RplLoad(ScriptBitReader reader)
	{
		Deserialize(reader);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool RplSave(ScriptBitWriter writer)
	{
		Serialize(writer);
		return true;
	}
};
