void OnLayoutChangeMethod(SCR_HUDLayout newLayout, SCR_HUDLayout oldLayout, SCR_HUDManagerComponent hudManager);
typedef func OnLayoutChangeMethod;
typedef ScriptInvokerBase<OnLayoutChangeMethod> OnLayoutChange;


[BaseContainerProps()]
class SCR_HUDManagerLayoutHandler : SCR_HUDManagerHandler
{
	[Attribute()]
	protected ref array<ref SCR_HUDLayout> m_aHUDLayouts;

	[Attribute()]
	protected string m_sMainLayout;

	protected SCR_HUDLayout m_ActiveLayout;

	protected ref OnLayoutChange m_OnLayoutChange;

	//------------------------------------------------------------------------------------------------
	OnLayoutChange GetOnLayoutChange()
	{
		if (!m_OnLayoutChange)
			m_OnLayoutChange = new OnLayoutChange();

		return m_OnLayoutChange;
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDLayout GetActiveLayout()
	{
		return m_ActiveLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAllHudElements(notnull out array<SCR_HUDElement> hudElements)
	{
		hudElements.Clear();
		
		foreach (SCR_HUDLayout hudLayout : m_aHUDLayouts)
		{
			hudLayout.GetHUDElements(hudElements);
		}
		
		return hudElements.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns a Group Widget with the given name.
	\param groupName Name of the group to look for.
	*/
	Widget GetGroupByName(string groupName)
	{
		if (!m_ActiveLayout)
			return null;

		return m_ActiveLayout.GetGroupWidgetByName(groupName);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns the Component responsible for managing and controlling a Group with the given name.
	\param groupName Name of the group to look for.
	*/
	SCR_HUDGroupUIComponent GetGroupComponent(string groupName)
	{
		if (!m_ActiveLayout)
			return null;

		return m_ActiveLayout.GetGroupComponent(groupName);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns the Component responsible for managing and controlling a Slot with the given name.
	\param slotName Name of the slot to look for.
	*/
	SCR_HUDSlotUIComponent GetSlotComponentByName(string slotName)
	{
		if (!m_ActiveLayout)
			return null;

		return m_ActiveLayout.FindSlotComponent(slotName);
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent FindSlotComponentFromAnyLayout(string slotName, out SCR_HUDLayout foundLayout = null)
	{
		SCR_HUDSlotUIComponent slotToReturn = GetSlotComponentByName(slotName);
		if (!slotToReturn)
		{
			foreach (SCR_HUDLayout layout : m_aHUDLayouts)
			{
				slotToReturn = layout.FindSlotComponent(slotName);
				if (slotToReturn)
				{
					foundLayout = layout;
					return slotToReturn;
				}
			}
		}

		foundLayout = m_ActiveLayout;
		return slotToReturn;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Searches for and returns a widget with the specified name from the Active Layout.
	\param widgetName Name of the slot to look for.
	*/
	Widget FindWidgetByName(string widgetName)
	{
		if (!m_ActiveLayout)
			return null;

		return m_ActiveLayout.FindWidgetByName(widgetName);
	}

	//------------------------------------------------------------------------------------------------
	Widget FindWidgetByNameFromAnyLayout(string widgetName, out SCR_HUDLayout foundLayout = null)
	{
		Widget foundWidget = FindWidgetByName(widgetName);
		if (foundWidget)
		{
			foundLayout = m_ActiveLayout;
			return foundWidget;
		}
		
		foreach (SCR_HUDLayout hudLayout : m_aHUDLayouts)
		{
			foundWidget = hudLayout.FindWidgetByName(widgetName);
			if (foundWidget)
			{
				foundLayout = hudLayout;
				return foundWidget;
			}
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDLayout FindHUDLayout(string layoutIdentifier)
	{
		foreach (SCR_HUDLayout hudLayout : m_aHUDLayouts)
		{
			if (hudLayout.GetIdentifier() == layoutIdentifier)
				return hudLayout;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeHUDLayouts()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		foreach (SCR_HUDLayout hudLayout : m_aHUDLayouts)
		{
			Widget layoutWidget = workspace.CreateWidgets(hudLayout.GetLayout(), null);
			hudLayout.SetRootWidget(layoutWidget);

			if (hudLayout.GetIdentifier() == m_sMainLayout)
			{
				m_ActiveLayout = hudLayout;
				continue;
			}

			layoutWidget.SetVisible(false);
			layoutWidget.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ChangeActiveHUDLayout(string layoutIdentifier)
	{
		SCR_HUDLayout newLayout = FindHUDLayout(layoutIdentifier);
		if (!newLayout || newLayout == m_ActiveLayout)
			return;

		if (m_ActiveLayout)
			SetLayoutEnabled(m_ActiveLayout.GetIdentifier(), false);
		
		SCR_HUDLayout oldActiveLayout = m_ActiveLayout;
		m_ActiveLayout = newLayout;
		SetLayoutEnabled(m_ActiveLayout.GetIdentifier(), true);
		
		array<SCR_HUDElement> allHudElements = {};
		GetAllHudElements(allHudElements);
		foreach (SCR_HUDElement element : allHudElements)
		{
			Widget elementWidget = element.GetWidget();
			if (!elementWidget)
				continue;

			Widget oldParentWidget = elementWidget.GetParent();
			Widget newParentWidget = newLayout.FindWidgetByName(element.GetParentWidgetName());
			
			if (!newParentWidget)
			{
				elementWidget.SetVisible(false);
				elementWidget.SetEnabled(false);

				continue;
			}
			
			elementWidget.SetVisible(true);
			elementWidget.SetEnabled(true);

			SCR_HUDLayout oldLayout = element.GetParentLayout();
			if (oldLayout)
				oldLayout.RemoveHudElement(element, true);

			newLayout.AddHudElement(element, true);
		}

		if (m_OnLayoutChange)
				m_OnLayoutChange.Invoke(newLayout, oldActiveLayout, m_HUDManager);
	}

	//------------------------------------------------------------------------------------------------
	void SetLayoutEnabled(string layoutIdentifier, bool isEnabled)
	{
		SCR_HUDLayout hudLayout = FindHUDLayout(layoutIdentifier);
		if (!hudLayout)
			return;

		Widget hudWidget = hudLayout.GetRootWidget();
		if (!hudWidget)
			return;

		hudWidget.SetVisible(isEnabled);
		hudWidget.SetEnabled(isEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void StartMainLayout()
	{
		if (m_HUDManager.GetHUDElements().IsEmpty())
			return;

		ChangeActiveHUDLayout(m_sMainLayout);
		m_HUDManager.RemoveHandlerFromUpdatableHandlers(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(notnull SCR_HUDManagerComponent owner)
	{
		super.OnInit(owner);

		InitializeHUDLayouts();
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(notnull SCR_HUDManagerComponent owner)
	{
		StartMainLayout();
	}

}
