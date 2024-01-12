[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableHealEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		return supportStationType == ESupportStationType.HEAL;
	}
};
