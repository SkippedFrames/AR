class SCR_GameOverScreenCombatControlPatrolUIComponent: SCR_GameOverScreenContentUIComponent
{
	/*[Attribute("GameOver_Score")]
	protected string m_sScore;*/
	
	[Attribute("GameOver_Time")]
	protected string m_sTime;
	
	[Attribute("GameOver_Tasks")]
	protected string m_sTasks;
	
	/*[Attribute("GameOver_Kills")]
	protected string m_sKills;
	
	[Attribute("GameOver_Deaths")]
	protected string m_sDeaths;*/
	
	protected ref array<SCR_BaseTask> 	m_aFinishedTasks = {};
	protected string 					timeElapsed;
	//protected int killsPlaceholder = 42;
	//protected int deathsPlaceholder = 12;
	
	override void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.InitContent(endScreenUIContent);
		
		RichTextWidget titleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTileName));
		RichTextWidget subtitleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sSubtitleName));
		//RichTextWidget scoreWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sScore));
		RichTextWidget timeWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTime));
		RichTextWidget tasksWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTasks));
		//RichTextWidget killsWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sKills));
		//RichTextWidget deathsWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sDeaths));
		
		if (titleWidget)
			titleWidget.SetTextFormat(endScreenUIContent.m_sTitle, endScreenUIContent.m_sTitleParam);

		if (subtitleWidget)
			subtitleWidget.SetTextFormat(endScreenUIContent.m_sSubtitle, endScreenUIContent.m_sSubtitleParam);
		
		//int scorePlaceholder = 5000;
		
		//if (scoreWidget)
		//	scoreWidget.SetTextFormat("Score: %1", scorePlaceholder);
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(baseGameMode.GetElapsedTime(), days, hours, minutes, seconds);
		
		timeElapsed = string.Format(WidgetManager.Translate("#AR-CareerProfile_TimePlayed_TotalValue", ""+days, ""+hours, ""+minutes, ""+seconds));
		
		if (timeWidget)
			timeWidget.SetTextFormat("<color rgba=\"226,168,80,255\">Total operation time:</color> <br/> %1", timeElapsed);
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		taskManager.GetFinishedTasks(m_aFinishedTasks);
		
		string tasksToShow = "<br/>";
		foreach (SCR_BaseTask task : m_aFinishedTasks)
		{
			if (tasksToShow == "<br/>")
				tasksToShow = tasksToShow + string.Format(WidgetManager.Translate(task.GetTitle()));
			else 
				tasksToShow = tasksToShow + "<br/>" + string.Format(WidgetManager.Translate(task.GetTitle()));
		}
		
		if (tasksWidget)
		{
			if (tasksToShow != "<br/>")
				tasksWidget.SetText(string.Format(WidgetManager.Translate("<color rgba=\"226,168,80,255\">Completed tasks:</color> %1", tasksToShow)));
			else
				tasksWidget.SetText(string.Format(WidgetManager.Translate("")));
		}
	/*	
		SCR_BaseScoringSystemComponent scoringSystem = SCR_BaseScoringSystemComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_BaseScoringSystemComponent));
		if (!scoringSystem)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		array<Faction> factions = new array<Faction>();
		factionManager.GetFactionsList(factions);
		Faction targetFaction;
		foreach (Faction faction : factions)
		{
			if (faction.GetFactionName() == "#AR-Faction_US")
				targetFaction = faction;
		}
		
		
		
		SCR_ScoreInfo factionScore = scoringSystem.GetFactionScoreInfo(targetFaction);
		Print(factionScore.m_iKills);
		Print(factionScore.m_iDeaths);

		if (killsWidget)
			killsWidget.SetTextFormat("Kills: %1", factionScore.m_iKills);
		
		if (deathsWidget)
			deathsWidget.SetTextFormat("Deaths: %1", factionScore.m_iDeaths);
		*/
	}
};
