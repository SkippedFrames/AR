//------------------------------------------------------------------------------------------------
enum SCR_ECannotAssignReasons
{
	TASK_IS_ASSIGNED,
	TASK_NOT_ASSIGNABLE,
	TASK_IS_ASSIGNED_TO_LOCAL_EXECUTOR,
	ASSIGNEE_TIMEOUT,
	TASK_ABANDONED,
	LOCAL_EXECUTOR_IS_ASSIGNED,
	IS_TASK_REQUESTER
};

//------------------------------------------------------------------------------------------------
enum SCR_EUnassignReason
{
	ASSIGNEE_TIMEOUT,
	ASSIGNEE_DISCONNECT,
	ASSIGNEE_ABANDON,
	GM_REASSIGN
};
