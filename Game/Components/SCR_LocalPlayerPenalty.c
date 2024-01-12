#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of player penalties, kicks, bans etc.", color: "0 0 255 255")]
class SCR_LocalPlayerPenaltyClass: Managed
{
};

//------------------------------------------------------------------------------------------------
class SCR_LocalPlayerPenalty: Managed
{
	//[Attribute("3", desc: "Penalty score for killing a friendly player.")]
	protected int m_iFriendlyPlayerKillPenalty;
	
	//[Attribute("1", desc: "Penalty score for killing a friendly AI.")]
	protected int m_iFriendlyAIKillPenalty;
	
	//[Attribute("10", desc: "Penalty score limit for a kick from the match.")]
	protected int m_iKickPenaltyLimit;
	
	//[Attribute("1800", desc: "Ban duration after a kick (in seconds, -1 for a session-long ban).")]
	protected int m_iBanDuration;
	
	//[Attribute("900", desc: "How often penalty score subtraction happens (in seconds).")]
	protected int m_iPenaltySubtractionPeriod;
	
	//[Attribute("2", desc: "How many penalty points get substracted after each subtraction period.")]
	protected int m_iPenaltySubtractionPoints;
	
	protected static SCR_LocalPlayerPenalty s_Instance;
	protected static const int EVALUATION_PERIOD = 1000;
	
