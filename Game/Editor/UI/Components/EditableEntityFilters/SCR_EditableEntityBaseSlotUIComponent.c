/** @ingroup Editor_Editable_Entity_UI
*/
/*!
Slot for icons representing editable entity.
One exists for every visible editable entity.
Attached to Frame widget inside of which icon widgets are created (one for each active SCR_EntitiesEditorUIRule).
*/
class SCR_EditableEntityBaseSlotUIComponent: ScriptedWidgetComponent
{		
	protected Widget m_Widget;
	protected SCR_EditableEntityComponent m_Entity;
	protected ref map<SCR_EntitiesEditorUIRule, SCR_BaseEditableEntityUIComponent> m_EntityWidgets = new map<SCR_EntitiesEditorUIRule, SCR_BaseEditableEntityUIComponent>;

	/*!
	Check if the slot is empty, i.e., doesn't contain icons for any rule.
	\return True when empty
	*/
	bool IsEmpty()
	{
		return m_EntityWidgets.IsEmpty();
	}
	/*!
	Get entity this slot represents.
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetEntity()
	{
		return m_Entity;
	}
	/*!
	Create icon from UI info and a rule.
	\param info UI info representing the entity
	\rule Visualization rule
	\return Created widget
	*/
	Widget CreateWidget(SCR_EditableEntityUIInfo info, SCR_EntitiesEditorUIRule rule)
	{
		ResourceName layout = rule.GetLayout(info.GetEntityType());		
		SCR_BaseEditableEntityUIComponent entityUI = CreateWidgetFromLayout(layout);
		entityUI.Init(info, this);
		return entityUI.GetWidget();
	}
	/*!
	Create icon from editable entity and a rule.
	\param entity Editable entity
	\rule Visualization rule
	\return Created widget
	*/
	Widget CreateWidget(SCR_EditableEntityComponent entity, SCR_EntitiesEditorUIRule rule)
	{
		ResourceName layout = rule.GetLayout(entity.GetEntityType());
		
		return CreateWidgetForEntity(entity, rule, layout);
	}
	/*!
	Create icon from editable entity and layout path.
	\param entity Editable entity
	\rule layout Path to the layout
	\return Created widget
	*/
	Widget CreateWidget(SCR_EditableEntityComponent entity, ResourceName layout)
	{
		return CreateWidgetForEntity(entity, null, layout);
	}
	
	protected Widget CreateWidgetForEntity(SCR_EditableEntityComponent entity, SCR_EntitiesEditorUIRule rule, ResourceName layout)
	{
		//--- There is already a widget for this rule, ignore
		if (m_EntityWidgets.Contains(rule))
			return null;
		
		if (!layout)
			return null;
		
		SCR_BaseEditableEntityUIComponent entityUI = CreateWidgetFromLayout(layout);
		entityUI.Init(entity, this);		
		m_EntityWidgets.Insert(rule, entityUI);
		
		return entityUI.GetWidget();
	}
	protected SCR_BaseEditableEntityUIComponent CreateWidgetFromLayout(ResourceName layout)
	{
		Widget entityWidget = GetGame().GetWorkspace().CreateWidgets(layout, m_Widget);
		if (!entityWidget)
		{
			Debug.Error2(Type().ToString(), string.Format("Unable to create widget '%1'!\n", layout));
			return null;
		}
		
		//--- Get UI component
		SCR_BaseEditableEntityUIComponent entityUI = SCR_BaseEditableEntityUIComponent.Cast(entityWidget.FindHandler(SCR_BaseEditableEntityUIComponent));
		if (!entityUI)
		{
			Debug.Error2(Type().ToString(), string.Format("SCR_BaseEditableEntityUIComponent component is missing in the root widget of '%1'!\n", layout));
			if (entityWidget)
			{
				entityWidget.RemoveFromHierarchy();
				entityWidget = null;
			}
			return null;
		}
		return entityUI;
	}
	void DeleteWidget(SCR_EntitiesEditorUIRule rule)
	{
		SCR_BaseEditableEntityUIComponent entityUI;
		if (!m_EntityWidgets.Find(rule, entityUI)) return;
		entityUI.Exit(this);
		m_EntityWidgets.Remove(rule);
	}
	protected void SetFactionColor()
	{
		Faction faction = m_Entity.GetFaction();
		if (faction)
			GetWidget().SetColor(faction.GetFactionColor());
		else if (m_Entity.HasEntityState(EEditableEntityState.DESTROYED))
			GetWidget().SetColor(UIColors.EDITOR_ICON_COLOR_DESTROYED);
		else
			GetWidget().SetColor(UIColors.EDITOR_ICON_COLOR_NEUTRAL);
			
	}
	protected void RefreshSlot()
	{
		foreach (SCR_EntitiesEditorUIRule rule, SCR_BaseEditableEntityUIComponent entityUI: m_EntityWidgets)
		{
			entityUI.OnRefresh(this);
		}
		GetGame().GetCallqueue().CallLater(SetFactionColor);
	}
	protected void ResetSlot()
	{
		foreach (SCR_EntitiesEditorUIRule rule, SCR_BaseEditableEntityUIComponent entityUI: m_EntityWidgets)
		{
			DeleteWidget(rule);
			CreateWidget(m_Entity, rule);
		}
		GetGame().GetCallqueue().CallLater(SetFactionColor);
	}
	vector UpdateSlot(int screenW, int screenH, vector posCenter, vector posCam)
	{
		return vector.Zero;
	}
	void InitSlot(SCR_EditableEntityComponent entity)
	{
		if (!entity || !entity.GetOwner()) return;
		
		m_Entity = entity;
		
		//--- Initialize faction color (set on the slot, child widgets inherit from it, so they don't need to set it individually)
		if (entity.HasEntityFlag(EEditableEntityFlag.HAS_FACTION))
		{
			SetFactionColor();
		}
		
		if (entity.GetOnUIRefresh())
			entity.GetOnUIRefresh().Insert(RefreshSlot);
		
		if (entity.GetOnUIReset())
			entity.GetOnUIReset().Insert(ResetSlot);
	}
	void DeleteSlot()
	{
		if (m_Widget)
		{
			m_Widget.RemoveFromHierarchy();
			m_Widget = null;
		}
		if (m_Entity)
		{		
			if (m_Entity.GetOnUIRefresh())
				m_Entity.GetOnUIRefresh().Remove(RefreshSlot);
	
			if (m_Entity.GetOnUIReset())
				m_Entity.GetOnUIReset().Remove(ResetSlot);
		}
	}
	bool IsPreview()
	{
		return false;
	}
	Widget GetWidget()
	{
		return m_Widget;
	}
	override void HandlerAttached(Widget w)
	{
		m_Widget = w;
	}
};
