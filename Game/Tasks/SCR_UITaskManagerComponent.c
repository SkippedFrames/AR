[EntityEditorProps(category: "GameScripted/Tasks", description: "This is a task manager related component that allows you to display the available tasks.", color: "0 0 255 255")]
class SCR_UITaskManagerComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_UITaskManagerComponent : ScriptComponent
{
	protected static const string NO_ASSIGNED_TASK = "#AR-Tasks_NoAssignedTaskTitle";
	protected static const string ASSIGN_TASK_HINT = "#AR-Tasks_NoAssignedTaskDescription";

	protected ref array<SCR_BaseTask> m_aHUDIconUpdatedTasks = {};
	
	static ref ScriptInvoker s_OnTaskListVisible = new ScriptInvoker();

	protected Widget m_wWidgetToFocus = null;
	protected Widget m_wTasksUI = null;
	protected Widget m_wParentWidget = null;
	protected Widget m_wUI = null;
	protected Widget m_wTaskDetail = null;
	protected Widget m_wExpandButton = null;

	static SCR_UITaskManagerComponent s_Instance;
	protected EventHandlerManagerComponent m_EventHandlerManager;

	ref array<Widget> m_aWidgets = {};
	ref map<SCR_BaseTask, TextWidget> m_mTasksTimers = new map<SCR_BaseTask, TextWidget>();

	protected bool m_bVisible;
	protected bool m_bDetailVisible;
	protected bool m_bPickAssigneeVisible;
	protected bool m_bCurrentTaskVisible;
	protected bool m_bTaskContextEnabled;
	protected bool m_bIsUnconscious;	//Character is unconscious --> Task menu control is disabled
	protected float m_fCurrentTaskTime;
	protected SCR_BaseTask m_SelectedTask;
	protected SCR_BaseTask m_LastSelectedTask;
	protected bool m_bShowSelectedTaskOnMap;

	[Attribute("{ACCF501DD69CAF7B}UI/layouts/Tasks/TaskList.layout")]
	ResourceName m_UIResource;

	[Attribute("{EE9497AB43F556F9}UI/layouts/Tasks/CurrentTask.layout")]
	protected ResourceName m_sCurrentTaskLayout;

	[Attribute("{728FD22A081ABB36}UI/layouts/Tasks/TaskDetail.layout")]
	protected ResourceName m_sTaskDetailLayout;

	[Attribute("5")]
	protected float m_fCurrentTaskShowTime;

	[Attribute("1")]
	protected bool m_bOpenTaskListOnMapOpen;

	[Attribute("{31C4EB3D607069C2}UI/layouts/Tasks/PickAssigneeListEntry.layout")]
	protected ResourceName m_wPlayerListEntry;

	[Attribute("0.761 0.386 0.08 1")]
	protected ref Color m_AssignedTaskColor;

	protected Widget m_wCurrentTask;

	//------------------------------------------------------------------------------------------------
	static SCR_UITaskManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsTaskListOpen()
	{
		return m_bVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedWidget(Widget w)
	{
		m_wWidgetToFocus = w;
	}

	//------------------------------------------------------------------------------------------------
	static void PanMapToTask(int taskID = -1, SCR_BaseTask task = null)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity || !mapEntity.IsOpen())
			return;

		if (!task)
		{
			if (taskID == -1)
				return;

			task = GetTaskManager().GetTask(taskID);
			if (!task)
				return;
		}

