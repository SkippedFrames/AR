//------------------------------------------------------------------------------------------------
[EntityEditorProps(description: "Achievements activator on client-side")]
class SCR_AchievementsHandlerClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_AchievementsHandler : ScriptComponent
{
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void UnlockOnClient(AchievementId achievement)
	{
		Print("Unlockg achievement with ID " + achievement, LogLevel.DEBUG);
		Achievements.UnlockAchievement(achievement);
	}
	
	void UnlockAchievement(AchievementId achievement)
	{
		Rpc(UnlockOnClient, achievement);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void IncrementOnClient(AchievementStatId achievementStat)
	{
		Print("Incrementing achievement stat with ID " + achievementStat, LogLevel.DEBUG);
		Achievements.IncrementAchievementProgress(achievementStat, 1);
	}
	
	void IncrementAchievementProgress(AchievementStatId achievementStat)
	{
		Rpc(IncrementOnClient, achievementStat);
	}
};
