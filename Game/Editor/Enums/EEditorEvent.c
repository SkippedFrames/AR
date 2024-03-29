/** @ingroup Editor_Entities
*/ 

//! Editor event processed in components.
enum EEditorEvent
{
	NONE,
	
	INIT,
	REQUEST_OPEN,
	REQUEST_CLOSE,
	DELETE,
	
	OPEN,
	OPEN_ALL,
	PRE_ACTIVATE,
	DEACTIVATE,
	DEACTIVATE_ASYNC,
	ACTIVATE,
	ACTIVATE_ASYNC,
	POST_DEACTIVATE,
	POST_ACTIVATE,
	CLOSE,
	
	UPDATE_MODE,
	EXIT_OPERATION
};
