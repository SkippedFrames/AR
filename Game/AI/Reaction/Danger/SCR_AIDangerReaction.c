// Danger reaction base class

[BaseContainerProps()]
class SCR_AIDangerReaction : SCR_AIReactionBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EAIDangerEventType) )]
	EAIDangerEventType m_eType;

	//eventualy move to some setting in component
	protected static const float BULLET_IMPACT_DISTANCE_SQ_MAX = 10*10;
	
	bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent) {}
};
