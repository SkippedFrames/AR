/*!
Notification Player
Displays a notification: %1 = PlayerID name
Displays a notification: %2 = Killfeed type or Killfeed receive type
SCR_NotificationData: m_iParam1 = PlayerID
SCR_NotificationData: m_iParam2 = EKillFeedType or EKillFeedReceiveType
SCR_NotificationData: m_iParam3 = bool If Kill Feed Receive type then true else it is the Killfeed type
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationOnKillfeedChanged : SCR_NotificationPlayer
{	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, killfeedType, isreceive;
		string killfeedChangedTo = "UNKNOWN";
		data.GetParams(playerID, killfeedType, isreceive);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return super.GetText(data);
		
		SCR_NotificationSenderComponent notificationSender = SCR_NotificationSenderComponent.Cast(gameMode.FindComponent(SCR_NotificationSenderComponent));
		if (!notificationSender)
			return super.GetText(data);
		
		//~ Get the correct killfeed name it was changed to, to display in the notification.
		if (isreceive == false)
		{
			array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames = new array<ref SCR_NotificationKillfeedTypeName>;
			notificationSender.GetKillFeedTypeNames(killFeedTypeNames);
			
			foreach (SCR_NotificationKillfeedTypeName killfeed: killFeedTypeNames)
			{
				if (killfeed.GetKillfeedType() == killfeedType)
				{
					killfeedChangedTo = killfeed.GetName();
					break;
				}
			}
		}
		//~ Get the correct killfeed receive name it was changed to, to display in the notification.
		else 
		{
			array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiveTypeNames = new array<ref SCR_NotificationKillfeedreceiveTypeName>;
			notificationSender.GetKillFeedReceiveTypeNames(killFeedReceiveTypeNames);
			
			foreach (SCR_NotificationKillfeedreceiveTypeName killfeed: killFeedReceiveTypeNames)
			{
				if (killfeed.GetKillfeedReceiveType() == killfeedType)
				{
					killfeedChangedTo = killfeed.GetName();
					break;
				}
			}
		}
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, killfeedChangedTo);
		return super.GetText(data);
	}	
};



