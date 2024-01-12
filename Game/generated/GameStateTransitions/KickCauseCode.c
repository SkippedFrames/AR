/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameStateTransitions
\{
*/

/*!
Opaque code representing various reasons for ending gameplay. It must be
manipulated using KickCauseCodeAPI functions, which provide access to
category and category-specific reason. Categories differentiate various
systems and each system should provide it's own list of possible reasons
that could cause gameplay to end.
*/
enum KickCauseCode
{
	/*!
	Predefined value used when gameplay ends as a result of user action and
	does not need to be explained.
	*/
	NONE,
}

/*!
\}
*/
