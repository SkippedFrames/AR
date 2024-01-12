/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Base
\{
*/

class BaseAnimPhysComponentClass: GameComponentClass
{
}

class BaseAnimPhysComponent: GameComponent
{
	/*!
	-----------------------------------------------------------------------------
	 commands
	-----------------------------------------------------------------------------
	*/
	proto external void SetCurrentCommand(AnimPhysCommandScripted pCommand);
	proto external AnimPhysCommandScripted GetCommandScripted();
	/*!
	-----------------------------------------------------------------------------
	 script binding
	-----------------------------------------------------------------------------
	*/
	proto external TAnimGraphCommand	BindCommand(string pCommandName);
	//! binds variable
	proto external TAnimGraphVariable	BindVariableFloat(string pVariableName);
	proto external TAnimGraphVariable	BindVariableInt(string pVariableName);
	proto external TAnimGraphVariable	BindVariableBool(string pVariableName);
	//! binds tag
	proto external TAnimGraphTag		BindTag(string pTagName);
	//! binds event
	proto external TAnimGraphEvent		BindEvent(string pEventName);
	//! binds prediction
	proto external TAnimGraphPrediction	BindPrediction(string pPredictionName);
	//! Functions for setting animation variables and calling animation commands. It is recommended to, instead of
	//! binding and setting the variables directly, use a ScriptedCommand object with a static table,
	//! and to set those variables in the PreAnimUpdate function using the functions provideed by ScriptedCommand instead.
	//! For an example on how to use those, see SCR_CharacterCommandSwim.
	//! Animation variables can also be overwritten by the character commands, as they are evaluated
	//! right before animations, leading to manually set values being discarded.
	proto external void SetVariableFloat(TAnimGraphVariable varIdx, float value);
	proto external void SetVariableInt(TAnimGraphVariable varIdx, int value);
	proto external void SetVariableBool(TAnimGraphVariable varIdx, bool value);
	proto external void CallCommand(TAnimGraphCommand pCmdIndex, int intParam, float floatParam);
	proto external void CallCommand4I(TAnimGraphCommand pCmdIndex, int intParam1, int intParam2, int intParam3, int intParam4, float floatParam);
	//! returns true when character is physically falling
	proto external bool PhysicsIsFalling();
	//! enables physics
	proto external void PhysicsEnableGravity(bool pState);
}

/*!
\}
*/
