/*
===========================================
Do not modify, this script is generated
===========================================
*/

class CompartmentDoorInfo: ScriptAndConfig
{
	proto external int GetAnimDoorIndex();
	proto external bool UseTeleportForGetIn();
	proto external bool UseTeleportForGetOut();
	proto external bool UseEntryPositionAligning();
	proto external bool UseSeatPositionAligning();
	proto external PointInfo GetEntryPointInfo();
	proto external PointInfo GetExitPointInfo();
	proto external bool HasExitPointInfoDefined();
	proto external ECharacterStanceChange GetStanceChangeOnExit();
}
