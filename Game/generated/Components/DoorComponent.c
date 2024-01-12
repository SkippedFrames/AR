/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class DoorComponent: GameComponent
{
	//! Returns the pivot around which the door open/close in world space
	proto external vector GetDoorPivotPointWS();
	//! Returns the range angle of the door in radians
	proto external float GetAngleRange();
	//! Returns true if an object of a given size can pass through the door
	proto external bool CanCharacterPass(float characterWidth);
	//! Returns true if the door is open
	proto external bool IsOpen();
	//! Returns true if the door is opening
	proto external bool IsOpening();
	//! Returns true if the door is closing
	proto external bool IsClosing();
	//! Sets the current action instigator.
	proto external void SetActionInstigator(IEntity instigator);
	//! Sets the control value of the door and updates the wanted state. Automatically clamped to <0, 1>
	proto external void SetControlValue(float controlValue);
	//! Returns the control value of the door ranging from <-1, 1>
	proto external float GetControlValue();
	//! Returns the current door state
	proto external float GetDoorState();
	//! Returns the normal of the door in it's current state
	proto external vector GetDoorNormal();
}

/*!
\}
*/
