enum EAudioSourceConfigurationFlag
{
	Static = 1,
	DistanceSignal = 2,
	FinishWhenEntityDestroyed = 4
}

[BaseContainerProps()]
class SCR_AudioSourceConfiguration
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project (acp)", "acp")]
	ResourceName m_sSoundProject;
	
	[Attribute("", UIWidgets.EditBox, desc: "Sound event name")]
	string m_sSoundEventName;
		
	[Attribute("1", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EAudioSourceConfigurationFlag))]
	EAudioSourceConfigurationFlag m_eFlags;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns true if m_sSoundProject and m_sSoundEventName are defined and SCR_AudioSourceConfiguration is valid for AudioSource creation
	*/	
	bool IsValid()
	{
		return (m_sSoundProject != string.Empty && m_sSoundEventName != string.Empty);
	}
};
