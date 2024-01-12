// Lobby Rooms handling
//

class ServerListCallback extends BackendCallback
{
	SCR_ServerListComponent m_ServerList;
};

class OnRoomsSearch extends ServerListCallback
{
	array<Room> m_Rooms;
	override void OnSuccess(int code)
	{
		GetGame().GetBackendApi().GetClientLobby().Rooms(m_Rooms);
		Print("OnSearchServers Success");
		
		//m_ServerList.OnGetServersRoom();		
	}
	override void OnError( int code, int restCode, int apiCode )
	{
		Print("OnSearchServers Error");
	}
	
	override void OnTimeout()
	{
		Print("OnSearchServers Timeout");
	}
};

class OnRoomJoin extends ServerListCallback
{	
	override void OnSuccess(int code)
	{
		//m_ServerList.OnJoined();
		Print("OnJoinRoomSB Success");		
	}
	override void OnError( int code, int restCode, int apiCode )
	{
		Print("OnJoinRoomSB Error");
	}
	override void OnTimeout()
	{
		Print("OnJoinRoomSB Timeout");
	}
};

//
// Lobby Rooms handling

//------------------------------------------------------------------------------------------------
class SCR_ServerListComponent : SCR_ButtonListViewComponent
{	
	// Server entries behavior list list  
	protected ref array<SCR_ServerBrowserEntryComponent> m_aRoomEntries = new array<SCR_ServerBrowserEntryComponent>;
	
	// Rooms data
	protected ref array<Room> m_aRooms = new array<Room>;
	
	//-------------------------------------
	// List component override
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup server entry component behavior  
	override protected void SetupEntryBehavior(Widget entry) 
	{
		super.SetupEntryBehavior(entry);
		
		// Entry check 
		if (!entry)
			return;
		
		// Find server entry component 
		SCR_ServerBrowserEntryComponent entryComp = SCR_ServerBrowserEntryComponent.Cast(entry.FindHandler(SCR_ServerBrowserEntryComponent));
		if (!entryComp)
			return;
		
		entryComp.GetOnFocus().Insert(OnRoomEntryFocus);
		entryComp.m_OnClick.Insert(OnRoomEntryClick);
		
		// Insert to list
		m_aRoomEntries.Insert(entryComp);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fill server entry with room data
	override protected void FillEntry(Widget w) 
	{
		// Find entry
		int id = m_aEntryWidgets.Find(w);
		SCR_ServerBrowserEntryComponent roomEntry = m_aRoomEntries[id];
		
		if (!roomEntry)		
			return;
		
		// Get room 
		Room room = null;
		
		int iPos = m_iLastScrollPosition;
		
		int roomId = id + iPos;
		if (roomId < m_aRooms.Count())
			room = m_aRooms[id + iPos];
		
		// Update info 
		roomEntry.SetRoomInfo(room);
		
		// Hide entry if no room 
		w.SetVisible(room != null);
		if (!room)
			return;
		
		// Animated appearing
		if (m_bAnimateListAppearing)
		{
			int delay = 100 / m_fAnimationAppearTime * id;
			roomEntry.AnimateOpacity(delay, m_fAnimationAppearTime, 1, 0);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Clear widgets from server list 
	override protected void ClearServerList()
	{
		// Remove entyr actions  
		foreach (SCR_ServerBrowserEntryComponent entry : m_aRoomEntries)
		{
			entry.GetOnFocus().Clear();
			entry.GetOnFocusLost().Clear();
		}
		
		// Clear list 
		m_aRoomEntries.Clear();
		m_aRooms.Clear();
		
		super.ClearServerList();
	}
	
	//-------------------------------------
	// public functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void UpdateRooms(array<Room> rooms, bool animated = false)
	{
		if (!rooms)
			return;
		
		m_aRooms = rooms;
		
		// Set entries to list 
		m_iEntriesCount = rooms.Count();
		GetGame().GetCallqueue().Remove(OpacityAnimation);
		UpdateScrollbar();
		UpdateEntries(animated);
	}
	
	//-------------------------------------
	// protected functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected Room RoomByWidget(Widget w)
	{
		// Find room entyr widget id by root widget 
		int id = m_aEntryWidgets.Find(w);
		if (id < 0)
			return null;
		
		// Add scroll to get actual room 
		id += Math.Floor(m_fScrollPosition);
		
		// Get entry room
		return m_aRoomEntries[id].GetRoomInfo();
	} 
	
	//-------------------------------------
	// Invoker actions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when entry in list is focused
	protected void OnRoomEntryFocus(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);
		
		// Get entry room
		Room room = serverEntry.GetRoomInfo();
		
		if (room)
			m_OnEntryFocus.Invoke(room);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRoomEntryClick(SCR_ServerBrowserEntryComponent entry) 
	{
		m_iFocusedEntryId = m_aRoomEntries.Find(entry) + Math.Floor(m_fScrollPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ServerBrowserEntryComponent> GetRoomEntries() { return m_aRoomEntries; }
};
