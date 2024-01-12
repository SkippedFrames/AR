[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableResupplyEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		return supportStationType == ESupportStationType.RESUPPLY_AMMO;
	}
};
