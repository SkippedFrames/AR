/*!
Base class for voting types.
Do not expose objects of this class outside of SCR_VotingManagerComponent!
Other systems should *always* go through the manager.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of the vote.", enums: ParamEnumArray.FromEnum(EVotingType))]
	protected EVotingType m_Type;
	
	[Attribute(desc: "When enabled, values are considered player IDs (e.g., a vote to KICK player).")]
	protected bool m_bIsValuePlayerID;
	
	[Attribute("120", desc: "The voting will end after this duration (in seconds) and outcome will be evaluated.")]
	protected float m_fDuration;
	
	[Attribute("0.5", UIWidgets.Slider, "When percentual number of votes for single value is *larger* than this threshold, the voting will instantly end.\ne.g., in a session with 8 players and threshold 0.5, 5 votes are needed to win.", params: "0 1 0.01")]
	protected float m_fThreshold;
	
	[Attribute("0.5", UIWidgets.Slider, "Winner can be declared only if this percentage of players participated in the vote.\nEvaluated together with 'Min Votes', at least one has to pass.", params: "0 1 0.01")]
	protected float m_iMinParticipation;
	
	[Attribute("3", desc: "Winner can be declare only if at least this amount of players participated in the vote.\nEvaluated together with 'Min Participation', at least one has to pass.")]
	protected int m_iMinVotes;
	
	[Attribute(desc: "Visual representation of the voting.")]
	protected ref SCR_VotingUIInfo m_Info;
	
	static const int DEFAULT_VALUE = -1;
	
	protected int m_iLocalValue = DEFAULT_VALUE;
	
	//~ How many players have voted in favour
	protected int m_iCurrentVoteCount;
	
	//~ Keep track of who voted (server only)
	protected ref array<int> m_aPlayersVoted_Server = {};
	
	//------------------------------------------------------------------------------------------------
	/*!
	A player has voted to approve (server only)
	\param playerID Id of player who voted
	\return True if player had not yet voted to approve
	*/
	bool AddPlayerVotedServer(int playerID)
	{
		if (m_aPlayersVoted_Server.Contains(playerID))
			return false;
		
		m_aPlayersVoted_Server.Insert(playerID);
		SetCurrentVoteCount(m_aPlayersVoted_Server.Count());
		
		//~ Vote debug
		Print("Player '" + playerID + "' approved vote | Vote Type: '" + typename.EnumToString(EVotingType, m_Type) + "' | Vote value: '" + m_iLocalValue + "' | Count (" + GetCurrentVoteCount() + "/" + GetVoteCountRequired() + ")");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	A player has voted to remove or abstain (server only)
	\param playerID Id of player who voted to remove or abstain
	\return True if player had previously voted to approve and the vote was removed
	*/
	bool RemovePlayerVotedServer(int playerID)
	{
		if (!m_aPlayersVoted_Server.Contains(playerID))
			return false;
		
		m_aPlayersVoted_Server.RemoveItem(playerID);
		SetCurrentVoteCount(m_aPlayersVoted_Server.Count());
		
		//~ Vote debug
		Print("Player '" + playerID + "' removed vote | Vote Type: '" + typename.EnumToString(EVotingType, m_Type) + "' | Vote value: '" + m_iLocalValue + "' | Count (" + GetCurrentVoteCount() + "/" + GetVoteCountRequired() + ")");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set the current amount of votes this vote has
	\param currentVoteCount New current vote amount
	*/
	void SetCurrentVoteCount(int currentVoteCount)
	{
		m_iCurrentVoteCount = currentVoteCount;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Current total votes the vote has received
	*/
	int GetCurrentVoteCount()
	{
		return m_iCurrentVoteCount;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, server
	/*!
	Set vote of given player.
	\param playerID Player ID
	\param value Cast vote
	*/
	void SetVote(int playerID, int value = DEFAULT_VALUE);
	/*!
	Remove vote of given player.
	\param playerID Player ID
	\return True if the voting should be canceled
	*/
	bool RemoveVote(int playerID);
	/*!
	Remove all votes for given value.
	\param value Value
	*/
	bool RemoveValue(int value);
	/*!
	Evaluate if the voting can end.
	\param[out] EVotingOutcome Outcome of the vote (used only when returned value is true)
	\return True if it can end
	*/
	
	/*!
	Check if notification can be shown for this voting
	\param value Voting value
	\return True if can send notification for this voting type
	*/
	bool CanSendNotification(int value)
	{
		return true;
	}
	
	bool Evaluate(out EVotingOutcome outcome)
	{
		return m_fDuration <= 0;
	}
	/*!
	Get winner of the vote.
	\return Winning value
	*/
	int GetWinner()
	{
		return DEFAULT_VALUE;
	}
	/*!
	Event called when the voting ends.
	\param value Voting value
	\param winner Winning value
	*/
	void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
	}
	int GetPlayerVote(int playerID)
	{
		return DEFAULT_VALUE;
	}	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, anywhere
	/*!
	Check if the voting is available in the current world.
	\param value Potential voting value
	\param isOngoing True if the voting was already initiated
	\return True when available
	*/
	bool IsAvailable(int value, bool isOngoing)
	{
		return true;
	}
	/*!
	Check if the voting is matching given params.
	\param type Voting type
	\param value Voting value
	*/
	bool IsMatching(EVotingType type, int value = DEFAULT_VALUE);

	void Log();
	/*!
	Get name of the value.
	\return Value name
	*/
	string GetValueName(int value)
	{
		if (m_bIsValuePlayerID && value > 0)
			return GetGame().GetPlayerManager().GetPlayerName(value);
		else
			return value.ToString();
	}
	/*!
	Get voting type.
	\return Type
	*/
	EVotingType GetType()
	{
		return m_Type;
	}
	/*!
	Get voting value.
	Applicable only if the voting targets specific value, e.g., is referendum about certain outcome.
	\return Value
	*/
	EVotingType GetValue()
	{
		return DEFAULT_VALUE;
	}
	/*!
	Get voting UI representation
	\return UI info
	*/
	SCR_VotingUIInfo GetInfo()
	{
		return m_Info;
	}
	/*!
	\return Remaining time of the voting in seconds
	*/
	float GetRemainingDuration()
	{
		return m_fDuration;
	}
	/*!
	Check if the voting is about player IDs.
	\return True when it's about player IDs
	*/
	bool IsValuePlayerID()
	{
		return m_bIsValuePlayerID;
	}
	/*!
	Save local vote.
	It's purely informative, does not affect voting outcome!
	\param value Voted value
	*/
	void SetVoteLocal(int value)
	{
		m_iLocalValue = value;
	}
	/*!
	Remove local vote.
	It's purely informative, does not affect voting outcome!
	\param value Voted value
	*/
	void RemoveVoteLocal()
	{
		m_iLocalValue = DEFAULT_VALUE;
	}
	/*!
	Get local vote.
	It's purely informative, does not affect voting outcome!
	\return Voted value
	*/
	int GetLocalVote()
	{
		return m_iLocalValue;
	}
	/*!
	Periodically update the voting.
	\param timeSlice Time since the last update
	*/
	void Update(float timeSlice)
	{
		m_fDuration = Math.Max(m_fDuration - timeSlice, 0);
	}
	/*!
	Initialize voting from given template.
	\param template Source template
	\param value Target value
	\param remainingDuration Remaining time until the voting ends in seconds (-1 to use default)
	*/
	void InitFromTemplate(SCR_VotingBase template, int value, float remainingDuration)
	{
		m_Type = template.m_Type;
		m_fThreshold = template.m_fThreshold;
		m_bIsValuePlayerID = template.m_bIsValuePlayerID;
		m_iMinParticipation = Math.Min(template.m_iMinParticipation, template.m_fThreshold); //--- Min participation cannot be stricter than threshold
		m_iMinVotes = template.m_iMinVotes;
		
		if (remainingDuration == -1)
			m_fDuration = template.m_fDuration;
		else
			m_fDuration = remainingDuration;
	}
	
	/*!
	Get total player count of players that are valid to vote for the issue
	return Total count
	*/
	int GetPlayerCount()
	{
		return Math.Max(GetGame().GetPlayerManager().GetPlayerCount(), 1);
	}
	
	/*!
	Get total players needed to make sure the vote is successfull
	return Total votes needed
	*/
	int GetVoteCountRequired()
	{
		float playersRequired = GetPlayerCount() * m_fThreshold;
		int playersRequiredInt = Math.Ceil(playersRequired);
		
		//~ Vote succeeds when vote count is GREATER than threshold. If players requered is == to playersRequired ceil than it means that there is 1 more player needed for the vote to succeed as otherwise the vote is equal
		if (playersRequired == playersRequiredInt)
			playersRequiredInt++;
		
		if (playersRequiredInt < m_iMinVotes)
			return m_iMinVotes;
		
		return playersRequiredInt;
	}
		
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected
	protected bool EvaluateParticipation(int voteCount)
	{
		return
			/*
				Absolute player limit
				With m_iMinVotes = 3, at least 3 players have to cast a vote. 
				If there are fewer than 3 players, total amount of players will be used instead (which means the vote has to be unanimous)
				Examples:
					Players		Votes needed
					1			1
					2			2
					3			3
					4			3
					5			3
					...			...
			*/
			voteCount >= Math.Min(m_iMinVotes, GetPlayerCount())
			/*
				Relative player limit
				With m_iMinParticipation = 0.5 (50%), at least half of players need to cast a vote.
				Examples:
					Players		Votes needed
					1			1
					2			1
					3			2
					4			2
					5			3
					...			...
			*/
			&& (float)(voteCount / GetPlayerCount()) >= m_iMinParticipation;
	}
};

/*!
Referendum about specific outcome.
Can have target value, e.g., player ID of whoever should be kicked.
Multiple votings of the same type can exist at the same type, e.g., KICK player 1 and KICK player 2.
The voting succeeds when proportion of votes is *larger* than given threshold and there was sufficient participation.
If time runs out, the voting is invalid.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingReferendum: SCR_VotingBase
{
	protected ref set<int> m_aPlayerIDs = new set<int>();
	protected int m_iValue;
	
	protected float GetRatio()
	{
		return (m_aPlayerIDs.Count() / GetPlayerCount());
	}
	override void SetVote(int playerID, int value = DEFAULT_VALUE)
	{
		m_aPlayerIDs.Insert(playerID);
	}
	override bool RemoveVote(int playerID)
	{
		int index = m_aPlayerIDs.Find(playerID);
		if (index >= 0)
		{
			m_aPlayerIDs.Remove(index);
			return m_aPlayerIDs.IsEmpty();
		}
		else
		{
			return false;
		}
	}
	override bool RemoveValue(int value)
	{
		return m_bIsValuePlayerID && m_iValue == value;
	}
	override bool IsMatching(EVotingType type, int value = DEFAULT_VALUE)
	{
		return m_Type == type && m_iValue == value;
	}
	override bool Evaluate(out EVotingOutcome outcome)
	{
		return super.Evaluate(outcome) || (GetRatio() > m_fThreshold && EvaluateParticipation(m_aPlayerIDs.Count()));
	}
	override int GetWinner()
	{
		if (GetRatio() > m_fThreshold && EvaluateParticipation(m_aPlayerIDs.Count()))
			return m_iValue; //--- Passed the threshold and minimum participation
		else
			return DEFAULT_VALUE; //--- Expired, didn't pass
	}
	override int GetPlayerVote(int playerID)
	{
		if (m_aPlayerIDs.Contains(playerID))
			return m_iValue;
		else
			return DEFAULT_VALUE;
	}
	override int GetValue()
	{
		return m_iValue;
	}
	override void Log()
	{
		PrintFormat("%1 (m_Type = %2, m_iValue = %3, m_iLocalValue = %3)", Type(), typename.EnumToString(EVotingType, m_Type), GetValueName(m_iValue), GetValueName(m_iLocalValue));
		foreach (int playerID: m_aPlayerIDs)
		{
			PrintFormat("    '%1'", GetGame().GetPlayerManager().GetPlayerName(playerID));
		}
	}
	override void InitFromTemplate(SCR_VotingBase template, int value, float remainingDuration)
	{
		m_iValue = value;
		super.InitFromTemplate(template, value, remainingDuration);
	}
};

/*!
Election of specific type.
Once it starts, players can vote for multiple values.
The voting ends when proportion of votes for single value is *larger* than threshold, or if time expires.
If the vote had enough participation, the value with the most votes wins, otherwise the vote fails.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingElection: SCR_VotingBase
{
	protected ref map<int, int> m_Votes = new map<int, int>(); // <playerID, value>
	protected int m_iHighestValue = DEFAULT_VALUE;
	protected int m_iHighestCount;
	
	protected void UpdateHighestValue()
	{
		map<int, int> tally = new map<int, int>(); // <value, count>
		m_iHighestCount = 0;
		set<int> winners = new set<int>();
		foreach (int playerID, int value: m_Votes)
		{
			int count;
			if (tally.Find(value, count))
				count++;
			else
				count = 1;
			
			tally.Set(value, count);
			
			if (count > m_iHighestCount)
			{
				m_iHighestCount = count;
				winners.Clear();
				winners.Insert(value);
			}
			else if (count == m_iHighestCount)
			{
				winners.Insert(value);
			}
		}
		
		if (winners.IsEmpty())
			m_iHighestValue = DEFAULT_VALUE;
		else
			//--- In case of multiple leaders, pick one by random
			m_iHighestValue = winners[Math.RandomInt(0, winners.Count())];
	}
	
	override void SetVote(int playerID, int value = DEFAULT_VALUE)
	{
		m_Votes.Set(playerID, value);
		UpdateHighestValue();
	}
	override bool RemoveVote(int playerID)
	{
		m_Votes.Remove(playerID);
		UpdateHighestValue();
		return m_Votes.IsEmpty();
	}
	override bool RemoveValue(int value)
	{
		for (int i = m_Votes.Count() - 1; i >= 0; i--)
		{
			if (m_Votes.GetElement(i) == value)
				m_Votes.RemoveElement(i);
		}
		UpdateHighestValue();
		return false;
	}
	override bool IsMatching(EVotingType type, int value = DEFAULT_VALUE)
	{
		return m_Type == type;
	}
	override bool Evaluate(out EVotingOutcome outcome)
	{
		float ratio = (m_iHighestCount / GetPlayerCount());
		return super.Evaluate(outcome) || (ratio > m_fThreshold && EvaluateParticipation(m_Votes.Count()));
	}
	override int GetWinner()
	{
		if (EvaluateParticipation(m_Votes.Count()))
			return m_iHighestValue;
		else
			return DEFAULT_VALUE; //--- Expired, didn't pass
	}
	override int GetPlayerVote(int playerID)
	{
		int value;
		if (m_Votes.Find(playerID, value))
			return value;
		else
			return DEFAULT_VALUE;
	}
	override void Log()
	{
		PrintFormat("%1 (m_Type = %2, m_iLocalValue = %3)", Type(), typename.EnumToString(EVotingType, m_Type), GetValueName(m_iLocalValue));
		PrintFormat("    Leading value: %1 with %2 votes", m_iHighestValue, m_iHighestCount);
		string valueName;
		foreach (int playerID, int value: m_Votes)
		{
			PrintFormat("    %1 votes for  %2", GetGame().GetPlayerManager().GetPlayerName(playerID), GetValueName(value));
		}
	}
};
