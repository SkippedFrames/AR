[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableFuelEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		return supportStationType == ESupportStationType.FUEL;
	}
};
