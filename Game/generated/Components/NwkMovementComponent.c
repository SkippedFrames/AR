/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class NwkMovementComponentClass: GameComponentClass
{
}

//! Base class for entity replication - e.g. vehicles, characters, animals
class NwkMovementComponent: GameComponent
{
	proto external bool IsInterpolating();
	proto external void EnableSimulation(bool enable);
	proto external void EnableInterpolation(bool enable);
	proto external bool IsInterpolationEnabled();
}

/*!
\}
*/
