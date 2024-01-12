/*
File for various server browser callbacks to keep ServerBrowserMenuUI clean.
*/

//------------------------------------------------------------------------------------------------
enum EServerBrowserRequestResult
{
	SUCCESS 	= 0,
	ERROR 		= 1,
	TIMEOUT 	= 2,
};

//------------------------------------------------------------------------------------------------
//! Base server browser callback
class ServerBrowserCallback extends BackendCallback
{
	// Cache 
	protected int m_iCode = -1;
	protected int m_iRestCode = -1;
	protected int m_iApiCode = -1;
	protected EServerBrowserRequestResult m_Result = -1;
	
	// Invokers 
	ref ScriptInvoker m_OnSuccess = new ref ScriptInvoker;
	ref ScriptInvoker m_OnFail = new ref ScriptInvoker;
	ref ScriptInvoker m_OnTimeOut = new ref ScriptInvoker;
	
	ref ScriptInvoker<ServerBrowserCallback> event_OnResponse = new ref ScriptInvoker;

	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		CacheLastResponse(EServerBrowserRequestResult.SUCCESS, code);
		//CacheLastResponse(EServerBrowserRequestResult.ERROR, code);
		
		m_OnSuccess.Invoke(this); 
		
		event_OnResponse.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode) 
	{
		CacheLastResponse(EServerBrowserRequestResult.ERROR, code, restCode, apiCode);
		m_OnFail.Invoke(this, code, restCode, apiCode);
		
		event_OnResponse.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout() 
	{
		CacheLastResponse(EServerBrowserRequestResult.TIMEOUT);
		m_OnTimeOut.Invoke(this);
		
		event_OnResponse.Invoke(this);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Protected API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Save information from last response 
	protected void CacheLastResponse(EServerBrowserRequestResult result, int code = -1, int restCode = -1, int apiCode = -1)
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
	EServerBrowserRequestResult GetResultType()
	{
		return m_Result; 
	}
};

//------------------------------------------------------------------------------------------------
//! Callback for searching servers dirrecly 
class OnDirectJoinCallback extends ServerBrowserCallback
{
	protected array<Room> m_Rooms;

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{		
		array<Room> rooms = new array<Room>;
		GetGame().GetBackendApi().GetClientLobby().Target(rooms);
		
		// No server 
		if (rooms.Count() == 0)
		{
			#ifdef SB_DEBUG
			Print("[ServerBrowserMenuCallback] No room found with given parameters!", LogLevel.ERROR);
			#endif
			
			m_OnFail.Invoke();
			return;
		}
		
		// Set room to join  
		m_Rooms = rooms;
		super.OnSuccess(code);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDirectJoinCallback()
	{
		//Print("OnDirectJoinCallback recreated");
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	array<Room> GetFoundRooms() { return m_Rooms; }
};

//------------------------------------------------------------------------------------------------
//! Scripted room callback specific for single room 
class SCR_RoomCallback : SCR_BackendCallback
{
	protected Room m_Room;
	
	//------------------------------------------------------------------------------------------------
	void SetRoom(Room room)
	{
		m_Room = room;
	}
	
	//------------------------------------------------------------------------------------------------
	Room GetRoom()
	{
		return m_Room;
	}
}

//------------------------------------------------------------------------------------------------
//! Callback for joining 
class OnJoinRoomSB extends ServerBrowserCallback
{	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);
		
		#ifdef SB_DEBUG
		Print("[ServerBrowserMenuCallback] OnJoinRoomSB Success");		
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{ 
		super.OnError(code, restCode, apiCode);
		
		#ifdef SB_DEBUG
		Print("[ServerBrowserMenuCallback] OnJoinRoomSB Error");
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout() 
	{
		super.OnTimeout();
		
		#ifdef SB_DEBUG
		Print("[ServerBrowserMenuCallback] OnJoinRoomSB Timeout");
		#endif
	}
};

//------------------------------------------------------------------------------------------------
//! Parameters for joining server
class RoomJoinData extends JsonApiStruct
{	
	string m_Password = "";
	
	// Ban related data
	string scope = "";
	string type = "";
	string reason = "";
	string issuer = "";
	int expiresAt = 0;
	int createdAt = 0;
	
	void RoomJoinData()
	{
		RegV("scope");
		RegV("type");
		RegV("reason");
		RegV("issuer");
		RegV("expiresAt");
		RegV("createdAt");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		StoreString("password", m_Password);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPassword(string password) { m_Password = password; }
};
