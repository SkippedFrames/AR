/*!

Scripted backend callback class base for simple handling of basic responses:
	- success, error, timeout

The class is unifying responses type into single response and different them with cashed response enum.
Each response type can be listened separately.
*/

//------------------------------------------------------------------------------------------------
//! Basic callback responses 
enum EBackendCallbackResponse
{
	SUCCESS,
	ERROR,
	TIMEOUT,
};

//------------------------------------------------------------------------------------------------
//! Scripted backend callback class unifying backend response 
class SCR_BackendCallback : BackendCallback
{
	// Cache 
	protected int m_iCode = -1;
	protected int m_iRestCode = -1;
	protected int m_iApiCode = -1;
	protected EBackendCallbackResponse m_Result = -1;
	
	// Invokers 
	protected ref ScriptInvoker<SCR_BackendCallback> Event_OnSuccess;
	protected ref ScriptInvoker<SCR_BackendCallback, int, int, int> Event_OnFail;
	protected ref ScriptInvoker<SCR_BackendCallback> Event_OnTimeOut;
	
	protected ref ScriptInvoker<SCR_BackendCallback> Event_OnResponse;

	//------------------------------------------------------------------------------------------------
	// Invokers api
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnSuccess()
	{
		if (Event_OnSuccess)
			Event_OnSuccess.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnSuccess()
	{
		if (!Event_OnSuccess)
			Event_OnSuccess = new ScriptInvoker();

		return Event_OnSuccess;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnFail(SCR_BackendCallback callback, int code, int restCode, int apiCode)
	{
		if (Event_OnFail)
			Event_OnFail.Invoke(callback, code, restCode, apiCode);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnFail()
	{
		if (!Event_OnFail)
			Event_OnFail = new ScriptInvoker();

		return Event_OnFail;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnTimeOut()
	{
		if (Event_OnTimeOut)
			Event_OnTimeOut.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnTimeOut()
	{
		if (!Event_OnTimeOut)
			Event_OnTimeOut = new ScriptInvoker();

		return Event_OnTimeOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnResponse()
	{
		if (Event_OnResponse)
			Event_OnResponse.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnResponse()
	{
		if (!Event_OnResponse)
			Event_OnResponse = new ScriptInvoker();

		return Event_OnResponse;
	}

	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		CacheLastResponse(EBackendCallbackResponse.SUCCESS, code);

		InvokeEventOnSuccess(); 
		
		InvokeEventOnResponse();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode) 
	{
		CacheLastResponse(EBackendCallbackResponse.ERROR, code, restCode, apiCode);
		InvokeEventOnFail(this, code, restCode, apiCode);
		
		InvokeEventOnResponse();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout() 
	{
		CacheLastResponse(EBackendCallbackResponse.TIMEOUT);
		InvokeEventOnTimeOut();
		
		InvokeEventOnResponse();
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Protected API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Save information from last response 
	protected void CacheLastResponse(EBackendCallbackResponse result, int code = -1, int restCode = -1, int apiCode = -1)
	{
		m_Result = result;
		m_iCode = code;
		m_iRestCode = restCode;
		m_iApiCode = m_iApiCode;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Getter API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	int GetCode()
	{
		return m_iCode; 
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRestCode()
	{
		return m_iRestCode; 
	}
	
	//------------------------------------------------------------------------------------------------
	int GetApiCode()
	{
		return m_iApiCode; 
	}
	
	//------------------------------------------------------------------------------------------------
	EBackendCallbackResponse GetResponseType()
	{
		return m_Result; 
	}
};
