/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class CoverQueryProperties: Managed
{
	proto external void SetThreatPosition(vector threatPosition);
	proto external void SetOriginPosition(vector originPosition);
	proto external void SetHeightLimits(float minHeight, float maxHeight);
	proto external void SetDistanceLimits(float distMin, float distMax);
	proto external void SetDesiredDirection(vector direction);
	proto external void SetWeightDesiredDirection(float weightDesiredDirection);
	proto external void SetCosAngleLimits(float cosMin);
	proto external void SetCosAngleDirVecToCoverLimits(float cosMin);
	proto external void CheckVisibility(bool checkVisibility);
}

/*!
\}
*/
