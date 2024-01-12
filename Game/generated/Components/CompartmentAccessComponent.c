/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class CompartmentAccessComponentClass: GameComponentClass
{
}

class CompartmentAccessComponent: GameComponent
{
	//! Returns the compartent we're in
	proto external BaseCompartmentSlot GetCompartment();
	//! Returns the first free compartment of a given type on \param targetEntity
	proto external BaseCompartmentSlot FindFreeCompartment(IEntity targetEntity, ECompartmentType compartmentType, bool useReserved = true);
	proto external BaseCompartmentSlot FindCompartmentReservedBy(IEntity targetEntity, IEntity reservedBy);
	//! Returns true if we're inside a compartment
	proto external bool IsInCompartment();
	//! Returns true if we're inside a compartment with enabled ADS
	proto external bool IsInCompartmentADS();
	//! Returns true if \param targetEntity is accessible for getting in (e.g. not upside down)
	proto external bool IsTargetVehicleAccessible(IEntity targetEntity);
	//! Returns true if \param targetEntity can be entered at this time
	proto external bool CanGetInVehicle(IEntity targetEntity);
	//! Returns the entity owning the component
	proto external IEntity GetOwner();
	//! Returns true if \param targetEntity can be entered at this time
	proto external bool CanGetInVehicleViaDoor(IEntity targetEntity, BaseCompartmentSlot targetCompartment, int doorInfoIndex);
	//! Make current entity get into \param targetEntity's compartment specified by \param targetCompartment
	proto external bool GetInVehicle(IEntity targetEntity, BaseCompartmentSlot targetCompartment, int doorInfoIndex);
	//! Make current entity move (teleport) into \param targetEntity's compartment specified by \param targetCompartment
	proto external bool MoveInVehicle(IEntity targetEntity, BaseCompartmentSlot targetCompartment);
	//! If true, last exiting of vehicle was by ejecting, so we should not reset vehicle inputs.
	proto external bool WasLastGettingOutEjecting();
	//! Returns true while getting in
	proto external bool IsGettingIn();
	//! Returns true while getting out
	proto external bool IsGettingOut();
	//! Returns true if compartment can be gotten out of
	proto external bool CanGetOutVehicle();
	//! Returns true if compartment can be gotten out of via the door with index \param doorIndex
	proto external bool CanGetOutVehicleViaDoor(int doorIndex);
	//! Get out of current entity via the door with index \param doorInfoIndex
	proto external bool GetOutVehicle(int doorInfoIndex, bool ejects);
	//! Move out of (teleport from) current vehicle via the door with index \param doorInfoIndex
	proto external bool MoveOutVehicle(int doorInfoIndex, vector targetTransform[4]);
	//! Teleport out of the vehicle normally - when exiting is not possible.
	proto external bool EjectOutOfVehicle();
	//! Returns true if current compartment can be jumped from
	proto external bool CanJumpOutVehicle();
	//! Jump out of current compartment
	proto external bool JumpOutVehicle();
	//! Returns the vehicle entity is in (root entity of whole hierarchy)
	static proto IEntity GetVehicleIn(IEntity entity);

	// callbacks

	event protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
	event protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
}

/*!
\}
*/
