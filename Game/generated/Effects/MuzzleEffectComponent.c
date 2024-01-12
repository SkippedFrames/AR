/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class MuzzleEffectComponentClass: BaseEffectComponentClass
{
}

class MuzzleEffectComponent: BaseEffectComponent
{
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	event void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity);
}

/*!
\}
*/
