class SCR_DefendWaypointClass: SCR_TimedWaypointClass
{
};

[BaseContainerProps()]
class SCR_DefendWaypointPreset
{
	[Attribute("", UIWidgets.EditBox, "Preset name, only informative. Switch using index.")];
	private string m_sName;
	
	[Attribute("true", UIWidgets.CheckBox, "Use turrets?")];
	bool m_bUseTurrets;
	
	[Attribute("", UIWidgets.Auto, "List tags to search in the preset")];
	ref array<string> m_aTagsForSearch;	
};

class SCR_DefendWaypoint : SCR_TimedWaypoint
{
	[Attribute("0", UIWidgets.Object, "Fast init - units will be spawned on their defensive locations")];
	private bool m_bFastInit;
	
	[Attribute("", UIWidgets.Object, "Defend presets")];
	private ref array<ref SCR_DefendWaypointPreset> m_DefendPresets;
	
	private int m_iCurrentDefendPreset;
	 
	//----------------------------------------------------------------------------------------
	array<string> GetTagsForSearch()
	{
		return m_DefendPresets[m_iCurrentDefendPreset].m_aTagsForSearch;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetUseTurrets()
	{
		return m_DefendPresets[m_iCurrentDefendPreset].m_bUseTurrets;
	}
	
	//----------------------------------------------------------------------------------------
	int GetCurrentDefendPreset()
	{
		return m_iCurrentDefendPreset;
	}
	
	//----------------------------------------------------------------------------------------
	bool SetCurrentDefendPreset(int newDefendPresetIndex)
	{
		if ((newDefendPresetIndex >= 0) && (newDefendPresetIndex < m_DefendPresets.Count()))
		{ 
			m_iCurrentDefendPreset = newDefendPresetIndex;
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetFastInit()
	{
		return m_bFastInit;
	}
	
	//----------------------------------------------------------------------------------------
	void SetFastInit(bool isFastInit)
	{
		m_bFastInit = isFastInit;
	}
	
	//----------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIDefendWaypointState(groupUtilityComp, this);
	}
};

class SCR_AIDefendWaypointState : SCR_AIWaypointState
{
	override void OnDeselected()
	{
		super.OnDeselected();
		
		SCR_AIGroup myGroup = m_Utility.m_Owner;
		
		if (!myGroup)
			return;
		
		array<AIAgent> groupMembers = {};
		myGroup.GetAgents(groupMembers);
				
		AICommunicationComponent mailbox = myGroup.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		foreach (AIAgent receiver: groupMembers)
		{
			if (receiver)
			{
				SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(receiver);
				if (!chimeraAgent)
					continue;
				
				SCR_AIInfoComponent aiInfo = chimeraAgent.m_InfoComponent;
				if (!aiInfo || !aiInfo.HasUnitState(EUnitState.IN_TURRET))
					continue;
				
				SCR_AIMessage_AttackStaticDone msg1 = new SCR_AIMessage_AttackStaticDone();
				
				msg1.SetText("Waypoint was deselected");
				msg1.SetReceiver(receiver);
				mailbox.RequestBroadcast(msg1, receiver);
			
				ChimeraCharacter character = ChimeraCharacter.Cast(receiver.GetControlledEntity());
				if (character && character.IsInVehicle())
				{
					SCR_AIMessage_GetOut msg2 = new SCR_AIMessage_GetOut();
					
					msg2.SetText("Waypoint was deselected, leave vehicle");
					msg2.SetReceiver(receiver);
					mailbox.RequestBroadcast(msg2, receiver);
				}
			}
		}
		myGroup.ReleaseCompartments();
	}
}
