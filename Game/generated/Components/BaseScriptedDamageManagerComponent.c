/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseScriptedDamageManagerComponentClass: SCR_DamageManagerComponentClass
{
}

class BaseScriptedDamageManagerComponent: SCR_DamageManagerComponent
{
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	event protected void OnDelete(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
	//! Must be first enabled with event mask
	event protected bool OnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeSlice);
	event protected void OnDamage(EDamageType type, float damage, HitZone pHitZone, notnull Instigator instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
	/*!
	Called whenever an instigator is going to be set.
	\param currentInstigator: This damage manager's last instigator
	\param newInstigator: The new instigator for this damage manager
	\return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	event bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator);
}

/*!
\}
*/
