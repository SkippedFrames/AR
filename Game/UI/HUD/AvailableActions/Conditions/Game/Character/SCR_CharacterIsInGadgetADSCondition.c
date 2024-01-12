//------------------------------------------------------------------------------------------------
//! Returns true if character has gadget in ads
[BaseContainerProps()]
class SCR_CharacterIsInGadgetADSCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetGadgetRaised();

		return GetReturnResult(result);
	}
};
