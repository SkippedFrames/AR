/*!
Notification Rank
Displays a notification: %1 = Player rank
SCR_NotificationData: m_iParam1 = player rank
Can be used for: Player (m_iParam1) requests something
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationRank : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{	
		SCR_ECharacterRank neededRank;
		int playerID;
		data.GetParams(playerID, neededRank);

		data.SetNotificationTextEntries(GetPlayerRankName(playerID));		
		return super.GetText(data);
	}
		
	override void SetPosition(SCR_NotificationData data)
	{		
		if (!CanSetPosition(data))
			return;
		
		int playerID;
		data.GetParams(playerID);
		SetPositionDataEditablePlayer(playerID, data);
	}	
	
	string GetPlayerRankName(int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return string.Empty;
		
		IEntity ent = playerManager.GetPlayerControlledEntity(playerID);
		if (!ent)
			return string.Empty;	
			
		SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(ent.FindComponent(SCR_CharacterRankComponent));
		if (!rankComponent)
			return string.Empty;
		
		return rankComponent.GetCharacterRankName(ent);
	}
};
