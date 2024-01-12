//------------------------------------------------------------------------------------------------
class SCR_TaskListEntryHandler : SCR_ButtonBaseComponent
{
	protected SCR_CollapseWidgetComponent m_CollapseHandler;

	protected SCR_InputButtonComponent m_AssignButton;
	protected SCR_InputButtonComponent m_ShowOnMapButton;

	protected SCR_BaseTask m_Task;
	protected Widget m_wMainWidget;

	protected static const string ACCEPT_TASK 						= "#AR-CampaignTasks_ButtonAccept";
	protected static const string ABANDON_TASK 						= "#AR-CampaignTasks_ButtonAbandon";
	protected static const string CANCEL_TASK 						= "#AR-CampaignTasks_ButtonCancel";
	protected static const string LOCAL_EXECUTOR_IS_ASSIGNED_TEXT 	= "#AR-CampaignTasks_ButtonBusy";
	protected static const string ASSIGNEE_TIMEOUT_TEXT 			= "#AR-CampaignTasks_ButtonWait";
	protected static const string TASK_ABANDONED_TEXT 				= "#AR-CampaignTasks_ButtonWait";

	protected static const float ANIM_SPEED = UIConstants.FADE_RATE_SUPER_FAST;

	protected string m_sAssignees = "AssigneesLayout";
	protected Widget m_wAssignees;
	protected Widget m_wRootWidget;

	[Attribute("0.761 0.386 0.08 1")]
	protected ref Color m_AssignedTaskColor;

	[Attribute("{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset", category: "Task icon", params: "imageset")]
	protected ResourceName m_sIconImageset;
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	void SetTask(SCR_BaseTask task)
	{
		m_Task = task;
	}

