class SCR_BaseScreenEffect : SCR_InfoDisplayExtended
{
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called when any settings are changed in the settings menu, upon DisplayStartDraw()
	//! Called from SCR_ScreenEffectsManager. So not every effect has to listen for this individually
	//! Relevant when your effect needs to adapt when particular settings are changed, such as switching from normal DoF to Bokeh
	void SettingsChanged()
	{
	}

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called every frame from SCR_ScreenEffectsManager
	void UpdateEffect(float timeSlice)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Automatically called upon DisplayStartDraw() and DisplayStopDraw() from SCR_ScreenEffectsManager
	//! Due to many screeneffects listening to invokers, ClearEffects() is often called manually OnControlledEntityChanged()
	void ClearEffects()
	{
	}
};
