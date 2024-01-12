/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Perception
\{
*/

class PerceivableComponentClass: GameComponentClass
{
}

class PerceivableComponent: GameComponent
{
	proto external EAIUnitType GetUnitType();
	//! Returns overall visual recognition factor
	proto external float GetVisualRecognitionFactor();
	//! Represents ambient light, dynamic lights, own lights.
	proto external float GetIlluminationFactor();
	//! Returns ambient light LV value (logarithmic). Can be used for diagnostics.
	proto external float GetAmbientLV();
	//! Returns our sound emission power
	proto external float GetSoundPower();
	//! Returns estimated size in meters. Currently it's calculated based on physics or visual bounding box.
	proto external float GetEstimatedVisualSize();
	//! Fills the provided array with the list of all aimpoints as locations in model space
	//! Returns the number of output elements.
	proto external int GetAimpoints(out notnull array<vector> outPoints);
	proto external int GetAimpointsOfType(out notnull array<ref AimPoint> outPoints, EAimPointType type);
	//! Returns true when this perceivable is considered disarmed. This value is updated periodically.
	proto external bool IsDisarmed();
}

/*!
\}
*/
