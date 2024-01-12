/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

enum EWeaponObstructedState
{
	//! Missing components, or obstruction test was not possible.
	INVALID,
	//! Obstruction is at 0.0.
	UNOBSTRUCTED,
	//! Obstruction is less than the breaking threshold - all actions should still be possible.
	SLIGHTLY_OBSTRUCTED_CAN_FIRE,
	//! Obstruction between breaking threshold and alpha threshold - shooting not possible anymore.
	SIGNIFICANTLY_OBSTRUCTED_CANT_FIRE,
	//! Obstruction higher than alpha threshold - weapon fully obstructed.
	FULLY_OBSTRUCTED_CANT_FIRE,
}

/*!
\}
*/
