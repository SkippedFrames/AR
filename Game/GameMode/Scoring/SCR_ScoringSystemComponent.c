//------------------------------------------------------------------------------------------------
class SCR_ScoringSystemComponentClass: SCR_BaseScoringSystemComponentClass
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class IScoringAction
{
	[Attribute("0", UIWidgets.ComboBox, "Scoring mode", "", ParamEnumArray.FromEnum(EScoringActionMode) )]
	protected EScoringActionMode m_eScoringMode;

	/*!
		Called via SCR_ScoringSystemComponent if player score changes.
		\param playerId PlayerId of target player
		\param scoreInfo New scoring info of provided player
	*/
	void OnPlayerScoreChanged(int playerId, SCR_BaseScoringSystemComponent scoring);

	/*!
		Called via SCR_ScoringSystemComponent if faction score changes.
		\param Faction Faction whose score changed
		\param scoreInfo New scoring info of provided player
	*/
	void OnFactionScoreChanged(Faction faction, SCR_BaseScoringSystemComponent scoring);
};


//------------------------------------------------------------------------------------------------
/*!
	Action type
*/
enum EScoringActionMode
{
	SLM_Player,
	SLM_Faction,
	SLM_Any
};

//------------------------------------------------------------------------------------------------
/*!
	Terminates the game once target score is met.
*/
[BaseContainerProps()]
class EndGameAction : IScoringAction
{
	[Attribute("100", UIWidgets.EditBox, "Scoring limit", "")]
	protected int m_iScoreLimit;
	
	int GetScoreLimit()
	{
		return m_iScoreLimit;
	}
	
	protected void Execute(SCR_GameModeEndData endData)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode.IsRunning())
			return;
		
		gameMode.EndGameMode(endData);
	}

	override void OnPlayerScoreChanged(int playerId, SCR_BaseScoringSystemComponent scoring)
	{
		if (m_eScoringMode != EScoringActionMode.SLM_Player && m_eScoringMode != EScoringActionMode.SLM_Any)
			return;

		int score = scoring.GetPlayerScore(playerId);
		if (score < m_iScoreLimit)
			return;

		ref SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerId: playerId);
		Execute(endData);
	}

	override void OnFactionScoreChanged(Faction faction, SCR_BaseScoringSystemComponent scoring)
	{
		if (m_eScoringMode != EScoringActionMode.SLM_Faction && m_eScoringMode != EScoringActionMode.SLM_Any)
			return;

		int score = scoring.GetFactionScore(faction);
		if (score < m_iScoreLimit)
			return;
		
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
		ref SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: factionIndex );
		Execute(endData);
	}
};

//------------------------------------------------------------------------------------------------
/*!
	This component implements logic of handling certain score limits.
*/
class SCR_ScoringSystemComponent : SCR_BaseScoringSystemComponent
{
	//Score multipliers
	[Attribute("1", UIWidgets.EditBox, "Kill score multiplier", category: "Scoring: Multipliers")]
	protected int m_iKillScoreMultiplier;
	[Attribute("-1", UIWidgets.EditBox, "Teamkill score multiplier", category: "Scoring: Multipliers")]
	protected int m_iTeamKillScoreMultiplier;
	[Attribute("0", UIWidgets.EditBox, "Death score multiplier", category: "Scoring: Multipliers")]
	protected int m_iDeathScoreMultiplier;
	[Attribute("-1", UIWidgets.EditBox, "Suicide score multiplier", category: "Scoring: Multipliers")]
	protected int m_iSuicideScoreMultiplier;
	[Attribute("1", UIWidgets.EditBox, "Objective score multiplier", category: "Scoring: Multipliers")]
	protected int m_iObjectiveScoreMultiplier;
	
	
	[Attribute("", UIWidgets.Object, "List of actions executed when score changes.", category: "Scoring: Actions")]
	ref array<ref IScoringAction> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns maximum allowed score or -1 if undefined.
	*/
	override int GetScoreLimit() 
	{
		foreach (IScoringAction scoringAction : m_aActions)
		{
			EndGameAction possibleAction = EndGameAction.Cast(scoringAction);
			if (possibleAction)
				return possibleAction.GetScoreLimit();
		}
		
		return super.GetScoreLimit();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int CalculateScore(SCR_ScoreInfo info)
	{
		int score = info.m_iKills 		* m_iKillScoreMultiplier +
					info.m_iTeamKills 	* m_iTeamKillScoreMultiplier +
					info.m_iDeaths		* m_iDeathScoreMultiplier +
					info.m_iSuicides 	* m_iSuicideScoreMultiplier +
					info.m_iObjectives 	* m_iObjectiveScoreMultiplier;
		
		if (score < 0)
			return 0;
		
		return score;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Handle and dispatch scoring logic for this event.
	*/
	protected override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		
		// Add death no matter what
		AddDeath(playerId);
		
		if (killer.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		int killerId = killer.GetInstigatorPlayerID();
		
		// Suicide?
		if (playerId == killerId)
		{
			AddSuicide(playerId);
			return;
		}
		
		SCR_ChimeraCharacter playerEntityChar = SCR_ChimeraCharacter.Cast(playerEntity);
		if (!playerEntityChar)
			return;
		
		// We have to resolve who and what faction they belong to
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerId));
		if (!factionKiller)
			return;
		
		if (factionKiller.IsFactionFriendly(playerEntityChar.GetFaction()))
			AddTeamKill(killerId);
		else
			AddKill(killerId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		super.OnGameModeEnd(data);
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Called on all machines when player score changes.
	*/
	protected override void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		super.OnPlayerScoreChanged(playerId, scoreInfo);
		
		foreach (IScoringAction action : m_aActions)
			action.OnPlayerScoreChanged(playerId, this);
	}

	//------------------------------------------------------------------------------------------------
	/*
		Called on all machines when faction score changes.
	*/
	protected override void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		super.OnFactionScoreChanged(faction, scoreInfo);
		
		foreach (IScoringAction action : m_aActions)
			action.OnFactionScoreChanged(faction, this);
	}
};
