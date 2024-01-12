void ScriptInvokerAIGroup(SCR_AIGroup group);
typedef func ScriptInvokerAIGroup;

[EntityEditorProps(category: "GameScripted/AI")]
class SCR_AIGroupClass : ChimeraAIGroupClass
{
	/*!
	Get group members and their offsets.
	\param entitySource Group's entity source
	\param[out] outPrefabs Array to be fillded with member prefabs
	\param[out] outOffsets Array to be fillded with local offsets
	\return Number of members
	*/
	static int GetMembers(IEntitySource entitySource, out array<ResourceName> outPrefabs, out array<vector> outOffsets)
	{
		//--- Not a group
		if (!entitySource || !entitySource.GetClassName().ToType().IsInherited(SCR_AIGroup)) return false;

		ArmaReforgerScripted game = GetGame();
		if (!game) return 0;

		AIWorld aiWorld = game.GetAIWorld();
		if (!aiWorld) return 0;

		//--- Get formation
		AIFormationDefinition formation;
		IEntityComponentSource componentSource;
		for (int i = 0, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(AIFormationComponent))
			{
				string formationName;
				componentSource.Get("DefaultFormation", formationName);
				formation = aiWorld.GetFormation(formationName);
				break;
			}
		}
		if (!formation) return 0;

		//--- Get member prefabs
		entitySource.Get("m_aUnitPrefabSlots", outPrefabs);

		//--- Get offsets
		outOffsets.Clear();
		int count = outPrefabs.Count();
		for (int i = 0; i < count; i++)
		{
			outOffsets.Insert(formation.GetOffsetPosition(i));
		}

		return count;
	}
};

class SCR_AIGroup_DelayedSpawn
{
	bool snapToTerrain;
	int index;
	ResourceName resourceName;
	bool editMode;
};	

class SCR_AIGroup : ChimeraAIGroup
{
	[Attribute("", UIWidgets.EditBox, "Faction", category: "Group")]
	string m_faction;

	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Entities in group non-ai included", params: "et", category: "Group Members")]
	ref array<ResourceName> m_aUnitPrefabSlots;

	[Attribute(defvalue: "1", desc: "When true, group members will be spawned above terrain, offset by group's ATL height.\nWhen false, group members will be levelled horizontally with the group.", category: "Group Members")]
	private bool m_bSnapToTerrain;

	[Attribute("", UIWidgets.EditBox, "List of Waypoint names found in the level", category: "Group Waypoints")]
	ref array<string> m_aStaticWaypoints;

