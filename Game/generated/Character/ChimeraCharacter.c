/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class ChimeraCharacterClass: GameEntityClass
{
}

class ChimeraCharacter: GameEntity
{
	// Returns HUD display for this character
	proto external BaseInfoDisplay GetInfoDisplay();
	//! Returns component which has animations controlling logic
	proto external CharacterAnimationComponent GetAnimationComponent();
	//! Returns component which stores information about compartment used by character
	proto external CompartmentAccessComponent GetCompartmentAccessComponent();
	//! Returns component which has character controlling logic
	proto external CharacterControllerComponent GetCharacterController();
	//! Returns component which handles damage
	proto external SCR_DamageManagerComponent GetDamageManager();
	//! Returns component which handles weapons
	proto external BaseWeaponManagerComponent GetWeaponManager();
	//! Returns true if the character is inside a vehicle.
	proto external bool IsInVehicle();
	//! Returns true if the character is in vehicle in ADS
	proto external bool IsInVehicleADS();
	//! Returns the aim position on the character
	proto external vector AimingPosition();
	//! Returns the world position of eyes
	proto external vector EyePosition();
	//! Returns the local position of eyes
	proto external vector EyePositionModel();
	//! Returns component which controls head aiming
	proto external AimingComponent GetHeadAimingComponent();
	//! Returns component which controls weapon aiming
	proto external AimingComponent GetWeaponAimingComponent();
	//! Start performing action by the current character
	proto external void DoStartObjectAction(BaseUserAction pAction);
	//! Perform action by current character
	proto external void DoPerformObjectAction(BaseUserAction pAction);
	//! Perform continuous action by current character
	proto external void DoPerformContinuousObjectAction(BaseUserAction pAction, float timeSlice);
	//! Cancel action performed by the current character
	proto external void DoCancelObjectAction(BaseUserAction pAction);
}

/*!
\}
*/
