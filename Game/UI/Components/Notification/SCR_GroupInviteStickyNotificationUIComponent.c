class SCR_GroupInviteStickyNotificationUIComponent: SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-Notification_Sticky_GroupInvite")]
	protected LocalizedString m_sGroupInviteText;
	
	SCR_GroupsManagerComponent m_GroupManager;
	SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	
	//Shows the sticky notification including player who invited you and the group callsign
	protected void ShowGroupInviteNotification(int groupId, bool isInit = false)
	{		
		SCR_AIGroup playerGroup = m_GroupManager.FindGroup(groupId);
		if (!playerGroup)
		{
			Print(string.Format("SCR_GroupInviteStickyNotificationUIComponent could not find group with ID %1!", groupId.ToString()), LogLevel.ERROR);
			return;
		}
		
		string company, platoon, squad, character, format;
		playerGroup.GetCallsigns(company, platoon, squad, character, format);
		
		string invitingPlayerName = m_PlayerControllerGroupComponent.GetGroupInviteFromPlayerName();
		
		m_Text.SetTextFormat(m_sGroupInviteText, company, platoon, squad, format, invitingPlayerName);
		
		SetStickyActive(true, !isInit);
	}
	
	protected void OnInviteReceived(int groupId, int fromPlayerId)
	{
		ShowGroupInviteNotification(groupId);
	}

	protected void OnInviteAcceptedOrCancelled(int groupId)
	{
		SetStickyActive(false);
	}
	
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{	
		super.OnInit(notificationLog);
				
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		
		if (!m_GroupManager)
		{
			//Print("SCR_GroupInviteStickyNotificationUIComponent could not find SCR_GroupsManagerComponent!", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerControllerGroupComponent)
		{
			Print("SCR_GroupInviteStickyNotificationUIComponent could not find SCR_PlayerControllerGroupComponent!", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent.GetOnInviteReceived().Insert(OnInviteReceived);
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Insert(OnInviteAcceptedOrCancelled);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Insert(OnInviteAcceptedOrCancelled);
		
		int groupInviteID = m_PlayerControllerGroupComponent.GetGroupInviteID();
		
		if (groupInviteID > -1)
			ShowGroupInviteNotification(groupInviteID, true);
		
		SetVisible(groupInviteID > -1);
	}
	
	protected override void OnButton()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		
		if (menuManager)
			menuManager.OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	protected override void OnDestroy()
	{
		if (!m_PlayerControllerGroupComponent)
			return;
		
		m_PlayerControllerGroupComponent.GetOnInviteReceived().Remove(OnInviteReceived);
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Remove(OnInviteAcceptedOrCancelled);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Remove(OnInviteAcceptedOrCancelled);
	}
};