	[Attribute(defvalue: "", UIWidgets.Object, desc: "Waypoints that should be spawned from prefabs", category: "Group Waypoints")]
	ref array<ref SCR_WaypointPrefabLocation> m_aSpawnedWaypoints;

	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "List of vehicles to use for movement", category: "Group Vehicles")]
	ref array<string> m_aStaticVehicles;

	[Attribute(defvalue: "1", desc: "When true, group members will be spawned durin OnInit, if false spawning must be called manually calling SpawnUnits()", category: "Group Members")]
	private bool m_bSpawnImmediately;

	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "Delay between spawns of individual members (ms)", category: "Group Members")]
	protected int m_fMemberSpawnDelay;

	[Attribute(defvalue: "1", UIWidgets.EditBox, desc: "When enabled, the group will be deleted when its last member dies or is deleted.\nThis will *not* delete the group when it starts empty.", category: "Group")]
	protected bool m_bDeleteWhenEmpty;

	protected static bool s_bIgnoreSnapToTerrain;
	protected static bool s_bIgnoreSpawning;

	protected ref array<IEntity> m_aSceneGroupUnitInstances;
	protected ref array<IEntity> m_aSceneWaypointInstances;
	protected ref array<IEntity> m_aUsableVehicles;
	protected ref array<BaseCompartmentSlot> m_aAllocatedCompartments;
	
	protected int m_iMaxUnitsToSpawn = int.MAX;
	protected ref ScriptInvoker Event_OnInit = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnEmpty = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnAgentAdded = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnAgentRemoved = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnLeaderChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnCurrentWaypointChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnWaypointCompleted = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnWaypointAdded = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnWaypointRemoved = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnFactionChanged = new ScriptInvoker;
	
	protected ref ScriptInvoker m_OnGroupMemberStateChange = new ScriptInvoker();
	
	protected int m_iNumOfMembersToSpawn;
	
	protected ref array<int> m_aAgentIDQueue = {};
	
	//player groups variables
	[Attribute(category: "Player settings")]
	protected int m_iGroupRadioFrequency;
	
	[Attribute(category: "Player settings", params: "1 100 1")]
	protected int m_iMaxMembers;
	
	[Attribute("0", desc: "Can players join this group?", category: "Player settings")]
	protected bool m_bPlayable, m_bPrivate;
	
	[Attribute("1", desc: "Does this group get deleted when empty?", category: "Player settings")]
	protected bool m_bDeleteIfNoPlayer;
	
	protected int m_iGroupID = -1;
	protected int m_iLeaderID = -1;
	protected ref SCR_AIGroupUIInfo m_UiInfo;
	
	//gamecode uses 0 as invalid playerID
	protected int m_iDescriptionAuthorID = 0;
	protected int m_iNameAuthorID = 0;
	
	
	protected string m_sCustomName = "";
	protected string m_sCustomDescription = "";
	protected ref array<int> m_aPlayerIDs = {};
	protected ref array<int> m_aDisconnectedPlayerIDs;
	protected static ref ScriptInvoker s_OnPlayerAdded = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnPlayerRemoved = new ScriptInvoker();
	protected static ref ScriptInvoker<int, int> s_OnPlayerLeaderChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnPrivateGroupChanged = new ScriptInvoker();
	protected static ref ScriptInvoker<SCR_AIGroup> s_OnCustomNameChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnFrequencyChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnMaxMembersChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnCustomDescChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnFlagSelected = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupRequest = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupConfirm = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupCancel = new ScriptInvoker();	
	
	protected ref array<int> m_aRequesterIDs = {};
	protected ref array<int> m_aDeniedRequesters = {};
	
	//commanding variables
	protected SCR_AIGroup m_SlaveGroup;
	protected SCR_AIGroup m_MasterGroup;
	protected ref array<SCR_ChimeraCharacter> m_aAIMembers = {};		
	
	// entity spawn list
	protected ref array<ref SCR_AIGroup_DelayedSpawn> m_delayedSpawnList = {};
	protected ref ScriptInvokerBase<ScriptInvokerAIGroup> Event_OnAllDelayedEntitySpawned;

	//------------------------------------------------------------------------------------------------
	int GetNumberOfMembersToSpawn()
	{
		return m_iNumOfMembersToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		// No more entities in the list? Turn off frame events
		if (m_delayedSpawnList.IsEmpty())
		{
			EndDelayedSpawn();
			return;
		}

		// Spawn a singular AI entity this frame
		int spawnIndex = m_delayedSpawnList.Count() - 1;
		SpawnDelayedGroupMember(spawnIndex);
		m_delayedSpawnList.Remove(spawnIndex);
		
		// Notify all delayed spawning is done
		if (m_delayedSpawnList.IsEmpty() && Event_OnAllDelayedEntitySpawned)
			Event_OnAllDelayedEntitySpawned.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnDelayedGroupMember(int spawnIndex)
	{
		SpawnGroupMember(
			m_delayedSpawnList.Get(spawnIndex).snapToTerrain,
			m_delayedSpawnList.Get(spawnIndex).index,
			m_delayedSpawnList.Get(spawnIndex).resourceName,
			m_delayedSpawnList.Get(spawnIndex).editMode,
			spawnIndex == 0 // isLast
		);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnAllImmediately()
	{
		for (int spawnIndex = m_delayedSpawnList.Count() - 1; spawnIndex >= 0; spawnIndex--)
			SpawnDelayedGroupMember(spawnIndex);
		
		m_delayedSpawnList.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void BeginDelayedSpawn()
	{
		SetEventMask(EntityEvent.FRAME);
		
		// Allow this entity to frame/tick while in Game Master Mode
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
	}

	//------------------------------------------------------------------------------------------------
	void EndDelayedSpawn()
	{
		ClearEventMask(EntityEvent.FRAME);
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasRequesterID(int id)
	{
		return m_aRequesterIDs.Contains(id);
	}
		
	//------------------------------------------------------------------------------------------------
	void RemoveRequester(int playerID)
	{
		if (!m_aRequesterIDs.Contains(playerID))
			return;
		
		RPC_DoRemoveRequester(playerID);
		Rpc(RPC_DoRemoveRequester, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDeniedRequesters(out array<int> valueArray)
	{
		return valueArray.Copy(m_aDeniedRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDeniedRequester(int playerID)
	{
		if (m_aDeniedRequesters.Contains(playerID))
			return;
		
		RPC_DoAddDeniedRequester(playerID);
		Rpc(RPC_DoAddDeniedRequester, playerID);
	}	
	
	//------------------------------------------------------------------------------------------------
	void ClearRequesters()
	{
		RPC_DoClearRequesterIDs();
		Rpc(RPC_DoClearRequesterIDs);
	}	
	
	//------------------------------------------------------------------------------------------------
	void ClearDeniedRequester()
	{
		RPC_DoClearDeniedRequester();
		Rpc(RPC_DoClearDeniedRequester);
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetRequesterIDs(out array<int> valueArray)
	{
		return valueArray.Copy(m_aRequesterIDs);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRequester(int playerID)
	{
		if (m_aRequesterIDs.Contains(playerID))
			return;
		
		RPC_DoAddRequester(playerID);
		Rpc(RPC_DoAddRequester, playerID);
	}			
	
	//------------------------------------------------------------------------------------------------
	void SetFlagIsFromImageSet(bool value)
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();
		
		m_UiInfo.SetFlagIsFromImageSet(value);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCustomGroupFlag(ResourceName flag)
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();
		
		m_UiInfo.SetGroupFlag(flag);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFlagIsFromImageSet()
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();
		
		bool flagIsFromSet = m_UiInfo.GetFlagIsFromImageSet();
		return flagIsFromSet;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupFlag()
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();
		
		ResourceName name = m_UiInfo.GetGroupFlag();		
		return name;
	}	
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnFlagSelected()
	{				
		return s_OnFlagSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupRequest()
	{
		return s_OnJoinPrivateGroupRequest;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupConfirm()
	{
		return s_OnJoinPrivateGroupConfirm;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupCancel()
	{
		return s_OnJoinPrivateGroupCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerInGroup(int playerID)
	{
		return m_aPlayerIDs.Contains(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeader(int playerID)
	{
		return playerID == m_iLeaderID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetLeaderID()
	{
		return m_iLeaderID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void SetPrivate(bool isPrivate)
	{
		RPC_SetPrivate(isPrivate);
		Rpc(RPC_SetPrivate, isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPrivate()
	{
		return m_bPrivate;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if the group is set to be deleted when all players leave.
	bool GetDeleteIfNoPlayer()
	{
		return m_bDeleteIfNoPlayer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the group should be deleted when all players leave.
	void SetCanDeleteIfNoPlayer(bool deleteEmpty)
	{
		m_bDeleteIfNoPlayer = deleteEmpty;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsFull()
	{
		return m_iMaxMembers <= m_aPlayerIDs.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsSlave()
	{
		if (!m_MasterGroup)
			return false;
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in the group
	\param checkMasterAndSlaves If true then it will also check any attached Master and/or slave group
	\return Total group count of all AI and player member
	*/
	int GetPlayerAndAgentCount(bool checkMasterAndSlaves = false)
	{
		if (!checkMasterAndSlaves)
			return GetPlayerCount() + GetAgentsCount();
		
		//~ Return count of group and master and slave groups
		return GetPlayerCount(true) + GetAgentCountIncludingMasterAndSlaves();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get the count of all AI in the group as well as any attached Master and slave groups
	*/
	int GetAgentCountIncludingMasterAndSlaves()
	{
		int totalAgentCount = GetAgentsCount();
		
		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalAgentCount += aiGroup.GetAgentsCount();
		
		aiGroup = GetMaster();
		if (aiGroup)
			totalAgentCount += aiGroup.GetAgentsCount();

		return totalAgentCount;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in both the group itself as well as the slave/master group(s)
	\return Total group count of all AI and player member
	*/
	int GetTotalAgentCount()
	{
		int totalGroupCount = GetAgentsCount();
		
		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalGroupCount += aiGroup.GetAgentsCount();
		
		aiGroup = GetMaster();
		if (aiGroup)
			totalGroupCount += aiGroup.GetAgentsCount();
		
		return totalGroupCount;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in both the group itself as well as the slave/master group(s)
	\return Total group count of all AI and player member
	*/
	int GetTotalPlayerCount()
	{
		int totalGroupCount = GetPlayerCount();
		
		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalGroupCount += aiGroup.GetPlayerCount();
		
		aiGroup = GetMaster();
		if (aiGroup)
			totalGroupCount += aiGroup.GetPlayerCount();
		
		return totalGroupCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetGroupFlag(int flagIndex, bool isFromImageset)
	{
		if(!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();
		
		RPC_DoSetGroupFlag(flagIndex, isFromImageset);
		Rpc(RPC_DoSetGroupFlag, flagIndex, isFromImageset);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupFlag(int flagIndex, bool isFromImageset)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
	    if (!groupManager)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(GetFaction());
		if (!scrFaction)
			return;		
		
		m_UiInfo.SetFlagIsFromImageSet(isFromImageset);
		
		if (isFromImageset)
		{		
			if (flagIndex >= 0)				
				m_UiInfo.SetGroupFlag(scrFaction.GetFlagName(flagIndex));
		}
		else
		{
			array<ResourceName> textures = {};
			scrFaction.GetGroupFlagTextures(textures);
			
			if (textures.IsIndexValid(flagIndex))				
				m_UiInfo.SetGroupFlag(textures[flagIndex]);
		}
		
		s_OnFlagSelected.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCustomName(string name, int authorID)
	{
		RPC_DoSetCustomName(name, authorID);
		Rpc(RPC_DoSetCustomName, name, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetCustomName(string name, int authorID)
	{
		m_sCustomName = name;
		m_iNameAuthorID = authorID;
		s_OnCustomNameChanged.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetCustomDescription(string desc, int authorID)
	{
		RPC_DoSetCustomDescription(desc, authorID);
		Rpc(RPC_DoSetCustomDescription, desc, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxGroupMembers(int value)
	{
		RPC_DoSetMaxGroupMembers(value);
		Rpc(RPC_DoSetMaxGroupMembers, value);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetMaxGroupMembers(int value)
	{
		m_iMaxMembers = value;
	}	
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetCustomDescription(string desc, int authorID)
	{
		m_sCustomDescription = desc;
		m_iDescriptionAuthorID = authorID;
		s_OnCustomDescChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCustomDescription()
	{	
		if (!GetGame().GetPlayerController().CanViewContentCreatedBy(m_iDescriptionAuthorID))
			return string.Empty;
		
		return m_sCustomDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCustomName()
	{	
		if (!GetGame().GetPlayerController().CanViewContentCreatedBy(m_iNameAuthorID))
			return string.Empty;
		
		return m_sCustomName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDescriptionAuthorID()
	{
		return m_iDescriptionAuthorID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetNameAuthorID()
	{
		return m_iNameAuthorID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCustomNameWithOriginal()
	{
		string company, platoon, squad, character, format;
		GetCallsigns(company, platoon, squad, character, format);
		string originalName, newName;
		originalName = string.Format(format, company, platoon, squad, character);
		
		if (m_sCustomName.IsEmpty() || !GetGame().GetPlayerController().CanViewContentCreatedBy(m_iNameAuthorID))
			return originalName;
				
		return m_sCustomName + " ( " + originalName + " )";
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupID(int id)
	{
		m_iGroupID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxMembers()
	{
		return m_iMaxMembers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetMaxMembers(int maxMembers)
	{
		if (maxMembers == GetMaxMembers() || maxMembers < 0)
			return;
		
		RPC_DoSetMaxMembers(maxMembers);
		Rpc(RPC_DoSetMaxMembers, maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetMaxMembers(int maxMembers)
	{
		m_iMaxMembers = maxMembers;
		s_OnMaxMembersChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	array<int> GetPlayerIDs()
	{
		return m_aPlayerIDs;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players in the group
	\param checkMasterAndSlaves If true then it will also check any attached Master and/or slave groups
	\return Total count of players
	*/
	int GetPlayerCount(bool checkMasterAndSlaves = false)
	{
		if (!checkMasterAndSlaves)
			return m_aPlayerIDs.Count();
		
		int totalPlayerCount = GetPlayerCount();
		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalPlayerCount += aiGroup.GetPlayerCount();
		
		aiGroup = GetMaster();
		if (aiGroup)
			totalPlayerCount += aiGroup.GetPlayerCount();

		return totalPlayerCount;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMemberStateChange()
	{
		return m_OnGroupMemberStateChange;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerAdded()
	{
		return s_OnPlayerAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerRemoved()
	{
		return s_OnPlayerRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerLeaderChanged()
	{
		return s_OnPlayerLeaderChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPrivateGroupChanged()
	{
		return s_OnPrivateGroupChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnCustomNameChanged()
	{
		return s_OnCustomNameChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnFrequencyChanged()
	{
		return s_OnFrequencyChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnCustomDescriptionChanged()
	{
		return s_OnCustomDescChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	bool BelongedToGroup(int playerID)
	{
		return m_aDisconnectedPlayerIDs != null && m_aDisconnectedPlayerIDs.Contains(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAgentFromControlledEntity(notnull IEntity controlledEntity)
	{
		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;
		
		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;
		
		AddAgent(agent);
		
		//we send notification to master group players
		if (IsSlave())
			NotificateGroupAIChange(controlledEntity, ENotification.GROUPS_AI_JOINED);
		
		OnGroupMemberStateChange();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveAgentFromControlledEntity(notnull IEntity controlledEntity)
	{
		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;
		
		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;
		RemoveAgent(agent);
		
		//we send notification to master group players
		if (IsSlave())
			NotificateGroupAIChange(controlledEntity, ENotification.GROUPS_AI_LEFT);
		
		OnGroupMemberStateChange();
	}
	
	//------------------------------------------------------------------------------------------------
	void NotificateGroupAIChange(IEntity controlledEntity, ENotification notificationType)
	{
		RplId rplId = -1;
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(controlledEntity.FindComponent(SCR_EditableEntityComponent));
		if (editableEntityComp)
			rplId = Replication.FindId(editableEntityComp);
		
		if (rplId.IsValid())
			SCR_NotificationsComponent.SendToGroup(GetMaster().GetGroupID(), notificationType, rplId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void OnPlayerDisconnected(int playerID)
	{
		// No need to null check m_aPlayerIDs, it always exists together with this entity
		int index = m_aPlayerIDs.Find(playerID);
		if (index < 0)
			return;
		
		// We have to check existence of m_aDisconnectedPlayerIDs, because we might be adding the first entry
		if (!m_aDisconnectedPlayerIDs)
			m_aDisconnectedPlayerIDs = {};
		
		RemovePlayer(playerID);
		m_aDisconnectedPlayerIDs.Insert(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void OnPlayerConnected(int playerID)
	{
		// No need to null check m_aDisconnectedPlayerIDs, this method is only called after entries are added to it
		int index = m_aDisconnectedPlayerIDs.Find(playerID);
		if (index < 0)
			return;
		
		m_aDisconnectedPlayerIDs.Remove(index);
		
		// Not full, we add the player, else bad luck, find a new group
		if (!IsFull())
			AddPlayer(playerID);
		
		// No more disconnected players from this group, let's stop listening
		if (m_aDisconnectedPlayerIDs.Count() == 0)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerConnected().Remove(OnPlayerConnected);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoOnGroupMemberStateChange()
	{
		GetGame().GetCallqueue().CallLater(m_OnGroupMemberStateChange.Invoke, 1, false, null, null, null, null, null, null, null, null, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should only be called on the server (authority)
	void OnGroupMemberStateChange()
	{
		RPC_DoOnGroupMemberStateChange(); //Local call
		Rpc(RPC_DoOnGroupMemberStateChange); //Broadcast to clients
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMemberDeath(notnull SCR_CharacterControllerComponent memberController, IEntity killerEntity, Instigator killer)
	{
		//This event is only called for members of the group, so it's safe to call QueueAddAgent automatically
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(memberController.GetCharacter());
		QueueAddAgent(playerID);
		RemovePlayerAgent(playerID);
		OnGroupMemberStateChange();
	}
	
	//------------------------------------------------------------------------------------------------
	void ListenToMemberDeath(notnull IEntity groupMember)
	{
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(groupMember.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
	
		characterController.GetOnPlayerDeathWithParam().Insert(OnMemberDeath);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddOnGadgetsLoadedListener(int playerID, notnull IEntity controlledEntity)
	{
		SCR_GadgetManagerComponent.GetOnGadgetInitDoneInvoker().Insert(OnControllableEntitySpawned);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(AddOnGadgetsLoadedListener);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnControllableEntitySpawned(IEntity controlledEntity, notnull SCR_GadgetManagerComponent gadgetManager)
	{
		if (!controlledEntity)
			return; 
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(controlledEntity);
		if (playerID == 0)
			return;
				
		int index = m_aAgentIDQueue.Find(playerID);
		if (index < 0)
			return;
		
		ListenToMemberDeath(controlledEntity);
		AddAgentFromControlledEntity(controlledEntity);
		m_aAgentIDQueue.Remove(index);
		
		if (!m_aAgentIDQueue.IsEmpty())
			return;
		
		SCR_GadgetManagerComponent.GetOnGadgetInitDoneInvoker().Remove(OnControllableEntitySpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	void QueueAddAgent(int playerID)
	{
		// Avoiding duplicate entries
		m_aAgentIDQueue.Insert(playerID);
		if (m_aAgentIDQueue.Count() != 1)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		// Waiting for spawn of a controllable entity with correct ID		
		gameMode.GetOnPlayerSpawned().Insert(AddOnGadgetsLoadedListener);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddPlayer(int playerID)
	{
		m_aPlayerIDs.Insert(playerID);
		s_OnPlayerAdded.Invoke(this, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!checks for a leader, if there is none, it will either use provided player as leader, or appoints next player in group as leader
	//! Called on the server (authority)
	void CheckForLeader(int playerID, bool noLeaderAllowed)
	{
		//check if leader is still in group, noleaderallowed is preparation for editor integration
		if (!IsPlayerInGroup(GetLeaderID()) && !noLeaderAllowed && playerID == -1)
		{
			if (m_aPlayerIDs.IsEmpty())
			{
				s_OnPlayerLeaderChanged.Invoke(m_iGroupID, -1);
				return;
			}
			SetGroupLeader(m_aPlayerIDs.Get(0));
			return;
		}
		
		//if we have a leader, do nothing
		if (GetLeaderID() != -1)
			return;
		
		//otherwise appoint provided player as a leader
		if (playerID != -1)
			SetGroupLeader(playerID);
		
	    
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void SetGroupLeader(int playerID)
	{
		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_PROMOTED_LEADER, playerID);
		RPC_SetLeaderID(playerID);
		Rpc(RPC_SetLeaderID, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetLeaderID(int playerID)
	{
		if (playerID == m_iLeaderID)
			return;
		m_iLeaderID = playerID;
		s_OnPlayerLeaderChanged.Invoke(m_iGroupID, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetPrivate(bool isPrivate)
	{
		m_bPrivate = isPrivate;
		s_OnPrivateGroupChanged.Invoke(m_iGroupID, isPrivate);
		
		if (!isPrivate)
		{
			ClearRequesters();		
			ClearDeniedRequester();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void AddPlayer(int playerID)
	{		
		// Avoiding duplicate entries
		if (m_aPlayerIDs.Contains(playerID))
			return;
		
		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_JOINED, playerID);
		RPC_DoAddPlayer(playerID);
		Rpc(RPC_DoAddPlayer, playerID);
		// Did this player re-connect?
		if (m_aDisconnectedPlayerIDs)
		{
			int index = m_aDisconnectedPlayerIDs.Find(playerID);
			if (index >= 0)
				m_aDisconnectedPlayerIDs.Remove(index);
		}
		
		// Start listening to disconnect events when we add the first player
		if (m_aPlayerIDs.Count() == 1)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		}
		// Now we need the player character's agent
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!controlledEntity)
			QueueAddAgent(playerID);
		else
			AddAgentFromControlledEntity(controlledEntity);
		
		GetGame().GetCallqueue().CallLater(CheckForLeader, 0, false, playerID, false);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddRequester(int playerID)
	{
		m_aRequesterIDs.Insert(playerID);
		s_OnJoinPrivateGroupRequest.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemoveRequester(int playerID)
	{
		m_aRequesterIDs.RemoveItem(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoClearDeniedRequester()
	{
		m_aDeniedRequesters.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoClearRequesterIDs()
	{
		m_aRequesterIDs.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddDeniedRequester(int playerID)
	{
		m_aDeniedRequesters.Insert(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemovePlayer(int playerID)
	{
		m_aPlayerIDs.RemoveItem(playerID);
		if (m_iLeaderID == playerID)
			m_iLeaderID = -1;
		
		s_OnPlayerRemoved.Invoke(this, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemovePlayerAgent(int playerID)
	{
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!controlledEntity)
			return;
		
		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;
		
		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;
		
		RemoveAgent(agent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void RemovePlayer(int playerID)
	{
		if (!m_aPlayerIDs.Contains(playerID))
			return;
		
		RPC_DoRemovePlayer(playerID);
		Rpc(RPC_DoRemovePlayer, playerID);
		CheckForLeader(-1, false);
		RemovePlayerAgent(playerID);
		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_LEFT, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetCallsigns(out string company, out string platoon, out string squad, out string character, out string format)
	{
		SCR_CallsignGroupComponent callsignComponent = SCR_CallsignGroupComponent.Cast(FindComponent(SCR_CallsignGroupComponent));
		if (!callsignComponent)
			return;
		
		callsignComponent.GetCallsignNames(company, platoon, squad, character, format);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadioFrequency(int frequency)
	{
		if (frequency == GetRadioFrequency() || frequency <= 0)
			return;
		
		RPC_DoSetFrequency(frequency);
		Rpc(RPC_DoSetFrequency, frequency);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetFrequency(int frequency)
	{
		m_iGroupRadioFrequency = frequency;
		s_OnFrequencyChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRadioFrequency()
	{
		return m_iGroupRadioFrequency;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayable()
	{
		return m_bPlayable;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateUnitEntities(bool editMode, array<ResourceName> entityResourceNames)
	{
		if (!GetGame().GetAIWorld())
		{
			Print(string.Format("Cannot spawn team members of group %1, AIWorld is missing in the world!", this), LogLevel.WARNING);
			return;
		}
		
#ifdef WORKBENCH
		if (!editMode)
		{
			//--- Are AI components valid?
			AIFormationComponent AIFormation = AIFormationComponent.Cast(FindComponent(AIFormationComponent));
			
			if (!AIFormation)
				Print(string.Format("Group %1 does not have AIFormationComponent! Team members will not be spawned.", this), LogLevel.WARNING);
			else
			{
				AIFormationDefinition formationDefinition = AIFormation.GetFormation();
				if (!formationDefinition)
					Print(string.Format("Formation of group %1 not found in SCR_AIWorld! Team members will not be spawned.", this), LogLevel.WARNING);
			}
		}
#endif
		//--- Apply global override
		bool snapToTerrain = m_bSnapToTerrain;
		if (s_bIgnoreSnapToTerrain)
		{
			snapToTerrain = false;
			s_bIgnoreSnapToTerrain = false;
		}
		if (Replication.IsClient())
			return;
		
		//--- We are in WB, prepare array so previews can be deleted later
		if (editMode && !m_aSceneGroupUnitInstances)
			m_aSceneGroupUnitInstances = new ref array<IEntity>;
		
		 m_iNumOfMembersToSpawn = Math.Min(entityResourceNames.Count(), m_iMaxUnitsToSpawn);
		//--- Create group members
		for (int i = m_iNumOfMembersToSpawn-1; i >= 0; i--)
		{
			// Spawn group across multiple frames
			SCR_AIGroup_DelayedSpawn delaySpawn = new SCR_AIGroup_DelayedSpawn();
			delaySpawn.snapToTerrain	= snapToTerrain;
			delaySpawn.index			= i;
			delaySpawn.resourceName		= entityResourceNames[i];
			delaySpawn.editMode			= editMode;
			
			m_delayedSpawnList.Insert(delaySpawn);
		}

		if (editMode)
		{
			//--- Edit mode has no game world, spawn immediately
			SpawnAllImmediately();
		}
		else
		{
			//--- Enable the frame event and frames when paused
			BeginDelayedSpawn();
		}

		//--- Call group init if it cannot be called by the last spawned entity
		if (m_iNumOfMembersToSpawn == 0)
			Event_OnInit.Invoke(this);
	}
	
	protected void SpawnGroupMember(bool snapToTerrain, int index, ResourceName res, bool editMode, bool isLast)
	{
		if (!GetGame().GetAIWorld().CanLimitedAIBeAdded())
		{
			if (isLast)
				Event_OnInit.Invoke(this);
			
			//Event_OnLastGroupMemberSpawned.Invoke(this);
			
			return;
		}
		BaseWorld world = GetWorld();
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(this.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetWorldTransform(spawnParams.Transform);
		vector pos = spawnParams.Transform[3];
		
		if (formationDefinition)
			pos = CoordToParent(formationDefinition.GetOffsetPosition(index));
		else
			pos = CoordToParent(Vector(index, 0, 0));
		
		if (snapToTerrain)
		{
			float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
			pos[1] = surfaceY;
		}
		
		//Snap to the nearest navmesh point
		AIPathfindingComponent pathFindindingComponent = AIPathfindingComponent.Cast(this.FindComponent(AIPathfindingComponent));
		if (pathFindindingComponent && pathFindindingComponent.GetClosestPositionOnNavmesh(pos, "10 10 10", pos))
		{
			float groundHeight = world.GetSurfaceY(pos[0], pos[2]);
			if (pos[1] < groundHeight)
				pos[1] = groundHeight;
			vector outWaterSurfacePoint;
			EWaterSurfaceType waterSurfaceType;
			vector transformWS[4];
			vector obbExtents;
			if (ChimeraWorldUtils.TryGetWaterSurface(GetWorld(), pos, outWaterSurfacePoint, waterSurfaceType, transformWS, obbExtents))
			{
				pos = outWaterSurfacePoint;
			}
		}
		
		spawnParams.Transform[3] = pos;
		
		IEntity member = GetGame().SpawnEntityPrefab(Resource.Load(res), world, spawnParams);
		if (!member)
			return;
		
		// Move in to vehicle 
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(member.FindComponent(SCR_EditableEntityComponent));
		
		
		if (editMode)
			m_aSceneGroupUnitInstances.Insert(member);
		
		AddAIEntityToGroup(member);
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(member.FindComponent(FactionAffiliationComponent));
		
		if (factionAffiliation)
			factionAffiliation.SetAffiliatedFactionByKey(m_faction);
	
		if (isLast)
			Event_OnInit.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypointParams(out AIWaypoint wp, SCR_WaypointPrefabLocation prefabParams)
	{
		if ( SCR_TimedWaypoint.Cast(wp) && !float.AlmostEqual(prefabParams.m_WPTimeOverride,0.0))
			SCR_TimedWaypoint.Cast(wp).SetHoldingTime(prefabParams.m_WPTimeOverride);
		if ( !float.AlmostEqual(prefabParams.m_WPRadiusOverride,0.0) )
			wp.SetCompletionRadius(prefabParams.m_WPRadiusOverride);
		wp.SetName(prefabParams.m_WPInstanceName);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddWaypointsDynamic(out array<IEntity> entityInstanceList, array<ref SCR_WaypointPrefabLocation> prefabs)
	{
		entityInstanceList = new ref array<IEntity>;
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		for (int i =  0, length = prefabs.Count(); i < length; i++)
		{
			IEntity entity;
			AIWaypoint wp;
			mat[3] = prefabs[i].m_WPWorldLocation;
			spawnParams.Transform = mat;
			
#ifdef WORKBENCH //includes game mode run from WB
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				Print(prefabs[i].m_WPPrefabName);
				entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), m_API.GetWorld(), spawnParams);
				entityInstanceList.Insert(entity);
				wp = AIWaypoint.Cast(entity);
			}
			else
			{
				entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), GetGame().GetWorld(), spawnParams);
				wp = AIWaypoint.Cast(entity);
				AddWaypoint(wp);
			}
#else		// game run from build
			entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), GetGame().GetWorld(), spawnParams);
			wp = AIWaypoint.Cast(entity);
			AddWaypoint(wp);
#endif
			if ( wp )
				SetWaypointParams(wp,prefabs[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddWaypointsStatic(array<string> aWaypointNames)
	{
		for (int i = 0, length = aWaypointNames.Count(); i < length; i++)
		{
#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				AddWaypoint(AIWaypoint.Cast(m_API.GetWorld().FindEntityByName(aWaypointNames[i])));
			}
			else
			{
				AddWaypoint(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
			}
#else
			AddWaypoint(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddVehiclesStatic(array<string> aVehicleNames)
	{
		for (int i = 0, length = aVehicleNames.Count(); i < length; i++)
		{
#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				AddUsableVehicle(m_API.GetWorld().FindEntityByName(aVehicleNames[i]));
			}
			else
			{
				AddUsableVehicle(GetGame().GetWorld().FindEntityByName(aVehicleNames[i]));
			}
#else
			AddUsableVehicle(GetGame().GetWorld().FindEntityByName(aVehicleNames[i]));
#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyEntities(out array<IEntity> entityList)
	{
		if (!entityList)
			return;
		
		for (int i=0, length = entityList.Count(); i < length; i++)
		{
			if ( AIWaypoint.Cast(entityList[i]) )
				RemoveWaypointFromGroup(AIWaypoint.Cast(entityList[i]));
			else
				RemoveAIEntityFromGroup(entityList[i]);
			delete entityList[i];
		}
		entityList.Clear();
		entityList = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveStaticWaypointRefs(array<string> aWaypointNames)
	{
		if (!aWaypointNames)
			return;
		
		for (int i=0, length = aWaypointNames.Count(); i < length; i++)
		{
#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				RemoveWaypointFromGroup(AIWaypoint.Cast(m_API.GetWorld().FindEntityByName(aWaypointNames[i])));
			}
			else
			{
				RemoveWaypointFromGroup(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
			}
#else
			RemoveWaypointFromGroup(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearRefs(out array<IEntity> entityList)
	{
		if ( entityList )
			entityList.Clear();
		entityList = null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AddAIEntityToGroup(IEntity entity)
	{
		if (!entity) return false;
		
		AIControlComponent control = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!control) return false;
		
		AIAgent agent = control.GetControlAIAgent();
		if (!agent) return false;
		
		control.ActivateAI();
		
		if (!agent.GetParentGroup())
			AddAgent(agent); //--- Add to group only if some other system (e.g., component on the group member) wasn't faster
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RemoveAIEntityFromGroup(IEntity entity)
	{
#ifdef WORKBENCH
		WorldEditorAPI m_API = _WB_GetEditorAPI();
		if ( !m_API && entity && entity.FindComponent(AIControlComponent))
		{
			ref AIAgent agent = AIControlComponent.Cast(entity.FindComponent(AIControlComponent)).GetControlAIAgent();
			RemoveAgent(agent);
			return true;
		}
#else
		if ( entity && entity.FindComponent(AIControlComponent))
		{
			ref AIAgent agent = AIControlComponent.Cast(entity.FindComponent(AIControlComponent)).GetControlAIAgent();
			RemoveAgent(agent);
			return true;
		}
#endif
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddWaypointToGroup(AIWaypoint waypoint)
	{
		if ( waypoint )
		{
			AddWaypoint(waypoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveWaypointFromGroup(AIWaypoint waypoint)
	{
		if ( waypoint )
		{
			RemoveWaypoint(waypoint)
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddUsableVehicle(IEntity vehicle)
	{
		if (Vehicle.Cast(vehicle) && m_aUsableVehicles.Find(vehicle) < 0)
			m_aUsableVehicles.Insert(vehicle);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveUsableVehicle(IEntity vehicle)
	{
		if (Vehicle.Cast(vehicle) && m_aUsableVehicles.Find(vehicle) > -1)
			m_aUsableVehicles.RemoveItem(vehicle);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetUsableVehicles(out array<IEntity> usableVehicles)
	{
		return usableVehicles.Copy(m_aUsableVehicles);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetUsableVehiclesCount()
	{
		return m_aUsableVehicles.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void GetAllocatedCompartments(out array<BaseCompartmentSlot> allocatedCompartments)
	{
		 allocatedCompartments = m_aAllocatedCompartments;
	}
	
	//------------------------------------------------------------------------------------------------
	void AllocateCompartment(BaseCompartmentSlot compartment)
	{
		if (!compartment)
			return;
		if (m_aAllocatedCompartments.Find(compartment) > -1)
		{
			Print("Trying to allocate same compartment twice!", LogLevel.WARNING);
			return;
		};
		m_aAllocatedCompartments.Insert(compartment);
		compartment.SetCompartmentAccessible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void ReleaseCompartment(BaseCompartmentSlot compartment)
	{
		int index = m_aAllocatedCompartments.Find(compartment);
		if (index > -1)
			m_aAllocatedCompartments.Remove(index);
		else
			Print("Trying to remove compartment that is not allocated!", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	void ReleaseCompartments()
	{
		foreach (BaseCompartmentSlot comp : m_aAllocatedCompartments)
		{
			if (comp)
				comp.SetCompartmentAccessible(true);
		}
		m_aAllocatedCompartments.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set name of group's faction (only if it's not set yet)
	\param factionKey, Faction key to set faction to
	\return True if the faction was set
	*/
	bool InitFactionKey(string factionKey)
	{
		if (m_faction != "") return false;
		m_faction = factionKey;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	If Faction is set change faction to given faction
	Server Only
	\param faction, Faction to set
	\return True if the faction was set
	*/
	bool SetFaction(Faction faction)
	{
		if (Replication.IsClient())
			return false;
		
		// If playable, we don't want to allow changing the faction.
		if (m_bPlayable && m_faction)
			return false;
		
		if (!faction)
			return false;
		
		m_faction = faction.GetFactionKey();
		
		array<AIAgent> agents = new array<AIAgent>;
		GetAgents(agents);
		array<IEntity> updatedVehicles = new array<IEntity>;
		//array<RplId> entitiesInVehiclesIds = new array<RplId>;
		IEntity vehicle;
		IEntity charEntity;
		
		foreach (AIAgent agent: agents)
		{
			charEntity = agent.GetControlledEntity();
			
			if (!charEntity)
				continue;
			
			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(charEntity.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				factionAffiliation.SetAffiliatedFaction(faction);
		}
		
		//Send out event
		Event_OnFactionChanged.Invoke(faction);
		
		FactionManager factionManager = GetGame().GetFactionManager();
		int factionIndex = factionManager.GetFactionIndex(faction);
		if (factionManager)
			GetGame().GetCallqueue().CallLater(SetFactionDelayed, 1, false, factionIndex);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFactionDelayed(int factionIndex)
	{
		Rpc(BroadCastSetFaction, factionIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//Send to update Editor UI
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void BroadCastSetFaction(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		m_faction = faction.GetFactionKey();
		Event_OnFactionChanged.Invoke(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get name of group's faction.
	\return Faction name (search for it in FactionManager)
	*/
	string GetFactionName()
	{
		return m_faction;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get group's faction from FactionManager
	\return Faction (or null when faction is not found)
	*/
	Faction GetFaction()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game) return null;
		
		FactionManager factionManager = game.GetFactionManager();
		if (!factionManager) return null;
		
		return factionManager.GetFactionByKey(m_faction);
	}
	
	/*!
	Get group's faction index from FactionManager
	\return faction index
	*/
	int GetFactionIndex()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;
		
		return factionManager.GetFactionIndex(GetFaction());
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Ignore snapping to terrain in the next spawned group.
	Used when some other system handles it and group's functionality would interfere with it.
	Has to be set before spawning a group, is reset to false afterwards.
	\param ignore True to ignore terrain snap
	*/
	static void IgnoreSnapToTerrain(bool ignore)
	{
		s_bIgnoreSnapToTerrain = ignore;
	}
	/*!
	Ignore spawning group members in the next spawned group.
	Used when some other system handles it and group's functionality would interfere with it.
	Has to be set before spawning a group, is reset to false afterwards.
	\param ignore True to ignore spawning
	*/
	static void IgnoreSpawning(bool ignore)
	{
		s_bIgnoreSpawning = ignore;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when all initial group members were spawned.
	Called only on server.
	Param is this aiGroup
	\return Script invoker
	*/
	ScriptInvoker GetOnInit()
	{
		return Event_OnInit;
	}
	
	/*!
	Get event called when the group becomes empty.
	Called only on server.
	No invoker params are passed.
	\return Script invoker
	*/
	ScriptInvoker GetOnEmpty()
	{
		return Event_OnEmpty;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time an agent is called to a group.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent agent
	\return Script invoker
	*/
	ScriptInvoker GetOnAgentAdded()
	{
		return Event_OnAgentAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time an agent is removed from a group.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent agent
	\return Script invoker
	*/
	ScriptInvoker GetOnAgentRemoved()
	{
		return Event_OnAgentRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time a group leader changes.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent currentLeader, AIAgent prevLeader
	\return Script invoker
	*/
	ScriptInvoker GetOnLeaderChanged()
	{
		return Event_OnLeaderChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when current waypoint of the group changes.
	Called only on server.
	Invoker params are: AIWaypoint currentWP, AIWaypoint prevWP
	\return Script invoker
	*/
	ScriptInvoker GetOnCurrentWaypointChanged()
	{
		return Event_OnCurrentWaypointChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when current waypoint of the group is completed.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointCompleted()
	{
		return Event_OnWaypointCompleted;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when a new waypoint is added to the group.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointAdded()
	{
		return Event_OnWaypointAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when a waypoint is removed from the group.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointRemoved()
	{
		return Event_OnWaypointRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when faction of group is changed
	\return Script invoker
	*/
	ScriptInvoker GetOnFactionChanged()
	{
		return Event_OnFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvokerAIGroup> GetOnAllDelayedEntitySpawned()
	{
		if (!Event_OnAllDelayedEntitySpawned)
			Event_OnAllDelayedEntitySpawned = new ScriptInvokerBase<ScriptInvokerAIGroup>(); 
		
		return Event_OnAllDelayedEntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEmpty()
	{
		Event_OnEmpty.Invoke();
		
		//--- Delete after delay, doing it directly in this event would be unsafe
		if (m_bDeleteWhenEmpty)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1, false, this);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAgentAdded(AIAgent child)
	{
		Event_OnAgentAdded.Invoke(child);
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(child);
		if (agent)
		{
			agent.OnGroupWaypointChanged(GetCurrentWaypoint());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAgentRemoved(AIAgent child)
	{
		Event_OnAgentRemoved.Invoke(this, child);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnLeaderChanged(AIAgent currentLeader, AIAgent prevLeader)
	{
		Event_OnLeaderChanged.Invoke(currentLeader, prevLeader);
		
		if (currentLeader && currentLeader.GetControlledEntity())
		{
			EventHandlerManagerComponent eventHandlerManagerComponent = EventHandlerManagerComponent.Cast(currentLeader.GetControlledEntity().FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManagerComponent)
				eventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", this, this.OnLeaderConsciousnessChanged, true);
		}
		
		if (prevLeader && prevLeader.GetControlledEntity())
		{
			EventHandlerManagerComponent eventHandlerManagerComponent = EventHandlerManagerComponent.Cast(prevLeader.GetControlledEntity().FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManagerComponent)
				eventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", this, this.OnLeaderConsciousnessChanged, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCurrentWaypointChanged(AIWaypoint currentWP, AIWaypoint prevWP)
	{
		InvokeSubagentsOnWaypointChanged(currentWP);
		
		Event_OnCurrentWaypointChanged.Invoke(currentWP, prevWP);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWaypointCompleted(AIWaypoint wp)
	{
		InvokeSubagentsOnWaypointChanged(null);
		
		Event_OnWaypointCompleted.Invoke(wp);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWaypointAdded(AIWaypoint wp)
	{
		Event_OnWaypointAdded.Invoke(wp);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWaypointRemoved(AIWaypoint wp, bool isCurrentWaypoint)
	{
		InvokeSubagentsOnWaypointChanged(null);
		
		Event_OnWaypointRemoved.Invoke(wp, isCurrentWaypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokes OnGroupWaypointChanged on all soldiers
	//! newWaypoint can be null
	protected void InvokeSubagentsOnWaypointChanged(AIWaypoint newWaypoint)
	{
		array<AIAgent> agents = {};
		GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			SCR_ChimeraAIAgent _agent = SCR_ChimeraAIAgent.Cast(agent);
			if (_agent)
				_agent.OnGroupWaypointChanged(newWaypoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "coords")
		{
			DestroyEntities(m_aSceneGroupUnitInstances);
			CreateUnitEntities(true,m_aUnitPrefabSlots);
		}
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_aUsableVehicles =  new ref array<IEntity>;
		m_aAllocatedCompartments = new ref array<BaseCompartmentSlot>;
		
		if (s_bIgnoreSpawning)
		{
			//--- Instantly mark as initialized if no team members are to be spawned
			Event_OnInit.Invoke(this);
		}
		else if (m_bSpawnImmediately)
		{
			SpawnUnits();
		}
		
		s_bIgnoreSpawning = false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetSpawnImmediately()
	{
		return m_bSpawnImmediately;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnImmediately(bool spawnImmediately)
	{
		m_bSpawnImmediately = spawnImmediately;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxUnitsToSpawn(int cnt)
	{
		m_iMaxUnitsToSpawn = cnt;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set delay (ms) between spawning of individual group members.
	Can be used only before they are spawned.
	\param memberSpawnDelay Delay
	*/
	void SetMemberSpawnDelay(int memberSpawnDelay)
	{
		m_fMemberSpawnDelay = memberSpawnDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnUnits()
	{
		if (SCR_Global.IsEditMode(this))
		{
			CreateUnitEntities(true, m_aUnitPrefabSlots);
			AddVehiclesStatic(m_aStaticVehicles);
			AddWaypointsStatic(m_aStaticWaypoints);
			AddWaypointsDynamic(m_aSceneWaypointInstances, m_aSpawnedWaypoints);
		}
		else
		{
			//--- Don't hardcode members array - it may change in run-time, and we don't want to delete members who meanwhile joined other group
			CreateUnitEntities(false, m_aUnitPrefabSlots);
			AddVehiclesStatic(m_aStaticVehicles);
			AddWaypointsStatic(m_aStaticWaypoints);
			AddWaypointsDynamic(null, m_aSpawnedWaypoints);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get slave subgroup that is used for AI characters when invited by players into this playable SCR_AIGroup.
	*/
	SCR_AIGroup GetSlave()
	{
		return m_SlaveGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set slave subgroup that is used for AI characters when invited by players into this playable SCR_AIGroup.
	*/
	void SetSlave(SCR_AIGroup group)
	{
		m_SlaveGroup = group;
		group.SetMaster(this);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get master group that gives orders to this slave subgroup
	*/
	SCR_AIGroup GetMaster()
	{
		return m_MasterGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set master group that gives orders to this slave subgroup
	*/
	void SetMaster(SCR_AIGroup group)
	{
		m_MasterGroup = group;
	}
	
		//------------------------------------------------------------------------------------------------
	array<SCR_ChimeraCharacter> GetAIMembers()
	{
		return m_aAIMembers;
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	returns true if SCR_ChimeraCharacter is a member of slave subgroup that is linked to this playable SCR_AIGroup
	*/
	bool IsAIControlledCharacterMember(SCR_ChimeraCharacter character)
	{
		if (!character)
			return false;
		//if group doesnt have slave group for AIs, AI is automatically not a member
		if (!m_SlaveGroup)
			return false;
		return m_SlaveGroup.m_aAIMembers.Find(character) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!m_UiInfo)
		 m_UiInfo = new SCR_AIGroupUIInfo();
		
		int factionIndex = -1;
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			factionIndex = factionManager.GetFactionIndex(GetFaction());
		
		writer.WriteInt(factionIndex);
		writer.WriteInt(m_iGroupRadioFrequency);
		writer.WriteInt(m_iGroupID);
		
		int count = m_aPlayerIDs.Count();
		writer.WriteInt(count);
		for (int i = count - 1; i >= 0; i--)
		{
			writer.WriteInt(m_aPlayerIDs[i]);
		}
		writer.WriteInt(m_iLeaderID);
		writer.WriteBool(m_bPrivate);
		
		writer.WriteString(m_sCustomDescription);
		writer.WriteString(m_sCustomName);
		
		writer.WriteString(m_UiInfo.GetGroupFlag());
		writer.WriteBool(m_UiInfo.GetFlagIsFromImageSet());
		
		RplId groupID;
		groupID = Replication.FindId(m_MasterGroup);
		writer.WriteRplId(groupID);
		groupID = Replication.FindId(m_SlaveGroup);
		writer.WriteRplId(groupID);
		
		writer.WriteInt(m_iDescriptionAuthorID);
		writer.WriteInt(m_iNameAuthorID);
		writer.WriteInt(m_iMaxMembers);
		
		//do rpcs for players join/leave
		//add invokers for players join/leave
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int factionIndex;
		reader.ReadInt(factionIndex);
		if (factionIndex >= 0)
			BroadCastSetFaction(factionIndex);
		
		reader.ReadInt(m_iGroupRadioFrequency);
		reader.ReadInt(m_iGroupID);
		
		int count, playerID;
		reader.ReadInt(count);
		for (int i = count - 1; i >= 0; i--)
		{
			reader.ReadInt(playerID);
			m_aPlayerIDs.Insert(playerID);
		}
		
		if (m_bPlayable)
		{
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (groupsManager)
			{
				groupsManager.RegisterGroup(this);
				groupsManager.ClaimFrequency(GetRadioFrequency(), GetFaction());
				groupsManager.OnGroupCreated(this);
			}
		}
		
		int temp;
		reader.ReadInt(temp);
		RPC_SetLeaderID(temp);
		
		reader.ReadBool(m_bPrivate);
		
		reader.ReadString(m_sCustomDescription);
		reader.ReadString(m_sCustomName);
		
		if (!m_UiInfo)
		 m_UiInfo = new SCR_AIGroupUIInfo();
		
		string flag;
		reader.ReadString(flag);
		m_UiInfo.SetGroupFlag(flag);
		
		bool isFromImageSet;
		reader.ReadBool(isFromImageSet);
		m_UiInfo.SetFlagIsFromImageSet(isFromImageSet);
		
		RplId groupID;
		reader.ReadRplId(groupID);
		m_MasterGroup = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		reader.ReadRplId(groupID);
		m_SlaveGroup = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		
		reader.ReadInt(m_iDescriptionAuthorID);
		reader.ReadInt(m_iNameAuthorID);
		reader.ReadInt(m_iMaxMembers);
		
		return true;
	}
	
	void OnLeaderConsciousnessChanged(bool conscious)
	{
		if (!conscious)
			SetNewConsciousLeader();
	}
	
	void SetNewConsciousLeader()
	{
		array<AIAgent> groupAgents = {};
		GetAgents(groupAgents);
		
		for (int i = 1; i < groupAgents.Count(); i++)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(groupAgents.Get(i).GetControlledEntity());
			if (!character)
				continue;
			
			CharacterControllerComponent charController = character.GetCharacterController();
			if (!charController)
				continue;
			
			if (!charController.IsUnconscious())
			{
				SetNewLeader(groupAgents.Get(i));
				return;
			}
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIGroup(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AIGroup()
	{
		// Group is playable so we have to unregister it locally as well
		if (m_bPlayable)
		{
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (groupsManager)
			{
				groupsManager.UnregisterGroup(this);
				groupsManager.GetOnPlayableGroupRemoved().Invoke(this);
			}
		}
		
		DestroyEntities(m_aSceneGroupUnitInstances);
		DestroyEntities(m_aSceneWaypointInstances);
		RemoveStaticWaypointRefs(m_aStaticWaypoints);
		ClearRefs(m_aUsableVehicles);
	}
};

//------------------------------------------------------------------------
enum EGroupState
{
	IDLE = 0,
	ATTACKING = 1,
	MOVING = 2,
	INVESTIGATING = 3,
	RETREATING = 4,
	DEFENDING = 5,
	MANEUVERING = 6,
	REQ_SUPPORT = 7,
	REQ_ORDERS = 8,
	FOLLOW = 9,
};

//------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_WaypointPrefabLocation
{
	[Attribute("{750A8D1695BD6998}AI/Entities/Waypoints/AIWaypoint.et", UIWidgets.ResourceAssignArray, "Prefab for the waypoint")]
	ResourceName m_WPPrefabName;
	
	[Attribute("", UIWidgets.EditBox, "Waypoint name")]
	string m_WPInstanceName;
	
	[Attribute("", UIWidgets.EditBox, "Waypoint location")]
	vector m_WPWorldLocation;
	
	[Attribute("0", UIWidgets.EditBox, "Waypoint completion radius (-1 dont override default)")]
	float m_WPRadiusOverride;
	
	[Attribute("0", UIWidgets.EditBox, "Waypoint completion time (-1 dont override default)")]
	float m_WPTimeOverride;
};
