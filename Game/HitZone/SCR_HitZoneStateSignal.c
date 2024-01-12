class SCR_HitZoneStateSignal
{
	ref array<ref ScriptedHitZone> m_aScriptedHitZones = {};	
	int m_iSignalIdx;
	
	//------------------------------------------------------------------------------------------------
	bool RegisterSignal(SCR_HitZoneContainerComponent hitZoneContainerComponent, SCR_HitZoneStateSignalData hitZoneStateSignalData, SignalsManagerComponent signalsManagerComponent)
	{	
		array<string> hitZoneNames = hitZoneStateSignalData.m_aHitZoneNames;
		if (!hitZoneNames || hitZoneNames.Count() == 0)
		{
			Print("AUDIO: SCR_HitZoneStateSignal: Missing HitZoneNames", LogLevel.WARNING);
			return false;
		}
		
		foreach (string hitZoneName : hitZoneNames) 
		{
			HitZone hitZone = hitZoneContainerComponent.GetHitZoneByName(hitZoneName);
			ScriptedHitZone scriptedHitZone = ScriptedHitZone.Cast(hitZone);
			if (!scriptedHitZone)
				continue;
			
			scriptedHitZone.GetOnDamageStateChanged().Insert(OnStateChanged);
			m_aScriptedHitZones.Insert(scriptedHitZone);
		}
		
		if (m_aScriptedHitZones.Count() == 0)
		{
			Print("AUDIO: SCR_HitZoneStateSignal: No HitZone found", LogLevel.WARNING);
			return false;
		}
		
		string signalName = hitZoneStateSignalData.m_sSignalName;
		if (signalName.IsEmpty())
		{
			Print("AUDIO: SCR_HitZoneStateSignal: Missing signal name", LogLevel.WARNING);
			return false;
		}
		
		m_iSignalIdx = signalsManagerComponent.AddOrFindSignal(signalName);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterSignal()
	{		
		foreach(ScriptedHitZone scriptedHitZone : m_aScriptedHitZones)
		{
			scriptedHitZone.GetOnDamageStateChanged().Remove(OnStateChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnStateChanged()
	{
		int max;
		
		foreach(ScriptedHitZone scriptedHitZone : m_aScriptedHitZones)
		{	
			int state = DamageStateToSignalValue(scriptedHitZone.GetDamageState());
			if (state > max)
				max = state;
		}
		
		IEntity owner = m_aScriptedHitZones[0].GetOwner();
		if (!owner)
			return;
		
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(owner.GetRootParent().FindComponent(SignalsManagerComponent));
		if (!signalsManager)
			return;
							
		signalsManager.SetSignalValue(m_iSignalIdx, max);		
	}
	
	//------------------------------------------------------------------------------------------------
	static int DamageStateToSignalValue(EDamageState damageState)
	{
		switch(damageState)
		{
			case EDamageState.UNDAMAGED: {return 0;};
			case EDamageState.INTERMEDIARY: {return 1;};
			case EDamageState.STATE1: {return 2;};
			case EDamageState.STATE2: {return 3;};
			case EDamageState.STATE3: {return 4;};
			case EDamageState.DESTROYED: {return 5;};
		}
		
		return 0;
	}
}
