/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SimpleSoundComponentClass: BaseSoundComponentClass
{
}

class SimpleSoundComponent: BaseSoundComponent
{
	//! Enables the dynamic simulation.
	proto external void EnableDynamicSimulation(bool value);
	//! Set flag for script callbacks.
	proto external void SetScriptedMethodsCall(bool state);
	//! TRUE when flag for script callbacks is set.
	proto external bool IsScriptedMethodsCallEnabled();
	proto external IEntity GetOwner();

	// callbacks

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
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
	//! Call when component is in range
	event protected void UpdateSoundJob(IEntity owner, float timeSlice);
	//! Called when dynamic simulation is started.
	event protected void OnUpdateSoundJobBegin(IEntity owner);
	//! Called when dynamic simulation is stopped.
	event protected void OnUpdateSoundJobEnd(IEntity owner);
}

/*!
\}
*/
