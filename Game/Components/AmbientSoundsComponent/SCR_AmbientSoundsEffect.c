[BaseContainerProps(configRoot: true)]
class SCR_AmbientSoundsEffect
{
	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;
	protected SignalsManagerComponent m_LocalSignalsManager;
	
	//------------------------------------------------------------------------------------------------
	/*
	Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	*/
	void Update(float worldTime, vector cameraPos)
	{
	}
	
	//------------------------------------------------------------------------------------------------	
	/*
	Called by SCR_AmbientSoundComponent in OnPostInit()
	*/	
	void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		m_AmbientSoundsComponent = ambientSoundsComponent;
		m_LocalSignalsManager = signalsManagerComponent;
	}
	
	//------------------------------------------------------------------------------------------------	
	/*
	Called by SCR_AmbientSoundComponent in OnInit()
	*/	
	void OnInit()
	{
		
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void UpdateDebug(float worldTime)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void ReloadConfig()
	{
	}
#endif
}
