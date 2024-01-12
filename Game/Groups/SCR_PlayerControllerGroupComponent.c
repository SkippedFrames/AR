[EntityEditorProps(category: "GameScripted/Groups", description: "This component should be attached to player controller and is used by groups to send requests to server.")]
class SCR_PlayerControllerGroupComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerControllerGroupComponent : ScriptComponent
{
	protected int m_iGroupID = -1;
	// Map with playerID and list of groups the player was invited to
	protected ref map<int, ref array<int>> m_mPlayerInvitesToGroups;
	protected ref ScriptInvoker<int, int> m_OnInviteReceived = new ScriptInvoker<int, int>();
	protected ref ScriptInvoker<int> m_OnInviteAccepted;
	protected ref ScriptInvoker<int> m_OnInviteCancelled;
	protected ref ScriptInvoker<int> m_OnGroupChanged;

	protected int m_iUISelectedGroupID = -1;
	protected int m_iGroupInviteID = -1;
	protected int m_iGroupInviteFromPlayerID = -1;
	protected string m_sGroupInviteFromPlayerName; //Player name is saved to get the name of the one who invited even if that player left the server	
	
	const ref Color DEFAULT_COLOR = new Color(0, 0, 0, 0.4);
	
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerGroupComponent GetPlayerControllerComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerGroupComponent GetLocalPlayerControllerGroupComponent()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestCreateGroup()
	{
		Rpc(RPC_AskCreateGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestKickPlayer(int playerID)
	{
		Rpc(RPC_AskKickPlayer, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestPromoteLeader(int playerID)
	{
		Rpc(RPC_AskPromoteLeader, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestPrivateGroupChange(int playerID, bool isPrivate)
	{
		Rpc(RPC_ChangePrivateGroup, playerID, isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayerRequestToJoinPrivateGroup(int playerID, RplId groupID)
	{		
		Rpc(RPC_PlayerRequestToJoinPrivateGroup, playerID, groupID);	
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearAllRequesters(RplId groupID)
	{		
		Rpc(RPC_ClearAllRequesters, groupID);	
		RPC_ClearAllRequesters(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		PlayerController playerController = PlayerController.Cast(GetOwner());
		if (!playerController)
			return -1;
		
		return playerController.GetPlayerId();
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanPlayerJoinGroup(int playerID, notnull SCR_AIGroup group)
	{
		// First we check the player is in the faction of the group
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			// TODO (langepau): Remove temporary debug logging when respawn issue is fixed.
			Faction playerFaction = factionManager.GetPlayerFaction(playerID);
			Faction groupFaction = group.GetFaction();
			if (playerFaction != groupFaction)
			{
				#ifdef DEPLOY_MENU_DEBUG
				Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - Faction mis-match! See, below:", playerID, group), LogLevel.ERROR);
				Print(playerFaction);
				if (playerFaction)
					Print(playerFaction.GetFactionKey());

				Print(groupFaction);
				if (groupFaction)
					Print(groupFaction.GetFactionKey());
				#endif

				return false;
			}
		}
		else
		{
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - No SCR_FactionManager!", playerID, group), LogLevel.ERROR);
			#endif
		}

		// Groups manager doesn't exist, no point in continuing, cannot join
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
		{	
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - No SCR_GroupsManagerComponent!", playerID, group), LogLevel.ERROR);
			#endif
			return false;
		}

		// Cannot join a full group
		if (group.IsFull())
			return false;

		// Cannot join the group we are in already
		if (groupsManager.GetPlayerGroup(playerID) == group)
		{
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - Already in group!", playerID, group), LogLevel.ERROR);
			#endif
			return false;
		}	

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeader(int playerID, notnull SCR_AIGroup group)
	{
		return playerID == group.GetLeaderID();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeaderOwnGroup()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;
			
		//get controller from owner, because if this is used on server we cannot get local players player controller :wesmart:
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
		if (!controller)
			return false;
		
		int playerID = controller.GetPlayerId();
		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return false;
		
		return IsPlayerLeader(playerID, playerGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanInvitePlayer(int playerID)
	{
		// Our group id is not valid -> cannot invite anyone
		if (m_iGroupID < 0)
			return false;
		
		// Groups manager doesn't exist, no point in continuing, cannot invite
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		// We get our group
		SCR_AIGroup group = groupsManager.GetPlayerGroup(GetPlayerID());
		
		if (!group)
			return false;
		
		// Check if the player can join us
		if (!CanPlayerJoinGroup(playerID, group))
			return false;
		
		// Already invited this player, cannot invite again
		if (WasAlreadyInvited(playerID))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool WasAlreadyInvited(int playerID)
	{
		// The map is not initialized -> didn't invite anyone yet
		if (!m_mPlayerInvitesToGroups)
			return false;
		
		// We didn't invite this player to any group yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			return false;
		
		// If our group is in the array of invites for this player, we return true (already invited)
		// Otherwise we return false (wasn't invited yet)
		return m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptJoinPrivateGroup(int playerID, bool accept)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		
		SCR_AIGroup group = groupManager.FindGroup(GetGroupID());
		if (!group)
			return;
			
		group.GetOnJoinPrivateGroupConfirm().Invoke(DEFAULT_COLOR); // Saphyr TODO: temporary before definition from art dept.
		
		if (accept)		
			Rpc(RPC_ConfirmJoinPrivateGroup,playerID, group.GetGroupID());
		else 
			Rpc(RPC_CancelJoinPrivateGroup, playerID, group.GetGroupID());	
			
	}	
	
	//------------------------------------------------------------------------------------------------
	void InvitePlayer(int playerID)
	{
		// When group id is not valid, return
		if (m_iGroupID < 0)
			return;
		
		// Init map if not initialized yet
		if (!m_mPlayerInvitesToGroups)
			m_mPlayerInvitesToGroups = new map<int, ref array<int>>();
		
		// Init array of groups the playerID was invited to if not initialized yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			m_mPlayerInvitesToGroups.Insert(playerID, {});
		
		// We already invited this player to our group, don't invite again
		if (m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID))
			return;
		
		// We didn't invite this player to our group yet
		// Get an array of all the groups the local player invited the other player to
		array<int> invitedGroups = m_mPlayerInvitesToGroups.Get(playerID);
		
		// Invite the player and log the invitation
		Rpc(RPC_AskInvitePlayer, playerID);
		invitedGroups.Insert(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void InviteThisPlayer(int groupID, int fromPlayerID)
	{
		Rpc(RPC_DoInvitePlayer, groupID, fromPlayerID)
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		if (m_iGroupInviteID >= 0)
		{
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager)
				return;
			
			SCR_AIGroup group = groupManager.FindGroup(m_iGroupInviteID);
			if (!group)
				return;
			
			group.RemoveRequester(GetPlayerID());
			
			RequestJoinGroup(m_iGroupInviteID);
			m_iGroupInviteID = -1;
			if (m_OnInviteAccepted)
				m_OnInviteAccepted.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteReceived()
	{
		return m_OnInviteReceived;
	}
	
	void OnGroupDeleted(SCR_AIGroup group)
	{
		if (!group)
			return; 
		
		//in case of the selected group being deleted, remove it from selected
		if (group.GetGroupID() == GetSelectedGroupID())
			SetSelectedGroupID(-1);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_PlayerRequestToJoinPrivateGroup(int playerID, RplId groupID)
	{	
		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (!group)
			return;
		
		group.AddRequester(playerID);		
		SCR_NotificationsComponent.SendToPlayer(group.GetLeaderID(), ENotification.GROUPS_REQUEST_JOIN_PRIVATE_GROUP, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ConfirmJoinPrivateGroup(int playerID, int groupID)
	{	
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
			
		SCR_AIGroup group = SCR_AIGroup.Cast(groupManager.FindGroup(groupID));
		if (!group)
			return;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		
		playerComponent.RequestJoinGroup(group.GetGroupID());	
					
		group.RemoveRequester(playerID);	
		
		SCR_NotificationsComponent.SendToPlayer(playerID, ENotification.GROUPS_REQUEST_ACCEPTED);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_CancelJoinPrivateGroup(int playerID, int groupID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(groupManager.FindGroup(groupID));
		if (!group)
			return;
		
		group.RemoveRequester(playerID);		
		group.AddDeniedRequester(playerID);	
		
		SCR_NotificationsComponent.SendToPlayer(playerID, ENotification.GROUPS_REQUEST_DENIED);	
	}	
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoInvitePlayer(int groupID, int fromPlayerID)
	{
		m_iGroupInviteID = groupID;
		m_iGroupInviteFromPlayerID = fromPlayerID;
		
		//Save player name so it can be obtained even if the player left
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			m_sGroupInviteFromPlayerName = playerManager.GetPlayerName(fromPlayerID);
		
		m_OnInviteReceived.Invoke(groupID, fromPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskInvitePlayer(int playerID)
	{
		PlayerController invitedPlayer = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!invitedPlayer)
			return;
		
		SCR_PlayerControllerGroupComponent invitedPlayerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(invitedPlayer.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!invitedPlayerGroupComponent)
			return;
		
		invitedPlayerGroupComponent.InviteThisPlayer(m_iGroupID, GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskCreateGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetPlayerFaction(GetPlayerID());
		if (!faction)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if(!scrFaction)
			return;
		
		// We check if there is any empty group already for our faction
		if (groupsManager.TryFindEmptyGroup(faction))
			return;
		
		// We check if other then predefined group can be created
		if(scrFaction.GetCanCreateOnlyPredefinedGroups())
			return;
		
		// No empty group found, we allow creation of new group		
		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(faction);
		
		// No new group was created, return
		if (!newGroup)
			return;
		
		// New group sucessfully created
		// The player should be automatically added/moved to it
		RPC_AskJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskKickPlayer(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		//requesting player is not leader of the targets group, do nothing
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		SCR_AIGroup newGroup = groupsManager.GetFirstNotFullForFaction(group.GetFaction(), group, true);
		if (!newGroup)
			newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());
				
		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPromoteLeader(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		groupsManager.SetGroupLeader(group.GetGroupID(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ChangePrivateGroup(int playerID, bool isPrivate)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		groupsManager.SetPrivateGroup(group.GetGroupID(), isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestJoinGroup(int groupID)
	{
		Rpc(RPC_AskJoinGroup, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoChangeGroupID(int groupID)
	{
		m_iGroupID = groupID;
		if (groupID == m_iGroupInviteID)
		{
			//reset the invite if player manually joined the group he is invited into
			m_iGroupInviteID = -1;
			if (m_OnInviteCancelled)
				m_OnInviteCancelled.Invoke();
		}
		if (m_OnGroupChanged)
			GetGame().GetCallqueue().CallLater(OnGroupChangedDelayed, 0, false, groupID);
	}

	//------------------------------------------------------------------------------------------------	
	protected void OnGroupChangedDelayed(int groupId)
	{
		m_OnGroupChanged.Invoke(groupId);	
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskJoinGroup(int groupID)
	{
		// Trying to join the same group, reject.
		if (groupID == m_iGroupID)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		int groupIDAfter;
		if (m_iGroupID != -1)
			groupIDAfter = groupsManager.MovePlayerToGroup(GetPlayerID(), m_iGroupID, groupID);
		else
			groupIDAfter = groupsManager.AddPlayerToGroup(groupID, GetPlayerID());
		
		if (groupIDAfter != m_iGroupID)
		{
			m_iGroupID = groupIDAfter;
			Rpc(RPC_DoChangeGroupID, groupIDAfter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool InitiateComponents(int playerID, out SCR_GroupsManagerComponent groupsManager, out SCR_PlayerControllerGroupComponent playerGroupController , out SCR_AIGroup group)
	{
		groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		if (!playerGroupController)
			return false;
		
		group = groupsManager.GetPlayerGroup(playerID);
		if (!group)
			return false;
		return true;
	}
	//------------------------------------------------------------------------------------------------
		int GetSelectedGroupID()
	{
		return m_iUISelectedGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteAccepted()
	{
		if (!m_OnInviteAccepted)
			m_OnInviteAccepted =  new ScriptInvoker<int>();
		return m_OnInviteAccepted;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnGroupChanged()
	{
		if (!m_OnGroupChanged)
			m_OnGroupChanged =  new ScriptInvoker<int>();
		return m_OnGroupChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInviteCancelled()
	{
		if (!m_OnInviteCancelled)
			m_OnInviteCancelled =  new ScriptInvoker<int>();
		return m_OnInviteCancelled;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupInviteID()
	{
		return m_iGroupInviteID;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetGroupInviteID(int value)
	{
		m_iGroupInviteID = value;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupInviteFromPlayerID()
	{
		return m_iGroupInviteFromPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetGroupInviteFromPlayerName()
	{
		return m_sGroupInviteFromPlayerName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedGroupID(int groupID)
	{
		m_iUISelectedGroupID = groupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetCustomGroupDescription(int groupID, string desc)
	{
		Rpc(RPC_AskSetCustomDescription, groupID, desc, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomDescription(int groupID, string desc, int authorID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomDescription(desc, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetGroupMaxMembers(int groupID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetMaxMembers() == maxMembers || maxMembers < 0)
			return;
		
		Rpc(RPC_AskSetGroupMaxMembers, groupID, maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetGroupMaxMembers(int groupID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetMaxMembers() == maxMembers || maxMembers < 0)
			return;
		
		group.SetMaxMembers(maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	//! sets custom frequency (KHz) for a group. Can set frequency that is already claimed. 
	//! Claims set frequency if not already claimed.
	//! Frequency set by this method will not be used by automatically created groups.
	void RequestSetCustomFrequency(int groupID, int frequency)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetRadioFrequency() == frequency || frequency < 0)
			return;
		
		Rpc(RPC_AskSetFrequency, groupID, frequency);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetNewGroupsAllowed(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		Rpc(RPC_AskSetNewGroupsAllowed, isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetCanPlayersChangeAttributes(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		Rpc(RPC_AskSetCanPlayersChangeAttributes, isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSetCustomGroupName(int groupID, string name)
	{
		Rpc(RPC_AskSetCustomName, groupID, name, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomName(int groupID, string name, int authorID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomName(name, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetNewGroupsAllowed(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		groupsManager.SetNewGroupsAllowed(isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCanPlayersChangeAttributes(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		groupsManager.SetCanPlayersChangeAttributes(isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetFrequency(int groupID, int frequency)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetRadioFrequency() == frequency || frequency < 0)
			return;
		
		SCR_Faction groupFaction = SCR_Faction.Cast(group.GetFaction());
		if (!groupFaction)
			return;
		
		int formerFrequency = group.GetRadioFrequency();
		int foundGroupsWithFrequency = 0;
		
		//no null check here because the array will always at least contain the group that was passed as parameter into this method
		array<SCR_AIGroup> existingGroups = groupsManager.GetPlayableGroupsByFaction(groupFaction);
		
		foreach (SCR_AIGroup checkedGroup: existingGroups)
		{
			if (checkedGroup.GetRadioFrequency() == formerFrequency)
				foundGroupsWithFrequency++;
		}
		
		//if there is only our group with this frequency or none, release it before changing our frequency
		if (foundGroupsWithFrequency <= 1)
			groupsManager.ReleaseFrequency(formerFrequency, groupFaction);
		
		//if the new frequency is unclaimed, claime it so newly created groups do not get it by default
		if (!groupsManager.IsFrequencyClaimed(frequency, groupFaction))
			groupsManager.ClaimFrequency(frequency, groupFaction);
		
		group.SetRadioFrequency(frequency);
	}
	 
	//------------------------------------------------------------------------------------------------
	void RequestSetGroupFlag(int groupID, int flagIndex, bool isFromImageset)
	{
		Rpc(RPC_AskSetGroupFlag, groupID, flagIndex, isFromImageset);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetGroupFlag(int groupID, int flagIndex, bool isFromImageset)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetGroupFlag(flagIndex, isFromImageset);		
	}
	
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ClearAllRequesters(RplId groupID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (!group)
			return;
		
		group.ClearRequesters();
		group.ClearDeniedRequester();	
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestCreateSlaveGroup(RplId rplCompID)
	{
		Rpc(RPC_AskCreateSlaveGroup, rplCompID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskCreateSlaveGroup(RplId rplCompID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(commandingManager.GetGroupPrefab()));
		if (!groupEntity)
			return;
		
		
		SCR_AIGroup group = SCR_AIGroup.Cast(groupEntity);
		if (!group)
			return;
		
		RplComponent slaveRplComp = RplComponent.Cast(group.FindComponent(RplComponent));
		if (!slaveRplComp)
			return;
		
		groupManager.RequestSetGroupSlave(rplCompID, slaveRplComp.Id());
		return;
	}
	
		
	//------------------------------------------------------------------------------------------------
	bool IsAICharacterInAnyGroup(SCR_ChimeraCharacter character, SCR_Faction faction)
	{
		//TODO: kuceramar: come up with better solution that doesnt include going through all groups
		//possible JIP problems due ot using SCR_CHimeraCharacter
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;
		array<SCR_AIGroup> groups = groupManager.GetPlayableGroupsByFaction(faction);
		
		bool isMember = false;
		foreach(SCR_AIGroup group : groups)
		{
			isMember = group.IsAIControlledCharacterMember(character);
			if (isMember)
				return isMember;
		}
		
		return isMember;
	}
		
	//------------------------------------------------------------------------------------------------
	void RequestAddAIAgent(SCR_ChimeraCharacter character, int playerID)
	{
		RplComponent rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		Rpc(RPC_AskAddAIAgent, rplComp.Id(), playerID);
	} 
	
		//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskAddAIAgent(RplId characterID, int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(characterID));
		if (!rplComp)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		
		if (!character)
			return;		
		if (!group.IsPlayerLeader(playerID))
			return;
		 
		AddAIToSlaveGroup(character, group);
	}
	
			
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void AddAIToSlaveGroup(notnull IEntity controlledEntity, SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;
		
		if (!slaveGroup.IsAIActivated())
			slaveGroup.Activate();
		
		slaveGroup.AddAgentFromControlledEntity(controlledEntity);
		
		RplId groupCompID, characterCompID;
		RplComponent rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		groupCompID = rplComp.Id();
		rplComp = RplComponent.Cast(controlledEntity.FindComponent(RplComponent)); 
		characterCompID = rplComp.Id();
		
		groupManager.AskAddAiMemberToGroup(groupCompID, characterCompID);
	}
	
		
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskRemoveAIAgent(RplId characterID, int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(characterID));
		if (!rplComp)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		
		if (!character)
			return;		
		
		if (!group.IsPlayerLeader(playerID))
			return;
		
		RemoveAiFromSlaveGroup(character, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void RemoveAiFromSlaveGroup(notnull IEntity controlledEntity, SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;
		
		//deactivate the group since we are removing last AI member
		if (slaveGroup.GetAgentsCount() == 1)
			slaveGroup.Deactivate();
		
		slaveGroup.RemoveAgentFromControlledEntity(controlledEntity);
		
		RplId groupCompID, characterCompID;
		RplComponent rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		groupCompID = rplComp.Id();
		rplComp = RplComponent.Cast(controlledEntity.FindComponent(RplComponent)); 
		if (!rplComp)
			return;
		
		characterCompID = rplComp.Id();
		
		groupManager.AskRemoveAiMemberFromGroup(groupCompID, characterCompID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestRemoveAgent(SCR_ChimeraCharacter character, int playerID)
	{
		RplComponent rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		Rpc(RPC_AskRemoveAIAgent, rplComp.Id(), playerID);
	}	
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_GROUPS, "Groups", "GameCode");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_GROUPS_ENABLE_DIAG, "", "Enable groups diag", "Groups");
		ConnectToDiagSystem(owner);
#endif
		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem(owner);
#endif
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetPlayersGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return null;
		
		return groupsManager.FindGroup(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActualGroupFrequency()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return 0;
		
		SCR_AIGroup group = groupsManager.FindGroup(m_iGroupID);
		if (!group)
			return 0;
		
		return group.GetRadioFrequency();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_GROUPS_ENABLE_DIAG))
			return;
		
		DbgUI.Begin("Groups");
				
		int playerID = SCR_PlayerController.GetLocalPlayerId();		
		Faction faction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			faction = factionManager.GetPlayerFaction(playerID);
		
		if (!faction)
		{
			DbgUI.Text("Groups do not support factionless players now!!");
			DbgUI.End();
			return;
		}
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		
		DbgUI.Text("Your Faction:" + faction.GetFactionKey());
		DbgUI.Spacer(2);
		
		DbgUI.Text("Playable groups of your faction:");		
		ListGroupsFromFaction(faction);
		DbgUI.Spacer(6);
		
		DbgUI.Text("Your group:");
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		if (group)
		{
			DbgUI.Text("." + group.ToString());
			Print(group.ToString());
		}
		DbgUI.Spacer(2);
		
		if (DbgUI.Button("Create and join group for my faction"))
			CreateAndJoinGroup(faction);
		
		DbgUI.Spacer(2);
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void ListGroupsFromFaction(Faction faction)
	{
		array<SCR_AIGroup> groups = {};
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		groups = groupsManager.GetPlayableGroupsByFaction(faction);
		if (!groups)
		{
			DbgUI.Text("No groups for your faction!!");
			return;
		}
	
		foreach(SCR_AIGroup group : groups)
		{
			DbgUI.Text(group.ToString());			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateAndJoinGroup(Faction faction)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || !groupsManager.IsPlayerInAnyGroup(SCR_PlayerController.GetLocalPlayerId()))
			return;
		
		SCR_AIGroup group = groupsManager.GetFirstNotFullForFaction(faction, null, true);
		if (group)
			playerGroupController.RequestJoinGroup(group.GetGroupID());
		else
			playerGroupController.RequestCreateGroup(); //requestCreateGroup automatically puts player to the newly created group
		
	}
};
