
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_BuilderNameTooltipDetail: SCR_EntityTooltipDetail
{	
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		TextWidget text = TextWidget.Cast(widget.FindAnyWidget("Text"));
		if (!text)
			return false;
		
		BaseGameMode gameMode = GetGame().GetGameMode();	
		if (!gameMode)
			return false;
				
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return false;
		
		string name = GetPlayerName(compositionComponent.GetBuilderId());
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(name))
			return false;
		
		text.SetText(name);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetPlayerName(int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return string.Empty;
		
		string playerName = playerManager.GetPlayerName(playerID);
		
		//Player name not found
		if (playerName.IsEmpty())
		{
			SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
			if (notificationsManager)
				playerName = notificationsManager.GetPlayerNameFromHistory(playerID);
		}
			
		return playerName;
	}
}
