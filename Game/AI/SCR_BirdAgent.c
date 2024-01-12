class SCR_BirdClass: AIFlockClass
{
};

class SCR_Bird : AIFlock
{	
	private const string IS_FLYING_SIGNAL_NAME = "IsFlying";
	private const string SOUND_FLOCK_TAKEOFF_EVENT_NAME = "SOUND_FLOCK_TAKEOFF";
	private const string SOUND_FLOCK_LAND_EVENT_NAME = "SOUND_FLOCK_LAND";
			
	override event protected void EOnInit(IEntity owner)
	{
	
		//--- Activate AI
		//AIControlComponent control = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		//if (control)
			//control.ActivateAI();

	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnReactToDanger()
	{	
		super.OnReactToDanger();
			
		SoundEvent(SOUND_FLOCK_TAKEOFF_EVENT_NAME);
		SetIsFlyingSignal(true);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnTakeOff()
	{
		super.OnTakeOff();
		
		SetIsFlyingSignal(true);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnLanding()
	{
		super.OnLanding();
		
		SoundEvent(SOUND_FLOCK_LAND_EVENT_NAME);
		SetIsFlyingSignal(false);
	}
	
	//------------------------------------------------------------------------------------------------	
	private void SetIsFlyingSignal(bool isFlying)
	{
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		if (signalsManagerComponent)
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal(IS_FLYING_SIGNAL_NAME), isFlying);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SoundEvent(string soundEvent)
	{
		SoundComponent soundComponent = SoundComponent.Cast(FindComponent(SoundComponent));
		if (soundComponent)
			soundComponent.SoundEvent(soundEvent);	
	}	
}
