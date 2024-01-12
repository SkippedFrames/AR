//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SetFireVehicleContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(SCR_Enum.GetDefault(EFireState.BURNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EFireState))]
	private EFireState m_eTargetFireState;

	[Attribute(defvalue: "-1")]
	private float m_fTargetFireStateWeight;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity.GetEntityType() == EEditableEntityType.VEHICLE;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		if (damageManager.GetState() == EDamageState.DESTROYED)
			return false;

		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(damageManager.GetDefaultHitZone());
		if (!flammableHitZone)
			return false;

		if (m_eTargetFireState == EFireState.NONE)
			return flammableHitZone.GetFireState() != EFireState.NONE;
		else 
			return flammableHitZone.GetFireState() == EFireState.NONE;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;

		if (damageManager.GetState() == EDamageState.DESTROYED)
			return;

		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(damageManager.GetDefaultHitZone());
		if (!flammableHitZone)
			return;

		flammableHitZone.SetFireState(m_eTargetFireState, m_fTargetFireStateWeight);
		flammableHitZone.LockFireRate();
	}
};
