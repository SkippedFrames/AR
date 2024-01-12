/*
Base class for Scenario and Server list menu entries
Child classes:
- SCR_ContentBrowser_ScenarioLineComponent
- SCR_ServerBrowserEntryComponent
*/
void ScriptInvokerListMenuEntryMethod(SCR_ListMenuEntryComponent entry, bool favorite);
typedef func ScriptInvokerListMenuEntryMethod;
typedef ScriptInvokerBase<ScriptInvokerListMenuEntryMethod> ScriptInvokerListMenuEntry;

//------------------------------------------------------------------------------------------------
class SCR_ListMenuEntryComponent : SCR_ScriptedWidgetComponent
{
	protected Widget m_wVersionWarningIcon;
	protected ref SCR_ModularButtonComponent m_FavComponent;

	// Text scrolling anims
	protected ref array<ref SCR_HorizontalScrollAnimationComponent> m_aScrollAnimations = {};

	// Mouse buttons
	protected bool m_bInnerButtonInteraction;
	protected ref array<ref SCR_ModularButtonComponent> m_aMouseButtons = {};
	protected ref array<ref SCR_ButtonEffectColor> m_aMouseButtonsColorEffects = {};

	// Main
	protected SCR_ModularButtonComponent m_MainModularButton;
	protected bool m_bFavorite;
	protected bool m_bDisabled;
	protected bool m_bFocused;
	protected bool m_bMouseButtonsEnabled = true;

	// Tooltip
	protected SCR_ScriptedWidgetTooltip m_CurrentTooltip;
	
	// Invokers
	// Called when a mouse interaction button is pressed
	protected ref ScriptInvokerString m_OnMouseInteractionButtonClicked;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocusLost;
	protected ref ScriptInvokerListMenuEntry m_OnFavorite;

	// ---- OVERRIDES ----
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Setup scroll animations
		EnableTextAnimations(false);

		// Mouse Buttons
		SCR_ButtonEffectColor colorEffect;
		foreach (SCR_ModularButtonComponent button : m_aMouseButtons)
		{
			button.m_OnMouseEnter.Insert(OnInnerButtonHover);
			button.m_OnMouseLeave.Insert(OnInnerButtonLeave);

			colorEffect = SCR_ButtonEffectColor.Cast(button.FindEffect("IconColor"));
			if (colorEffect)
				m_aMouseButtonsColorEffects.Insert(colorEffect);
		}

		// Favorite button
		if (m_FavComponent)
			m_FavComponent.m_OnClicked.Insert(OnFavoriteClicked);

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_OnFocus)
			m_OnFocus.Clear();

		if (m_OnFocusLost)
			m_OnFocusLost.Clear();

		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnDoubleClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_OnFocus)
			m_OnFocus.Invoke(this);

		m_bFocused = true;
		UpdateModularButtons();

		if (!m_aScrollAnimations.IsEmpty())
			EnableTextAnimations(true);

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_OnFocusLost)
			m_OnFocusLost.Invoke(this);

		m_bFocused = false;
		UpdateModularButtons();

		// Stop anim
		EnableTextAnimations(false);

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		return super.OnFocusLost(w, x, y);
	}

	// ---- PROTECTED ----
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		m_CurrentTooltip = tooltipClass;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInnerButtonHover()
	{
		m_bInnerButtonInteraction = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInnerButtonLeave()
	{
		m_bInnerButtonInteraction = false
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavoriteClicked(SCR_ModularButtonComponent comp)
	{
		SetFavorite(!m_bFavorite);
	}

	// ----
	//------------------------------------------------------------------------------------------------
	//! Apply behavior on all potentially longer entry text
	protected void EnableTextAnimations(bool enable)
	{
		foreach (SCR_HorizontalScrollAnimationComponent anim : m_aScrollAnimations)
		{
			HandleTextAnimation(anim, enable);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle animation enabled-disabling
	//! enable = false restarts positions
	protected void HandleTextAnimation(SCR_HorizontalScrollAnimationComponent anim, bool enable)
	{
		if (!anim)
			return;

		if (enable)
		{
			if (!anim.GetContentFit())
			{
				anim.AnimationStart();
			}
			else
			{
				anim.AnimationStop();
				anim.ResetPosition();
			}

			return;
		}

		// Disabled
		anim.AnimationStop();
		anim.ResetPosition();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModularButtons()
	{
		// Mouse butons Color
		Color color = UIColors.IDLE_DISABLED;
		if (m_bFocused)
			color = UIColors.NEUTRAL_ACTIVE_STANDBY;

		int index;
		foreach (SCR_ButtonEffectColor effect : m_aMouseButtonsColorEffects)
		{
			if (m_aMouseButtons.IsIndexValid(index) && m_aMouseButtons[index].GetEnabled())
			{
				effect.m_cDefault = color;
				effect.m_cToggledOff = color;
				m_aMouseButtons[index].InvokeAllEnabledEffects(false);
			}

			index++;
		}

		// Main elements color
		// Toggling the line's main modular button component is used as a quck way to change the line's elements while mantaining it active
		if (!m_MainModularButton)
			m_MainModularButton = SCR_ModularButtonComponent.FindComponent(m_wRoot);
		if (m_MainModularButton)
		{
			m_MainModularButton.SetToggled(m_bDisabled, false);

			color = UIColors.NEUTRAL_INFORMATION;
			if (m_bDisabled)
				color = UIColors.IDLE_DISABLED;

			SCR_ButtonEffectColor effect = SCR_ButtonEffectColor.Cast(m_MainModularButton.FindEffect("NameColor"));
			if (effect)
				effect.m_cFocusLost = color;

			m_MainModularButton.InvokeAllEnabledEffects(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(int time, float opacityEnd)
	{
		AnimateWidget.Opacity(GetRootWidget(), opacityEnd, time);
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	//! Animate whole widget opacity
	void AnimateOpacity(int delay, float animationTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			GetRootWidget().SetOpacity(opacityStart);

		GetGame().GetCallqueue().Remove(OpacityAnimation);
		
		// Start animating with a delay to prevent flickering
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, animationTime, opacityEnd);
	}

	//------------------------------------------------------------------------------------------------
	void ShowMouseInteractionButtons(bool show)
	{
		m_bMouseButtonsEnabled = show;
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	bool SetFavorite(bool favorite)
	{
		if (m_FavComponent)
			m_FavComponent.SetToggled(favorite);

		if (m_bFavorite == favorite)
			return false;

		m_bFavorite = favorite;

		if (m_OnFavorite)
			m_OnFavorite.Invoke(this, favorite);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInnerButtonInteraction()
	{
		return m_bInnerButtonInteraction;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFavorite()
	{
		return m_bFavorite;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnMouseInteractionButtonClicked()
	{
		if (!m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked = new ScriptInvokerString();

		return m_OnMouseInteractionButtonClicked;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvokerScriptedWidgetComponent();

		return m_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocusLost()
	{
		if (!m_OnFocusLost)
			m_OnFocusLost = new ScriptInvokerScriptedWidgetComponent();

		return m_OnFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerListMenuEntry GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerListMenuEntry();

		return m_OnFavorite;
	}
}
