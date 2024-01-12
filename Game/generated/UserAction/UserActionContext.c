/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

/*!
Object that can be placed in ActionsManagerComponent to define a contextual collection of BaseUserActions.
*/
class UserActionContext: ScriptAndConfig
{
	//! Returns the identifier of this context or string.Empty if none.
	proto external owned string GetContextName();
	/*!
	Returns the name of this context or string.Empty if none, as defined in UI Info.
	*/
	proto external owned string GetName();
	//! Fills the provided outActions array with user actions that belong to this context.
	//! Returns the number of output elements.
	proto external int GetActionsList(out notnull array<BaseUserAction> outActions);
	//! Returns the amount of actions registered in this action context.
	proto external int GetActionsCount();
	//! Fills the provided vector array with the transformation matrix of this context in model (local) space.
	//! Returns true if PointInfo is valid and matrix is output, false otherwise
	proto external bool GetTransformationModel(out vector outMat[4]);
	//! Fills the provided vector array with the transformation matrix of this context in world (global) space.
	//! Returns true if PointInfo is valid and matrix is output, false otherwise.
	proto external bool GetTransformationWorld(out vector outMat[4]);
	//! Returns whether this context is accessible from all directions or not.
	proto external bool IsOmnidirectional();
	//! \return Returns the radius of this context in meters.
	proto external float GetRadius();
	//! Returns the context position in world space.
	proto external vector GetOrigin();
	/*!
	Returns true if this context is in visibility angle for the provided position.
	Does not perform any distance checks!
	\param position Position in world space to check against.
	*/
	proto external bool IsInVisibilityAngle(vector position);
	//! Returns the UIInfo set for this context or null if none.
	proto external UIInfo GetUIInfo();
}

/*!
\}
*/
