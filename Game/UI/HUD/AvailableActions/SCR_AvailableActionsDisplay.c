//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsWidget
{
	protected Widget m_wRootWidget;
	protected OverlayWidget m_wOverlayWidget;
	protected RichTextWidget m_wRichTextWidget;
	protected TextWidget m_hintText;

	//------------------------------------------------------------------------------------------------
	void SetText(string text, string name)
	{
		if (m_wRichTextWidget)
			m_wRichTextWidget.SetText(text);


		if (m_hintText)
			m_hintText.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTextSize(int size)
	{
		if (m_wRichTextWidget)
			m_wRichTextWidget.SetDesiredFontSize(size);
		
		if (m_hintText)
			m_hintText.SetDesiredFontSize(size);
	}

	//------------------------------------------------------------------------------------------------
	void SetVisible(bool isVisible)
	{
		if (m_wRootWidget)
			m_wRootWidget.SetVisible(isVisible);
	}
	
	
	void SetPadding(float padding)
	{
		if (!m_wOverlayWidget)
			return;
		
		float left, top, right, bottom;
		OverlaySlot.GetPadding(m_wOverlayWidget, left, top, right, bottom);
		OverlaySlot.SetPadding(m_wOverlayWidget, left, padding, right, padding);
	}

	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		if (m_wRootWidget)
			return m_wRootWidget.IsVisible();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AvailableActionsWidget CreateActionsWidget(string layout, Widget parent, string text = "", string name = "")
	{
		if (layout == string.Empty)
			return null;

		if (!parent)
			return null;

		SCR_AvailableActionsWidget instance = new SCR_AvailableActionsWidget();
		if (!instance)
			return null;

		Widget root = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		instance.m_wRootWidget = root;
		if (root)
		{
			instance.m_wRichTextWidget = RichTextWidget.Cast(root.FindAnyWidget("SlotRichText"));
			instance.m_hintText = TextWidget.Cast(root.FindAnyWidget("Text"));
			instance.m_wOverlayWidget = OverlayWidget.Cast(root.FindAnyWidget("Overlay0"));
		}
		instance.SetText(text, name);

		return instance;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRootWidget; }

	//------------------------------------------------------------------------------------------------
	private void SCR_AvailableActionsWidget() {}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AvailableActionsWidget()
	{
		if (m_wRootWidget)
			m_wRootWidget.RemoveFromHierarchy();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionContextTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sAction");
		if (index == -1)
			return false;

		// Tag string
		string tag = "";
		source.Get("m_sTag", tag);

		source.Get("m_sAction", title);

		// Enabled string
		bool enabled;
		source.Get("m_bEnabled", enabled);

		string enabledStr = "x";
		if (enabled)
			enabledStr = "on";

		// Setup title string
		title = "(" + title + ") " + tag + " - " + enabledStr;
		return true;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_AvailableActionContext
{
	[Attribute("true")]
	protected bool m_bEnabled;

	[Attribute("0", UIWidgets.EditBox, "Amount of mili seconds to hide this hint, 0 and less means no hidding")]
	protected int m_iTimeForHide;

	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_AvailableActionCondition> m_aConditions;

	[Attribute("DefaultTag", UIWidgets.EditBox, "Tag for quick search and recongnition. Change default tag to allow easier search.")]
	protected string m_sTag;

	[Attribute("", UIWidgets.EditBox, "Name of the action in action manager")]
	protected string m_sAction;

	[Attribute("", UIWidgets.EditBox, "Name of the action in to be displayed in UI")]
	protected string m_sName;

	protected const string MARKUP_FORMAT = "<action name=\"%1\"/>";

	protected bool m_bActivated = false;
	protected bool m_bHideTimeOver = true;

	//------------------------------------------------------------------------------------------------
	string ToString(bool forceText = true)
	{
		string text = "";
		if (forceText)
			text = string.Format(MARKUP_FORMAT, m_sAction);
		else
			text = string.Format(MARKUP_FORMAT, m_sAction);

		return string.Format("%1", text);
	}

	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sAction;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: This should possibly be setup somewhere in the actions manager,
	// so it can be reused instead of having to rely on setting it up manually in this object?
	string GetUIName()
	{
		if (m_sName == string.Empty)
			return m_sAction;

		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!m_bEnabled)
			return false;

		bool isOk = true;
		foreach (auto cond : m_aConditions)
		{
			if (!cond.IsAvailable(data))
			{
				if (m_bActivated)
					m_bActivated = false;

				GetGame().GetCallqueue().Remove(CountHideTime);
				m_bHideTimeOver = true;

				isOk = false;
				break;
			}

			// Restart hide time counter
			if (m_iTimeForHide > 0 && m_bHideTimeOver && !m_bActivated)
			{
				m_bHideTimeOver = false;
				GetGame().GetCallqueue().Remove(CountHideTime);
				GetGame().GetCallqueue().CallLater(CountHideTime, m_iTimeForHide, false);
			}

			if (!m_bActivated)
				m_bActivated = true;
		}

		// Hide if time is over
		if (m_iTimeForHide > 0 && m_bHideTimeOver)
			isOk = false;

		return isOk;
	}

	//------------------------------------------------------------------------------------------------
	protected void CountHideTime()
	{
		m_bHideTimeOver = true;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsDisplay : SCR_InfoDisplayExtended
{
	//! List of all actions to process at any given moment, these are filtered and available ones are displayed
	[Attribute("", UIWidgets.Object, "List of all actions to process at any given moment, these are filtered and available ones are displayed")]
	protected ref array<ref SCR_AvailableActionContext> m_aActions;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout used for individual action widgets", params: "layout")]
	protected ResourceName m_sChildLayout;

	[Attribute("1", UIWidgets.CheckBox, "Should the rich text widget show action as text only (no icons)?")]
	protected bool m_bForceText;

	[Attribute()]
	protected float m_fDefaultOffsetY;

	protected float m_fOffsetY;
	protected float m_fAdditionalOffsetY = 0;

	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref AvailableActionLayoutBehaviorBase> m_aBehaviors;

	//! Count of maximum elements that will be pre-cached
	protected const int PRELOADED_WIDGETS_COUNT = 16;

	//! List of available action widget containers
	protected ref array<ref SCR_AvailableActionsWidget> m_aWidgets = new ref array<ref SCR_AvailableActionsWidget>();

	//! Layout widget in root or null if new
	protected Widget m_wLayoutWidget;

	//! Amount of previously shown widgets
	protected int m_iLastCount;

	//! Game settings
	protected bool m_bIsEnabledSettings;

	//!
	protected ref SCR_AvailableActionsConditionData m_data;
	protected SCR_InfoDisplaySlotHandler m_slotHandler;

	//! Timer for fetching data limitation
	protected float m_fDataFetchTimer;
	
	protected int m_iMaxActionsUntilResize = 5;

	const int HEIGHT_DEVIDER = 50;
	const int HINT_SIZE_Y = 34;
	const int DEFAULT_FONT_SIZE = 20;
	const int MIN_FONT_SIZE = 16;	
	const float DEFAULT_OVERLAY_PADDING = 6;
	const float MIN_OVERLAY_PADDING = 3;

	//protected ref array<SCR_AvailableActionContext> availableActions;
	//protected int actionsCount;

	//------------------------------------------------------------------------------------------------
	//! Go through the list of passed in action names of inActions
	//! and populate the outActions list with actions that are currently active (available)
	//! Returns count of available actions or -1 in case of error
	protected int GetAvailableActions(SCR_AvailableActionsConditionData data, array<ref SCR_AvailableActionContext> inActions, out array<SCR_AvailableActionContext> outActions)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;

		InputManager inputManager = game.GetInputManager();
		if (!inputManager)
			return 0;

		outActions.Clear();
		int count = 0;
		foreach (auto action : inActions)
		{
			auto actionName = action.GetActionName();
			if (actionName == string.Empty)
				continue;

			if (inputManager.IsActionActive(actionName) && action.IsAvailable(data))
			{
				outActions.Insert(action);
				count++;
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeShown(IEntity controlledEntity)
	{
		if (SCR_EditorManagerEntity.IsOpenedInstance())
			return true;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.IsUnconscious() || controller.IsDead())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateIsEnabled()
	{
		// In case settings are not found, fallback to enabled
		m_bIsEnabledSettings = true;

		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
			gameplaySettings.Get("m_bControlHints", m_bIsEnabledSettings);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return; // Mandatory

		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		
		//~ Can show checks if controlledEntity is valid or if editor is open
		if (m_bIsEnabledSettings && CanBeShown(controlledEntity))
			m_wRoot.SetVisible(true);
		else
		{
			m_wRoot.SetVisible(false);
			return;
		}

		if (!m_data)
			m_data = new SCR_AvailableActionsConditionData();

		m_fDataFetchTimer += timeSlice;

		if (m_fDataFetchTimer >= 0.25)
		{
			m_data.FetchData(controlledEntity, m_fDataFetchTimer);
			DisplayWidgetsUpdate();

			m_fDataFetchTimer = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	private void DisplayWidgetsUpdate()
	{
		array<SCR_AvailableActionContext> availableActions = new array<SCR_AvailableActionContext>();
		int actionsCount = GetAvailableActions(m_data, m_aActions, availableActions);

		// Enable additional ones
		if (actionsCount > m_iLastCount)
		{
			for (int i = m_iLastCount; i < actionsCount; i++)
			{
				// OOR
				if (i >= m_aWidgets.Count())
					break;

				if (m_aWidgets[i])
					DisplayHint(m_aWidgets[i].GetRootWidget(), UIConstants.FADE_RATE_SUPER_FAST, UIConstants.FADE_RATE_SUPER_FAST);
			}
		}
		// Or hide previously shown
		else
		{
			for (int i = actionsCount; i < m_iLastCount; i++)
			{
				// OOR
				if (i >= m_aWidgets.Count())
					break;

				/*if (m_aWidgets[i])
					m_aWidgets[i].SetVisible(false);*/
				if (m_aWidgets[i])
					HintFadeOut(m_aWidgets[i].GetRootWidget(), UIConstants.FADE_RATE_DEFAULT, UIConstants.FADE_RATE_FAST);
			}
		}

		for (int i = 0; i < actionsCount; i++)
		{
			// OOR
			if (i >= m_aWidgets.Count())
				break;

			m_bForceText = false;

			if (m_aWidgets[i] && availableActions[i])
				m_aWidgets[i].SetText(availableActions[i].ToString(m_bForceText), availableActions[i].GetUIName());
			
			if (actionsCount > m_iMaxActionsUntilResize)
			{
				m_aWidgets[i].SetTextSize(MIN_FONT_SIZE);
				m_aWidgets[i].SetPadding(MIN_OVERLAY_PADDING);
			}	
			else
			{
				m_aWidgets[i].SetTextSize(DEFAULT_FONT_SIZE);
				m_aWidgets[i].SetPadding(DEFAULT_OVERLAY_PADDING);
			}
		}

		// Acknowledge new count
		m_iLastCount = actionsCount;
			

		ApplyLayoutBehavior();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayHint(Widget widget, float delayFade, float delayShrink)
	{
		widget.SetOpacity(0);
		VerticalLayoutSlot.SetPadding(widget, 0, -HINT_SIZE_Y, 0, 0);
		widget.SetVisible(true);

		// Clear hiding
		GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);

		// Animations
		float padding[4] = {0, 0, 0, 0};
		AnimateWidget.Padding(widget, padding, delayShrink);
		AnimateWidget.Opacity(widget, 1, delayFade);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintFadeOut(Widget widget, float delayFade, float delayShrink)
	{
		/*GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);*/

		AnimateWidget.Opacity(widget, 0, delayFade);
		GetGame().GetCallqueue().CallLater(HintShrink, 1000 / delayFade, false, widget, delayShrink);
		//HintShrink(widget, delayShrink);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintShrink(Widget widget, float delayShrink)
	{
		float padding[4] = {0, -HINT_SIZE_Y, 0, 0};
		AnimateWidget.Padding(widget, padding, delayShrink);
		GetGame().GetCallqueue().CallLater(HintHide, 1000 / delayShrink, false, widget);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintHide(Widget widget)
	{
		widget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Set available actions layout y position
	void SetOffsetY(float offset = -1)
	{
		return;
		// Default position
		if (offset == -1)
			offset = m_fDefaultOffsetY;

		m_fOffsetY = offset;

		// Set layout position
		//ApplyOffsets();
	}

	//------------------------------------------------------------------------------------------------
	void SetAdditionalOffsetY(float offset)
	{
		return;
		m_fAdditionalOffsetY = offset;
		//ApplyOffsets();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyOffsets()
	{
		return;
		float sum = (-m_fOffsetY) + (-m_fAdditionalOffsetY);

		if (m_wLayoutWidget)
			FrameSlot.SetPosY(m_wLayoutWidget, sum);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)
	{
		if (m_wRoot)
			m_wLayoutWidget = m_wRoot.FindAnyWidget("VerticalLayout");

		// Layout setup
		SetOffsetY();

		int count = m_aWidgets.Count();
		int toGenerate = PRELOADED_WIDGETS_COUNT - count;
		for (int i = 0; i < toGenerate; i++)
		{
			if (i < toGenerate)
			{
				SCR_AvailableActionsWidget widgetContainer = SCR_AvailableActionsWidget.CreateActionsWidget(m_sChildLayout, m_wLayoutWidget, "");
				if (widgetContainer)
					widgetContainer.SetVisible(false);

				m_aWidgets.Insert(widgetContainer);
			}

			if (m_aWidgets[i])
				m_aWidgets[i].SetVisible(false);
		}
		
		m_slotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if(m_slotHandler)
			m_slotHandler.GetSlotUIComponent().GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		m_aWidgets.Clear();
		m_wLayoutWidget = null;
		m_iLastCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayInit(IEntity owner)
	{		
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController)
		{
			Print("SCR_AvailableActionsDisplay is not an object in HUDManagerComponent attached onto a PlayerController entity! May result in undefined behaviour.", LogLevel.WARNING);
		}

		UpdateIsEnabled();
		GetGame().OnUserSettingsChangedInvoker().Insert(UpdateIsEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSlotUIResize()
	{
		m_iMaxActionsUntilResize = (int)m_slotHandler.GetSlotUIComponent().GetHeight() / HEIGHT_DEVIDER;
		if (m_iMaxActionsUntilResize < 1)
			m_iMaxActionsUntilResize = 1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyLayoutBehavior()
	{
		AvailableActionLayoutBehaviorBase selectedBehavior;

		// Go through each behavior
		foreach (AvailableActionLayoutBehaviorBase beh : m_aBehaviors)
		{
			if (beh.ConditionsChecked(this))
			{
				// Default
				if (!selectedBehavior)
				{
					selectedBehavior = beh;
					continue;
				}

				// Select this behavior if it has bigger priority
				if (selectedBehavior.m_iPriority < beh.m_iPriority)
					selectedBehavior = beh;
			}
		}

		if (selectedBehavior)
			selectedBehavior.ApplyBehavior(this);
		else
			SetOffsetY();
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionLayoutBehaviorBase
{
	[Attribute("")]
	float m_fOffsetY;

	[Attribute("0")]
	int m_iPriority;

	//------------------------------------------------------------------------------------------------
	bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		display.SetOffsetY(m_fOffsetY);
	}
};

//------------------------------------------------------------------------------------------------
//! Variables that should be applied on available actions layout whenever given HUD is active
[BaseContainerProps()]
class AvailableActionLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	ResourceName m_sCheckHUD;

	[Attribute("0")]
	protected bool m_bOffsetFromDisplay;

	[Attribute("0")]
	protected float m_fAddOffset;

	protected float m_fAutoOffset;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		SCR_HUDManagerComponent HUDManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!HUDManager)
			return false;

		Widget hud = HUDManager.FindLayoutByResourceName(m_sCheckHUD);
		if (!hud)
			return false;

		// Callculate auto offset
		if (m_bOffsetFromDisplay)
		{
			float w;

			SCR_InfoDisplay hudCmp = HUDManager.FindInfoDisplayByResourceName(m_sCheckHUD);
			if (hudCmp)
				hudCmp.GetDimensions(w, m_fAutoOffset);
		}

		return (!SCR_EditorManagerEntity.IsOpenedInstance() && hud && hud.IsEnabled());
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		// Manual offset
		if (!m_bOffsetFromDisplay)
		{
			super.ApplyBehavior(display);
			return;
		}

		// Auto offset from check HUD
		display.SetOffsetY(m_fAutoOffset + m_fAddOffset);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionMenuLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("0", UIWidgets.ComboBox, "Is this menu active", "", ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	ChimeraMenuPreset m_ActiveMenu;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		return (GetGame().GetMenuManager().FindMenuByPreset(m_ActiveMenu) != null);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionEditorLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("1", UIWidgets.ComboBox, "In which mode should be this behavior applied", "", ParamEnumArray.FromEnum(EEditorMode))]
	EEditorMode m_eEditorMode;

	[Attribute("0", desc: "Additional offset applied when editor is no legal in given scenario.")]
	protected float m_fIllegalEditorOffset;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return false;

		EEditorMode mode = editorManagerEntity.GetCurrentMode();

		return (SCR_EditorManagerEntity.IsOpenedInstance() && mode == m_eEditorMode);
	}
	override void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		float offset = m_fOffsetY;

		if (m_fIllegalEditorOffset != 0)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
			{
				SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
				if (editorSettings && !editorSettings.IsUnlimitedEditorLegal())
					offset += m_fIllegalEditorOffset;
			}
		}

		display.SetOffsetY(offset);
	}
};
