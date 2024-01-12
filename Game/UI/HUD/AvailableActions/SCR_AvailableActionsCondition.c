//------------------------------------------------------------------------------------------------
//! A single available action condition representation
[BaseContainerProps()]
class SCR_AvailableActionCondition
{	
	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bNegateCondition;
	
	//------------------------------------------------------------------------------------------------
	//! Based on the state of m_bNegateCondition returns our desired result
	//! returns desiredResult if !m_bNegateCondition
	//! returns !desiredResult if m_bNegateCondition
	protected bool GetReturnResult(bool desiredResult)
	{
		if (m_bNegateCondition)
			return !desiredResult;
		
		return desiredResult;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override and implement this method in any inherited conditions.
	//! Use GetReturnResult prior to returning the value to take m_bNegateCondition into account
	bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(true);
	}
};
