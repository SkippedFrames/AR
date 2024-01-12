//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WeaponIsRaisedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetIsCharacterWeaponRaised());
	}
};
