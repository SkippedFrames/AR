class SCR_GroupMenu : SCR_SuperMenuBase
{			
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();		
		
		Widget cancelButton = GetRootWidget().FindAnyWidget("Back");		
		
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(cancelButton.FindHandler(SCR_InputButtonComponent));		
		cancel.m_OnClicked.Insert(Close);
		cancel.m_OnActivated.Insert(Close);
		
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Insert(UpdateTabs);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateTabs);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		UpdateTabs();		
	}
	
	
	override void OnMenuClose()
	{
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Remove(UpdateTabs);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateTabs);
	}
		
	//------------------------------------------------------------------------------------------------
	void UpdateTabs()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		 
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		if (!group)
			return;		
		
		Widget tab = GetRootWidget().FindAnyWidget("TabViewRoot0");
		if (!tab)
			return;
		
		SCR_TabViewComponent tabView = SCR_TabViewComponent.Cast(tab.FindHandler(SCR_TabViewComponent));
		if (!tabView)
			return;
				
		if (!groupsManager.IsPlayerInAnyGroup(playerID) || !group.IsPlayerLeader(playerID))
		{
			tabView.EnableTab(1, false, true);	
			return;
		}
		
		tabView.EnableTab(1, true, true);
			
		array<int> requesters = {};
		group.GetRequesterIDs(requesters);
				
		if (requesters.IsEmpty())
		{	
			tabView.ShowIcon(1, false);
		}
		else
		{
			tabView.ShowIcon(1, true);	
					
			Widget notificationIcon = tabView.GetEntryIcon(1);
			if (!notificationIcon)
				return;
			
			TextWidget text = TextWidget.Cast(notificationIcon.FindAnyWidget("NotificationCount"));
			if (!text)
				return;
			
			if (requesters.Count() > 99)
				text.SetText("99+");
			else			
				text.SetText(requesters.Count().ToString());
		}		
		
	}
};
