#include "scripts/Game/config.c"
class SCR_HintUIComponent: ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sTitleWidgetName;
	
	[Attribute()]
	protected string m_sDescriptionWidgetName;
	
	[Attribute()]
	protected string m_sIconWidgetName;
	
	[Attribute()]
	protected string m_sToggleButtonWidgetName;
	
	[Attribute()]
	protected string m_sContextButtonWidgetName;
	
	[Attribute()]
	protected string m_sTimeLeftWidgetName;
	
	[Attribute()]
	protected string m_sPageWidgetName;	
	
	[Attribute()]
	protected string m_sVisibilitySelectorName;
	
	[Attribute()]
	protected string m_sColorWidgetName;
	
	[Attribute("Name of a widget under the same parent in which highlights will be created.\nOptional; when undefined, highlights will be created directly in workspace.")]
	protected string m_sHighlightParentWidgetName;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_ToggleButtonTextHide;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_ToggleButtonTextNext;
	
	[Attribute("1", desc: "When enabled, the widget will show currently shown hint opn init.\nWhen disabled, currently shown hint will be cleared upon init.")]
	protected bool m_bPreserveCurrentHint;
	
	[Attribute(params: "layout")]
	protected ResourceName m_HighlightLayout;
	
	[Attribute(defvalue: "0.76078 0.39216 0.07843 1.0", UIWidgets.ColorPicker, desc: "Reforger default color")]
	protected ref Color m_cReforgerColor;
	
	[Attribute(defvalue: "0.760006 0.392004 0.078004 1.000000", UIWidgets.ColorPicker, desc: "Color of the time bar")]
	protected ref Color m_cTimerColor;
	
	protected Widget m_Widget;
	protected TextWidget m_NameWidget;
	protected TextWidget m_DescriptionWidget;
	protected ImageWidget m_IconWidget;
	protected Widget m_HighlightParentWidget;
	protected ref array<Widget> m_aHighlightWidgets;
	protected Widget m_ToggleButtonWidget;
	protected Widget m_ContextButtonWidget;
	protected TextWidget m_PageWidget;
	protected SizeLayoutWidget m_TimeWidget;
	protected LocalizedString m_sPageText;
	protected ImageWidget m_BarColor;
	protected SCR_InputButtonComponent m_ToggleButton;
	protected MenuBase m_Menu;
	protected SCR_WLibProgressBarComponent m_ProgressBar;
	protected bool m_bMenuScanned;
	protected Widget m_VisibilitySelector;
	
	protected void OnHintShow(SCR_HintUIInfo info, bool isSilent)
	{
		//--- When part of a menu, check if the menu is focused
		if (!m_bMenuScanned && SCR_WidgetTools.InHierarchy(m_Widget, GetGame().GetWorkspace()))
		{
			m_bMenuScanned = true;
			m_Menu = SCR_WidgetTools.FindMenu(m_Widget);
		}
		
		m_Widget.SetVisible(true);

		if (m_Widget.IsVisibleInHierarchy() && info && info.HasDescription() && (!m_Menu || m_Menu.IsFocused()))
		{
			bool showName = info.SetNameTo(m_NameWidget);
			bool showDescription = info.SetDescriptionTo(m_DescriptionWidget);
			bool showIcon = info.SetIconTo(m_IconWidget);
			
			m_NameWidget.SetVisible(showName);
			m_DescriptionWidget.SetVisible(showDescription);
			m_IconWidget.SetVisible(showIcon);
			m_ContextButtonWidget.SetVisible(info.GetFieldManualLink() != EFieldManualEntryId.NONE);
			
			//--- Initialize pagination label
			int sequencePage = info.GetSequencePage();
			int sequenceCount = info.GetSequenceCount();
			bool showNextButton;
			if (sequenceCount > 1)
			{
				m_PageWidget.SetTextFormat(m_sPageText, sequencePage, sequenceCount);
				m_PageWidget.SetVisible(true);
				showNextButton = sequenceCount != sequencePage;
			}
			else
			{
				m_PageWidget.SetVisible(false);
			}
			
			m_ProgressBar.StopProgressAnimation();
			
			if (showNextButton)
				m_ToggleButton.SetLabel(m_ToggleButtonTextNext);
			else
				m_ToggleButton.SetLabel(m_ToggleButtonTextHide);
			
			float startvalue;
			int duration;
			#ifndef AR_HINT_UI_TIMESTAMP
			int timeDifferenceFromStart = Replication.Time() - info.GetTimeStarted();
			
			if (info.GetTimeStarted() == -1 || Replication.Time() == info.GetTimeStarted() || info.GetDuration() == -1 || !info.IsTimerVisible())
			{
				startvalue = 1;
				duration = info.GetDuration();
			}
			else	
			{
 				startvalue = 1 - timeDifferenceFromStart / info.GetDuration() / 1000;
				duration = info.GetDuration() - timeDifferenceFromStart / 1000;
			}
			#else
			ChimeraWorld world = GetGame().GetWorld();
			WorldTimestamp currentTime = world.GetServerTimestamp();
			if (info.GetTimeStarted() != 0 || currentTime == info.GetTimeStarted() || info.GetDuration() == -1 || !info.IsTimerVisible())
			{
				startvalue = 1;
				duration = info.GetDuration();
			}
			else
			{
				int timeDifferenceFromStart = currentTime.DiffMilliseconds(info.GetTimeStarted());
 				startvalue = 1 - timeDifferenceFromStart / info.GetDuration() / 1000;
				duration = info.GetDuration() - timeDifferenceFromStart / 1000;
			}
			#endif
			
			if (info.IsTimerVisible() && m_TimeWidget)
			{
				m_TimeWidget.SetVisible(true);
				m_BarColor.SetColor(m_cTimerColor);
				m_ProgressBar.SetValue(startvalue, false);
				m_ProgressBar.SetAnimationTime(duration);
				m_ProgressBar.SetValue(0);
			}
			else if (m_TimeWidget)
			{
				m_TimeWidget.SetVisible(false);
			}
			
			if (!info.GetName())
				m_VisibilitySelector.SetVisible(false);
			else
				m_VisibilitySelector.SetVisible(true);
			
			if (info.GetDuration() == -1 || !info.IsTimerVisible())
			{
				m_BarColor.SetColor(m_cReforgerColor);
				m_ProgressBar.SetValue(1, false);
			}
				
			AnimateWidget.Opacity(m_Widget, 1, UIConstants.FADE_RATE_FAST);
			
			//--- Create highlights a bit later, so widgets which are part of the menu this hint UI belongs to have time to register
			if (m_HighlightLayout)
				GetGame().GetCallqueue().CallLater(CreateHighlights, 1, false, info);
		
			if (!isSilent)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.HINT, true);
		}
		else
		{
			OnHintHide(info, isSilent);
		}
	}
	protected void OnHintHide(SCR_HintUIInfo info, bool isSilent)
	{
		AnimateWidget.Opacity(m_Widget, 0, UIConstants.FADE_RATE_DEFAULT, true);

		if (m_aHighlightWidgets)
		{
			foreach (Widget hightlightWidget: m_aHighlightWidgets)
			{
				if (hightlightWidget)
					hightlightWidget.RemoveFromHierarchy();
			}
		}
	}
	protected void CreateHighlights(SCR_HintUIInfo info)
	{
		if (!m_HighlightParentWidget && m_sHighlightParentWidgetName)
			m_HighlightParentWidget = m_Widget.GetParent().FindAnyWidget(m_sHighlightParentWidgetName);
		
		array<string> hightlightWidgetNames = {};
		m_aHighlightWidgets = {};
		Widget hightlightWidget;
		for (int i, count = info.GetHighlightWidgetNames(hightlightWidgetNames); i < count; i++)
		{
			hightlightWidget = SCR_WidgetHighlightUIComponent.CreateHighlight(hightlightWidgetNames[i], m_HighlightLayout, m_HighlightParentWidget);
			if (hightlightWidget)
				m_aHighlightWidgets.Insert(hightlightWidget);
		}
	}
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (SCR_WidgetTools.InHierarchy(w, m_ToggleButtonWidget))
		{
			SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
			if (hintManager)
				hintManager.Toggle();
		}
		if (SCR_WidgetTools.InHierarchy(w, m_ContextButtonWidget))
		{
			SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
			if (hintManager)
				hintManager.OpenContext();
		}
		return false;
	}
	
	//--- Refresh hint, as it sometimes uses device-specific lines
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.Refresh();
	}
	
	override void HandlerAttached(Widget w)
	{
		m_Widget = w;
		
		m_ToggleButtonWidget = m_Widget.FindAnyWidget(m_sToggleButtonWidgetName);
		m_ToggleButton = SCR_InputButtonComponent.Cast(m_ToggleButtonWidget.FindHandler(SCR_InputButtonComponent));
		if (m_ToggleButton)
			m_ToggleButton.SetLabel(m_ToggleButtonTextHide);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_Widget.SetOpacity(0);
		m_Widget.SetVisible(false);
		
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		m_NameWidget = TextWidget.Cast(m_Widget.FindAnyWidget(m_sTitleWidgetName));
		m_DescriptionWidget = TextWidget.Cast(m_Widget.FindAnyWidget(m_sDescriptionWidgetName));
		m_IconWidget = ImageWidget.Cast(m_Widget.FindAnyWidget(m_sIconWidgetName));
		m_ContextButtonWidget = m_Widget.FindAnyWidget(m_sContextButtonWidgetName);
		m_PageWidget = TextWidget.Cast(m_Widget.FindAnyWidget(m_sPageWidgetName));
		m_sPageText = m_PageWidget.GetText();
		m_TimeWidget = SizeLayoutWidget.Cast(m_Widget.FindAnyWidget(m_sTimeLeftWidgetName));
		m_VisibilitySelector = m_Widget.FindAnyWidget(m_sVisibilitySelectorName);
		m_BarColor = ImageWidget.Cast(m_Widget.FindAnyWidget(m_sColorWidgetName));
		m_ProgressBar = SCR_WLibProgressBarComponent.GetProgressBar(m_sTimeLeftWidgetName, m_Widget, true);
		
		hintManager.GetOnHintShow().Insert(OnHintShow);
		hintManager.GetOnHintHide().Insert(OnHintHide);
		
		SCR_HintUIInfo currentHint = hintManager.GetCurrentHint();
		if (currentHint)
		{
			if (m_bPreserveCurrentHint)
			{
				//--- Show the current hint again in the new area
				OnHintShow(currentHint, true);
			}
			else if (!currentHint.IsInSequence())
			{
				//--- Hide the current hint because it doesn't make sense in the current context (not sequence hints, they should not be interrupted)
				hintManager.Hide();
			}
		}
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}
	override void HandlerDeattached(Widget w)
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		hintManager.GetOnHintShow().Remove(OnHintShow);
		hintManager.GetOnHintHide().Remove(OnHintHide);
	}
};
