//#define WAYPOINT_LINES_DEBUG

class SCR_WaypointLinesEditorUIComponent: SCR_BaseEditorUIComponent
{
	[Attribute(desc: "Show waypoint lines when a group or a waypoint has one of these states.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_States;
	
	[Attribute(desc: "Color of lines", defvalue: "0 0 0 1")]
	protected ref Color m_LineColor;
	
	[Attribute(defvalue: string.Format("%1", ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER | ShapeFlags.ONCE), desc: "Shape flags of lines.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(ShapeFlags))]
	protected ShapeFlags m_ShapeFlags;
	
	protected ref map<SCR_EditableEntityComponent, int> m_Groups = new map<SCR_EditableEntityComponent, int>; //--- ToDo: Save entities in an array if tokens prove to be unreliable
	protected int m_iLineColorPacked;
	
	protected void OnMenuUpdate(float timeSlice)
	{
		SCR_EditableEntityComponent child, prevWaypoint;
		SCR_EditableWaypointComponent waypoint;
		vector points[2];
		vector pos1, pos2;
		foreach (SCR_EditableEntityComponent group, int groupTokens: m_Groups)
		{
			if (!group)
				continue;
			
			for (int i = 0, count = group.GetChildrenCount(true); i < count; i++)
			{
				child = group.GetChild(i);
				if (child.GetEntityType() != EEditableEntityType.WAYPOINT)
					continue;
				
				waypoint = SCR_EditableWaypointComponent.Cast(child);
				prevWaypoint = waypoint.GetPrevWaypoint();
				
				if (!waypoint.GetPos(pos1) || !prevWaypoint.GetPos(pos2))
					continue;
				
				points = {pos1, pos2};
				Shape.CreateLines(m_iLineColorPacked, m_ShapeFlags, points, 2);
			}
			
		}
		
#ifdef WAYPOINT_LINES_DEBUG
		DbgUI.Begin("");
		DbgUI.Text(m_Groups.Count().ToString());
		DbgUI.End();
#endif
	}
	
	protected void OnChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
#ifdef WAYPOINT_LINES_DEBUG
		string stateName = typename.EnumToString(EEditableEntityState, state);
#endif
		
		if (entitiesRemove)
		{
			SCR_EditableEntityComponent group;
			int groupTokens;
			foreach (SCR_EditableEntityComponent entity: entitiesRemove)
			{
				group = entity.GetAIGroup();
				if (!group)
					continue;
				
				groupTokens = 0;
				if (!m_Groups.Find(group, groupTokens))
					continue;
				
				groupTokens--;
				if (groupTokens == 0)
					m_Groups.Remove(group);
				else
					m_Groups.Set(group, groupTokens);
				
#ifdef WAYPOINT_LINES_DEBUG
				entity.Log(stateName + ": " + groupTokens.ToString(), true, LogLevel.WARNING);
#endif
			}
		}
		if (entitiesInsert)
		{
			SCR_EditableEntityComponent group;
			int groupTokens;
			foreach (SCR_EditableEntityComponent entity: entitiesInsert)
			{
				group = entity.GetAIGroup();
				if (!group)
					continue;
				
				groupTokens = 0;
				m_Groups.Find(group, groupTokens);
				
				groupTokens++;
				m_Groups.Set(group, groupTokens);
				
#ifdef WAYPOINT_LINES_DEBUG
				entity.Log(stateName + ": " + groupTokens.ToString(), true);
#endif
			}
		}
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (!entitiesManager)
			return;
		
		m_iLineColorPacked = m_LineColor.PackToInt();
		
		EEditableEntityState state;
		SCR_BaseEditableEntityFilter filter;
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		array<int> states = {};
		for (int i = 0, count = SCR_Enum.BitToIntArray(m_States, states); i < count; i++)
		{
			state = states[i];
			filter = entitiesManager.GetFilter(state);
			if (!filter)
				continue;
			
			filter.GetEntities(entities);
			filter.GetOnChanged().Insert(OnChanged);
			OnChanged(state, entities, null);
		}
		
		MenuRootBase menu = GetMenu();
		if (menu)
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
	}
	override void HandlerDeattached(Widget w)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (!entitiesManager)
			return;
		
		EEditableEntityState state;
		SCR_BaseEditableEntityFilter filter;
		array<int> states = {};
		for (int i = 0, count = SCR_Enum.BitToIntArray(m_States, states); i < count; i++)
		{
			state = states[i];
			filter = entitiesManager.GetFilter(state);
			if (!filter)
				continue;
			
			filter.GetOnChanged().Remove(OnChanged);
		}
		
		MenuRootBase menu = GetMenu();
		if (menu)
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
};
