[BaseContainerProps(configRoot: true)]
class SCR_HUDLayout
{
	// This map stores the Group Widgets by their Widget Name
	protected ref map<string, ref Widget> m_mGroups = new map<string, ref Widget>();
	protected ref array<ref SCR_HUDElement> m_aElements = {};

	[Attribute()]
	protected string m_sIdentifier;

	[Attribute(params: "layout")]
	protected ResourceName m_sLayout;

	protected Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	int GetHUDElements(notnull out array<SCR_HUDElement> hudElements)
	{
		foreach (SCR_HUDElement element : m_aElements)
		{
			hudElements.Insert(element);
		}

		return hudElements.Count();
	}

	//------------------------------------------------------------------------------------------------
	string GetIdentifier()
	{
		return m_sIdentifier;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayout()
	{
		return m_sLayout;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	void SetRootWidget(notnull Widget widget)
	{
		m_wRoot = widget;

		Widget iteratedWidget = m_wRoot.GetChildren();
		while (iteratedWidget)
		{
			Widget iteratedChildWidget = iteratedWidget.GetChildren();
			while (iteratedChildWidget)
			{
				SCR_HUDGroupUIComponent groupComponent = SCR_HUDGroupUIComponent.Cast(iteratedChildWidget.FindHandler(SCR_HUDGroupUIComponent));
				if (!groupComponent)
				{
					iteratedChildWidget = iteratedChildWidget.GetSibling();
					continue;
				}

				m_mGroups.Insert(iteratedWidget.GetName(), iteratedChildWidget);
				iteratedChildWidget = iteratedChildWidget.GetSibling();
			}

			iteratedWidget = iteratedWidget.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddHudElement(notnull SCR_HUDElement element, bool replaceParent = false)
	{
		Widget slotWidget = element.GetWidget();
		if (!slotWidget)
			return;

		Widget parentWidget = m_wRoot.FindAnyWidget(element.GetParentWidgetName());
		if (!parentWidget)
			return;

		if (replaceParent)
			parentWidget.AddChild(slotWidget);

		m_aElements.Insert(element);
		element.SetParentLayout(this);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveHudElement(notnull SCR_HUDElement element, bool replaceParent = false)
	{
		Widget elementWidget = element.GetWidget();
		if (!elementWidget)
			return;

		Widget parentWidget = elementWidget.GetParent();
		if (replaceParent && parentWidget)
			parentWidget.RemoveChild(elementWidget);
		
		m_aElements.RemoveItem(element);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns a Group Widget with the given name.
	\param groupName Name of the group to look for.
	*/
	Widget GetGroupWidgetByName(string groupName)
	{
		return m_wRoot.FindAnyWidget(groupName);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns the Component responsible for managing and controlling a Group with the given name.
	\param groupName Name of the group to look for.
	*/
	SCR_HUDGroupUIComponent GetGroupComponent(string groupName)
	{
		Widget group = GetGroupWidgetByName(groupName);
		if (!group)
			return null;
		return SCR_HUDGroupUIComponent.Cast(group.FindHandler(SCR_HUDGroupUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Fills the provided array with all the GroupComponents this HUD Layout owns.
	\param groupComponents the array to be filled.
	*/
	int GetAllGroupComponents(notnull out array<SCR_HUDGroupUIComponent> groupComponents)
	{
		groupComponents.Clear();
		foreach (Widget groupWidget : m_mGroups)
		{
			SCR_HUDGroupUIComponent groupComponent = SCR_HUDGroupUIComponent.Cast(groupWidget.FindHandler(SCR_HUDGroupUIComponent));
			if (groupComponent)
				groupComponents.Insert(groupComponent);
		}

		return groupComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Fills the provided array with all the Slots that exist within this HUD Layout.
	\param slotComponents the array to be filled.
	*/
	int GetAllSlotComponents(notnull out array<SCR_HUDSlotUIComponent> slotComponents)
	{
		slotComponents.Clear();

		foreach (Widget groupWidget : m_mGroups)
		{
			Widget childWidget = groupWidget.GetChildren();
			while (childWidget)
			{
				SCR_HUDSlotUIComponent slotComponent = SCR_HUDSlotUIComponent.Cast(childWidget.FindHandler(SCR_HUDSlotUIComponent));
				if (!slotComponent)
				{
					childWidget = childWidget.GetSibling();
					continue;
				}

				slotComponents.Insert(slotComponent);
				childWidget = childWidget.GetSibling();
			}
		}

		return slotComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent FindSlotComponent(string slotName)
	{
		Widget slotWidget = m_wRoot.FindAnyWidget(slotName);
		if (!slotWidget)
			return null;

		return SCR_HUDSlotUIComponent.Cast(slotWidget.FindHandler(SCR_HUDSlotUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	Widget FindWidgetByName(string widgetName)
	{
		return m_wRoot.FindAnyWidget(widgetName);
	}
}
