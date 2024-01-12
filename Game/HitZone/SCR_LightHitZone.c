class SCR_LightHitZone : SCR_DestructibleHitzone
{
	private BaseLightManagerComponent m_pLightManager;
	private ref array<BaseLightSlot> m_aLightSlots = {};
	
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		IEntity parent = pOwnerEntity;
		while (parent && !m_pLightManager)
		{
			m_pLightManager = BaseLightManagerComponent.Cast(parent.FindComponent(BaseLightManagerComponent));
			parent = parent.GetParent();
		}
		
		if (!m_pLightManager)
			return;
		
		SCR_LightSlot scrLightSlot;
		string name = GetName();
		array<BaseLightSlot> lights = {}; m_pLightManager.GetLights(lights);
		foreach (BaseLightSlot lightSlot: lights)
		{
			scrLightSlot = SCR_LightSlot.Cast(lightSlot);
			if (scrLightSlot && name.Compare(scrLightSlot.GetHitZoneName(), false) == 0)
				m_aLightSlots.Insert(lightSlot);
		}
		
		UpdateLightState();
	}
	
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		UpdateLightState();
	}
	
	void UpdateLightState()
	{
		bool isAlive = GetDamageState() != EDamageState.DESTROYED;
		int surfaceID;
		foreach (BaseLightSlot lightSlot: m_aLightSlots)
		{
			if (!lightSlot)
				continue;
			
			lightSlot.SetLightFunctional(isAlive);
			
			if (!m_pLightManager)
				continue;
			
			surfaceID = lightSlot.GetSurfaceID();
			if (surfaceID > -1)
				m_pLightManager.TrySetSurfaceFunctional(surfaceID, isAlive);
		}
	}
	
	void ~SCR_LightHitZone()
	{
		m_aLightSlots.Clear();
		m_aLightSlots = null;
	}
};
