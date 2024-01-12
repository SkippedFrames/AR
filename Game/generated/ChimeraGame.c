/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ChimeraGame: Game
{
	//! Is the game experimental build
	proto external bool IsExperimentalBuild();
	//! Returns the world clock
	proto external ScriptWorldClock GetClock();
	//! returns the global signals manager
	proto external GameSignalsManager GetSignalsManager();
	//! returns entity holding info about player
	proto external PlayerController GetPlayerController();
	//! PlayerManager holding info about players who joined MP. It always returns a valid instance.
	proto external PlayerManager GetPlayerManager();
	// returns the Perception Manager
	proto external PerceptionManager GetPerceptionManager();
	//! returns entity holding info about factions
	proto external FactionManager GetFactionManager();
	//! Returns the chat entity
	proto external BaseChatEntity GetChat();
	//! Returns the camera manager
	proto external CameraManager GetCameraManager();
	//! Returns the game mode
	proto external BaseGameMode GetGameMode();
	//! Returns the map manager entity
	proto external MapEntity GetMapManager();
	//! Sets view distance in meters (far clipping plane of cameras)
	proto external void SetViewDistance(float viewDistance);
	//! Returns view distance in meters (far clipping plane of cameras)
	proto external float GetViewDistance();
	//! Returns absolute maximum view distance per platform as defined by global chimera settings.
	proto external float GetMaximumViewDistance();
	//! Returns absolute minimum view distance per platform as defined by global chimera settings.
	proto external float GetMinimumViewDistance();
	//! Sets grass distance in meters
	proto external void SetGrassDistance(int grassDistance);
	//! Returns grass distance in meters
	proto external int GetGrassDistance();
	//! Returns absolute maximum grass distance
	proto external int GetMaximumGrassDistance();
	//! Returns absolute minimum grass distance
	proto external int GetMinimumGrassDistance();
	//! Returns the maximum view distance set by server (never relevant to client) set via ServerConfig
	proto external float GetViewDistanceServerLimit();
	//! Returns true if the VON UI is disabled by the server
	proto external bool IsVONUIDisabledByServer();
	//! Returns true if the VON direct speech UI is disabled by the server
	proto external bool IsVONDirectSpeechUIDisabledByServer();
	//! Returns true if the VONCanTransmitCrossFaction is enabled by the server
	proto external bool GetVONCanTransmitCrossFaction();
	//! Returns currently active mission or null if none
	proto external MissionHeader GetMissionHeader();
	//! Returns the AI World
	proto external AIWorld GetAIWorld();
	//! Returns analytics Api
	proto external GameStatsApi GetStatsApi();
	/*!
	Enable or disable the logging of FPS.
	\param msec gap in milliseconds at which time the log/calculate FPS is calculated. <= 0 to disable it.
	*/
	proto external void LogFPS(int msec);
	/*!
	Checks is a prefab can be spawned localy. Required for multiplayer
	*/
	static proto bool CanSpawnEntityPrefab(notnull Resource templateResource, EntitySpawnParams params = null);
	/*!
	Spawns a prefab which will exist only on local machine.
	*/
	proto external IEntity SpawnEntityPrefabLocal(notnull Resource templateResource, BaseWorld world = null, EntitySpawnParams params = null);

	// callbacks

	//! Getter for where authority is for vehicles
	event bool GetIsClientAuthority();
	/*!
	\brief Called when a mission header is set (to both a valid one or to null as well)
	\param mission can be passed in as null when mission is cleared, make sure to nullptr check!
	*/
	event protected void OnMissionSet(MissionHeader mission);
	//! Called after player was kicked from game back to main menu, providing reason for the kick.
	event protected void OnKickedFromGame(KickCauseCode kickCode);
	//! Shows dialog with a provided string.
	event protected void ShowErrorMessage(string msg);
	//! Called when the player data is requested
	event protected ref Managed GetPlayerDataStats(int playerID);
	/*!
	\brief Called when DS downloads required addons and is ready to run a world
	FIXME: I'm not named properly!
	*/
	event protected string GetMissionName();
}