	protected ref array<ref SCR_LocalPlayerPenaltyData> m_aPlayerPenaltyData = {};
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int playerId)
	{
		//GetPlayerPenaltyData creates the PlayerPenaltyStandaloneData for this playerId if it doesn't exist
		GetPlayerPenaltyData(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnControllableDestroyed(IEntity entity, IEntity killerEntity, Instigator instigator)
	{
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER || (m_iFriendlyAIKillPenalty == 0 && m_iFriendlyPlayerKillPenalty == 0))
			return;
		
		SCR_ChimeraCharacter victimChar = SCR_ChimeraCharacter.Cast(entity);
		
		if (!victimChar)
			return;
		
		int killerPlayerId = instigator.GetInstigatorPlayerID();
		int victimPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(victimChar);
		
		//Suicide is not punishable by the Player Penalty system
		if (killerPlayerId == victimPlayerId)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerPlayerId));
		if (!factionKiller)
			return;
		
		//If it's no friendly kill, no wrongdoing was committed
		if (!factionKiller.IsFactionFriendly(victimChar.GetFaction()))
			return;
		
		SCR_LocalPlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(killerPlayerId);
		
		if (victimPlayerId <= 0)
			playerPenaltyData.AddPenaltyScore(m_iFriendlyAIKillPenalty);
		else
			playerPenaltyData.AddPenaltyScore(m_iFriendlyPlayerKillPenalty);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_LocalPlayerPenalty GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//IT SHOULD NOT BE STATIC. TODO: IMPROVE THIS
	static SCR_ChimeraCharacter GetInstigatorFromVehicle(IEntity veh, bool gunner = false)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
			
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
		
		for (int i = 0, cnt = compartmentManager.GetCompartments(compartments); i < cnt; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			
			if (slot && (!gunner && slot.Type() == PilotCompartmentSlot) || (gunner && slot.Type() == TurretCompartmentSlot))
				return SCR_ChimeraCharacter.Cast(slot.GetOccupant());
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_LocalPlayerPenaltyData GetPlayerPenaltyData(int playerId)
	{
		if (playerId == 0)
			return null;
		
		SCR_LocalPlayerPenaltyData playerPenaltyData;

		// Check if the client is reconnecting
		//Also if there's going to be many entries, the search might prove very expensive.
		for (int i = 0, count = m_aPlayerPenaltyData.Count(); i < count; i++)
		{
			if (m_aPlayerPenaltyData[i].GetPlayerId() == playerId)
			{
				playerPenaltyData = m_aPlayerPenaltyData[i];
				break;
			}
		}
		
		// Client reconnected, return saved data
		if (playerPenaltyData)
			return playerPenaltyData;
		
		// Check validity of playerId before registering new data
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!pc)
		{
			Print(string.Format("SCR_PlayerPenaltyComponent: No player with playerId %1 found.", playerId), LogLevel.ERROR);
			return null;
		}
		
		// First connection, register new data
		playerPenaltyData = new SCR_LocalPlayerPenaltyData();
		playerPenaltyData.SetPlayerId(playerId);
		m_aPlayerPenaltyData.Insert(playerPenaltyData);
		
		return playerPenaltyData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluatePlayerPenalties()
	{
		#ifdef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		#endif
		for (int i = 0, count = m_aPlayerPenaltyData.Count(); i < count; i++)
		{
			SCR_LocalPlayerPenaltyData playerPenaltyData = m_aPlayerPenaltyData[i];
			
			// Periodically forgive a portion of penalty score, don't go below zero
			#ifndef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubtractionTimestamp() < Replication.Time())
			#else
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubtractionTimestamp().Less(currentTime))
			#endif
			{
				int forgivenScore;
				
				if (m_iPenaltySubtractionPoints > playerPenaltyData.GetPenaltyScore())
					forgivenScore = playerPenaltyData.GetPenaltyScore();
				else
					forgivenScore = m_iPenaltySubtractionPoints;
				
				playerPenaltyData.AddPenaltyScore(-forgivenScore);
			}
			
			int playerId = playerPenaltyData.GetPlayerId();
			
			// Player is not connected
			if (!GetGame().GetPlayerManager().GetPlayerController(playerId))
				continue;
			
			// Player is host
			if (playerId == SCR_PlayerController.GetLocalPlayerId())
				continue;
			
			// Check penalty limit for kick / ban
			if (m_iKickPenaltyLimit > 0 && playerPenaltyData.GetPenaltyScore() >= m_iKickPenaltyLimit)
			{
				// TODO: Use callback from backend instead
				KickPlayer(playerId, m_iBanDuration, SCR_PlayerManagerKickReason.FRIENDLY_FIRE);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void KickPlayer(int playerId, int duration, SCR_PlayerManagerKickReason reason)
	{	
		SCR_LocalPlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (playerPenaltyData)
			playerPenaltyData.AddPenaltyScore(-playerPenaltyData.GetPenaltyScore());
		
		GetGame().GetPlayerManager().KickPlayer(playerId, reason, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPenaltySubtractionPeriod()
	{
		return m_iPenaltySubtractionPeriod * 1000;	// Converting s to ms
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_LocalPlayerPenalty(int friendlyPlayerKillPenalty, int friendlyAIKillPenalty, int penaltyLimit, int banDuration, int penaltySubtractionPeriod, int penaltySubtractionPoints)
	{		
		m_iFriendlyPlayerKillPenalty = friendlyPlayerKillPenalty;
		m_iFriendlyAIKillPenalty = friendlyAIKillPenalty;
		m_iKickPenaltyLimit = penaltyLimit;
		m_iBanDuration = banDuration;
		m_iPenaltySubtractionPeriod = penaltySubtractionPeriod;
		m_iPenaltySubtractionPoints = penaltySubtractionPoints;
		
		if (m_iBanDuration < m_iPenaltySubtractionPeriod)
			Print("SCR_PlayerPenaltyComponent: Ban duration is shorter than Penalty substraction period. This will cause the player to remain banned until their penalty is substracted.", LogLevel.WARNING);
		
		s_Instance = this;
		//Looping every EVALUATION_PERIOD seconds and don't need other EOn events
		GetGame().GetCallqueue().CallLater(EvaluatePlayerPenalties, EVALUATION_PERIOD, true);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_LocalPlayerPenaltyData
{
	protected int m_iPlayerId;
	protected int m_iPenaltyScore;
	#ifndef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
	protected float m_fNextPenaltySubtractionTimestamp;
	#else
	protected WorldTimestamp m_fNextPenaltySubtractionTimestamp;
	#endif
	protected SCR_PlayerManagerKickReason m_eKickReason = SCR_PlayerManagerKickReason.DISRUPTIVE_BEHAVIOUR;
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerId(int playerId)
	{
		m_iPlayerId = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPenaltyScore(int points)
	{
		m_iPenaltyScore += points;
		
		// Start the timer on penalty substraction when player was penalized while the timer was stopped
		#ifndef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
		if ((points > 0 && m_fNextPenaltySubtractionTimestamp < Replication.Time()) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubtractionTimestamp = Replication.Time() + SCR_LocalPlayerPenalty.GetInstance().GetPenaltySubtractionPeriod();
		#else
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if ((points > 0 && m_fNextPenaltySubtractionTimestamp.Less(currentTime)) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubtractionTimestamp = currentTime.PlusMilliseconds(SCR_LocalPlayerPenalty.GetInstance().GetPenaltySubtractionPeriod());
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPenaltyScore()
	{
		return m_iPenaltyScore;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
	void SetNextPenaltySubstractionTimestamp(float timestamp)
	#else
	void SetNextPenaltySubstractionTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fNextPenaltySubtractionTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_LOCAL_PLAYER_PENALTY_TIMESTAMP
	float GetNextPenaltySubtractionTimestamp()
	#else
	WorldTimestamp GetNextPenaltySubtractionTimestamp()
	#endif
	{
		return m_fNextPenaltySubtractionTimestamp;
	}
};
