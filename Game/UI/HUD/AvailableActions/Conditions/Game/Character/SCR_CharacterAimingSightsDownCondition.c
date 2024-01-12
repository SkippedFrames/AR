//------------------------------------------------------------------------------------------------
//! Returns true if character is in ADS
[BaseContainerProps()]
class SCR_CharacterAimingSightsDownCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is ADS
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetIsCharacterADS();
		return GetReturnResult(result);
	}
};
