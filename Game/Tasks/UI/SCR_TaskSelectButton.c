class SCR_TaskSelectButton : ScriptedWidgetComponent
{
	protected SCR_MapUITask m_MapUiTask;
	protected ref ScriptInvoker m_OnMapIconClick;

	static const int VISIBLE = 1;
	static const int NOT_VISIBLE = 0;

	//------------------------------------------------------------------------------
	ScriptInvoker GetOnMapIconClick()
	{
		if (!m_OnMapIconClick)
			m_OnMapIconClick = new ScriptInvoker();

		return m_OnMapIconClick;
	}

	//------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_OnMapIconClick)
			m_OnMapIconClick.Invoke();

		SCR_UITaskManagerComponent utm = SCR_UITaskManagerComponent.GetInstance();
		if (utm)
			utm.Action_ShowTasks(null, m_MapUiTask.GetTask());

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_MapUiTask.UpdateFocusedTask();

		ButtonWidget assignButton = ButtonWidget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("TaskTitleButton"));
		Widget assignees = Widget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("Assignee"));
		Widget iconHover = Widget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("TaskIconHover"));

		if (!assignButton || !assignees || !iconHover)
			return false;

		assignButton.SetVisible(true);

		assignees.SetEnabled(HasAssigneeBool());
		assignees.SetOpacity(HasAssigneeInt());

		iconHover.SetEnabled(true);
		iconHover.SetOpacity(VISIBLE);

		return false;
	}

	//------------------------------------------------------------------------------
	bool HasAssigneeBool()
	{
		SCR_BaseTask task = m_MapUiTask.GetTask();
		if (!task)
			return false;

		if (task.GetAssigneeCount() == 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	int HasAssigneeInt()
	{
		SCR_BaseTask task = m_MapUiTask.GetTask();
		if (!task)
			return NOT_VISIBLE;

		if (task.GetAssigneeCount() == 0)
			return NOT_VISIBLE;

		return VISIBLE;
	}

	//------------------------------------------------------------------------------
	void SetRootWidgetHandler(SCR_MapUITask mapUiTask)
	{
		m_MapUiTask = mapUiTask;
	}
};
