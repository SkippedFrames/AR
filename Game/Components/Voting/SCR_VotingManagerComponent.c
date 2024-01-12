void ScriptInvoker_VotingManagerStartMethod(EVotingType type, int value);
typedef func ScriptInvoker_VotingManagerStartMethod;
typedef ScriptInvokerBase<ScriptInvoker_VotingManagerStartMethod> ScriptInvoker_VotingManagerStart;

void ScriptInvoker_VotingManagerEndMethod(EVotingType type, int value, int winner);
typedef func ScriptInvoker_VotingManagerEndMethod;
typedef ScriptInvokerBase<ScriptInvoker_VotingManagerEndMethod> ScriptInvoker_VotingManagerEnd;

void ScriptInvoker_VotingManagerPlayerMethod(EVotingType type, int value, int playerID);
typedef func ScriptInvoker_VotingManagerPlayerMethod;
typedef ScriptInvokerBase<ScriptInvoker_VotingManagerPlayerMethod> ScriptInvoker_VotingManagerPlayer;

void ScriptInvoker_VotingManagerVoteCountChangedMethod(EVotingType type, int value, int voteCount);
typedef func ScriptInvoker_VotingManagerVoteCountChangedMethod;
typedef ScriptInvokerBase<ScriptInvoker_VotingManagerVoteCountChangedMethod> ScriptInvoker_VotingManagerVoteCountChanged;

[ComponentEditorProps(category: "GameScripted/Voting", description: "")]
class SCR_VotingManagerComponentClass: SCR_BaseGameModeComponentClass
{
};
class SCR_VotingManagerComponent: SCR_BaseGameModeComponent
{
	[Attribute(desc: "Voting Templates, please use configs and the VotingManagerComponent prefab for default gamemode voting templates")]
	protected ref array<ref SCR_VotingBase> m_aVotingTemplates;
	
	[Attribute("1")]
	protected float m_fUpdateStep;
	
	protected int m_iHostPlayerID = -1;
	protected float m_fUpdateLength;
	protected ref array<ref SCR_VotingBase> m_aVotingInstances = {};
	protected ref set<SCR_VotingBase> m_aAbstainedVotingInstances = new set<SCR_VotingBase>(); //~ Player locally abstained from voting
	protected ref array<ref Tuple2<int, int>> m_LocalVoteRecords = {};
	
