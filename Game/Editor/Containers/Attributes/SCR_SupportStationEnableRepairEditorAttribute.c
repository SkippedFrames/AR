[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableRepairEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		return supportStationType == ESupportStationType.REPAIR || supportStationType == ESupportStationType.FIELD_REPAIR || supportStationType == ESupportStationType.EMERGENCY_REPAIR;
	}
};
