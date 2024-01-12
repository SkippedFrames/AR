/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class CarControllerComponent_SA_BClass: VehicleControllerComponent_SAClass
{
}

class CarControllerComponent_SA_B: VehicleControllerComponent_SA
{
	//! Returns the simulation component associated with this controller.
	proto external VehicleWheeledSimulation_SA_B GetWheeledSimulation();
	//! returns true if throttle "turbo" modifier is active
	proto external bool IsThrottleTurbo();
	//! returns true if handbrake is active
	proto external bool GetHandBrake();
	//! returns true if persistent handbrake is active
	proto external bool GetPersistentHandBrake();
	//! sets persistent handbrake state
	proto external void SetPersistentHandBrake(bool newValue);
	proto external bool HasAutomaticGearbox();
	proto external int GetFutureGear();
	proto external int GetCurrentGear();
	//! returns simulation component of this controllers
	proto external VehicleWheeledSimulation_SA_B GetSimulation();
}

/*!
\}
*/
