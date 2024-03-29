enum EGameOverTypes
{
	ENDREASON_UNDEFINED = -1,
	ENDREASON_TIMELIMIT = -2,
	ENDREASON_SCORELIMIT = -3,
	ENDREASON_DRAW = -4,
	SERVER_RESTART = -5,
	
	//~ Default
	UNKNOWN = 0,
	NEUTRAL = 1,
	
	//~ Faction
	FACTION_NEUTRAL = 100,
	FACTION_VICTORY_SCORE = 101,
	FACTION_DEFEAT_SCORE = 102,
	FACTION_VICTORY_TIME = 103,
	FACTION_DEFEAT_TIME = 104,
	FACTION_DRAW = 105,
	
	//~ Death match
	DEATHMATCH_VICTORY_SCORE = 201,
	DEATHMATCH_DEFEAT_SCORE = 202,
	DEATHMATCH_VICTORY_TIME = 203,
	DEATHMATCH_DEFEAT_TIME = 204,
	DEATHMATCH_DRAW = 205,
	
	//~ Editor
	EDITOR_NEUTRAL = 1000,
	EDITOR_FACTION_NEUTRAL = 1001,
	EDITOR_FACTION_VICTORY = 1002,
	EDITOR_FACTION_DEFEAT = 1003,
	EDITOR_FACTION_DRAW = 1004,
	
	//~ Combat Patrol
	COMBATPATROL_VICTORY = 2000,
	COMBATPATROL_PARTIAL_SUCCESS = 2001,
	COMBATPATROL_DRAW = 2002,
	COMBATPATROL_DEFEAT = 2003,
};
