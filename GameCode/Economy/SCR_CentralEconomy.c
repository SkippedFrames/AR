//! Placement setup for sides (if required) and other flags
enum EPlacementFlags
{
	PF_NONE,

	PF_LEFT,
	PF_RIGHT,
	PF_TOP,
	PF_BOTTOM,
	PF_FRONT,
	PF_BACK,

	PF_LEFTRIGHT,
	PF_TOPBOTTOM,
	PF_FRONTBACK,

	PF_ALL,
	
	PF_AIRBORN,	// spawn airborne or levitating, do not align anyhow
};


class SCR_CentralEconomyClass: CentralEconomyClass
{
};

class SCR_CentralEconomy : CentralEconomy
{

	//! When something get spawned
	override void EOnSpawn( IEntity entity )
	{
		Print( "[CE] Spawned: " );
		Print(entity);
	};	
	//! When something get removed
	override void EOnRemove()
	{
	};
};
