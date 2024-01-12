[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableTaskComponentClass: SCR_EditableDescriptorComponentClass
{
	[Attribute("#AR-Tasks_Objective", desc: "Name of objective type eg: Attack objective. Used among in, among other things, in notifications", category: "Visualization")]
	protected  LocalizedString m_sObjectiveTypeName;
	
	/*!
	Get objective type name. The name is the same for each prefab objective type
	\return Objective type name
	*/
	LocalizedString GetObjectiveTypeName()
	{
		return m_sObjectiveTypeName;
	}
	
};

/** @ingroup Editable_Entities
*/

/*!
Editable SCR_BaseTask.
*/
class SCR_EditableTaskComponent: SCR_EditableDescriptorComponent
{	
	protected SCR_EditorTask m_Task;
	protected Faction m_TargetFaction;
	protected int m_iTextIndex;
	
	/*!
	Reveal the task to all players.
	*/
	void ActivateTask()
	{
		if (!GetTaskManager())
			return;
		
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
		if (supportEntity)
			supportEntity.SetTargetFaction(m_Task, m_TargetFaction);
	}
	/*!
	Check if the task is actived, i.e., shown to players.
	\return True when actived
	*/
	bool IsTaskActivated()
	{
		return m_Task.GetTargetFaction() == m_TargetFaction;
	}
	
	/*!
	Get type of custom texts this task should use.
	\return Type of texts
	*/
	ETaskTextType GetTextType()
	{
		return m_Task.GetTextType();
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
		SetTextIndexBroadcast(index);
		Rpc(SetTextIndexBroadcast, index);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTextIndexBroadcast(int index)
	{
		m_iTextIndex = index;
		m_Task.SetTextIndex(index);
		UpdateText();
	}
	
	/*!
	Get the task completion type
	\return MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	EEditorTaskCompletionType GetTaskCompletionType()
	{
		return m_Task.GetTaskCompletionType();
	}
	
	/*!
	Set the task completion type
	\param completionType MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	void SetTaskCompletionType(EEditorTaskCompletionType completionType)
	{
		if (m_Task.GetTaskCompletionType() == completionType)
			return;
		
		SetTaskCompletionTypeBroadcast(completionType);
		Rpc(SetTaskCompletionTypeBroadcast, completionType);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTaskCompletionTypeBroadcast(EEditorTaskCompletionType completionType)
	{
		m_Task.SetTaskCompletionType(completionType);
	}
	
	protected void UpdateText()
	{
		UpdateInfo(m_Task.GetInfo());
		
		m_Task.SetTextIndex(m_iTextIndex);
		m_Task.SetLocationName(m_UIInfoDescriptor.GetLocationName());
	}
	override protected void GetOnLocationChange(SCR_EditableEntityComponent nearestLocation)
	{
		UpdateText();
	}
	
	override Faction GetFaction()
	{
		return m_TargetFaction;
	}
	
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{	
		super.SetTransform(transform, changedByUser);
		UpdateNearestLocation();
	}
	
	override bool Serialize(out SCR_EditableEntityComponent outTarget = null, out int outTargetIndex = -1, out EEditableEntitySaveFlag outSaveFlags = 0)
	{
		outTargetIndex = GetGame().GetFactionManager().GetFactionIndex(m_TargetFaction);
		return super.Serialize(outTarget, outTargetIndex, outSaveFlags);
	}
	override void Deserialize(SCR_EditableEntityComponent target, int targetValue)
	{
		super.Deserialize(target, targetValue);
		
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
		if (!supportEntity)
			return;
		
		m_TargetFaction = GetGame().GetFactionManager().GetFactionByIndex(targetValue);
		supportEntity.SetTargetFaction(m_Task, m_TargetFaction);
	}
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		int factionIndex;
		if (GetGame().GetFactionManager())
			factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_TargetFaction);
		
		writer.WriteInt(factionIndex);
		writer.WriteInt(m_iTextIndex);
			
		if (m_Task)
			m_Task.Serialize(writer);
		
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		int factionIndex;
		reader.ReadInt(factionIndex);
		if (GetGame().GetFactionManager())
			m_TargetFaction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);
		
		reader.ReadInt(m_iTextIndex);
		m_Task.SetTextIndex(m_iTextIndex);
		
		m_Task.Deserialize(reader);
		
		UpdateText();
		
		return true;
	}
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue)
	{
		if (recipient)
		{
			m_TargetFaction = recipient.GetFaction();
			if (m_TargetFaction)
			{
				//--- When the task is placed as inactive, don't assign faction yet, do it only upon manual activation
				if (!SCR_Enum.HasFlag(flags, EEditorPlacingFlags.TASK_INACTIVE))
					m_Task.SetTargetFaction(m_TargetFaction);
				
				UpdateNearestLocation();
			}
		}
		return this;
	}
	void SCR_EditableTaskComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Task = SCR_EditorTask.Cast(ent);
	}
	
	
	override bool Delete(bool changedByUser = false, bool updateNavmesh = true)
	{
		if (m_Task)
			m_Task.ShowTaskNotification(ENotification.EDITOR_TASK_DELETED, true);
		
		return super.Delete(updateNavmesh);
	}
};
