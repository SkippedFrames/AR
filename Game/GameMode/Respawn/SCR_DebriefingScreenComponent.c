//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Debriefing screen shown after game ends")]
class SCR_DebriefingScreenComponentClass : SCR_DeployMenuBaseScreenComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Debriefing screen component intended to be added to the GameMode.
class SCR_DebriefingScreenComponent : SCR_DeployMenuBaseScreenComponent
{
};

//------------------------------------------------------------------------------------------------
//! Class handling layout for UnevenFourTiles.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenUnevenFourTiles : SCR_DeployMenuBaseScreenLayout
{
	[Attribute(defvalue: "{F7138F20BD16D89A}UI/layouts/Menus/DeployMenu/DebriefingScreenFourUnevenTiles.layout")]
	protected ResourceName m_sFourUnevenTilesLayout;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_TopLeftTile;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_TopRightTile;

	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_BottomLeftTile;
	
	[Attribute()]
	protected ref SCR_WelcomeScreenBaseContent m_BottomRightTile;
	
	protected const string WELCOME_CONTENT = "WelcomeContent";
	
	protected const string TOP_LEFT_TILE = "TopLeftTile";
	protected const string TOP_RIGHT_TILE = "TopRightTile";
	protected const string BOTTOM_LEFT_TILE = "BottomLeftTile";
	protected const string BOTTOM_RIGHT_TILE = "BottomRightTile";
	
	protected const string TOP_LEFT_TILE_BUTTON = "TopLeftTileButton";
	protected const string TOP_RIGHT_TILE_BUTTON = "TopRightTileButton";
	protected const string BOTTOM_LEFT_TILE_BUTTON = "BottomLeftTileButton";
	protected const string BOTTOM_RIGHT_TILE_BUTTON = "BottomRightTileButton";

	//------------------------------------------------------------------------------------------------
	//! Initializes content for given menu.
	override void InitContent(SCR_WelcomeScreenMenu menu)
	{
		Widget targetColumn = menu.GetRootWidget().FindAnyWidget(WELCOME_CONTENT);
		if (!targetColumn)
			return;

		Widget fourUnevenTiles = GetGame().GetWorkspace().CreateWidgets(m_sFourUnevenTilesLayout, targetColumn);

		if (m_TopLeftTile)
		{
			m_aScreenBaseContents.Insert(m_TopLeftTile);
			m_TopLeftTile.InitContent(menu, TOP_LEFT_TILE, TOP_LEFT_TILE_BUTTON);
		}

		if (m_TopRightTile)
		{
			m_aScreenBaseContents.Insert(m_TopRightTile);
			m_TopRightTile.InitContent(menu, TOP_RIGHT_TILE, TOP_RIGHT_TILE_BUTTON);
		}

		if (m_BottomLeftTile)
		{
			m_aScreenBaseContents.Insert(m_BottomLeftTile);
			m_BottomLeftTile.InitContent(menu, BOTTOM_LEFT_TILE, BOTTOM_LEFT_TILE_BUTTON);
		}
		
		if (m_BottomRightTile)
		{
			m_aScreenBaseContents.Insert(m_BottomRightTile);
			m_BottomRightTile.InitContent(menu, BOTTOM_RIGHT_TILE, BOTTOM_RIGHT_TILE_BUTTON);
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Class handling big image content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenBigImageContent : SCR_WelcomeScreenBaseContent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Big image to be shown", params: "edds")]
	protected ResourceName m_sContentImage;

	[Attribute(defvalue: "{6CD5DC3DE2C54BBE}UI/layouts/Menus/DeployMenu/DebriefingScreenBigImageTile.layout")]
	protected ResourceName m_sBigImageLayout;

	//------------------------------------------------------------------------------------------------
	//! Initializes content for given column
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		Widget bigImageWidget = GetGame().GetWorkspace().CreateWidgets(m_sBigImageLayout, targetTile);
		if (!bigImageWidget)
			return;

		ImageWidget imageWidget = ImageWidget.Cast(bigImageWidget.FindAnyWidget("ImageWidget"));
		if (imageWidget && m_sContentImage)
		imageWidget.LoadImageTexture(0, m_sContentImage, false, false);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get image
	\return image
	*/
	ResourceName GetImage()
	{
		return m_sContentImage;
	}
};

//------------------------------------------------------------------------------------------------
//! Class handling summary content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenSummaryContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute()]
	protected string m_sSubtitleText;
	
	[Attribute()]
	protected string m_sSubtitleLostText;
	
	[Attribute()]
	protected string m_sDescriptionText;

	[Attribute(defvalue: "{10A7983E48474CB2}UI/layouts/Menus/DeployMenu/DebriefingScreenSummaryTile.layout")]
	protected ResourceName m_sSummaryLayout;
	
	protected Widget m_wSummaryWidget;

	//------------------------------------------------------------------------------------------------
	//! Initializes content for given column
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		m_wSummaryWidget = GetGame().GetWorkspace().CreateWidgets(m_sSummaryLayout, targetTile);
		if (!m_wSummaryWidget)
			return;

		FillSummaryWidget();
		
		//Invokers for in-game possibility to update these strings
		//SOMETHING.SOMEINVOKER().Insert(UpdateSummarySubtitle);
		//SOMETHING.SOMEINVOKER().Insert(UpdateSummaryDescription);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills content widget with summary
	protected void FillSummaryWidget()
	{
		string characterRank = "";
		string subtitle = GetSubtitleLostText();
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (player)
		{
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(player.FindComponent(SCR_CharacterRankComponent));
			if (rankComponent)
				characterRank = rankComponent.GetCharacterRankName(player);
		}
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
			
		Faction faction = Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!faction)
			return;
		
		SCR_GameModeEndData endData = gamemode.GetEndGameData();
		if (endData)
		{
			array<int> winnerIds = {};
			array<int> winnerFactionIds = {};
			endData.GetWinnerIds(winnerIds);
			endData.GetFactionWinnerIds(winnerFactionIds);
			if (!winnerFactionIds.IsEmpty())
			{
				if (winnerFactionIds.Contains(factionManager.GetFactionIndex(faction)))
					subtitle = GetSubtitleText();
			}
			else if (!winnerIds.IsEmpty())
			{
				int playerID = GetGame().GetPlayerController().GetPlayerId();
				if (winnerIds.Contains(playerID))
					subtitle = GetSubtitleText();
			}
		}
		
		RichTextWidget subtitleText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("SubtitleText"));
		if (subtitleText)
			subtitleText.SetTextFormat(subtitle, string.Format(WidgetManager.Translate("%1 <color rgba=\"226,168,80,255\">%2</color>", characterRank, GetGame().GetPlayerManager().GetPlayerName(GetGame().GetPlayerController().GetPlayerId()))), "");

		RichTextWidget descriptionText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("DescriptionText"));
		if (!descriptionText)
			return;
		
		descriptionText.SetText(GetDescriptionText());
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(GetDescriptionText())) 
		{
			SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
			if (!gameOverScreenMgr)
				return;
			
			SCR_GameOverScreenConfig gameOverConfig = gameOverScreenMgr.GetGameOverConfig();
			if (!gameOverConfig)
				return;
			
			SCR_BaseGameOverScreenInfo gameOverScreenInfo;
			gameOverConfig.GetGameOverScreenInfo(gameOverScreenMgr.GetCurrentGameOverType(), gameOverScreenInfo);
			if (!gameOverScreenInfo)
				return;
			
			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(gameOverScreenInfo.GetDebriefing(faction, null))) 
				descriptionText.SetText(gameOverScreenInfo.GetDebriefing(faction, null));
			else
				descriptionText.SetText(gameOverScreenInfo.GetSubtitle(faction, null));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates summary subtitle - intended for runtime updates
	void UpdateSummarySubtitle(string subtitle)
	{
		m_sSubtitleText = subtitle;
		RichTextWidget subtitleText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("SubtitleText"));
		if (subtitleText)
			subtitleText.SetText(subtitle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates summary description - intended for runtime updates
	void UpdateSummaryDescription(string description)
	{
		m_sDescriptionText = description;
		RichTextWidget descriptionText = RichTextWidget.Cast(m_wSummaryWidget.FindAnyWidget("DescriptionText"));
		if (descriptionText)
			descriptionText.SetText(description);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get title
	\return title
	*/
	string GetTitleText()
	{
		return m_sTitleText;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get subtitle
	\return subtitle
	*/
	string GetSubtitleText()
	{
		return m_sSubtitleText;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get subtitle lost
	\return subtitle lost
	*/
	string GetSubtitleLostText()
	{
		return m_sSubtitleLostText;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get description
	\return description
	*/
	string GetDescriptionText()
	{
		return m_sDescriptionText;
	}
};

//------------------------------------------------------------------------------------------------
//! Class handling finished mission objectives content.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenFinishedObjectivesContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute(defvalue: "{0B7DDF77E24F0C63}UI/layouts/Menus/DeployMenu/DebriefingScreenFinishedObjectivesTile.layout")]
	protected ResourceName m_sFinishedObjectivesTileLayout;
	
	[Attribute(defvalue: "{6B9E63C758849B7F}UI/layouts/Menus/DeployMenu/DebriefingScreenMissionObjective.layout")]
	protected ResourceName m_sFinishedObjectivesLayout;
	
	protected Widget m_wFinishedObjectivesWidget;
	protected ButtonWidget m_wColumnButton;
	protected ref array<SCR_BaseTask> m_aFinishedTasks = {};
	protected ref array<Widget> m_aFinishedObjectivesWidgets = {};
	protected int m_iCurrentPage;
	protected int m_iFinishedTasksCount;
	
	//------------------------------------------------------------------------------------------------
	//! Initializes content for given column
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
				return;
		
		m_wFinishedObjectivesWidget = GetGame().GetWorkspace().CreateWidgets(m_sFinishedObjectivesTileLayout, targetTile);
		if (!m_wFinishedObjectivesWidget)
				return;
		
		Widget finishedObjectivesContent = m_wFinishedObjectivesWidget.FindAnyWidget("FinishedObjectivesLayout");
		if (!finishedObjectivesContent)
				return;

		m_wColumnButton = ButtonWidget.Cast(menu.GetRootWidget().FindAnyWidget(columnButton));
		if (!m_wColumnButton)
			return;
		
		FillFinishedObjectivesWidget(finishedObjectivesContent);
		InitPagination();
		HandlePagination();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggles interactions for this content
	override void ToggleInteractions(bool enabled)
	{
		HandlePagination(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processes tasks according to the player faction and temporarily sorts out issues with Conflict.
	//! TODO - Will be fixed in the future
	void ProcessTasks()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction faction = Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!faction)
			return;
		
		array<SCR_BaseTask> activeTasks = {};
		taskManager.GetFilteredTasks(activeTasks, faction);
		taskManager.GetFilteredFinishedTasks(m_aFinishedTasks, faction);
		
		m_iFinishedTasksCount = m_aFinishedTasks.Count();
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
		
		//TODO: Due to how Task system works, we need to process this task list
		//Just for the Conflict so only the tasks for control points that are also under faction control are displayed
		SCR_CampaignMilitaryBaseComponent baseComponent;
		SCR_CampaignTask campaignTask;
		map<SCR_CampaignMilitaryBaseComponent, SCR_BaseTask> finishedTasksMap = new map <SCR_CampaignMilitaryBaseComponent, SCR_BaseTask>();
		
		//We fetch all the finished tasks and fill the map with them using SCR_CampaignMilitaryBaseComponent as a key
		foreach (SCR_BaseTask task : m_aFinishedTasks)
		{
			campaignTask = SCR_CampaignTask.Cast(task);
			if (!campaignTask)
				continue;
			
			baseComponent = campaignTask.GetTargetBase();
			//We let through only control points that have signal to the HQ
			if (baseComponent && !finishedTasksMap.Contains(baseComponent) && baseComponent.IsControlPoint() && baseComponent.IsHQRadioTrafficPossible(faction, SCR_ECampaignHQRadioComms.RECEIVE))
				finishedTasksMap.Set(baseComponent, task);
		}
		
		//We fetch all the active tasks and remove all the finished tasks that have the same SCR_CampaignMilitaryBaseComponent
		//Because that means that this task is in fact not finished as the base was lost and task recreated
		foreach(SCR_BaseTask activeTask : activeTasks)
		{
			campaignTask = SCR_CampaignTask.Cast(activeTask);
			if (!campaignTask)
				continue;
		
			baseComponent = campaignTask.GetTargetBase();
			if (baseComponent)
				finishedTasksMap.Remove(baseComponent);
		}
		
		m_aFinishedTasks.Clear();
		foreach (SCR_BaseTask task : finishedTasksMap)
		{
			m_aFinishedTasks.Insert(task);
		}

		m_iFinishedTasksCount = m_aFinishedTasks.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills content widget with finished objectives
	void FillFinishedObjectivesWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wFinishedObjectivesWidget.FindAnyWidget("TitleText"));
		if (titleText)
			titleText.SetText(GetTitleText());
		
		ProcessTasks();
		
		int cycleCount;
		if (m_iFinishedTasksCount < 7)
			cycleCount = m_iFinishedTasksCount;
		else
			cycleCount = 6;

		//Statistics are capped for 6 at the time
		for (int i = 0; i < cycleCount; ++i)
		{
			Widget objective = GetGame().GetWorkspace().CreateWidgets(m_sFinishedObjectivesLayout, content);
			m_aFinishedObjectivesWidgets.Insert(objective);
			
			RichTextWidget name = RichTextWidget.Cast(objective.FindAnyWidget("ObjectiveText"));
			SCR_EditorTask editorTask = SCR_EditorTask.Cast(m_aFinishedTasks[i]);
			SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(m_aFinishedTasks[i]);
			if (editorTask)
				name.SetTextFormat(editorTask.GetTitle(), editorTask.GetLocationName());
			else if (campaignTask)
				name.SetTextFormat(campaignTask.GetTitle(), campaignTask.GetBaseNameWithCallsign());
			else
				name.SetText(m_aFinishedTasks[i].GetTitle());
		}
		
		if (m_iFinishedTasksCount < 7)
		{
			Widget paginationWidget = m_wFinishedObjectivesWidget.FindAnyWidget("BottomTitleSizeLayout");
			paginationWidget.SetOpacity(0);
		}
		else
		{
			Widget pages = m_wFinishedObjectivesWidget.FindAnyWidget("Pages");
			if (!pages)
				return;
			
			SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
			if (!pagesVisualised)
				return;
			
			pagesVisualised.SetItemCount(m_iFinishedTasksCount/6);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes pagination by registering button actions
	protected void InitPagination()
	{
		Widget previousButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (previousButton)
			previousButton.m_OnClicked.Insert(ButtonClicked);

		Widget nextButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("NextButton");
		if (!nextButtonWidget)
			return;
		
		SCR_PagingButtonComponent nextButton = SCR_PagingButtonComponent.Cast(nextButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (nextButton)
			nextButton.m_OnClicked.Insert(ButtonClicked);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes focus to the column from which that button originates. It is supposed to be invoked.
	protected void ButtonClicked()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wColumnButton);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles inputs of buttons
	protected void HandlePagination(bool enabled = false)
	{
		Widget previousButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("PrevButton");
		if (!previousButtonWidget)
			return;
		
		SCR_PagingButtonComponent previousButton = SCR_PagingButtonComponent.Cast(previousButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (!previousButton)
			return;
		
		if (enabled)
			previousButton.m_OnActivated.Insert(PreviousButtonActivated);
		else
			previousButton.m_OnActivated.Remove(PreviousButtonActivated);

		Widget nextButtonWidget = m_wFinishedObjectivesWidget.FindAnyWidget("NextButton");
		if (!nextButtonWidget)
			return;
		
		SCR_PagingButtonComponent nextButton = SCR_PagingButtonComponent.Cast(nextButtonWidget.FindHandler(SCR_PagingButtonComponent));
		if (!nextButton)
			return;
		
		if (enabled)
			nextButton.m_OnActivated.Insert(NextButtonActivated);
		else
			nextButton.m_OnActivated.Remove(NextButtonActivated);
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the previous one
	protected void PreviousButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage--;
		if (currentPage < 0)
		{
			FlipPage(0);
			return;
		}

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Flips page to the next one
	protected void NextButtonActivated()
	{
		int currentPage = GetCurrentPage();
		currentPage++;
		if (currentPage == Math.Ceil(m_iFinishedTasksCount/6))
			return;

		FlipPage(currentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the content of current page based on the provided number
	protected void FlipPage(int currentPage)
	{
		for (int i = 0; i < 6; ++i)
		{
			m_aFinishedObjectivesWidgets[i].SetOpacity(1);
			if (currentPage == 0)
			{
				RichTextWidget name = RichTextWidget.Cast(m_aFinishedObjectivesWidgets[i].FindAnyWidget("ObjectiveText"));
				if (i < m_iFinishedTasksCount)
				{
					SCR_EditorTask editorTask = SCR_EditorTask.Cast(m_aFinishedTasks[i]);
					SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(m_aFinishedTasks[i]);
					if (editorTask)
						name.SetTextFormat(editorTask.GetTitle(), editorTask.GetLocationName());
					else if (campaignTask)
						name.SetTextFormat(campaignTask.GetTitle(), campaignTask.GetBaseNameWithCallsign());
					else
						name.SetText(m_aFinishedTasks[i].GetTitle());
				}
				else
				{
					m_aFinishedObjectivesWidgets[i].SetOpacity(0);
				}
			}
			else
			{
				RichTextWidget name = RichTextWidget.Cast(m_aFinishedObjectivesWidgets[i].FindAnyWidget("ObjectiveText"));
				if ((currentPage * 6) + i < m_iFinishedTasksCount)
				{
					SCR_EditorTask editorTask = SCR_EditorTask.Cast(m_aFinishedTasks[(currentPage * 6) + i]);
					SCR_CampaignTask campaignTask = SCR_CampaignTask.Cast(m_aFinishedTasks[(currentPage * 6) + i]);
					if (editorTask)
						name.SetTextFormat(editorTask.GetTitle(), editorTask.GetLocationName());
					else if (campaignTask)
						name.SetTextFormat(campaignTask.GetTitle(), campaignTask.GetBaseNameWithCallsign());
					else
						name.SetText(m_aFinishedTasks[(currentPage * 6) + i].GetTitle());
				
				}
				else
				{
					m_aFinishedObjectivesWidgets[i].SetOpacity(0);
				}
			}
		}
		
		SetCurrentPage(currentPage);
		
		Widget pages = m_wFinishedObjectivesWidget.FindAnyWidget("Pages");
		if (!pages)
			return;
		
		SCR_SelectionHintComponent pagesVisualised = SCR_SelectionHintComponent.Cast(pages.FindHandler(SCR_SelectionHintComponent));
		if (!pagesVisualised)
			return;
		
		pagesVisualised.SetItemCount(m_iFinishedTasksCount/6);
		pagesVisualised.SetCurrentItem(currentPage);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get current page
	\return current page
	*/
	int GetCurrentPage()
	{
		return m_iCurrentPage;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets current page number which just changes the member variable and nothing more. 
	//! Method FlipPage actually performs UI Changes based on this member variable.
	void SetCurrentPage(int page)
	{
		m_iCurrentPage = page;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get title
	\return title
	*/
	string GetTitleText()
	{
		return m_sTitleText;
	}
};

//------------------------------------------------------------------------------------------------
//! Class handling Statistics retrieved from the DataCollector
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DebriefingScreenStatisticsContent : SCR_WelcomeScreenBaseContent
{
	[Attribute()]
	protected string m_sTitleText;
	
	[Attribute()]
	protected ref array<ref SCR_DebriefingScreenStatisticBaseClass> m_aStatistics;
	
	[Attribute(defvalue: "{64B98222099C391E}UI/layouts/Menus/DeployMenu/DebriefingScreenStatisticsTile.layout")]
	protected ResourceName m_sStatisticsTileLayout;
	
	[Attribute(defvalue: "{F2B6C48FDA2E261E}UI/layouts/Menus/DeployMenu/DebriefingScreenStatistic.layout")]
	protected ResourceName m_sStatisticsLayout;
	
	protected Widget m_wStatisticsWidget;
	
	//------------------------------------------------------------------------------------------------
	//! Initializes content for given column
	override void InitContent(SCR_WelcomeScreenMenu menu, string column, string columnButton)
	{
		m_sContentName = column;
		
		Widget targetTile = menu.GetRootWidget().FindAnyWidget(column);
		if (!targetTile)
			return;
		
		m_wStatisticsWidget = GetGame().GetWorkspace().CreateWidgets(m_sStatisticsTileLayout, targetTile);
		if (!m_wStatisticsWidget)
			return;
		
		Widget statisticsContent = m_wStatisticsWidget.FindAnyWidget("StatsLayout");
		if (!statisticsContent)
			return;

		FillStatisticsWidget(statisticsContent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills content widget with statistics
	void FillStatisticsWidget(Widget content)
	{
		RichTextWidget titleText = RichTextWidget.Cast(m_wStatisticsWidget.FindAnyWidget("TitleText"));
		titleText.SetText(GetTitleText());
		
		array<ref SCR_DebriefingScreenStatisticBaseClass> statistics = {};
		int statisticsCount;
		statisticsCount = GetStatistics(statistics);
		
		int statsCycleCount;
		if (statisticsCount < 7)
			statsCycleCount = statisticsCount;
		else
			statsCycleCount = 6;
		
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(0, true, false);
		//We Calculate the stats difference only applicable for this particular session
		array<float> stats = playerData.CalculateStatsDifference();
		//Statistics are capped for 6 at the time
		for (int i = 0; i < statsCycleCount; ++i)
		{
			//statsCycleCount--;
			Widget statistic = GetGame().GetWorkspace().CreateWidgets(m_sStatisticsLayout, content);

			statistics[i].InitStatistic(stats);
			RichTextWidget name = RichTextWidget.Cast(statistic.FindAnyWidget("StatisticResult"));
			name.SetText(statistics[i].GetStatisticResult());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get statistics
	\param[out] statistics array of statistics
	\return number of statistics
	*/
	int GetStatistics(out array<ref SCR_DebriefingScreenStatisticBaseClass> statistics)
	{
		statistics = m_aStatistics;

		return m_aStatistics.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get title
	\return title
	*/
	string GetTitleText()
	{
		return m_sTitleText;
	}
};

//------------------------------------------------------------------------------------------------
//! Base class that holds and handles information about statistic
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticBaseClass
{
	protected string m_sStatisticResult;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	void InitStatistic(notnull array<float> stats)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get statistic result
	\return statistic result
	*/
	string GetStatisticResult()
	{
		return m_sStatisticResult;
	}
};

//------------------------------------------------------------------------------------------------
//! Class that holds information and handles information about shots fired
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticShotsFired : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Rounds</b>", Math.Floor(stats[SCR_EDataStats.SHOTS])));
	}
};

//------------------------------------------------------------------------------------------------
//! Class that holds information and handles information about kills
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticKills : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Kills</b>", Math.Floor(stats[SCR_EDataStats.AI_KILLS] + stats[SCR_EDataStats.KILLS])));
	}
};

//------------------------------------------------------------------------------------------------
//! Class that holds information and handles information about deaths
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDeaths : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_Deaths</b>", Math.Floor(stats[SCR_EDataStats.DEATHS])));
	}
};

//------------------------------------------------------------------------------------------------
//! Class that holds information and handles information about distance walked
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDistanceWalked : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_DistanceWalked</b>", Math.Floor(stats[SCR_EDataStats.DISTANCE_WALKED]/1000)));
	}
};

//------------------------------------------------------------------------------------------------
//! Class that holds information and handles information about distance driven
[BaseContainerProps()]
class SCR_DebriefingScreenStatisticDistanceDriven : SCR_DebriefingScreenStatisticBaseClass
{
	//------------------------------------------------------------------------------------------------
	//! Initialises statistics and performs necessary data retrieval and assigment
	override void InitStatistic(notnull array<float> stats)
	{
		m_sStatisticResult = string.Format(WidgetManager.Translate("<b>#AR-DebriefingScreen_Statistics_DistanceDriven</b>", Math.Floor(stats[SCR_EDataStats.DISTANCE_DRIVEN]/1000)));
	}
};
