class SCR_RequestToJoinSubmenu : SCR_SubMenuBase
{		
	protected const string REQUESTER_ENTRY_LAYOUT = "{B3381965FF7747CE}UI/layouts/Menus/GroupSlection/GroupRequestEntry.layout";
	protected ref ScriptInvoker m_OnJoinRequestRespond = new ScriptInvoker();	
	protected ref array<Widget> m_aEntryWidgets = {};
	protected VerticalLayoutWidget m_wContent;
				
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);	
		
		UpdateRequesters(parentMenu);
		m_OnJoinRequestRespond.Insert(UpdateRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);
		m_OnJoinRequestRespond.Remove(UpdateRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnJoinRequestRespond()
	{
		return m_OnJoinRequestRespond;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRequesters(SCR_SuperMenuBase parentMenu)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.GetPlayerGroup(GetGame().GetPlayerController().GetPlayerId());
		if (!group)
			return;
		
		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(parentMenu.GetRootWidget().FindAnyWidget("Content"));
		if (!content)
			return;				
		
		for (int i = 0, count = m_aEntryWidgets.Count(); i < count ;i++)
		{
			content.RemoveChild(m_aEntryWidgets[i]);
		}
		
		m_aEntryWidgets.Clear();
		
		array<int> requesterIDs = {};
		
		group.GetRequesterIDs(requesterIDs);	
					
		for (int i = 0, count = requesterIDs.Count(); i < count ;i++)
		{
			Widget entryWidget = GetGame().GetWorkspace().CreateWidgets(REQUESTER_ENTRY_LAYOUT, content);
			if (!entryWidget)
				continue;
			
			TextWidget playerName = TextWidget.Cast(entryWidget.FindAnyWidget("PlayerName"));
			if (!playerName)
				continue;
			
			ButtonWidget refuseWidget = ButtonWidget.Cast(entryWidget.FindAnyWidget("Refuse"));
			if (!refuseWidget)
				continue;
			
			ButtonWidget acceptWidget = ButtonWidget.Cast(entryWidget.FindAnyWidget("Accept"));
			if (!acceptWidget)
				continue;
			
			SCR_ButtonComponent refuseButton = SCR_ButtonComponent.Cast(refuseWidget.FindHandler(SCR_ButtonComponent));
			if (!refuseButton)
				continue;
			
			SCR_ButtonComponent acceptButton = SCR_ButtonComponent.Cast(acceptWidget.FindHandler(SCR_ButtonComponent));
			if (!acceptButton)
				continue;
			
			SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(entryWidget.FindHandler(SCR_JoinRequestEntry));
			if (!entry)
				continue;
			
			m_aEntryWidgets.Insert(entryWidget);
			
			entry.SetPlayerID(requesterIDs[i]);
			
			playerName.SetText(GetGame().GetPlayerManager().GetPlayerName(requesterIDs[i]));
			
			acceptButton.m_OnClicked.Insert(AcceptToJoinPrivateGroup);
			refuseButton.m_OnClicked.Insert(RefuseJoinPrivateGroup);			
		}		
		
		SCR_GroupMenu groupMenu = SCR_GroupMenu.Cast(m_ParentMenu);
		if (!groupMenu)
			return;
		
		groupMenu.UpdateTabs();
	}	
	
	//------------------------------------------------------------------------------------------------
	void AcceptToJoinPrivateGroup()
	{			
		Widget widgetEntry = GetRootWidget().FindAnyWidget("RequesterEntry");
		if (!widgetEntry)
			return;
		
		SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(widgetEntry.FindHandler(SCR_JoinRequestEntry));
		if (!entry)
			return;
	
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;
				
		playerGroupComponent.AcceptJoinPrivateGroup(entry.GetPlayerID(), true);				
		
		GetGame().GetCallqueue().CallLater(InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}	
	
	//------------------------------------------------------------------------------------------------
	void RefuseJoinPrivateGroup()
	{
		Widget widgetEntry = GetRootWidget().FindAnyWidget("RequesterEntry");
		if (!widgetEntry)
			return;
		
		SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(widgetEntry.FindHandler(SCR_JoinRequestEntry));
		if (!entry)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;
		
		playerGroupComponent.AcceptJoinPrivateGroup(entry.GetPlayerID(), false);
		
		GetGame().GetCallqueue().CallLater(InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}
	
	//------------------------------------------------------------------------------------------------
	void InvokeOnJoinRequestRespond()
	{
		GetOnJoinRequestRespond().Invoke(m_ParentMenu);
	}
}
