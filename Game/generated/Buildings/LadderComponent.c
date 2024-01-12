/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Buildings
\{
*/

class LadderComponentClass: GameComponentClass
{
}

class LadderComponent: GameComponent
{
	/*!
	Check if the position is above the ladder.
	\param pPositionWS Position in world space which will be checked
	\return Return true if the position is above the ladder
	*/
	proto external bool IsAbove(vector pPositionWS);
	/*!
	Enable or disable the top front entry of the ladder.
	\param frontTop Desired state of entry.
	*/
	proto external void SetEnabledEntry(bool frontTop);
}

/*!
\}
*/
