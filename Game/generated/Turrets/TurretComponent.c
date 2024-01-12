/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Turrets
\{
*/

class TurretComponentClass: AimingComponentClass
{
}

class TurretComponent: AimingComponent
{
	proto external bool HasMoveableBase();
	proto external PointInfo GetCameraAttachmentSlot();
	proto external bool IsVehicleMounted();
	/*!
	Calculates aiming angle to target and returns horizontal and vertical excess of target angle compared to aiming limits, in degrees.
		outExcess[0] - horizontal excess
		outExcess[1] - vertical excess
	Example:
		Limits = -10...10. TgtAngle = 12. Excess = 2.
		Limits = -10...10. TgtAngle = -13. Excess = -3.
		Limits = -10...10. TgtAngle = 9. Excess = 0.
	*/
	proto external vector GetAimingAngleExcess(vector tgtPosWorld);
}

/*!
\}
*/
