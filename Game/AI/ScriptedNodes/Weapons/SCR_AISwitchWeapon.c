class SCR_AISwitchWeapon : SCR_AIWeaponHandlingBase
{
	protected static const string PORT_WEAPON_COMPONENT = "WeaponComponent";
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgrComp || !m_ControlComp || !m_InventoryMgr)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent newWeaponComp = null;
		GetVariableIn(PORT_WEAPON_COMPONENT, newWeaponComp);
		if (!newWeaponComp)
			return ENodeResult.FAIL;
		
		// Resolve which weapon manager to use
		BaseCompartmentSlot compartmentSlot = m_CompartmentAccessComp.GetCompartment();
		BaseWeaponManagerComponent weaponMgr;
		if (compartmentSlot)
			weaponMgr = BaseWeaponManagerComponent.Cast(compartmentSlot.GetOwner().FindComponent(BaseWeaponManagerComponent));
		else
			weaponMgr = m_WeaponMgrComp;
		
		if (!weaponMgr)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent currentWeaponComp = weaponMgr.GetCurrentWeapon();
		
		if (compartmentSlot)
		{
			//-----------------------------------------
			// Turret weapon switching
			
			IEntity compartmentParentEntity = compartmentSlot.GetOwner();
			TurretControllerComponent turretController = TurretControllerComponent.Cast(compartmentParentEntity.FindComponent(TurretControllerComponent));
			
			if (!turretController)
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch failed: no turret controller on turret", LogLevel.WARNING);
				#endif
				return ENodeResult.FAIL;
			}
			else if (currentWeaponComp == newWeaponComp) // Return success if done
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch completed");
				#endif
				return ENodeResult.SUCCESS;
			}
			else
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("StartWeaponSwitchTurret: %1 %2 %3", newWeaponComp, newWeaponComp.GetOwner(), newWeaponComp.GetOwner().GetPrefabData().GetPrefabName()));
				#endif
				SCR_AIWeaponHandling.StartWeaponSwitchTurret(turretController, newWeaponComp, owner.GetControlledEntity());
			}
			
		}
		else
		{
			//-----------------------------------------
			// Character weapon switching
			
			if (m_ControlComp.IsChangingItem())
			{
				return ENodeResult.RUNNING;
			}
			else if (currentWeaponComp == newWeaponComp) // Return success if done
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch completed");
				#endif
				return ENodeResult.SUCCESS;
			}
			else if (!m_InventoryMgr.Contains(newWeaponComp.GetOwner()))
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch failed: weapon is not in inventory", LogLevel.WARNING);
				#endif
				
				return ENodeResult.FAIL;
			}
			else
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("StartWeaponSwitchCharacter: %1 %2 %3", newWeaponComp, newWeaponComp.GetOwner(), newWeaponComp.GetOwner().GetPrefabData().GetPrefabName()));
				#endif
				SCR_AIWeaponHandling.StartWeaponSwitchCharacter(m_ControlComp, newWeaponComp);
				return ENodeResult.RUNNING;
			}
		}
		
		return ENodeResult.FAIL;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_WEAPON_COMPONENT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override bool VisibleInPalette() { return true; }
}
