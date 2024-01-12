[EntityEditorProps(category: "GameScripted/Sound", description: "")]
class SCR_SoundDataComponentClass : ScriptComponentClass
{
	[Attribute("", UIWidgets.Auto, desc: "Audio Source Configuration)")]
	ref array<ref SCR_AudioSourceConfiguration> m_aAudioSourceConfiguration;
};

class SCR_SoundDataComponent : ScriptComponent
{	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns audio project resource for coresponding sound event 
	\eventName Sound event name used for seach for audio project resource
	*/		
	SCR_AudioSourceConfiguration GetAudioSourceConfiguration(string eventName)
	{
		SCR_SoundDataComponentClass prefabData = SCR_SoundDataComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;
		
		foreach (SCR_AudioSourceConfiguration audioSourceConfiguration : prefabData.m_aAudioSourceConfiguration)
		{
			if (audioSourceConfiguration.m_sSoundEventName == eventName)
			{
				if (audioSourceConfiguration.m_sSoundProject)
					return audioSourceConfiguration;
				else
					return null;
			}
		}		
		return null;
	}
};