	protected ref ScriptInvoker_VotingManagerStart m_OnVotingStart = new ScriptInvoker_VotingManagerStart();
	protected ref ScriptInvoker_VotingManagerEnd m_OnVotingEnd = new ScriptInvoker_VotingManagerEnd();
	protected ref ScriptInvoker_VotingManagerPlayer m_OnVote = new ScriptInvoker_VotingManagerPlayer();
	protected ref ScriptInvoker_VotingManagerPlayer m_OnRemoveVote = new ScriptInvoker_VotingManagerPlayer();
	protected ref ScriptInvoker_VotingManagerStart m_OnVoteLocal = new ScriptInvoker_VotingManagerStart();
	protected ref ScriptInvoker_VotingManagerStart m_OnRemoveVoteLocal = new ScriptInvoker_VotingManagerStart();
	protected ref ScriptInvoker_VotingManagerStart m_OnAbstainVoteLocal = new ScriptInvoker_VotingManagerStart();
	protected ref ScriptInvoker_VotingManagerVoteCountChanged m_PlayerVoteCountChanged = new ScriptInvoker_VotingManagerVoteCountChanged();
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, static, anywhere
	/*!
	Get local instance of this component.
	\return Local voting manager component
	*/
	static SCR_VotingManagerComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SCR_VotingManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_VotingManagerComponent));
		else
			return null;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, server
	/*!
	Vote!
	If the voting does not exist yet, start it.
	To be called from SCR_VoterComponent! Do not call from elsewhere unless you want to subvert democracy!
	\param playerID Player who cast the vote
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void Vote(int playerID, EVotingType type, int value)
	{
		//--- Prevent voting fraud, non-existent players cannot vote!
		if (!GetGame().GetPlayerManager().IsPlayerConnected(playerID))
		{
			Print(string.Format("Non-existent player %1 is attempting to vote in %2 for %3", playerID, typename.EnumToString(EVotingType, type), value), LogLevel.WARNING);
			return;
		}
		
		//--- Find the voting, or create it if it doesn't exist yet
		SCR_VotingBase voting = FindVoting(type, value);
		if (!voting)
		{
			if (!StartVoting(type, value))
				return;
			
			voting = FindVoting(type, value);
		}
		
		//--- Voting for the same value, ignore
		if (voting.GetPlayerVote(playerID) == value)
			return;
		
		//--- Set the vote
		voting.SetVote(playerID, value);
		
		//~ If vote was successfully added send RPC to update vote count for players
		if (voting.AddPlayerVotedServer(playerID))
			Rpc(RPC_PlayerVoteCountChanged, type, value, voting.GetCurrentVoteCount());
		
		m_OnVote.Invoke(type, value, playerID);
		
		//--- Check if the vote completed the voting
		EVotingOutcome outcome = EVotingOutcome.EVALUATE;
		if (voting.Evaluate(outcome))
			EndVoting(voting, outcome);
	}
	/*!
	Remove previously cast vote.
	To be called from SCR_VoterComponent! Do not call from elsewhere unless you want to subvert democracy!
	\param playerID Player who cast the vote
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void RemoveVote(int playerID, EVotingType type, int value)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
		{
			//--- Cancel the vote when no votes are left (i.e., last player canceled theirs) or when player who is target of the vote cancels their vote
			if (voting.RemoveVote(playerID) || (voting.IsValuePlayerID() && playerID == value))
				EndVoting(type, value, EVotingOutcome.FORCE_FAIL);

			//~ If vote was successfully removed send RPC to update vote count for players
			if (voting.RemovePlayerVotedServer(playerID))
				Rpc(RPC_PlayerVoteCountChanged, type, value, voting.GetCurrentVoteCount());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_PlayerVoteCountChanged(EVotingType type, int value, int voteCount)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
		{
			voting.SetCurrentVoteCount(voteCount);
			m_PlayerVoteCountChanged.Invoke(type, value, voteCount);
		}
	}
	
	/*!
	Start voting.
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return True if the voting started
	*/
	bool StartVoting(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		if (FindVoting(type, value))
			return false;
		
		StartVotingBroadcast(type, value);
		Rpc(StartVotingBroadcast, type, value);
		
		//-- Find the voting; cannot use returned value, StartVotingBroadcast is replicated and such functions cannot return anything
		SCR_VotingBase voting = FindVoting(type, value);
		return voting != null;
	}
	/*!
	End voting.
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\param outcome How should the winner be evaluated
	*/
	void EndVoting(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE, EVotingOutcome outcome = EVotingOutcome.EVALUATE)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
			EndVoting(voting, outcome);
	}
	/*!
	Get value cast by given player.
	\param playerID Player who cast the vote
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return Cast value, or default when player did not vote
	*/
	int GetPlayerVote(int playerID, EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
			return voting.GetPlayerVote(playerID);
		else
			return SCR_VotingBase.DEFAULT_VALUE;
	}
	
	//------------------------------------------------------------------------------------------------
 	/*!
	Get the amount of players needed
	\param type Type of the vote
	\param value Value of the vote
	\param[out] currentVotes Current votes the vote has
	\param[out] votesRequired Votes required to be successfull
	return True if successfully obtained the requirement data
	*/
	bool GetVoteCounts(EVotingType type, int value, out int currentVotes, out int votesRequired)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (!voting)
			return false;
		
		currentVotes = voting.GetCurrentVoteCount();
		votesRequired = voting.GetVoteCountRequired();
		
		return votesRequired > 0;
	}
	
	/*!
	Get event called everywhere when a new voting is created.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerStart GetOnVotingStart()
	{
		return m_OnVotingStart;
	}
	/*!
	Get event called everywhere when a player voted or abstained
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerVoteCountChanged GetOnVoteCountChanged()
	{
		return m_PlayerVoteCountChanged;
	}
	/*!
	Get event called everywhere when a voting ends (e.g., time runs out, number of votes reaches threshold, or the player about whom the voting was disconnects).
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerEnd GetOnVotingEnd()
	{
		return m_OnVotingEnd;
	}
	/*!
	Get event called on server when a player casts a vote.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerPlayer GetOnVote()
	{
		return m_OnVote;
	}
	/*!
	Get event called on server when a player removes their vote.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerPlayer GetOnRemoveVote()
	{
		return m_OnRemoveVote;
	}
	/*!
	Get event called on player when they cast a vote.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerStart GetOnVoteLocal()
	{
		return m_OnVoteLocal;
	}
	/*!
	Get event called on player when they remove their vote.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerStart GetOnRemoveVoteLocal()
	{
		return m_OnRemoveVoteLocal;
	}	
	
	/*!
	Get event called on player when they remove their vote.
	\return Script invoker
	*/
	ScriptInvoker_VotingManagerStart GetOnAbstainVoteLocal()
	{
		return m_OnAbstainVoteLocal;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, anywhere
	/*
	Get player ID of the host player.
	\return Player ID. 0 on dedicated server.
	*/
	int GetHostPlayerID()
	{
		if (m_iHostPlayerID == -1)
			m_iHostPlayerID = SCR_PlayerController.GetLocalPlayerId();
		
		return m_iHostPlayerID;
	}
	/*!
	Check if a voting is available in the current world.
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return True if available
	*/
	bool IsVotingAvailable(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingBase template = FindTemplate(type);
		return template && (template.IsAvailable(value, IsVoting(type, value)) || DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VOTING_ENABLE_ALL));
	}
	/*!
	Check if a voting with given params is currently in effect.
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return True if the voting is ongoing
	*/
	bool IsVoting(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		return FindVoting(type, value) != null;
	}
	/*!
	Check if there is a vote of certain type about given player.
	E.g., is there a vote about player being kicked?
	\param playerID Player ID
	\param type Type of the vote
	\return True if the voting is ongoing
	*/
	bool IsVotingAboutPlayer(int playerID, EVotingType type)
	{
		SCR_VotingBase voting = FindVoting(type, playerID);
		if (voting)
			return voting.IsValuePlayerID();
		else
			return SCR_VotingBase.DEFAULT_VALUE;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Local player has abstained from voting eg: VoteRemove was called
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	*/
	void AbstainVoteLocally(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingBase vote = FindVoting(type, value);
		if (!vote || m_aAbstainedVotingInstances.Contains(vote))
			return;
		
		m_aAbstainedVotingInstances.Insert(vote);
		
		//~ Script invoker
		m_OnAbstainVoteLocal.Invoke(type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	If local player has abstained from voting eg: VoteRemove was called
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return True if the voting was abstained
	*/
	bool HasAbstainedLocally(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		if (IsLocalVote(type, value))
			return false;
		
		SCR_VotingBase vote = FindVoting(type, value);
		if (vote && m_aAbstainedVotingInstances.Contains(vote))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRemainingDurationOfVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingBase vote = FindVoting(type, value);
		if (vote)
			return vote.GetRemainingDuration();
		
		return -1;
	}
	
	/*!
	Get all votings about given player.
	Returns only referendums, not elections
	e.g., vote to KICK a player is a referendum, but vote to make him/her an ADMIN is election in which multiple players can compete, therefore it's not a vote about the player)
	\param playerID Player ID
	\param[out] outVotingTypes Array to be filled with vote types
	\param onlyTemplates True to scan all voting templates, not only active votings
	\param onlyAvailable True to scan only votings which can be available in current situation, evaluated by SCR_VotingBase.IsAvailable(). Note that Vote class specific values such as m_bFactionSpecific in SCR_VotingKick are not properly checked and need a IsVotingAvailable specifically if not checking for templates only
	\return Number of votings
	*/
	int GetVotingsAboutPlayer(int playerID, out notnull array<EVotingType> outVotingTypes, bool onlyTemplates = false, bool onlyAvailable = false)
	{
		array<ref SCR_VotingBase> votings;
		if (onlyTemplates)
			votings = m_aVotingTemplates;
		else
			votings = m_aVotingInstances;
		
		outVotingTypes.Clear();
		foreach (SCR_VotingBase voting: votings)
		{
			//if (template.IsAvailable(playerID, IsVoting(template.GetType(), playerID)) || DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VOTING_ENABLE_ALL))
			bool isMatchingValue = onlyTemplates || voting.GetValue() == playerID;
			bool isAvailable = !onlyAvailable || voting.IsAvailable(playerID, IsVoting(voting.GetType(), playerID)) || (onlyTemplates && DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VOTING_ENABLE_ALL));
			if (voting.IsValuePlayerID() && isMatchingValue && isAvailable)
				outVotingTypes.Insert(voting.GetType());
		}
		return outVotingTypes.Count();
	}
	
	/*!
	Get all active player vote types (not caring which player is the target) 
	Does not care what player the target is or if it is a  referendum or election
	\param about Player True to return votings that are about players, false to return those which are *not* about players
	\param[out] outVotingTypes Array to be filled with vote types
	\param onlyTemplates True to scan all voting templates, not only active votings
	\param onlyAvailable True to scan only votings which can be available in current situation, evaluated by SCR_VotingBase.IsAvailable(). Note that Vote class specific values such as m_bFactionSpecific in SCR_VotingKick are not properly checked and need a IsVotingAvailable specifically if not checking for templates only
	\return Number of votings
	*/
	int GetAllVotingsAboutPlayer(bool aboutPlayer, out notnull array<EVotingType> outVotingTypes, bool onlyTemplates = false, bool onlyAvailable = false)
	{
		array<ref SCR_VotingBase> votings;
		if (onlyTemplates)
			votings = m_aVotingTemplates;
		else
			votings = m_aVotingInstances;
		
		outVotingTypes.Clear();
		foreach (SCR_VotingBase voting: votings)
		{
			if (aboutPlayer == voting.IsValuePlayerID() && (!onlyAvailable || voting.IsAvailable(voting.GetValue(), true)))
				outVotingTypes.Insert(voting.GetType());
		}
		return outVotingTypes.Count();
	}
	
	/*!
	Get all votings with value defined.
	Returns only referendums, not elections
	\param[out] outVotingTypes Array to be filled with vote types
	\param[out] outValues Array to be filled with vote values
	\param onlyTemplates True to scan all voting templates, not only active votings
	\param onlyAvailable True to scan only votings which can be available in current situation, evaluated by SCR_VotingBase.IsAvailable(). Note that Vote class specific values such as m_bFactionSpecific in SCR_VotingKick are not properly checked and need a IsVotingAvailable specifically if not checking for templates only
	\return Number of votings
	*/
	int GetAllVotingsWithValue(out notnull array<EVotingType> outVotingTypes, array<EVotingType> outValues, bool onlyTemplates = false, bool onlyAvailable = false)
	{
		array<ref SCR_VotingBase> votings;
		if (onlyTemplates)
			votings = m_aVotingTemplates;
		else
			votings = m_aVotingInstances;
		
		outVotingTypes.Clear();
		foreach (SCR_VotingBase voting: votings)
		{
			if (voting.GetValue() != SCR_VotingBase.DEFAULT_VALUE && (!onlyAvailable || voting.IsAvailable(voting.GetValue(), true)))
			{
				outVotingTypes.Insert(voting.GetType());
				outValues.Insert(voting.GetValue());
			}
		}
		return outVotingTypes.Count();
	}
	
	/*!
	Get a array of all active votes with the specific voteType
	\param type The type of active vote that needs to be searched
	\param[out] outValues list of all active vote values of the given type
	\param onlyTemplates True to scan all voting templates, not only active votings
	\param onlyAvailable True to scan only votings which can be available in current situation, evaluated by SCR_VotingBase.IsAvailable(). Note that Vote class specific values such as m_bFactionSpecific in SCR_VotingKick are not properly checked and need a IsVotingAvailable specifically if not checking for templates only
	\return count of active vote values
	*/
	int GetAllVotingValues(EVotingType type, out notnull array<int> outValues, bool onlyTemplates = false, bool onlyAvailable = false)
	{
		array<ref SCR_VotingBase> votings;
		if (onlyTemplates)
			votings = m_aVotingTemplates;
		else
			votings = m_aVotingInstances;
		
		outValues.Clear();
		foreach (SCR_VotingBase voting: votings)
		{
			if (voting.GetType() == type && (!onlyAvailable || voting.IsAvailable(voting.GetValue(), true)))
				outValues.Insert(voting.GetValue());
		}
		return outValues.Count();
	}
	
	/*!
	Get UI representation of given voting.
	\param type Type of the vote
	\return Voting UI info
	*/
	SCR_VotingUIInfo GetVotingInfo(EVotingType type)
	{
		SCR_VotingBase template = FindTemplate(type);
		if (template)
			return template.GetInfo();
		else
			return null;
	}
	/*!
	Get name of the value.
	\param type Type of the vote
	\param value Value
	\return Value name
	*/
	string GetValueName(EVotingType type, int value)
	{
		SCR_VotingBase template = FindTemplate(type);
		if (template)
			return template.GetValueName(value);
		else
			return value.ToString();
	}
	/*!
	Save local vote for given voting.
	It's purely informative, does not affect voting outcome!
	To be called from SCR_VoterComponent!
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void VoteLocal(EVotingType type, int value)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
		{
			voting.SetVoteLocal(value);
			m_OnVoteLocal.Invoke(type, value);
			
			//~ Vote Cast Notification
			SCR_VotingBase template = FindTemplate(type);
			if (template && template.CanSendNotification(value))
			{
				SCR_VotingUIInfo uiInfo = template.GetInfo();
				
				if (uiInfo && uiInfo.GetLocalVoteCastNotification() != ENotification.UNKNOWN)
					SCR_NotificationsComponent.SendLocal(uiInfo.GetLocalVoteCastNotification(), value);
			}
		}
		else
		{
			//--- Voting doesn't exist yet - cache it, it will be restored by StartVotingBroadcast
			m_LocalVoteRecords.Insert(new Tuple2<int, int>(type, value));
		}
	}
	/*!
	Remove local vote for given voting.
	It's purely informative, does not affect voting outcome!
	To be called from SCR_VoterComponent!
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void RemoveVoteLocal(EVotingType type, int value)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
		{
			m_aAbstainedVotingInstances.Insert(voting);
			
			voting.RemoveVoteLocal();
			m_OnRemoveVoteLocal.Invoke(type, value);
			
			//~ Vote Abstained Notification
			SCR_VotingBase template = FindTemplate(type);
			if (template && template.CanSendNotification(value))
			{
				SCR_VotingUIInfo uiInfo = template.GetInfo();
				
				if (uiInfo && uiInfo.GetLocalVoteAbstainedNotification() != ENotification.UNKNOWN)
					SCR_NotificationsComponent.SendLocal(uiInfo.GetLocalVoteAbstainedNotification(), value);
			}
		}
	}
	/*!
	Get local vote for given voting.
	It's purely informative, does not affect voting outcome!
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return Voted value
	*/
	int GetLocalVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (voting)
			return voting.GetLocalVote();
		else
			return SCR_VotingBase.DEFAULT_VALUE;
	}
	/*!
	Check if local vote was cast.
	It's purely informative, does not affect voting outcome!
	\param type Type of the vote
	\param value Target value, depends on the type (e.g., for KICK it's player ID)
	\return True if the vote was cast
	*/
	bool IsLocalVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		return GetLocalVote(type, value) != SCR_VotingBase.DEFAULT_VALUE;
	}
	/*!
	Print out information about all ongoing voting instances.
	*/
	void Log()
	{
		Print("---------------------------------------------");
		PrintFormat("Ongoing votings: %1", m_aVotingInstances.Count());
		foreach (SCR_VotingBase voting: m_aVotingInstances)
		{
			voting.Log();
		}
		Print("---------------------------------------------");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected, server
	protected void EndVoting(SCR_VotingBase voting, EVotingOutcome outcome = EVotingOutcome.EVALUATE)
	{
		EVotingType type = voting.GetType();
		int value = voting.GetValue();
		int winner = SCR_VotingBase.DEFAULT_VALUE;
		
		switch (outcome)
		{
			case EVotingOutcome.EVALUATE:
				winner = voting.GetWinner();
				break;
			case EVotingOutcome.FORCE_WIN:
				winner = value;
				break;
		}
		
		voting.OnVotingEnd(value, winner);
		
		EndVotingBroadcast(type, value, winner);
		Rpc(EndVotingBroadcast, type, value, winner);
	}
	protected SCR_VotingBase FindVoting(EVotingType type, int value)
	{		
		for (int i, count = m_aVotingInstances.Count(); i < count; i++)
		{
			if (m_aVotingInstances[i].IsMatching(type, value))
				return m_aVotingInstances[i];
		}
		return null;
	}
	
	protected SCR_VotingBase FindTemplate(EVotingType type)
	{	
		for (int i, count = m_aVotingTemplates.Count(); i < count; i++)
		{
			if (m_aVotingTemplates[i].GetType() == type)
				return m_aVotingTemplates[i];
		}
		return null;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected, anywhere
	protected SCR_VotingBase CreateVotingInstance(EVotingType type, int value, float remainingDuration = -1, int currentVoteCount = -1)
	{
		if (FindVoting(type, value))
			return null;
		
		//--- Find voting template
		SCR_VotingBase template = FindTemplate(type);
		if (!template)
		{
			Debug.Error2("SCR_VotingManagerComponent", string.Format("Cannot initiate voting of type %1, it does not have a template!", typename.EnumToString(EVotingType, type)));
			return null;
		}
		
		//--- Voting about player who is not connected, ignore
		if (Replication.IsServer() && template.IsValuePlayerID() && value != SCR_VotingBase.DEFAULT_VALUE && !GetGame().GetPlayerManager().IsPlayerConnected(value))
		{
			Print(string.Format("Cannot create voting %1 about %2, player with given ID does not exist!", typename.EnumToString(EVotingType, type), value), LogLevel.WARNING);
			return null;
		}
		
		//--- Start new voting
		SCR_VotingBase voting = SCR_VotingBase.Cast(template.Type().Spawn());
		voting.InitFromTemplate(template, value, remainingDuration);
		
		if (currentVoteCount > 0)
			voting.SetCurrentVoteCount(currentVoteCount);
		
		m_aVotingInstances.Insert(voting);
		
		m_OnVotingStart.Invoke(type, value);
		Print(string.Format("Voting %1 started with value %2.", typename.EnumToString(EVotingType, type), voting.GetValueName(value)), LogLevel.VERBOSE);
		
		//~ Vote start Notification
		if (template.CanSendNotification(value))
		{
			if (template.GetInfo() && template.GetInfo().GetVotingStartNotification() != ENotification.UNKNOWN)
			{
				SCR_NotificationsComponent.SendLocal(template.GetInfo().GetVotingStartNotification(), value);
			}
		}
		
		//--- First voting, start updating countdown
		if (m_aVotingInstances.Count() == 1)
			SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		return voting;
	}
	protected void DeleteVotingInstance(EVotingType type, int value, int winner)
	{
		SCR_VotingBase voting = FindVoting(type, value);
		if (!voting)
			return;
		
		if (m_aAbstainedVotingInstances.Contains(voting))
			m_aAbstainedVotingInstances.RemoveItem(voting);
		
		m_aVotingInstances.RemoveItem(voting);
		
		m_OnVotingEnd.Invoke(type, value, winner);
		Print(string.Format("Voting %1 ended, the winner is %2.", typename.EnumToString(EVotingType, type), voting.GetValueName(winner)), LogLevel.VERBOSE);
		
		SCR_VotingBase template = FindTemplate(type);
		
		//~ Vote End Notification
		if (template && template.CanSendNotification(value))
		{
			if (template.GetInfo())
			{
				//~ Vote succeeded notification
				if (winner != SCR_VotingBase.DEFAULT_VALUE && template.GetInfo().GetVotingSucceedNotification() != ENotification.UNKNOWN)
					SCR_NotificationsComponent.SendLocal(template.GetInfo().GetVotingSucceedNotification(), value);
				//~ Vote failed notification
				else if (winner == SCR_VotingBase.DEFAULT_VALUE && template.GetInfo().GetVotingFailNotification() != ENotification.UNKNOWN)
					SCR_NotificationsComponent.SendLocal(template.GetInfo().GetVotingFailNotification(), value);
			}
		}
		
		if (m_aVotingInstances.IsEmpty())
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void StartVotingBroadcast(EVotingType type, int value)
	{
		SCR_VotingBase voting = CreateVotingInstance(type, value);
		
		//--- Restore local vote from cache
		foreach (int index, Tuple2<int, int> record: m_LocalVoteRecords)
		{
			if (type == record.param1 && value == record.param2)
			{
				if (voting)
					VoteLocal(record.param1, record.param2);
				
				m_LocalVoteRecords.Remove(index);
			}
		}
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void EndVotingBroadcast(EVotingType type, int value, int winner)
	{
		DeleteVotingInstance(type, value, winner);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Overrides
	override void OnPlayerConnected(int playerId)
	{		
		//--- Force instant refresh in EOnFrame
		m_fUpdateLength = m_fUpdateStep;
	}
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		for (int i = m_aVotingInstances.Count() - 1; i >= 0; i--)
		{
			//--- Remove player's vote
			m_aVotingInstances[i].RemoveVote(playerId);
			
			//--- If the player was a target of a vote, update the vote
			if (m_aVotingInstances[i].RemoveValue(playerId))
			{
				EndVoting(m_aVotingInstances[i]);
			}
		}
		
		//--- Force instant refresh in EOnFrame
		m_fUpdateLength = m_fUpdateStep;
	}
	override bool RplSave(ScriptBitWriter writer)
	{
		int votingTypeMin, votingTypeMax;
		SCR_Enum.GetRange(EVotingType, votingTypeMin, votingTypeMax);
		
		writer.WriteInt(GetHostPlayerID());
		
		int count = m_aVotingInstances.Count();
		writer.WriteInt(count);
		
		for (int i; i < count; i++)
		{
			writer.WriteIntRange(m_aVotingInstances[i].GetType(), votingTypeMin, votingTypeMax);
			writer.WriteInt(m_aVotingInstances[i].GetValue());
			writer.WriteFloat(m_aVotingInstances[i].GetRemainingDuration());
			writer.WriteFloat(m_aVotingInstances[i].GetCurrentVoteCount());
		}
		
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		int votingTypeMin, votingTypeMax;
		SCR_Enum.GetRange(EVotingType, votingTypeMin, votingTypeMax);
		
		reader.ReadInt(m_iHostPlayerID);
		
		int count;
		reader.ReadInt(count);
		
		EVotingType type;
		int value, currentVoteCount;
		float remainingDuration;
		for (int i; i < count; i++)
		{
			reader.ReadIntRange(type, votingTypeMin, votingTypeMax);
			reader.ReadInt(value);
			reader.ReadFloat(remainingDuration);
			reader.ReadInt(currentVoteCount);
			CreateVotingInstance(type, value, remainingDuration, currentVoteCount);
		}
		
		return true;
	}
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fUpdateLength += timeSlice;
		if (m_fUpdateLength >= m_fUpdateStep)
		{
			if (Replication.IsServer())
			{
				//--- Server, manager all votings
				if (GetGame().GetPlayerManager().GetPlayerCount() > 0)
				{
					//--- Update countdowns (only if some players are present; otherwise votings are paused)
					for (int i = m_aVotingInstances.Count() - 1; i >= 0; i--)
					{
						m_aVotingInstances[i].Update(m_fUpdateLength);
						
						//--- End voting when time expired
						EVotingOutcome outcome = EVotingOutcome.EVALUATE;
						if (m_aVotingInstances[i].Evaluate(outcome))
						{
							EndVoting(m_aVotingInstances[i], outcome);
						}
					}
				}
			}
			else
			{
				//--- Client, merely update countdowns
				for (int i = m_aVotingInstances.Count() - 1; i >= 0; i--)
				{
					m_aVotingInstances[i].Update(m_fUpdateLength);
				}
			}
			
			m_fUpdateLength = 0; //--- Must be after the code above, as it's passed inside Update() as a parameter
		}
	}
	void SCR_VotingManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_VOTING, "Voting", "Game");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_VOTING_ENABLE_ALL, "", "Enable All Vote Types", "Voting");
	}
};
