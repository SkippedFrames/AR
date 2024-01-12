//------------------------------------------------------------------------------------------------
class SCR_PlayInstrument : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("0", UIWidgets.ComboBox, "Instrument type", "", ParamEnumArray.FromEnum(SCR_EInstrumentType) )]
	protected SCR_EInstrumentType m_eInstrumentType;
	
	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;
	
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	protected static ref ScriptInvokerInt2 s_onInstrumentPlayed;
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity) 
	{
		if (s_onInstrumentPlayed)
			s_onInstrumentPlayed.Invoke(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), m_eInstrumentType);
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
				
		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(GetOwner(), m_AudioSourceConfiguration);
		if (!audioSource)
			return;
		
		vector mat[4];
		mat[3] = GetOwner().CoordToParent(m_vSoundOffset);
					
		AudioSystem.TerminateSound(m_AudioHandle);
		soundManagerEntity.PlayAudioSource(audioSource, mat);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		AudioSystem.TerminateSound(m_AudioHandle);
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
				
		if (!m_AudioSourceConfiguration || m_AudioSourceConfiguration.m_sSoundProject == string.Empty)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration;
		audioSourceConfiguration.m_sSoundProject = m_AudioSourceConfiguration.m_sSoundProject;
		audioSourceConfiguration.m_eFlags = m_AudioSourceConfiguration.m_eFlags;
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_STOP_PLAYING;
				
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(pOwnerEntity, audioSourceConfiguration);
		if (!audioSource)
			return;
		
		vector mat[4];
		mat[3] = pOwnerEntity.CoordToParent(m_vSoundOffset);
					
		soundManagerEntity.PlayAudioSource(audioSource, mat);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
		
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt2 GetOnInstrumentPlayed()
	{
		if (!s_onInstrumentPlayed)
			s_onInstrumentPlayed = new ScriptInvokerInt2();
		
		return s_onInstrumentPlayed;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayInstrument()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};

enum SCR_EInstrumentType
{
	PIANO,
	ORGAN
}
