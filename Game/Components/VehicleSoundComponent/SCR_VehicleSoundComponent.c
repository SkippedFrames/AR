[EntityEditorProps(category: "GameScripted/Sound", description: "Testing component")]
class SCR_VehicleSoundComponentClass : VehicleSoundComponentClass
{
	[Attribute("", UIWidgets.Auto, "HitZone State Signals")]
	ref array<ref SCR_HitZoneStateSignalData> m_aHitZoneStateSignalData;
};

class SCR_VehicleSoundComponent : VehicleSoundComponent
{		
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_SignalDefinition> m_aSignalDefinition;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Mins OOB Point for rain sound")]
	protected vector m_vMins;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Maxs OOB Point for rain sound")]
	protected vector m_vMaxs;
		
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected ref array<ref SCR_HitZoneStateSignal> m_aHitZoneStateSignal = {};
	
	// Audio Handles
	protected AudioHandle m_RainSoundAudioHandle = AudioHandle.Invalid;
	
	protected GameSignalsManager m_GameSignalsManager;
	protected int m_iRainIntensitySignalIdx;
	protected const static float RAIN_INTENSITY_THRESHOLD = 0.1;
	protected const static string RAIN_INTENSITY_SIGNAL_NAME = "RainIntensity";
	
	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_HitZoneStateSignalData> GetHitZoneStateSignalData()
	{
		SCR_VehicleSoundComponentClass prefabData = SCR_VehicleSoundComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
		{
			return prefabData.m_aHitZoneStateSignalData;
		}
		
		return null;	
	}
	//------------------------------------------------------------------------------------------------	
	private vector GetRainSoundPositionOffset()
	{
		vector cameraTransform[4];
		GetGame().GetWorld().GetCurrentCamera(cameraTransform);
				
		// Local camara position
		vector camera = GetOwner().CoordToLocal(cameraTransform[3]);
			
		// Get offset
		vector offset;
		offset[0] = Math.Clamp(camera[0], m_vMins[0], m_vMaxs[0]);
		offset[1] = Math.Clamp(camera[1], m_vMins[1], m_vMaxs[1]);;
		offset[2] = Math.Clamp(camera[2], m_vMins[2], m_vMaxs[2]);
				
		return GetOwner().VectorToParent(offset);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		HandleGeneratedSignals(timeSlice);
		HandleRainSound();
	}

	//------------------------------------------------------------------------------------------------
	private void HandleGeneratedSignals(float timeSlice)
	{
		if (!m_SignalsManagerComponent)
			return;
					
		foreach (SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
			m_SignalsManagerComponent.SetSignalValue(signalDefinition.m_iSignalIdx, signalDefinition.GetSignalValue(timeSlice));
	}
	
	//------------------------------------------------------------------------------------------------
	private void HandleRainSound()
	{
		if (m_GameSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx) >= RAIN_INTENSITY_THRESHOLD)
		{
			if (IsFinishedPlaying(m_RainSoundAudioHandle))
				m_RainSoundAudioHandle = SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_RAIN);
			
			if (!IsHandleValid(m_RainSoundAudioHandle))
				return;
				
			// Update position
			vector mat[4];
			mat[3] = GetRainSoundPositionOffset() + GetOwner().GetOrigin();
			SetSoundTransformation(m_RainSoundAudioHandle, mat);
		}	
		else if (m_RainSoundAudioHandle != AudioHandle.Invalid)
		{
			Terminate(m_RainSoundAudioHandle);
			m_RainSoundAudioHandle = AudioHandle.Invalid;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		RegisterHitZoneSignals();
	}
		
	//------------------------------------------------------------------------------------------------
	protected void RegisterHitZoneSignals()
	{	
		if (!m_SignalsManagerComponent)
			return;
		
		SCR_HitZoneContainerComponent hitZoneContainerComponent = SCR_HitZoneContainerComponent.Cast(GetOwner().FindComponent(SCR_HitZoneContainerComponent));
		if (!hitZoneContainerComponent)
			return;
		
		array<ref SCR_HitZoneStateSignalData> hitZoneStateSignalData = GetHitZoneStateSignalData();
		if (!hitZoneStateSignalData)
			return;
					
		foreach(SCR_HitZoneStateSignalData data : hitZoneStateSignalData)
		{		
			SCR_HitZoneStateSignal hitZoneSignal = new SCR_HitZoneStateSignal;			
			if (hitZoneSignal.RegisterSignal(hitZoneContainerComponent, data, m_SignalsManagerComponent))
				m_aHitZoneStateSignal.Insert(hitZoneSignal);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterHitZoneSignals()
	{				
		foreach(SCR_HitZoneStateSignal hitZoneStateSignal : m_aHitZoneStateSignal)
		{
			hitZoneStateSignal.UnregisterSignal();
		}
		
		m_aHitZoneStateSignal.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		
		if (m_SignalsManagerComponent)
		{
			foreach (SCR_SignalDefinition signalDefinition : m_aSignalDefinition)
				signalDefinition.m_iSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(signalDefinition.m_sSignalName);
		}
						
		// Get Game Signals Manger
		m_GameSignalsManager = GetGame().GetSignalsManager();
		
		// Get RainIntensity signal index
		m_iRainIntensitySignalIdx = m_GameSignalsManager.AddOrFindSignal(RAIN_INTENSITY_SIGNAL_NAME);
		
		// Collision sounds setup
		SetMinTimeAfterImpact(300);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_VehicleSoundComponent()
	{
		UnregisterHitZoneSignals();
	}

};
