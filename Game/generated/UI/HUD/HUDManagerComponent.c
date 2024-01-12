/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD
\{
*/

class HUDManagerComponentClass: SCR_BaseHUDComponentClass
{
}

class HUDManagerComponent: SCR_BaseHUDComponent
{
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	event protected void OnUpdate(IEntity owner);
	event protected void OnControlledEntityChanged(IEntity from, IEntity to);
}

/*!
\}
*/