		vector taskPos = task.GetOrigin();
		float screenX, screenY;
		mapEntity.WorldToScreen(taskPos[0], taskPos[2], screenX, screenY);
		mapEntity.PanSmooth(screenX, screenY);

	}

	//------------------------------------------------------------------------------------------------
	void StopHUDIconUpdates(SCR_BaseTask task)
	{
		m_aHUDIconUpdatedTasks.RemoveItem(task);
		task.ToggleHUDIcon(false, false);
	}

	//------------------------------------------------------------------------------------------------
	void KeepHUDIconUpdated(int msTime, SCR_BaseTask task)
	{
		if (m_aHUDIconUpdatedTasks.Find(task) != 0)
			return;

		m_aHUDIconUpdatedTasks.Insert(task);
		GetGame().GetCallqueue().CallLater(StopHUDIconUpdates, msTime, false, task);
	}

	//------------------------------------------------------------------------------------------------
	void OnTaskDeleted(SCR_BaseTask task)
	{
		if (m_aHUDIconUpdatedTasks.Find(task) != -1)
			m_aHUDIconUpdatedTasks.RemoveItem(task);
	}

	//------------------------------------------------------------------------------------------------
	void SelectTask(SCR_BaseTask task, Widget w = null)
	{
		m_SelectedTask = task;
		if (task)
			m_LastSelectedTask = task;
	}

	//------------------------------------------------------------------------------------------------
	SCR_BaseTask GetSelectedTask()
	{
		return m_SelectedTask;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wUI;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetParentWidget()
	{
		return m_wParentWidget;
	}

	//------------------------------------------------------------------------------------------------
	array<Widget> GetWidgetsArray()
	{
		return m_aWidgets;
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterTimer(SCR_BaseTask task)
	{
		if (m_mTasksTimers)
			m_mTasksTimers.Remove(task);
	}

	//------------------------------------------------------------------------------------------------
	void RegisterTimer(SCR_BaseTask task, TextWidget timerWidget)
	{
		if (m_mTasksTimers)
			m_mTasksTimers.Set(task, timerWidget);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates assignee texts if needed.
	// This method is not used
	void SetupAssigneeTexts(notnull SCR_BaseTask task, Widget parent)
	{
		TextWidget assigneeNames = TextWidget.Cast(parent.FindAnyWidget("TaskAssignees"));

		TextWidget assigneeTime = TextWidget.Cast(parent.FindAnyWidget("AssigneeTime"));
		if (!assigneeTime)
			return;

		if (task.IsIndividual())
		{
			if (assigneeTime)
			{
				RegisterTimer(task, assigneeTime);
				assigneeTime.SetVisible(task.GetAssignee() && task.IsIndividual());

				float remainingTime = task.GetAssigneeTimeLeft();
				string formattedTimerText = GetFormattedTimerText(remainingTime);
				assigneeTime.SetText(formattedTimerText);
			}
		}
		else
			assigneeTime.SetVisible(false);

		if (!assigneeNames)
			return;

		array<SCR_BaseTaskExecutor> assignees = new array<SCR_BaseTaskExecutor>();
		task.GetAssignees(assignees);

		bool hasAssignees = assignees.Count() > 0;
		assigneeNames.SetVisible(hasAssignees);

		if (!hasAssignees)
			return;

		string namesText = "";

		for (int i = 0, count = assignees.Count(); i < count; i++)
		{
			if (i > 0)
				namesText += ", ";
			namesText += assignees[i].GetPlayerName();
		}

		assigneeNames.SetText(namesText);
	}
	
	//------------------------------------------------------------------------------------------------
	void GenerateUI(Widget rootWidget ,SCR_BaseTask taskToFocus = null)
	{
		if (!GetTaskManager())
			return;
			
		if (!rootWidget)
			return;
		
		SCR_BaseTask locallyRequestedTask;
		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
		if (supportEntity)
			locallyRequestedTask = supportEntity.GetLocallyRequestedTask();
		
		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (!localTaskExecutor)
			return;
		
		

		Faction faction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			faction = factionManager.GetLocalPlayerFaction();

		array<SCR_BaseTask> tasks = {};
		GetTaskManager().GetFilteredTasks(tasks, faction);

		if (locallyRequestedTask)
		{
			if (!m_wWidgetToFocus)
				m_wWidgetToFocus = locallyRequestedTask.GenerateTaskDescriptionUI(rootWidget, m_aWidgets);
			else
				locallyRequestedTask.GenerateTaskDescriptionUI(rootWidget, m_aWidgets);
		}

		SCR_BaseTask locallyAssignedTask = SCR_BaseTaskExecutor.GetLocalExecutor().GetAssignedTask();

		Widget currentTaskUI;
		for (int i = 0; i < tasks.Count(); i++)
		{
			SCR_BaseTask task = tasks[i];

			if (!task)
			{
				Print("Null found in task list!", LogLevel.ERROR);
				continue;
			}

			if (task == locallyRequestedTask)
				continue;

			currentTaskUI = task.GenerateTaskDescriptionUI(rootWidget, m_aWidgets);

			if (!m_wWidgetToFocus)
				m_wWidgetToFocus = currentTaskUI;

			if (task == locallyAssignedTask)
			{
				RichTextWidget textWidget = RichTextWidget.Cast(currentTaskUI.FindAnyWidget("TaskTitle"));
				textWidget.SetColor(m_AssignedTaskColor);
			}

			if (task == taskToFocus)
			{
				m_wWidgetToFocus = currentTaskUI;
				SCR_TaskListEntryHandler handler = SCR_TaskListEntryHandler.Cast(m_wWidgetToFocus.FindHandler(SCR_TaskListEntryHandler));
				if (handler)
					handler.SetCollapsed(false);
			}
		}
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_TASK_MENU_OPEN);
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateRequestButtons()
	{
		// note: turned off now
		return;
		array<SCR_BaseTaskSupportEntity> supportedTasks = GetTaskManager().GetSupportedTasks();
		if (!supportedTasks)
			return;

		m_wParentWidget = m_wUI.FindAnyWidget("MainPanel");
		if (!m_wParentWidget)
			return;

		for (int i = supportedTasks.Count() - 1; i >= 0; i--)
		{
			supportedTasks[i].OnTaskListOpen(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearUI()
	{
		if (!m_aWidgets || !m_mTasksTimers)
			return;

		m_mTasksTimers.Clear();

		foreach (Widget widget: m_aWidgets)
		{
			if (widget)
				widget.RemoveFromHierarchy();
		}
		
		m_aWidgets.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		for (int i = m_aHUDIconUpdatedTasks.Count() - 1; i >= 0; i--)
		{
			m_aHUDIconUpdatedTasks[i].UpdateHUDIcon();
		}

		if (m_bVisible || m_bTaskContextEnabled)
		{
			GetGame().GetInputManager().ActivateContext("TaskListContext");
			// if (m_bPickAssigneeVisible)
			// 	GetGame().GetInputManager().ActivateContext("PickAssigneeContext");
		}

		if (m_bCurrentTaskVisible)
		{
			if (m_fCurrentTaskTime > 0)
			{
				m_fCurrentTaskTime -= timeSlice;
				if (m_fCurrentTaskTime < 0)
					ToggleCurrentTask();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	string GetFormattedTimerText(float time)
	{
		int minutes = Math.Floor(time / 60);
		int seconds = Math.Floor(time - (minutes * 60));

		string minutesString;
		if (minutes < 10)
			minutesString = "0";
		minutesString += minutes.ToString();

		string secondsString;
		if (seconds < 10)
			secondsString = "0";
		secondsString += seconds.ToString();

		return minutesString + ":" + secondsString;
	}

	//------------------------------------------------------------------------------------------------
	Widget InitCurrentTaskWidget()
	{
		return GetGame().GetWorkspace().CreateWidgets(m_sCurrentTaskLayout);
	}

	//------------------------------------------------------------------------------------------------
	bool HasLocallyAssignedTask()
	{
		if (!GetTaskManager())
			return false;
		
		SCR_BaseTask locallyRequestedTask;
		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
		if (supportEntity)
			locallyRequestedTask = supportEntity.GetLocallyRequestedTask();
		
		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (!localTaskExecutor || !localTaskExecutor.GetAssignedTask())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ToggleCurrentTask(bool fade = true)
	{
		m_bCurrentTaskVisible = !m_bCurrentTaskVisible;

		if (!m_wCurrentTask)
			m_wCurrentTask = InitCurrentTaskWidget();

		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		SCR_BaseTask assignedTask = localTaskExecutor.GetAssignedTask();

		float speed = 1;
		float targetOpacity;
		if (m_bCurrentTaskVisible)
		{
			targetOpacity = 1;

			if (localTaskExecutor)
			{
				TextWidget textWidget = TextWidget.Cast(m_wCurrentTask.FindAnyWidget("TaskTitle"));

				if (assignedTask)
					assignedTask.SetTitleWidgetText(textWidget, assignedTask.GetTaskListTaskTitle());
				else
					textWidget.SetTextFormat(NO_ASSIGNED_TASK);

				textWidget = TextWidget.Cast(m_wCurrentTask.FindAnyWidget("TaskDescription"));
				if (assignedTask)
					assignedTask.SetTitleWidgetText(textWidget, assignedTask.GetTaskListTaskText());
				else
					textWidget.SetTextFormat(ASSIGN_TASK_HINT);

				textWidget = TextWidget.Cast(m_wCurrentTask.FindAnyWidget("CurrentObjectiveText"));
				if (assignedTask)
					textWidget.SetVisible(true);
				else
					textWidget.SetVisible(false);
			}
		}
		else
		{
			targetOpacity = 0;
		}

		if (fade)
		{
			AnimateWidget.Opacity(m_wCurrentTask, targetOpacity, speed);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wCurrentTask, WidgetAnimationOpacity);
			m_wCurrentTask.SetOpacity(targetOpacity);
		}

		m_fCurrentTaskTime = m_fCurrentTaskShowTime;
	}

	//------------------------------------------------------------------------------------------------
	void HidePickAssignee()
	{
		m_bPickAssigneeVisible = false;
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.PickAssignee);
	}

	//------------------------------------------------------------------------------------------------
	void HideDetail()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.TaskDetail);
		m_bDetailVisible = false;

		m_wUI.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	void Action_PickAssignee()
	{
		Widget pickAssigneeWidget;

		if (!m_bPickAssigneeVisible)
		{
			m_bPickAssigneeVisible = true;
			MenuBase menu = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.PickAssignee, DialogPriority.CRITICAL, 0, true);
			pickAssigneeWidget = menu.GetRootWidget();
			pickAssigneeWidget.SetZOrder(m_wUI.GetZOrder());

			Widget playerList = pickAssigneeWidget.FindAnyWidget("Players");
			if (!playerList)
				return;

			while (playerList.GetChildren())
			{
				playerList.RemoveChild(playerList.GetChildren());
			}

			int entriesCount;
			SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
			map<SCR_BaseTaskExecutor, int> taskExecutors = SCR_BaseTaskExecutor.GetTaskExecutorsMap();
			for (int i = taskExecutors.Count() - 1; i >= 0; i--)
			{
				SCR_BaseTaskExecutor executor = taskExecutors.GetKey(i);

				if (executor == localExecutor)
					continue;

				SCR_BaseTask executorTask = executor.GetAssignedTask();

				//Executor is already assigned to this task
				if (executorTask == m_LastSelectedTask)
					continue;

				entriesCount++;

				Widget playerEntry = playerList.GetWorkspace().CreateWidgets(m_wPlayerListEntry, playerList);

				if (executorTask)
					playerEntry.FindAnyWidget("IsAssigned").SetVisible(true);

				SCR_TaskPlayerListEntryHandler entryHandler = SCR_TaskPlayerListEntryHandler.Cast(playerEntry.FindHandler(SCR_TaskPlayerListEntryHandler));
				if (entryHandler)
					entryHandler.SetExecutor(executor);

				TextWidget playerName = TextWidget.Cast(playerEntry.FindAnyWidget("PlayerName"));
				playerName.SetText(executor.GetPlayerName());
			}

			if (entriesCount < 1)
				HidePickAssignee();

			return;
		}

		SCR_PickAssigneeDialog pickAssigneeMenu = SCR_PickAssigneeDialog.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.PickAssignee));
		if (!pickAssigneeMenu)
			return;

		SCR_BaseTaskExecutor assignee = pickAssigneeMenu.GetSelectedExecutor();
		if (!assignee)
			return;

		m_bPickAssigneeVisible = false;

		SCR_TaskNetworkComponent taskNetworkComponent = SCR_TaskNetworkComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_TaskNetworkComponent));
		if (taskNetworkComponent)
			taskNetworkComponent.AssignTaskToPlayer(SCR_BaseTaskExecutor.GetLocalExecutorID(), m_LastSelectedTask.GetTaskID(), SCR_BaseTaskExecutor.GetTaskExecutorID(pickAssigneeMenu.GetSelectedExecutor()));

		GetGame().GetMenuManager().CloseMenu(pickAssigneeMenu);
	}

	//------------------------------------------------------------------------------------------------
	void Action_TasksOpen()
	{
		if (m_bIsUnconscious)
			return;
		
		if (!m_bVisible)
			Action_ShowTasks(GetRootWidget());
		else
			Action_TasksClose();
	}

	//------------------------------------------------------------------------------------------------
	void Action_ShowHint()
	{
		if (m_bIsUnconscious)
			return;
		
		if (m_bVisible)
			return;
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity || !mapEntity.IsOpen())
		{
			bool fade = !m_bCurrentTaskVisible;
			ToggleCurrentTask(fade);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_TasksClose()
	{
		m_SelectedTask = null;
		
		if (m_bVisible)
			Action_HideTasks();
	}

	//------------------------------------------------------------------------------------------------
	void Action_ShowOnMap()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		if (m_bDetailVisible)
			HideDetail();

		if (!mapEntity.IsOpen())
		{
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			if (!player)
				return;
			
			SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
			if (!gadgetManager)
				return;
					
			IEntity mapGadget = gadgetManager.GetGadgetByType(EGadgetType.MAP);
			if (!mapGadget)
				return;
			
			m_bShowSelectedTaskOnMap = true;
			gadgetManager.SetGadgetMode(mapGadget, EGadgetMode.IN_HAND);
		}
		else
		{
			PanMapToTask(task: m_SelectedTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_AssignTask(SCR_BaseTask task = null)
	{
		if (!task)
		{
			if (!m_SelectedTask)
				return;
		}

		SCR_TaskNetworkComponent taskNetworkComp = SCR_TaskNetworkComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_TaskNetworkComponent));
		if (!taskNetworkComp)
			return;

		SCR_BaseTask curTask = SCR_BaseTaskExecutor.GetLocalExecutor().GetAssignedTask();
		if (curTask)
		{
			int curTaskId = curTask.GetTaskID();
			taskNetworkComp.AbandonTask(curTaskId);
		}

		int taskId;
		if (task)
			taskId = task.GetTaskID();
		else
			taskId = m_SelectedTask.GetTaskID();

		if (curTask && curTask == m_SelectedTask)
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_CANCELED);
			taskNetworkComp.AbandonTask(taskId);
		}
		else
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_ACCEPT);
			taskNetworkComp.RequestAssignment(taskId);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_ShowDetail()
	{
		if (!m_SelectedTask)
			return;

		if (m_bDetailVisible)
			return;

		MenuBase menu;
		if (!m_wTaskDetail)
		{
			menu = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.TaskDetail, DialogPriority.CRITICAL, 0, true);
			m_wTaskDetail = menu.GetRootWidget();
		}

		m_wTaskDetail.SetZOrder(m_wUI.GetZOrder() + 1);
		m_bDetailVisible = true;
		m_wUI.SetOpacity(0);

		//richTextWidget is task title
		RichTextWidget richTextWidget = RichTextWidget.Cast(m_wTaskDetail.FindAnyWidget("TaskTitle"));
		m_SelectedTask.SetTitleWidgetText(richTextWidget, m_SelectedTask.GetTaskListTaskTitle());

		//richTextWidget is task description
		richTextWidget = RichTextWidget.Cast(m_wTaskDetail.FindAnyWidget("TaskDescription"));
		m_SelectedTask.SetDescriptionWidgetText(richTextWidget, m_SelectedTask.GetTaskListTaskText());

		ImageWidget image = ImageWidget.Cast(m_wTaskDetail.FindAnyWidget("TaskIcon"));
		image.SetEnabled(false);
		image.SetOpacity(0);
		m_SelectedTask.SetWidgetIcon(image);
	}

	//------------------------------------------------------------------------------------------------
	void Action_Expand()
	{
		if (!m_SelectedTask)
			return;

		foreach (Widget w : m_aWidgets)
		{
			if (!w)
				continue;

			SCR_TaskListEntryHandler handler = SCR_TaskListEntryHandler.Cast(w.FindHandler(SCR_TaskListEntryHandler));
			if (handler && handler.GetTask() == m_SelectedTask)
				handler.ExpandTaskLayout();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration cfg)
	{
		if (cfg.MapEntityMode == EMapEntityMode.EDITOR)
			return;
		
		if (!SCR_PlayerController.GetLocalControlledEntity() || !m_bOpenTaskListOnMapOpen) 
			return;
			
		if (m_bShowSelectedTaskOnMap && m_LastSelectedTask)
			PanMapToTask(-1, m_LastSelectedTask);
	}

	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_bTaskContextEnabled = false;

		if (m_bVisible)
			Action_HideTasks();
	}

	//------------------------------------------------------------------------------------------------
	void HideAllHUDIcons(bool fade = true)
	{
		for (int i = m_aHUDIconUpdatedTasks.Count() - 1; i >= 0; i--)
		{
			m_aHUDIconUpdatedTasks[i].ToggleHUDIcon(false, fade);
			m_aHUDIconUpdatedTasks.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_ShowTasks(Widget targetWidget = null, SCR_BaseTask taskToFocus = null)
	{
		ClearUI();
		m_bVisible = true;
		
		if (!targetWidget)
			targetWidget = GetRootWidget();
		
		m_wUI = targetWidget;
		
		GenerateUI(targetWidget, taskToFocus);
		if (m_wUI)
			m_wUI.SetVisible(true);
		
		s_OnTaskListVisible.Invoke(true);
		
		if (m_wWidgetToFocus)
			GetGame().GetWorkspace().SetFocusedWidget(m_wWidgetToFocus);
	}

	//------------------------------------------------------------------------------------------------
	void Action_HideTasks()
	{
		//HUD Icon
		//HideAllHUDIcons(false);
		m_bVisible = false;
		ClearUI();
		if (m_wUI)
			m_wUI.SetVisible(false);
		s_OnTaskListVisible.Invoke(false);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_TASK_MENU_CLOSE);
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_SelectTask(notnull SCR_BaseTask task)
	{
		foreach (Widget w: m_aWidgets)
		{
			if (!w)
				continue;
			
			SCR_TaskListEntryHandler handler = SCR_TaskListEntryHandler.Cast(w.FindHandler(SCR_TaskListEntryHandler));
			if (handler && handler.GetTask() && handler.GetTask() == task)
			{
				GetGame().GetWorkspace().SetFocusedWidget(w);
			}
		}		
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveTaskFromList(SCR_BaseTask task)
	{
		if (!task)
			return;

		if (task == m_LastSelectedTask)
			m_LastSelectedTask = null;

		foreach (Widget w : m_aWidgets)
		{
			if (!w)
				continue;

			SCR_TaskListEntryHandler handler = SCR_TaskListEntryHandler.Cast(w.FindHandler(SCR_TaskListEntryHandler));
			if (handler)
			{
				if (handler.GetTask() && handler.GetTask() == task)
					w.RemoveFromHierarchy();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the timers in task descriptions.
	void UpdateTimers()
	{
		for (int i = 0, count = m_mTasksTimers.Count(); i < count; i++)
		{
			TextWidget timer = m_mTasksTimers.GetElement(i);
			SCR_BaseTask task = m_mTasksTimers.GetKeyByValue(timer);
			if (!task)
				continue;

			float remainingTime = task.GetAssigneeTimeLeft();
			if (remainingTime < 0)
				continue;

			string formattedTimerText = GetFormattedTimerText(remainingTime);

			timer.SetText(formattedTimerText);
		}
	}

	//------------------------------------------------------------------------------------------------
	void EnableTaskContext(bool enable)
	{
		m_bTaskContextEnabled = enable;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (m_wExpandButton)
		{
			m_wExpandButton.SetVisible(isGamepad);
			m_wExpandButton.SetEnabled(isGamepad);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Listeners are added back because they are previously removed on
	//! unconsciousness and if character dies, they need to added back.
	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{		
		m_bIsUnconscious = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddActionListeners()
	{
		GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, Action_TasksOpen);
		GetGame().GetInputManager().AddActionListener("TasksClose", EActionTrigger.DOWN, Action_TasksClose);
		GetGame().GetInputManager().AddActionListener("TasksShowHint", EActionTrigger.DOWN, Action_ShowHint);
		GetGame().GetInputManager().AddActionListener("TasksExpand", EActionTrigger.DOWN, Action_Expand);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveActionListeners()
	{
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, Action_TasksOpen);
		GetGame().GetInputManager().RemoveActionListener("TasksClose", EActionTrigger.DOWN, Action_TasksClose);
		GetGame().GetInputManager().RemoveActionListener("TasksShowHint", EActionTrigger.DOWN, Action_ShowHint);
		GetGame().GetInputManager().RemoveActionListener("TasksExpand", EActionTrigger.DOWN, Action_Expand);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit Task manager Component when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{		
		Action_TasksClose();
		
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
			m_EventHandlerManager = null;
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (!character)
			return;
		
		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController)
			return;
		
		m_bIsUnconscious = characterController.IsUnconscious();
		
        m_EventHandlerManager = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
        if (m_EventHandlerManager)
            m_EventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
	}
	
	void OnConsciousnessChanged(bool conscious)
	{
		m_bIsUnconscious = !conscious;
		
		if (!conscious)
			Action_TasksClose();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		CreateTaskList();
		AddActionListeners();

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
//		SCR_RespawnSuperMenu.Event_OnMenuOpen.Insert(OnMapClose);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
        
		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnected);
	}
	
	//------------------------------------------------------------------------------------------------
    void OnPlayerConnected(int playerID)
    {
        SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
        if (!playerController)
            return;
		
        playerController.m_OnDestroyed.Insert(OnDestroyed);
        int localPlayerID = playerController.GetLocalPlayerId();
        if (playerID != localPlayerID)
            return;
        
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
    }
	
	//------------------------------------------------------------------------------------------------
	Widget CreateTaskList(Widget w = null)
	{
		if (w)
			m_wUI = GetGame().GetWorkspace().CreateWidgets(m_UIResource, w);
		else
			m_wUI = GetGame().GetWorkspace().CreateWidgets(m_UIResource);
		
		if (m_wUI)
		{
			m_wTasksUI = m_wUI.FindAnyWidget("Tasks");
			m_wUI.SetVisible(m_bVisible);
			m_wExpandButton = m_wUI.FindAnyWidget("ExpandTaskButton");
		}


		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);

		SCR_InputButtonComponent hideTasks = SCR_InputButtonComponent.GetInputButtonComponent("HideTasksButton", m_wUI);
		if (hideTasks)
			hideTasks.m_OnActivated.Insert(Action_TasksClose);

		SCR_BaseTaskManager.s_OnTaskDeleted.Insert(RemoveTaskFromList);
		
		return m_wUI;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearWidget()
	{
		if (m_wUI)
			m_wUI.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_UITaskManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_UITaskManagerComponent()
	{
		s_Instance = null;
		m_aWidgets = null;
		if (m_wUI)
			m_wUI.RemoveFromHierarchy();
		
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
	}
};
