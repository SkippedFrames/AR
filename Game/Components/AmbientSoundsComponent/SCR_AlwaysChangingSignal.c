/*
AlwaysChanging is defined using SCR_AlwaysChangingSignalDef
Signal is constantly oscillating between randomly generated values in <m_fSignalValueMin, m_fSignalValueMax> range
Speed of oscillation is driven by randomized time values in <m_fInterpolationTimeMin, m_fInterpolationTimeMax> range
*/
[BaseContainerProps(configRoot: true)]
class SCR_AlwaysChangingSignal: SCR_AmbientSoundsEffect
{	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Always changing signal resource config", params: "conf")]
	private ResourceName m_AlwaysChangingSignalDefResource;
	
	private ref SCR_AlwaysChangingSignalDef m_AlwaysChangingSignalDef;

	//------------------------------------------------------------------------------------------------
	/*
	Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	*/	
	override void Update(float worldTime, vector cameraPos)
	{
		float fInterpolationTimeCurrent = (worldTime - m_AlwaysChangingSignalDef.m_fTimer) / m_AlwaysChangingSignalDef.m_fInterpolationTime;
						
		// New interpolation values
		if (fInterpolationTimeCurrent > 1)
		{
			m_AlwaysChangingSignalDef.m_fTimer = worldTime;
			fInterpolationTimeCurrent = 0;
			m_AlwaysChangingSignalDef.m_fSignalTargetLast = m_AlwaysChangingSignalDef.m_fSignalTarget;
			m_AlwaysChangingSignalDef.m_fInterpolationTime = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fInterpolationTimeMin, m_AlwaysChangingSignalDef.m_fInterpolationTimeMax);
			m_AlwaysChangingSignalDef.m_fSignalTarget = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fSignalValueMin, m_AlwaysChangingSignalDef.m_fSignalValueMax);
		}
		m_LocalSignalsManager.SetSignalValue(m_AlwaysChangingSignalDef.m_iSignalIdx, Math.Lerp(m_AlwaysChangingSignalDef.m_fSignalTargetLast, m_AlwaysChangingSignalDef.m_fSignalTarget, fInterpolationTimeCurrent));
	}
	
	//------------------------------------------------------------------------------------------------	
	/*
	Called by SCR_AmbientSoundComponent in EOnPostInit()
	*/	
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, signalsManagerComponent);
		
		// Load config	
		SCR_AlwaysChangingSignalDef alwaysChangingSignalDef;
		if (m_AlwaysChangingSignalDefResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_AlwaysChangingSignalDefResource);
			if (holder)
				alwaysChangingSignalDef = SCR_AlwaysChangingSignalDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!alwaysChangingSignalDef)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Always Changing Signal Config", LogLevel.WARNING);
			return;
		}
		
		m_AlwaysChangingSignalDef = alwaysChangingSignalDef;
		
		// Set init values
		m_AlwaysChangingSignalDef.m_fInterpolationTime = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fInterpolationTimeMin, m_AlwaysChangingSignalDef.m_fInterpolationTimeMax);
		m_AlwaysChangingSignalDef.m_fSignalTarget = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fSignalValueMin, m_AlwaysChangingSignalDef.m_fSignalValueMax);
		m_AlwaysChangingSignalDef.m_iSignalIdx = m_LocalSignalsManager.AddOrFindSignal(m_AlwaysChangingSignalDef.m_sSignalName);
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void ReloadConfig()
	{
		super.ReloadConfig();
		
		SCR_AlwaysChangingSignalDef alwaysChangingSignalDef;
		if (m_AlwaysChangingSignalDefResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_AlwaysChangingSignalDefResource);
			if (holder)
				alwaysChangingSignalDef = SCR_AlwaysChangingSignalDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (alwaysChangingSignalDef)	
			m_AlwaysChangingSignalDef = alwaysChangingSignalDef;
		
		// Set init values
		m_AlwaysChangingSignalDef.m_fInterpolationTime = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fInterpolationTimeMin, m_AlwaysChangingSignalDef.m_fInterpolationTimeMax);
		m_AlwaysChangingSignalDef.m_fSignalTarget = Math.RandomFloat(m_AlwaysChangingSignalDef.m_fSignalValueMin, m_AlwaysChangingSignalDef.m_fSignalValueMax);
		m_AlwaysChangingSignalDef.m_iSignalIdx = m_LocalSignalsManager.AddOrFindSignal(m_AlwaysChangingSignalDef.m_sSignalName);
	}
#endif
}
