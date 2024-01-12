[EntityEditorProps(category: "GameScripted/Components", description: "Base lock component.")]
class SCR_BaseLockComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseLockComponent : ScriptComponent
{
	private SCR_VehicleSpawnProtectionComponent m_pVehicleSpawnProtection 
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_pVehicleSpawnProtection = SCR_VehicleSpawnProtectionComponent.Cast(owner.FindComponent(SCR_VehicleSpawnProtectionComponent));
		ClearEventMask(owner, EntityEvent.INIT);
	}
		
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCannotPerformReason(IEntity user)
	{
		if (user && m_pVehicleSpawnProtection)
			return m_pVehicleSpawnProtection.GetReasonText(user);
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLocked(IEntity user, BaseCompartmentSlot compartmentSlot)
	{
		if (!user || !compartmentSlot)
			return false;
				
		if (m_pVehicleSpawnProtection)	
			return m_pVehicleSpawnProtection.IsProtected(user, compartmentSlot);
		
		return false;
	}

};
