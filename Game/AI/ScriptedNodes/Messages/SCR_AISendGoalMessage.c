// Obsolete, don't use, instead use autogenerated SCR_AISendGoalMessage_... nodes
class SCR_AISendGoalMessage: SCR_AISendMessageGeneric
{
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	private EMessageType_Goal m_messageType;
	
	[Attribute("0", UIWidgets.CheckBox, "Priority level")]
	int m_fPriorityLevel;
	
	[Attribute("0", UIWidgets.CheckBox, "Goal is ordered by waypoint or is autonomous?")]
	bool m_bIsWaypointRelated;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		Print("SCR_AISendGoalMessage is obsolete!", LogLevel.WARNING);
		return ENodeResult.FAIL;
	}
	
	override string GetOnHoverDescription() 
	{ 
		return "Obsolete! Use autogenerated SCR_AISendGoalMessage_... nodes instead!";
	};
};
