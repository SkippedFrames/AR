/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterHeadAimingComponentClass: AimingComponentClass
{
}

class CharacterHeadAimingComponent: AimingComponent
{
	proto external CharacterAimingComponent GetCharacterAimingComponent();
	proto external void SetPitchLimitReductionMultiplier(float value);
	proto external vector GetLookAngles();
	//! computes look transformation in model space relative to Pivot if set in properties
	proto external void GetLookTransformationMS(int nodeID, EDirectBoneMode mode, inout vector offsetFromHead, inout vector baseRotation, inout vector transformFromBone[4]);
	//! computes look transformation in local space relative to Pivot if set in properties
	proto external void GetLookTransformationLS(int nodeID, EDirectBoneMode mode, inout vector offsetFromHead, inout vector baseRotation, inout vector transformFromBone[4]);
}

/*!
\}
*/
