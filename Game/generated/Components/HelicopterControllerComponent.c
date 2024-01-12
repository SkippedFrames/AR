/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class HelicopterControllerComponentClass: VehicleControllerComponent_SAClass
{
}

class HelicopterControllerComponent: VehicleControllerComponent_SA
{
	//! return true if autohover system is enabled
	proto external bool GetAutohoverEnabled();
	//! enables autohover system
	proto external void SetAutohoverEnabled(bool enabled);
	//! returns true if wheel brake is active
	proto external bool GetWheelBrake();
	//! returns true if persistent wheel brake is active
	proto external bool GetPersistentWheelBrake();
}

/*!
\}
*/
