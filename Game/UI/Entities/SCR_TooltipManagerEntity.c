/*!
Entity which manages custom tooltips.
This custom solution was implemented before the changes to the built in tooltips API (ScriptedWidgetTooltip), and is now used to provide gamepad triggered tooltips togheter with SCR_BrowserHoverTooltipComponent, as they are not yet available in ScriptedWidgetTooltip.
*/

class SCR_TooltipManagerEntityClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_TooltipManagerEntity : GenericEntity
{
	protected static SCR_TooltipManagerEntity s_Instance = null;

	protected bool m_bFollowCursor;											// When true, tooltip will be following cursor
	protected Widget m_wTooltip;											// The tooltip widget
	protected ref SCR_ScriptedWidgetTooltipPreset m_Preset;					// The tooltip settings
	protected Widget m_wHoverWidget;										// When cursor leaves this widget, or this widget gets destroyed, current tooltip will be reset
	protected SCR_ETooltipAlignmentHorizontal m_eHorizontalAlignment;		// Tooltip position in relation to the hover Widget
	protected SCR_ETooltipAlignmentVertical m_eVerticalAlignment;			// Tooltip position in relation to the hover Widget

	protected bool m_bIsRendered;
	protected Widget m_wInteractableWidget;	// Currently focused or hovered widget
	protected WorkspaceWidget m_wWorkspace;

	const int TOOLTIP_Z_ORDER = 10000;	// Z order of tooltip. Tooltip widget is created in Workspace and must be above other widgets.

	//------------------------------------------------------------------------------------------------
	// PUBLIC
	//------------------------------------------------------------------------------------------------
	static SCR_TooltipManagerEntity GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Creates a tooltip from a .layout resource. Previous tooltip is deleted.
	- preset - settings of the tooltip
	- hoverWidget - widget over which the cursor must stay.
	When cursor leaves the widget, or widget is deleted, tooltip is deleted automatically.
	*/
	static Widget CreateTooltip(notnull SCR_ScriptedWidgetTooltipPreset preset, notnull Widget hoverWidget)
	{
		SCR_TooltipManagerEntity ent = GetInstance();

		if (!ent)
			return null;

		return ent.Internal_CreateTooltip(preset, hoverWidget);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Deletes current tooltip, if it exists.
	*/
	static void DeleteTooltip()
	{
		SCR_TooltipManagerEntity ent = GetInstance();

		if (!ent)
			return;

		ent.Internal_DeleteTooltip();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Returns current tooltip widget, or null if there is none.
	*/
	static Widget GetTooltip()
	{
		SCR_TooltipManagerEntity ent = GetInstance();

		if (!ent)
			return null;

		return ent.m_wTooltip;
	}

	//------------------------------------------------------------------------------------------------
	// PROTECTED
	//------------------------------------------------------------------------------------------------
	protected Widget Internal_CreateTooltip(notnull SCR_ScriptedWidgetTooltipPreset preset, notnull Widget hoverWidget)
	{
		m_Preset = preset;
		m_wHoverWidget = hoverWidget;
		m_bFollowCursor = m_Preset.m_bFollowCursor;
		m_bIsRendered = false;

		// Delete the old tooltip
		DeleteTooltip();

		m_wWorkspace = GetGame().GetWorkspace();
		m_wTooltip = m_wWorkspace.CreateWidgets(preset.m_sContentLayout, m_wWorkspace);

		if (m_wTooltip)
			m_wTooltip.SetZOrder(TOOLTIP_Z_ORDER);

		UpdatePosition(true);

		// Activate lazily - only when creating some tooltip
		SetEventMask(EntityEvent.FRAME);

		return m_wTooltip;
	}

	//------------------------------------------------------------------------------------------------
	protected void Internal_DeleteTooltip()
	{
		if (m_wTooltip)
			GetGame().GetWorkspace().RemoveChild(m_wTooltip);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_wWorkspace = GetGame().GetWorkspace();

		// Bail, deactivate, delete tooltip if hover widget is different or is gone
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			m_wInteractableWidget = WidgetManager.GetWidgetUnderCursor();
		else
			m_wInteractableWidget = m_wWorkspace.GetFocusedWidget();

		if (!m_wHoverWidget || m_wHoverWidget != m_wInteractableWidget)
		{
			Internal_DeleteTooltip();
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		// Bail, deactivate if there is no tooltip any more (smth else deleted it or we deleted it)
		if (!m_wTooltip)
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		// Check for first render
		if (!m_bIsRendered)
		{
			// Check if the widget has been rendered
			float tooltipSizeX, tooltipSizeY;
			m_wTooltip.GetScreenSize(tooltipSizeX, tooltipSizeY);
			m_bIsRendered = tooltipSizeX != 0 && tooltipSizeY != 0;

			UpdatePosition(true);
			return;
		}

		// Update current tooltip position if required
		if (m_bFollowCursor)
			UpdatePosition();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePosition(bool force = false)
	{
		vector goalPosition, goalAlignment, goalAlpha;
		bool shouldUpdate = m_Preset.GetGoalPosition(m_wTooltip, m_wHoverWidget, goalPosition, goalAlignment, goalAlpha, force);
		if (!shouldUpdate)
			return;

		FrameSlot.SetAlignment(m_wTooltip, goalAlignment[0], goalAlignment[1]);
		FrameSlot.SetPos(m_wTooltip, goalPosition[0], goalPosition[1]);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_TooltipManagerEntity(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_TooltipManagerEntity()
	{
		Internal_DeleteTooltip();
	}
}
