/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseItemAnimationComponentClass: AnimationControllerComponentClass
{
}

class BaseItemAnimationComponent: AnimationControllerComponent
{
	proto external IEntity GetOwner();
	//! Syncs item with character and subscribing to it's variable changes and command calls
	//! returns true on success
	proto external bool SyncWithCharacter(ChimeraCharacter pCharacter);
	//! Removes previously synced character reference
	proto external bool RemoveSyncReference(ChimeraCharacter pCharacter);
	proto external bool IsAnimationTag(AnimationTagID tagID);

	// callbacks

	event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd);
	//! Called every frame right before animation controller will be updated
	//! return true to stop default animation behavior of an item, return false otherwise (default)
	event protected bool OnPrepareAnimInput(IEntity owner, float ts) { return false; };
	//! Called every frame right before animation controller will be updated
	//! return true to stop default animation behavior of an item, return false otherwise (default)
	event protected bool OnProcessAnimOutput(IEntity owner, float ts) { return false; };
	//! Called when variable was changed in synced character's animation logic
	event protected void OnCharacterFloatVariablet(int variableID, float value);
	event protected void OnCharacterIntVariable(int variableID, int value);
	event protected void OnCharacterBoolVariable(int variableID, bool value);
	//! Called when animation command was called in synced character's animation logic
	event protected void OnCharacterCommand(int commandID, int intValue, float floatValue);
}

/*!
\}
*/
