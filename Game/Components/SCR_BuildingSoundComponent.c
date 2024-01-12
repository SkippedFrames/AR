[ComponentEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.")]
class SCR_BuildingSoundComponentClass: SoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingSoundComponent : SoundComponent
{	
	[Attribute("8", UIWidgets.Slider, "Time interval when maximum WindSpeed [s]", "0 60 1")]
	int m_iTimeIntervalMin;
	
	[Attribute("20", UIWidgets.Slider, "Time interval when minimum WindSpeed [s]", "0 60 1")]
	int m_iTimeIntervalMax;
				
	[Attribute("0 0 0", UIWidgets.Coords, "Mins OOB Point")]
	vector m_vMins;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Maxs OOB Point")]
	vector m_vMaxs;
				
	private static const float RANDOM_PERCENTAGE_MIN = 0.7;
	private static const float RANDOM_PERCENTAGE_MAX = 1.3;
	
	protected float m_fTriggerInterval;

	//------------------------------------------------------------------------------------------------				
	private void TriggeredSoundHandler(IEntity owner, float timeSlice)
	{	
		vector mat[4];
		mat[3] = GetSoundPosition(owner);
	
		//Set Interior signal
		SetInteriorSignal(owner);

		// Play sound
		SoundEventTransform(SCR_SoundEvent.SOUND_CREAK, mat);
		
		// Get remetition time
		GetTimeInterval();
	}
	
	private void SetInteriorSignal(IEntity owner)
	{
		bool interior;
		
		float gInterior = GetGame().GetSignalsManager().GetSignalValue(GetGame().GetSignalsManager().AddOrFindSignal("GInterior"));
		
		if (gInterior == 1)
		{
			PlayerController playerControler = GetGame().GetPlayerController();
			if (!playerControler)
				return;
			
			PlayerCamera playerCamera = playerControler.GetPlayerCamera();
			if (!playerCamera)
				return;
						
			interior = IsInWorldBounds(playerCamera.GetOrigin(), owner);
		}
	
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));		
		if (signalsManagerComponent)
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("Interior"), interior);		
	}
	
	private bool IsInWorldBounds(vector point, IEntity entity)
	{
		vector mins = vector.Zero;
		vector maxs = vector.Zero;
		
		entity.GetWorldBounds(mins, maxs);
			
		return !(point[0] < mins[0] || point[0] > maxs[0] || point[1] < mins[1] || point[1] > maxs[1] || point[2] < mins[2] || point[2] > maxs[2]);
	}
	
	private void GetTimeInterval()
	{		
		float windSpeed = GetGame().GetSignalsManager().GetSignalValue(GetGame().GetSignalsManager().AddOrFindSignal("WindSpeed"));		
		float timeInterval = Interpolate(windSpeed, SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX, m_iTimeIntervalMax, m_iTimeIntervalMin);
			
		m_fTriggerInterval = Math.RandomFloat(timeInterval * RANDOM_PERCENTAGE_MIN, timeInterval * RANDOM_PERCENTAGE_MAX);
	}
	
	private vector GetSoundPosition(IEntity owner)
	{		
		vector v;
		v[0] = Math.RandomFloat(m_vMins[0], m_vMaxs[0]);
		v[1] = Math.RandomFloat(m_vMins[1], m_vMaxs[1]);
		v[2] = Math.RandomFloat(m_vMins[2], m_vMaxs[2]);
		
		// From Local To World Space;	
		v = owner.CoordToParent(v);
			
		return v;	
	}
		
	private float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;
			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
		
	//------------------------------------------------------------------------------------------------		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		m_fTriggerInterval -= timeSlice;
	
		if (m_fTriggerInterval > 0)
			return;
		
		TriggeredSoundHandler(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdateSoundJobBegin(IEntity owner)
	{
		GetTimeInterval();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BuildingSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingSoundComponent()
	{
	}
};
