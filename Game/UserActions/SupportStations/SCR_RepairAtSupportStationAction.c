class SCR_RepairAtSupportStationAction : SCR_BaseDamageHealSupportStationAction
{	
	[Attribute("#AR-SupportStation_Repair_ActionInvalid_NoDamage", desc: "Text shown on action if undamaged", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidDamaged;
	
	[Attribute("#AR-SupportStation_Repair_ActionInvalid_Needs_SupportStation_Field", desc: "Vehicle cannot be repaired any more as max healing is reached", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMaxHealingReached_Field;
	
	[Attribute("#AR-SupportStation_Repair_ActionInvalid_Needs_SupportStation_Emergency", desc: "Vehicle cannot be repaired any more as max healing is reached", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMaxHealingReached_Emergency;
	
	[Attribute("#AR-Editor_ContextAction_Extinguish_Name", desc: "Action name when entity is on fire. Only when hitzones are FlammableHitZones", uiwidget: UIWidgets.LocaleEditBox, category: "Heal/Repair Support Station")]
	protected LocalizedString m_sOnFireOverrideActionName;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Which hitzone groups will be healed, Leave empty if all need to be healed. (VIRTUAL is invalid and will be skipped). The first entry will be used for notifications", enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected ref array<EVehicleHitZoneGroup> m_aHitZoneGroups;
	
	[Attribute(desc: "Used to get the group name of which is going to be repaired")]
	protected ref SCR_HitZoneGroupNameHolder m_HitZoneGroupNames;
	
	[Attribute(desc: "Any addition Hitzone ID. This is generic and used for example to get the correct fuel tank using it as an ID")]
	protected ref array<int> m_aHitZoneAdditionalIDs;
	
	[Attribute("0", desc: "For some repair actions you want the system to know what it will be repairing, eg: Wheels, but the wheels have no hitzones with wheel hitzone group so just get all the hitzones on the entity. But the action and notifications still know they system is repairing the wheels")]
	protected bool m_bAlwaysRepairAllHitZones;
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.REPAIR;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ Entity is undamaged
		if (reasonInvalid == ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED)
			return m_sInvalidDamaged;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD)
			return m_sInvalidMaxHealingReached_Field;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY)
			return m_sInvalidMaxHealingReached_Emergency;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetHitZoneGroups(out notnull array<EVehicleHitZoneGroup> hitZoneGroup)
	{
		hitZoneGroup.Copy(m_aHitZoneGroups);
		return hitZoneGroup.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool RequiresGadget()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{		
		if (!m_DamageManagerComponent || m_aHitZonesToHeal.IsEmpty())
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool CanShowDestroyed()
	{
		//~ If HULL is not included then it can always be repaired
		if (!m_aHitZoneGroups.Contains(EVehicleHitZoneGroup.HULL))
			return true;
		
		//~ Don't show if hull is destrpued
		return super.CanShowDestroyed();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{		
		if (!m_DamageManagerComponent.CanBeHealed())
		{
			SetCanPerform(false, ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED);
			
			//~ Resets the support station so override action name is removed
			m_SupportStationComponent = null;
			return false;
		}
			
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SetHitZonesToHeal()
	{
		//~ Virtual gets all hitzones. Or if wheels do the same as wheels have no additional hitzones
		if (m_aHitZoneGroups.IsEmpty() || m_bAlwaysRepairAllHitZones)
		{
			super.SetHitZonesToHeal();
			return;
		}
		
		m_aHitZonesToHeal.Clear();
		
		//~ Get the correct fuel tank to repair
		if (!m_aHitZoneAdditionalIDs.IsEmpty() && m_aHitZoneGroups.Contains(EVehicleHitZoneGroup.FUEL_TANKS))
		{
			SCR_FuelHitZone fuelHitZone;
			ScriptedHitZone scrHitZone;
			
			array<HitZone> allHitZones = {};
			
			m_DamageManagerComponent.GetAllHitZones(allHitZones);
			
			foreach (HitZone hitZone : allHitZones)
			{
				scrHitZone =  ScriptedHitZone.Cast(hitZone);
				
				if (scrHitZone.GetHitZoneGroup() != EVehicleHitZoneGroup.FUEL_TANKS)
					continue;
				
				//~ Not a fuel hitzone but part of the group
				fuelHitZone = SCR_FuelHitZone.Cast(hitZone);
				if (!fuelHitZone)
					m_aHitZonesToHeal.Insert(hitZone);
				
				if (!m_aHitZoneAdditionalIDs.Contains(fuelHitZone.GetFuelTankID()))
					continue;
				
				//~ Insert Fuel tank
				m_aHitZonesToHeal.Insert(hitZone);
			}
		}
		
		//~ Get hitzone groups (Fuel tanks are already obtained)
		foreach (EVehicleHitZoneGroup hitZoneGroup : m_aHitZoneGroups)
		{
			if (hitZoneGroup == EVehicleHitZoneGroup.VIRTUAL || (!m_aHitZoneAdditionalIDs.IsEmpty() && hitZoneGroup == EVehicleHitZoneGroup.FUEL_TANKS))
				continue;
			
			m_DamageManagerComponent.GetHitZonesOfGroup(hitZoneGroup, m_aHitZonesToHeal, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if any of the hitzone to heal is on fire
	protected bool IsHitZoneOnFire()
	{
		SCR_FlammableHitZone flammableHitZone;
		array<HitZone> hitZones = {};
		GetHitZonesToHeal(hitZones);
		
		foreach (HitZone hitZone : hitZones)
		{
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (!flammableHitZone)
				continue;
			
			if (flammableHitZone.GetFireState() == EFireState.BURNING)// || flammableHitZone.GetFireState() == EFireState.SMOKING_IGNITING)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{
		//~ Don't show damage state when on fire
		if (IsHitZoneOnFire())
			return string.Empty;
		
		return super.GetActionStringParam();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_HitZoneGroupNames)
			return super.GetActionNameScript(outName);
		
		//~ Set extinguish fire action name 
		if (IsHitZoneOnFire())
			outName = m_sOnFireOverrideActionName;
		//~ Override action name
		else if (m_SupportStationComponent)
			outName = m_SupportStationComponent.GetOverrideUserActionName();
		
		//~ Default action name
		if (outName.IsEmpty())
		{
			UIInfo uiInfo = GetUIInfo();
			if (!uiInfo)
				return super.GetActionNameScript(outName);
			
			outName = uiInfo.GetName();
		}
		
		outName = WidgetManager.Translate(outName, m_HitZoneGroupNames.GetHitZoneGroupName(m_aHitZoneGroups[0]));
		return super.GetActionNameScript(outName);
	}
};
