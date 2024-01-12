//------------------------------------------------------------------------------------------------
//! Returns true if character is bleeding for ceratain time
[BaseContainerProps()]
class SCR_CharacterBleedingCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Minimum bleeding time to activate this hint", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetIsCharacterBleeding() && data.GetCharacterBleedingTime() > m_fValue;
		return GetReturnResult(result);
	}
};
