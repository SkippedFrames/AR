/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SoundComponentClass: SimpleSoundComponentClass
{
}

class SoundComponent: SimpleSoundComponent
{
	//! Play a sound from the owner entity's position
	proto external AudioHandle SoundEvent(string eventName);
	//! Play a sound from a set transformation
	proto external AudioHandle SoundEventTransform(string eventName, vector transf[]);
	//! Play a sound from the owner entity's position
	proto external AudioHandle SoundEventBone(string eventName, string bone);
	//! Play a sound with a given offset from the owner entity
	proto external AudioHandle SoundEventOffset(string eventName, vector offset);
}

/*!
\}
*/
