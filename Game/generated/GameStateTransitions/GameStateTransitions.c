/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameStateTransitions
\{
*/

sealed class GameStateTransitions
{
	private void GameStateTransitions();
	private void ~GameStateTransitions();

	//! Check whether transition to different game state has been requested this frame and will start next frame.
	static proto bool IsTransitionRequested();
	//! Check whether transition to different game state is currently happening.
	static proto bool IsTransitionInProgress();
	static proto bool IsTransitionRequestedOrInProgress();
	//! Request transition from main menu to hosted multiplayer session (registered in server browser).
	static proto bool RequestPublicServerTransition(JsonApiStruct config);
	[Obsolete("Not supported!")]
	static proto bool RequestConnectViaIP(string IPAddr);
	//! Request transition from main menu to gameplay as client connected to server in multiplayer session.
	static proto bool RequestConnectViaRoom(Room room);
	//! Request change of mission
	[Obsolete("Use RequestScenarioChangeTransition instead!")]
	static proto bool RequestMissionChangeTransition(MissionHeader mission);
	//! Request change of world, keeping other properties of the game session.
	[Obsolete("Use RequestScenarioChangeTransition instead!")]
	static proto bool RequestWorldChangeTransition(string worldPath);
	/*!
	Request change of scenario.
	\param resourceStr Uses string instead of ResourceName. ResourceName would be invalidated on addon reload
	\param addonList List of addons to load. Leave empty for vanilla.
	*/
	static proto bool RequestScenarioChangeTransition(string resourceStr, string addonList);
	//! Request reload of ongoing multiplayer game.
	[Obsolete("Use RequestScenarioRestart instead!")]
	static proto bool RequestServerReload();
	/*!
	Request restart of scenario in both SP and MP.
	Must be called on server in case of MP game.
	*/
	static proto bool RequestScenarioRestart();
	//! Request server config change and reload the game.
	static proto bool RequestServerConfigChange(notnull ServerConfigMeta configMeta);
	//! Request transition from whatever is current multiplayer session mode back to main menu.
	static proto void RequestGameplayEndTransition(KickCauseCode code = KickCauseCode.NONE);
	//! Request graceful shut down of the game from whatever is current game state (eg. online vs. offline).
	static proto void RequestGameTerminateTransition();
	//! Return true if in main menu is loaded for the first time
	static proto bool IsFirstMainMenu();
}

/*!
\}
*/
