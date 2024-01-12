/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseCollimatorSightsComponentClass: SightsComponentClass
{
}

class BaseCollimatorSightsComponent: SightsComponent
{
	/*!
	Set virtual reticle size.
	\param angularSize apparent angular size of the reticle, in degrees
	\param reticlePortion the portion of the texture (in percent) that covers the angular size, i.e. the reticle "diameter"
	*/
	proto external void SetReticleSize(float angularSize, float reticlePortion);
	//! Get the default angular size of the reticle, in degrees
	proto external float GetReticleAngularSize();
	//! Get the default portion of the reticle that covers the given angular size
	proto external float GetReticlePortion();
	//! Get the number of reticle infos
	proto external int GetNumReticles();
	//! Is reticle index valud
	proto external bool IsReticleValid(int index);
	//! Get a reticle by index
	proto external BaseCollimatorReticleInfo GetReticleByIndex(int index);
	//! Get current reticle shape
	proto external int GetCurrentReticleShape();
	//! Set the next reticle shape. This always works, but might not do anything if only one reticle is defined
	proto external void ReticleNextShape();
	//! Set the previous reticle shape. This always works, but might not do anything if only one reticle is defined
	proto external void ReticlePreviousShape();
	//! Set reticle shape by index. Returns true if successful
	proto external bool SetReticleShapeByIndex(int iIndex);
	//! Get the number of reticle colors
	proto external int GetNumColors();
	//! Is color index valid
	proto external int IsColorValid(int index);
	//! Get a color record by index
	proto external BaseCollimatorReticleColor GetColorByIndex(int index);
	//! Get current reticle color index
	proto external int GetCurrentColor();
	//! Set the next reticle color. This always works, but might not do anything if only one or no color is defined
	proto external void ReticleNextColor();
	//! Set the previous reticle color. This always works, but might not do anything if only one or no color is defined
	proto external void ReticlePreviousColor();
	//! Set reticle color by index. Return true if successful
	proto external bool SetReticleColorByIndex(int iIndex);

	// callbacks

	//! Called to update the sight position U/V
	event void UpdateReticlePosition(float u, float v, float uScale, float vScale);
	//! Called on PostInit when all components are added
	event void OnPostInit(IEntity owner);
	//! Called to update the Reticle shape
	event void UpdateReticleShapeIndex(int index);
	//! Called to update reticle color
	event void UpdateReticleColor(vector inner, vector glow);
	event void OnSightADSActivate();
	event void OnSightADSDeactivated();
}

/*!
\}
*/