	//------------------------------------------------------------------------------------------------
	void SetAssigneeCount(int count)
	{
		TextWidget text = TextWidget.Cast(m_wAssignees.FindAnyWidget("Text"));
		if (text)
			text.SetText(count.ToString());
		m_wAssignees.SetVisible(count > 0);
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskTextColor()
	{
		SCR_BaseTask localTask = SCR_BaseTaskExecutor.GetLocalExecutor().GetAssignedTask();
		RichTextWidget textWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TaskTitle"));
		if (m_Task == localTask)
			textWidget.SetColor(m_AssignedTaskColor);
		else
			textWidget.SetColor(Color.White);
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskIconColor()
	{
		SCR_BaseTask localTask = SCR_BaseTaskExecutor.GetLocalExecutor().GetAssignedTask();
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Outline"));
		ImageWidget symbol = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Symbol"));
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Background"));

		if (!outline || !symbol || !background || !m_Task)
			return;

		SCR_Faction faction = SCR_Faction.Cast(m_Task.GetTargetFaction());
		if (!faction)
			return;

		if (m_Task == localTask)
		{
			background.SetColor(faction.GetFactionColor());
		}
		else if (m_Task.IsPriority())
		{
			background.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_BG_M);
			background.SetColor(Color.White);
			outline.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_O_M);
		}
		else
		{
			background.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_BG);
			background.SetColor(Color.White);
			outline.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_O);
		}
			
		symbol.LoadImageFromSet(0, m_Task.GetIconImageset(), m_Task.GetTaskListIconName() + m_Task.GetIconSuffix());
		outline.SetColor(faction.GetOutlineFactionColor());
		symbol.SetColor(faction.GetOutlineFactionColor());
	}

	//------------------------------------------------------------------------------------------------
	void UpdateTask(SCR_BaseTask task)
	{
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Outline"));
		ImageWidget symbol = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Symbol"));
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Background"));
		
		if (!outline || !symbol || !background || !m_Task)
			return;
		
		if (task == m_Task)
		{
			SetAssigneeCount(task.GetAssigneeCount());
			UpdateAssignButtonText(task);
			SetTaskTextColor();
			SetTaskIconColor();

			TextWidget textWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("TaskTitle"));
			if (textWidget)
				task.SetTitleWidgetText(textWidget, task.GetTaskListTaskTitle());

			textWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("TaskDescription"));
			if (textWidget)
				task.SetTitleWidgetText(textWidget, task.GetTaskListTaskText());
		}
		
		if (m_Task.IsPriority())
		{
			background.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_BG_M);
			outline.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_O_M);
		}
		else
		{
			background.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_BG);
			outline.LoadImageFromSet(0,m_sIconImageset, SCR_BaseTask.TASK_O);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAssignButtonText(SCR_BaseTask task)
	{
		SCR_ECannotAssignReasons reason;
		if (task.CanBeAssigned(reason))
		{
			if (m_AssignButton)
				m_AssignButton.SetLabel(ACCEPT_TASK);
		}
		else
		{
			switch (reason)
			{
				case SCR_ECannotAssignReasons.TASK_IS_ASSIGNED:
				{
					m_AssignButton.SetLabel(ABANDON_TASK)
				} break;

				case SCR_ECannotAssignReasons.TASK_IS_ASSIGNED_TO_LOCAL_EXECUTOR:
				{
					m_AssignButton.SetLabel(ABANDON_TASK);
				} break;

				case SCR_ECannotAssignReasons.IS_TASK_REQUESTER:
				{
					m_AssignButton.SetLabel(CANCEL_TASK);
				} break;

				// Another task is already assigned to a local executor, so CanBeAssigned returns false. However in this case the task can be assigned by clicking on assign button.
				case SCR_ECannotAssignReasons.LOCAL_EXECUTOR_IS_ASSIGNED:
				{
					m_AssignButton.SetLabel(ACCEPT_TASK);
				} break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Expand(bool expand)
	{
		float targetOpacity = 0;
		if (expand)
			targetOpacity = 1;

		m_AssignButton.SetEnabled(expand);
		Widget assignBtn = m_wRoot.FindAnyWidget("AcceptButton");
		AnimateWidget.Opacity(assignBtn, targetOpacity, ANIM_SPEED);

		Widget mapBtn = m_wRoot.FindAnyWidget("MapButton");
		AnimateWidget.Opacity(mapBtn, targetOpacity, ANIM_SPEED);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		InputManager im = GetGame().GetInputManager();
		if (!im.IsUsingMouseAndKeyboard() || !IsAnyButtonHovered())
			return false;

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_ACCEPT);
		ExpandTaskLayout();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ExpandTaskLayout()
	{
		if (!m_CollapseHandler)
			return;
		
		m_CollapseHandler.SetCollapsed(!m_CollapseHandler.IsCollapsed());
		Expand(!m_CollapseHandler.IsCollapsed());

		SCR_UITaskManagerComponent uiTaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!uiTaskManager)
			return;

		foreach (Widget task : uiTaskManager.GetWidgetsArray())
		{
			SCR_TaskListEntryHandler collapse = SCR_TaskListEntryHandler.Cast(task.FindHandler(SCR_TaskListEntryHandler));
			if (collapse && collapse != this && !collapse.IsCollapsed())
			{
				collapse.Expand(false);
				collapse.SetCollapsed(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		SCR_UITaskManagerComponent tm = SCR_UITaskManagerComponent.GetInstance();
		if (!tm)
			return false;

		tm.SelectTask(null);
		tm.SetSelectedWidget(w);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		SCR_UITaskManagerComponent tm = SCR_UITaskManagerComponent.GetInstance();
		if (!tm)
			return false;

		tm.SelectTask(m_Task);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetCollapsed(bool collapse, bool anim = true)
	{
		m_CollapseHandler.SetCollapsed(collapse, anim);
		Widget assignBtns = m_wRoot.FindAnyWidget("AssignButtons");
		if (assignBtns)
		{
			assignBtns.SetOpacity(collapse);
			assignBtns.SetEnabled(collapse);

			Widget mapBtn = m_wRoot.FindAnyWidget("MapButton");
			if (mapBtn)
			{
				mapBtn.SetOpacity(collapse);
				mapBtn.SetEnabled(collapse);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsAnyButtonHovered()
	{
		return (!m_ShowOnMapButton.IsHovering() &&
				!m_AssignButton.IsHovering());
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsCollapsed()
	{
		return m_CollapseHandler.IsCollapsed();
	}

	//------------------------------------------------------------------------------------------------
	protected void AcceptTask()
	{
		SCR_UITaskManagerComponent utm = SCR_UITaskManagerComponent.GetInstance();
		if (utm)
			utm.Action_AssignTask(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	protected void PickAssignee()
	{
		SCR_UITaskManagerComponent utm = SCR_UITaskManagerComponent.GetInstance();
		if (utm)
			utm.Action_PickAssignee();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowOnMap()
	{
		SCR_UITaskManagerComponent utm = SCR_UITaskManagerComponent.GetInstance();
		if (utm)
			utm.Action_ShowOnMap();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(UpdateTask);

		m_wAssignees = w.FindAnyWidget(m_sAssignees);

		m_AssignButton = SCR_InputButtonComponent.GetInputButtonComponent("AcceptButton", w);
		if (m_AssignButton)
		{
			m_AssignButton.SetEnabled(false);
			m_AssignButton.m_OnActivated.Insert(AcceptTask);
		}

		m_ShowOnMapButton = SCR_InputButtonComponent.GetInputButtonComponent("MapButton", w);
		if (m_ShowOnMapButton)
			m_ShowOnMapButton.m_OnActivated.Insert(ShowOnMap);

		Widget collapse = w.FindAnyWidget("Description");
		if (collapse)
		{
			m_CollapseHandler = SCR_CollapseWidgetComponent.Cast(collapse.FindHandler(SCR_CollapseWidgetComponent));
			//GetGame().GetCallqueue().CallLater(SetCollapsed, 10, false, true, false);
		}

		m_wRootWidget = w;
		
		Widget assignBtn = m_wRoot.FindAnyWidget("AcceptButton");
		if (assignBtn)
			assignBtn.SetOpacity(0);

		Widget mapBtn = m_wRoot.FindAnyWidget("MapButton");
		if (mapBtn)
			mapBtn.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_BaseTaskManager.s_OnTaskUpdate.Remove(UpdateTask);
	}
};
