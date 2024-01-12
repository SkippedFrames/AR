/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Network
\{
*/

//! Statistical Api - Analytics
class GameStatsApi
{
	//! Generic Player Event - script invokable
	proto external void PlayerEvent(int iPlayerID, Managed params);
	//! Player Score - script invokable
	proto external void PlayerScore(int iPlayerID, Managed params);
	//! Editor Start - script invokable
	proto external void EditorStart();
	//! Editor End - script invokable
	proto external void EditorClosed();
	//! Increment Player's Editor counter - script invokable
	proto external void IncrementEditorCounter(int iPlayerID);
	//! Generic Mod related Event - script invokable
	proto external void ModEvent(Managed params);
}

/*!
\}
*/
