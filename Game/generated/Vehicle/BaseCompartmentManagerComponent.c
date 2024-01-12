/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseCompartmentManagerComponentClass: GameComponentClass
{
}

class BaseCompartmentManagerComponent: GameComponent
{
	//! Searches for compartment by ID of slot and compartment manager.
	proto external BaseCompartmentSlot FindCompartment(int compartmentID, int mgrId = -1);
	//! Returns a list and count of all compartments
	proto external int GetCompartments(out notnull array<BaseCompartmentSlot> outCompartments);
}

/*!
\}
*/
