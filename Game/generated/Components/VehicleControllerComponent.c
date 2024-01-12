/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class VehicleControllerComponentClass: CompartmentControllerComponentClass
{
}

class VehicleControllerComponent: CompartmentControllerComponent
{
	proto external bool CanSwitchSeat();
	//! Returns the global driving assistance mode.
	static proto EVehicleDrivingAssistanceMode GetDrivingAssistanceMode();
	//! Sets the global driving assistance mode.
	static proto void SetDrivingAssistanceMode(EVehicleDrivingAssistanceMode mode);
	//! Returns the base of simulation component associated with this controller.
	proto external VehicleBaseSimulation GetBaseSimulation();
	//! Returns the fuel manager associated with this controller.
	proto external FuelManagerComponent GetFuelManager();
	//! Returns the light manager associated with this controller.
	proto external BaseLightManagerComponent GetLightManager();
	//! Returns the pilot compartment slot associated with this controller.
	proto external PilotCompartmentSlot GetPilotCompartmentSlot();
	//! Returns the signals manager associated with this controller.
	proto external SignalsManagerComponent GetSignalsManager();
	//! Returns the weapon manager associated with this controller.
	proto external BaseWeaponManagerComponent GetWeaponManager();
	//! Try to start the engine with the chance of getting the engine not started based on engine startup chance.
	proto external void TryStartEngine();
	/*!
	Start the engine directly without any chance of startup involved.
	\return Returns true if the engine is started otherwise false.
	*/
	proto external bool StartEngine();
	/*!
	Stop the engine.
	\param playDriverAnimation The driver should play the animation or not.
	*/
	proto external void StopEngine(bool playDriverAnimation = true);
	proto external bool IsEngineOn();
	//! Returns the engine startup chance in <0, 100>.
	proto external float GetEngineStartupChance();
	/*!
	Set the engine startup chance.
	\param chance Startup chance in <0, 100>.
	*/
	proto external void SetEngineStartupChance(float chance);
	//! Returns true if the engine is drowned.
	proto external bool GetEngineDrowned();
	/*!
	Set the engine drowned.
	\param drowned True to set the engine drowned, false otherwise.
	*/
	proto external void SetEngineDrowned(bool drowned);
}

/*!
\}
*/
