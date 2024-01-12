[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableFactionComponentClass: SCR_EditableEntityComponentClass
{
	static override bool GetEntitySourceBudgetCost(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		// Avoid fallback entityType cost
		return true;
	}
};

/** @ingroup Editable_Entities
*/

/*!
Special configuration for editable faction.
*/
class SCR_EditableFactionComponent : SCR_EditableEntityComponent
{
	[Attribute()]
	protected ref array<ref SCR_ArsenalItemCountConfig> m_MaxCountPerItemType;
	
	[RplProp()]
	protected int m_iFactionIndex = -1;
	
	protected Faction m_Faction;
	protected SCR_Faction m_ScrFaction;
	protected ref SCR_UIInfo m_Info;
	
	//Faction info
	protected int m_iSpawnPointCount = -1;
	protected int m_iTaskCount = -1;
	
	//Task Safty
	protected SCR_BaseTask m_PrevTask = null;
	
	//Script invokers
	protected ref ScriptInvoker Event_OnSpawnPointCountChanged = new ref ScriptInvoker;
	protected ref ScriptInvoker Event_OnTaskCountChanged = new ref ScriptInvoker;
	
	//Arsenal
	protected SCR_EArsenalItemType m_AllowedArsenalItemTypes;
	protected ref map<SCR_EArsenalItemType, int> m_aCurrentItemTaken = new map<SCR_EArsenalItemType, int>();
	
	/*!
	Assign faction to this editable entity.
	\param index Index of the faction in FactionManager array
	*/
	void SetFactionIndex(int index)
	{
		if (m_iFactionIndex != -1)
			return;
		
		SetFactionIndexBroadcast(index);
		Rpc(SetFactionIndexBroadcast, index);
	}
	/*!
	Get index of a faction represented by this delegate.
	\return Faction index
	*/
	int GetFactionIndex()
	{
		return m_iFactionIndex;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionIndexBroadcast(int index)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		//--- Find faction on the manager (factions are local, that's why they're identified by index)
		m_Faction = factionManager.GetFactionByIndex(index);
		if (!m_Faction)
			return;
		
		m_ScrFaction = SCR_Faction.Cast(m_Faction);
		if (!m_ScrFaction)
			return;
		
		//--- Set the value
		m_iFactionIndex = index;
		
		//--- Copy UI info into editable entity
		m_Info = SCR_UIInfo.CreateInfo(m_Faction.GetUIInfo());
		SetInfoInstance(m_Info);
		
		//--- Register the faction back in the manager on local machine
		SCR_DelegateFactionManagerComponent delegatesManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegatesManager)
			delegatesManager.SetFactionDelegate(m_Faction, this);
		
		//--- Hide non-playable faction
		SetVisible(m_ScrFaction.IsPlayable());
		
		//If Workbench or client
		if (SCR_Global.IsEditMode(GetOwner()) || Replication.IsClient())
			return;
	
		//Init count. Call one frame later to make sure the counts are correct
		GetGame().GetCallqueue().CallLater(InitSpawnPointCount); 
		GetGame().GetCallqueue().CallLater(InitTaskCount);
		
		//Spawnpoints
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(OnSpawnPointsChanged);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnpointFactionChanged);
		
		//Tasks
		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTasksChanged);
		SCR_BaseTaskManager.s_OnTaskDeleted.Insert(OnTasksChanged);
	}
	
	//======================================== FACTION SPAWNPOINTS ========================================\\
	//---------------------------------------- Init Faction spawnpoint count ----------------------------------------\\
	//Called on server
	protected void InitSpawnPointCount()
	{		
		int spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(m_Faction.GetFactionKey());
		
		InitSpawnPointCountBroadcast(spawnPointCount);
		Rpc(InitSpawnPointCountBroadcast, spawnPointCount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitSpawnPointCountBroadcast(int spawnPointCount)
	{
		m_iSpawnPointCount = spawnPointCount;
	}
	
	protected void OnSpawnpointFactionChanged(SCR_SpawnPoint spawnPoint)
	{
		OnSpawnPointsChanged(m_Faction.GetFactionKey());
	}
	
	//---------------------------------------- On Faction Spawnpoints changed ----------------------------------------\\
	//Called on server
	protected void OnSpawnPointsChanged(string factionKey)
	{		
		if (factionKey != m_Faction.GetFactionKey()) 
			return;
		
		int spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(m_Faction.GetFactionKey());
		
		//No change
		if (spawnPointCount == m_iSpawnPointCount)
			return;
		
		//Notification no spawns
		if (spawnPointCount == 0)
		{
			int thisRplId = Replication.FindId(this);
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_FACTION_NO_SPAWNS, thisRplId);
		}
		else if (spawnPointCount == 1 && !m_ScrFaction.IsPlayable())
		{
			//SetFactionPlayableServer(true);
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_FACTION_CHANGED_NO_GM);
		}
		
		//Update Spawn count
		OnSpawnPointCountChangedBroadcast(spawnPointCount);
		Rpc(OnSpawnPointCountChangedBroadcast, spawnPointCount);
	}

	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnSpawnPointCountChangedBroadcast(int spawnPointCount)
	{
		m_iSpawnPointCount = spawnPointCount;
		Event_OnSpawnPointCountChanged.Invoke(m_Faction, spawnPointCount);
	}

	//======================================== FACTION TASKS ========================================\\
	//---------------------------------------- Init Faction Task count ----------------------------------------\\
	//Called on server
	protected void InitTaskCount()
	{		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>;
		int factionTaskCount = taskManager.GetFilteredTasks(tasks, m_Faction);
		
		InitTaskCountBroadcast(factionTaskCount);
		Rpc(InitTaskCountBroadcast, factionTaskCount);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitTaskCountBroadcast(int factionTaskCount)
	{
		m_iTaskCount = factionTaskCount;
	}	
	
	//---------------------------------------- Faction Task Count Changed ----------------------------------------\\
	//Called on server
	protected void OnTasksChanged(notnull SCR_BaseTask task)
	{		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		Faction faction = task.GetTargetFaction();
		if (!faction || faction != m_Faction)
			return;
		
		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>;
		int factionTaskCount = taskManager.GetFilteredTasks(tasks, m_Faction);
		
		//Safty as task Update is called on any change in tasks, but it should catch all the OnTaskUpdates types regardless. So if the same value is given don't do anything
		if (m_PrevTask == task && m_iTaskCount == factionTaskCount)
			return;
		
		m_PrevTask = task;
		
		//Update task count
		OnTaskCountChangedBroadcast(factionTaskCount);
		Rpc(OnTaskCountChangedBroadcast, factionTaskCount);	
	}	

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnTaskCountChangedBroadcast(int factionTaskCount)
	{		
		m_iTaskCount = factionTaskCount;
		Event_OnTaskCountChanged.Invoke(m_Faction, factionTaskCount);
	}
	
	//======================================== FACTION PLAYABLE ========================================\\
	/*!
	Set faction Playable, can only be called from the server side
	\param factionPlayable bool for setting faction playable
	*/
	void SetFactionPlayableServer(bool factionPlayable)
	{
		//If client cancel
		if (Replication.IsClient())
			return;
		
		if (m_ScrFaction.IsPlayable() == factionPlayable)
			return;
		
		//Update enabled
		OnFactionplayableChangedBroadcast(factionPlayable);
		Rpc(OnFactionplayableChangedBroadcast, factionPlayable);	
	}
	
	//Broad cast to client
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnFactionplayableChangedBroadcast(bool factionPlayable)
	{	
		m_ScrFaction.SetIsPlayable(factionPlayable, true);
		
		//--- Hide non-playable faction, so it's unselected if it was currently selected
		SetVisible(factionPlayable);
	}
		
	
	//======================================== GETTERS ========================================\\
	/*!
	Get Spawnpoint count of faction
	\return int Spawn Point count
	*/
	int GetFactionSpawnPointCount()
	{	
		return m_iSpawnPointCount;
	}
	
	/*!
	Get Task count of faction
	\return int Task count
	*/
	int GetFactionTasksCount()
	{				
		return m_iTaskCount;
	}
	
	/*!
	Get On Spawn Point Count Changed Script Invoker
	\return ScriptInvoker Event_OnSpawnPointCountChanged
	*/
	ScriptInvoker GetOnSpawnPointCountChanged()
	{
		return Event_OnSpawnPointCountChanged;
	}
	
	/*!
	Get On Task Count Changed Script Invoker
	\return ScriptInvoker Event_OnTaskCountChanged
	*/
	ScriptInvoker GetOnTaskCountChanged()
	{
		return Event_OnTaskCountChanged;
	}
	
	SCR_EArsenalItemType GetAllowedArsenalItemTypes()
	{
		return m_AllowedArsenalItemTypes;
	}
	
	void SetAllowedArsenalItemTypes(SCR_EArsenalItemType allowedArsenalItemTypes)
	{
		m_AllowedArsenalItemTypes = allowedArsenalItemTypes;
	}
	
	override bool CanDuplicate(out notnull set<SCR_EditableEntityComponent> outRecipients)
	{
		return false;
	}
	
	override Faction GetFaction()
	{
		return m_Faction;
	}
	override bool GetPos(out vector pos)
	{
		return false;
	}
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		// Return true and empty cost array, avoid fallback entityType cost
		return true;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		owner.SetFlags(EntityFlags.NO_LINK, true);
	}
	
		//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
	{	
		writer.WriteInt(m_iFactionIndex); 
		writer.WriteInt(m_iSpawnPointCount); 
		writer.WriteInt(m_iTaskCount);
		writer.WriteBool(m_ScrFaction.IsPlayable());
		
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		int spawnPointCount, taskCount;
		bool isPlayable;
		
		reader.ReadInt(m_iFactionIndex);
		reader.ReadInt(spawnPointCount);
		reader.ReadInt(taskCount);
		reader.ReadBool(isPlayable);
		
		m_ScrFaction.InitFactionIsPlayable(isPlayable);
		SetFactionIndexBroadcast(m_iFactionIndex);
		InitSpawnPointCountBroadcast(spawnPointCount);
		InitTaskCountBroadcast(taskCount);
		
		return true;
	}
	
	
	void ~SCR_EditableFactionComponent()
	{
		if (Replication.IsServer() && m_Faction)
		{
			//Spawnpoints
			SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(OnSpawnPointsChanged);
			SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnpointFactionChanged);
		
			//Tasks
			SCR_BaseTaskManager.s_OnTaskUpdate.Remove(OnTasksChanged);
			SCR_BaseTaskManager.s_OnTaskDeleted.Remove(OnTasksChanged);
		}
	}
};
