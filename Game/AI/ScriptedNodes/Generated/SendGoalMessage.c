//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Cancel
class SCR_AISendGoalMessage_Cancel : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Cancel msg = new SCR_AIMessage_Cancel();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Attack
class SCR_AISendGoalMessage_Attack : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Attack msg = new SCR_AIMessage_Attack();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackCluster
class SCR_AISendGoalMessage_AttackCluster : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetClusterState",
		"AllowInvestigate"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackCluster msg = new SCR_AIMessage_AttackCluster();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetClusterState", msg.m_TargetClusterState);
		
		GetVariableIn("AllowInvestigate", msg.m_bAllowInvestigate);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackClusterDone
class SCR_AISendGoalMessage_AttackClusterDone : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackClusterDone msg = new SCR_AIMessage_AttackClusterDone();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackStatic
class SCR_AISendGoalMessage_AttackStatic : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackStatic msg = new SCR_AIMessage_AttackStatic();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackStaticDone
class SCR_AISendGoalMessage_AttackStaticDone : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackStaticDone msg = new SCR_AIMessage_AttackStaticDone();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_KeepFormation
class SCR_AISendGoalMessage_KeepFormation : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_KeepFormation msg = new SCR_AIMessage_KeepFormation();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Move
class SCR_AISendGoalMessage_Move : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	EMovementType m_eMovementType;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Move msg = new SCR_AIMessage_Move();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		msg.m_eMovementType = m_eMovementType;
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eMovementType: %1\n", typename.EnumToString(EMovementType, m_eMovementType));
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Follow
class SCR_AISendGoalMessage_Follow : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	EMovementType m_eMovementType;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles",
		"Distance"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Follow msg = new SCR_AIMessage_Follow();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		msg.m_eMovementType = m_eMovementType;
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		GetVariableIn("Distance", msg.m_fDistance);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eMovementType: %1\n", typename.EnumToString(EMovementType, m_eMovementType));
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Investigate
class SCR_AISendGoalMessage_Investigate : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	float m_fRadius;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsDangerous;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIUnitType))]
	EAIUnitType m_eTargetUnitType;
	
	[Attribute("", UIWidgets.Auto)]
	float m_fDuration;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"ObjectEntity",
		"MovePosition",
		"Radius",
		"IsDangerous",
		"Duration"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Investigate msg = new SCR_AIMessage_Investigate();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("ObjectEntity", msg.m_ObjectEntity);
		
		GetVariableIn("MovePosition", msg.m_vMovePosition);
		
		if(!GetVariableIn("Radius", msg.m_fRadius))
			msg.m_fRadius = m_fRadius;
		
		if(!GetVariableIn("IsDangerous", msg.m_bIsDangerous))
			msg.m_bIsDangerous = m_bIsDangerous;
		
		msg.m_eTargetUnitType = m_eTargetUnitType;
		
		if(!GetVariableIn("Duration", msg.m_fDuration))
			msg.m_fDuration = m_fDuration;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_fRadius: %1\n", m_fRadius);
		s = s + string.Format("m_bIsDangerous: %1\n", m_bIsDangerous);
		s = s + string.Format("m_eTargetUnitType: %1\n", typename.EnumToString(EAIUnitType, m_eTargetUnitType));
		s = s + string.Format("m_fDuration: %1\n", m_fDuration);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_SeekAndDestroy
class SCR_AISendGoalMessage_SeekAndDestroy : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	EMovementType m_eMovementType;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_SeekAndDestroy msg = new SCR_AIMessage_SeekAndDestroy();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		msg.m_eMovementType = m_eMovementType;
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eMovementType: %1\n", typename.EnumToString(EMovementType, m_eMovementType));
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Heal
class SCR_AISendGoalMessage_Heal : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"EntityToHeal"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Heal msg = new SCR_AIMessage_Heal();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("EntityToHeal", msg.m_EntityToHeal);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_ProvideAmmo
class SCR_AISendGoalMessage_ProvideAmmo : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"AmmoConsumerEntity",
		"MagazineWellType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_ProvideAmmo msg = new SCR_AIMessage_ProvideAmmo();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("AmmoConsumerEntity", msg.m_AmmoConsumer);
		
		GetVariableIn("MagazineWellType", msg.m_MagazineWell);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_PickupInventoryItems
class SCR_AISendGoalMessage_PickupInventoryItems : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"PickupPosition",
		"MagazineWellType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_PickupInventoryItems msg = new SCR_AIMessage_PickupInventoryItems();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("PickupPosition", msg.m_vPickupPosition);
		
		GetVariableIn("MagazineWellType", msg.m_MagazineWellType);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Defend
class SCR_AISendGoalMessage_Defend : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	float m_fDefendAngularRange;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"DefendLocation",
		"DefendAngularRange"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Defend msg = new SCR_AIMessage_Defend();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("DefendLocation", msg.m_vDefendLocation);
		
		if(!GetVariableIn("DefendAngularRange", msg.m_fDefendAngularRange))
			msg.m_fDefendAngularRange = m_fDefendAngularRange;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_fDefendAngularRange: %1\n", m_fDefendAngularRange);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Retreat
class SCR_AISendGoalMessage_Retreat : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Retreat msg = new SCR_AIMessage_Retreat();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_HealWait
class SCR_AISendGoalMessage_HealWait : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"HealProviderEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_HealWait msg = new SCR_AIMessage_HealWait();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("HealProviderEntity", msg.m_HealProvider);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_PerformAction
class SCR_AISendGoalMessage_PerformAction : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	string m_sSmartActionTag;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"SmartAction",
		"SmartActionEntity",
		"SmartActionTag"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_PerformAction msg = new SCR_AIMessage_PerformAction();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("SmartAction", msg.m_SmartActionComponent);
		
		GetVariableIn("SmartActionEntity", msg.m_SmartActionEntity);
		
		if(!GetVariableIn("SmartActionTag", msg.m_SmartActionTag))
			msg.m_SmartActionTag = m_sSmartActionTag;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_sSmartActionTag: %1\n", m_sSmartActionTag);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GetIn
class SCR_AISendGoalMessage_GetIn : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAICompartmentType))]
	EAICompartmentType m_eRoleInVehicle;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"VehicleEntity",
		"RoleInVehicle"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GetIn msg = new SCR_AIMessage_GetIn();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("VehicleEntity", msg.m_Vehicle);
		
		if(!GetVariableIn("RoleInVehicle", msg.m_eRoleInVehicle))
			msg.m_eRoleInVehicle = m_eRoleInVehicle;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eRoleInVehicle: %1\n", typename.EnumToString(EAICompartmentType, m_eRoleInVehicle));
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GetOut
class SCR_AISendGoalMessage_GetOut : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"VehicleEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GetOut msg = new SCR_AIMessage_GetOut();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("VehicleEntity", msg.m_Vehicle);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Flee
class SCR_AISendGoalMessage_Flee : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Flee msg = new SCR_AIMessage_Flee();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_ThrowGrenadeTo
class SCR_AISendGoalMessage_ThrowGrenadeTo : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetEntity",
		"TargetPosition"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_ThrowGrenadeTo msg = new SCR_AIMessage_ThrowGrenadeTo();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetEntity", msg.m_TargetEntity);
		
		GetVariableIn("TargetPosition", msg.m_vTargetPosition);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

