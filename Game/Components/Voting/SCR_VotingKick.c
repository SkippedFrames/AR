[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingKick: SCR_VotingReferendum
{	
	[Attribute(desc: "When true, only players on the same faction as target of the vote can vote.")]
	protected bool m_bFactionSpecific;
	
	//~ Used in voting to make sure there is a min kick duration (in seconds)
	static const int PLAYER_VOTE_KICK_DURATION = 1200;
	
	//------------------------------------------------------------------------------------------------
	override void InitFromTemplate(SCR_VotingBase template, int value, float remainingDuration)
	{
		super.InitFromTemplate(template, value, remainingDuration);
		
		SCR_VotingKick votingKickTemplate = SCR_VotingKick.Cast(template);		
		m_bFactionSpecific = votingKickTemplate.isFactionSpecific();
	}

	//------------------------------------------------------------------------------------------------
	bool isFactionSpecific()
	{
		return m_bFactionSpecific;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Cannot kick yourself
		if (value == SCR_PlayerController.GetLocalPlayerId())
			return false;
		
		//--- Cannot kick admin
		if (SCR_Global.IsAdmin(value))
			return false;

		//--- When faction specific, allow voting only for players on the same faction as target of the vote
		if (m_bFactionSpecific)
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
				return false;
				
			Faction targetFaction = factionManager.GetPlayerFaction(value); //--- value is ID of the player who is target of the vote
			Faction playerFaction = factionManager.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			if (targetFaction != playerFaction || !targetFaction)
				return false;
		}
		
		//--- Cannot vote to kick out session host
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return false;
		
		//~ Cannot kick if auto kick vote already active
		if (m_Type == EVotingType.KICK && votingManager.IsVoting(EVotingType.AUTO_KICK, value))
			return false;
		
		return votingManager && votingManager.GetHostPlayerID() != value;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetPlayerCount()
	{
		int playerCount;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		Faction targetFaction;
		
		if (factionManager)
			targetFaction = factionManager.GetPlayerFaction(m_iValue);
		
		if (m_bFactionSpecific && targetFaction)
		{
			//--- When faction specific, count only players on the same faction as target of the vote
			//--- e.g., with 50% vote limit, only half of BLUFOR players will have to vote, not half of all players
			
			Faction playerFaction; 
			array<int> players = {};
			
			for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
			{
				//~ Ignore player that is the target of vote
				if (players[i] != m_iValue)
				{
					//~ Check if player faction is the same as target faction
					playerFaction = factionManager.GetPlayerFaction(players[i]);
					if (targetFaction == playerFaction)
						playerCount++;
				}
			}
		}
		else
		{
			//--- Ignore target player by subtracting 1
			playerCount = super.GetPlayerCount() - 1;
		}
		//--- Limit to prevent instant completion in a session with less than limited participants
		return Math.Max(playerCount, m_iMinVotes);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		//~ Vote failed
		if (winner == DEFAULT_VALUE)
			return;
		
		SCR_DataCollectorCrimesModule crimesModule = SCR_DataCollectorCrimesModule.Cast(GetGame().GetDataCollector().FindModule(SCR_DataCollectorCrimesModule));
		//~ Kick using crime module
		if (crimesModule)
		{
			switch(m_Type)
			{
				case EVotingType.KICK:
					crimesModule.KickOrBanPlayer(winner, PlayerManagerKickReason.KICK_VOTED, PLAYER_VOTE_KICK_DURATION);
					return;
				case EVotingType.AUTO_KICK:
					crimesModule.KickOrBanPlayer(winner, PlayerManagerKickReason.KICK, 0);
					return;
				case EVotingType.AUTO_LIGHTBAN:
					crimesModule.KickOrBanPlayer(winner, PlayerManagerKickReason.TEMP_BAN, SCR_DataCollectorCrimesModule.MIN_AUTO_BAN_DURATION);
					return;
				case EVotingType.AUTO_HEAVYBAN:
					crimesModule.KickOrBanPlayer(winner, PlayerManagerKickReason.BAN, SCR_DataCollectorCrimesModule.MIN_AUTO_BAN_DURATION);
					return;
			}
		}
		//~ For some reason the crime module does not exist so default kick/ban is called. Note that it can never heavy ban/backend ban only temp ban
		else 
		{
			if (m_Type == EVotingType.KICK)
				GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.KICK_VOTED, PLAYER_VOTE_KICK_DURATION);
			else if (m_Type == EVotingType.AUTO_KICK)
				GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.KICK, 0);
			else 
			{
				//~ Ban for which ever time is longer
				if (SCR_DataCollectorCrimesModule.MIN_AUTO_BAN_DURATION > PLAYER_VOTE_KICK_DURATION)
					GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.TEMP_BAN, SCR_DataCollectorCrimesModule.MIN_AUTO_BAN_DURATION);
				else 
					GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.TEMP_BAN, PLAYER_VOTE_KICK_DURATION);
			}
		}
	}
};
