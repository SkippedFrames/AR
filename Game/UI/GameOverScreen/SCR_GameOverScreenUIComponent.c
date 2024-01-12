class SCR_GameOverScreenUIComponent: ScriptedWidgetComponent
{	
	[Attribute("TabView", desc: "Tab view will only be used if the widget in the layout is visible. This is because some changes that no longer required the tabview")]
	protected string m_sTabViewName;
	
	[Attribute("Base_ContentHolder")]
	protected string m_sContentHolderName;
	
	[Attribute("Base_Background")]
	protected string m_sBackgroundName;
	
	[Attribute("Base_ColorOverlay")]
	protected string m_sBackgroundColorOverlayName;
	
	[Attribute("#AR-PauseMenu_ReturnTitle", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMainMenuPopUpTitle;
	
	[Attribute("#AR-PauseMenu_ReturnText", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMainMenuPopUpMessage;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", params: "imageset")]
	protected ResourceName m_sMainMenuPopUpImageSet;
	
	[Attribute("exit")]
	protected string m_sMainMenuPopUpImage;
	
	[Attribute("Back")]
	protected string m_sBackButtonName;
	
	[Attribute("ChatButton")]
	protected string m_sChatButtonName;
	
	[Attribute("Debriefing")]
	protected string m_sDebriefingButtonName;
	
	[Attribute("ButtonHolder")]
	protected string m_sButtonHolderName;
	
	[Attribute("0.05")]
	protected float m_fBackgroundColorOverlayOpacity;
	
	[Attribute("2")]
	protected float m_fContentFadeInSpeed;
	
	[Attribute("0", desc: "Which tab will show the end screen content eg: Victory, Defeat, etc.")]
	protected float m_iContentTabIndex;
	
	protected Widget m_wRoot;
	protected Widget m_wEndscreenContent;
	protected SCR_TabViewComponent m_TabViewComponent;
	protected SCR_FadeUIComponent m_OverlayBackgroundColorFadeUIComponent;
	protected SCR_FadeUIComponent m_ContentFadeComponent;
	protected SCR_FadeUIComponent m_TabViewFadeComponent;
	protected SCR_FadeUIComponent m_ButtonHolderFadeComponent;
	protected ref SCR_GameOverScreenUIContentData m_EndScreenUIContentInfo;
	
	//~ On tab changed
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget w, int tabIndex)
	{
		OnEndScreenContentShow(tabIndex == m_iContentTabIndex);
	}
	
	//~ On Tab created
	protected void OnTabCreated(SCR_TabViewComponent tabView, Widget w)
	{
		if (!w)
			return;
		
		SCR_BaseGameOverTabUIComponent baseGameOverTab = SCR_BaseGameOverTabUIComponent.Cast(w.FindHandler(SCR_BaseGameOverTabUIComponent));
		if (baseGameOverTab)
			baseGameOverTab.GameOverTabInit(m_EndScreenUIContentInfo);
	}
	
	//~ When tabview fade is done
	protected void OnTabViewFadeDone()
	{
		if (m_TabViewComponent)
			m_TabViewComponent.EnableAllTabs(true);
	}
	
	//~ Show end screen content eg: Victory, defeat, etc.
	protected void OnEndScreenContentShow(bool show)
	{
		if (m_wEndscreenContent)
			m_wEndscreenContent.SetVisible(show);
	}
	
	/*!
	Create the gameover screen, spawn the layout and fading it in
	\param endScreenUIContent contains the layout and any neccessary information for the endscreen content widget
	*/
	void InitGameOverScreen(SCR_GameOverScreenUIContentData endScreenUIContent)
	{			
		//~ Already has end screen
		if (m_wEndscreenContent)
			return;
		
		m_EndScreenUIContentInfo = endScreenUIContent;
		
		Widget contentHolder = m_wRoot.FindAnyWidget(m_sContentHolderName);
		if (!contentHolder)
			return;
		
		//Spawn gameover layout content
		if (!endScreenUIContent.m_sGameOverLayout.IsEmpty())
		{
			m_wEndscreenContent = GetGame().GetWorkspace().CreateWidgets(endScreenUIContent.m_sGameOverLayout, contentHolder);
			
			if (m_wEndscreenContent)
			{
				SCR_GameOverScreenContentUIComponent gameOverWidgetContent = SCR_GameOverScreenContentUIComponent.Cast(m_wEndscreenContent.FindHandler(SCR_GameOverScreenContentUIComponent));
			
				if (gameOverWidgetContent)
					gameOverWidgetContent.InitContent(endScreenUIContent);
			}
		}
		
		if (endScreenUIContent.m_cVignetteColor)
		{
			ImageWidget colorOverlay =  ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundColorOverlayName));
			if (colorOverlay)
				colorOverlay.SetColor(endScreenUIContent.m_cVignetteColor);
		}
		
		if (m_ContentFadeComponent)
			m_ContentFadeComponent.FadeIn();
		if (m_TabViewFadeComponent)
			m_TabViewFadeComponent.FadeIn();
		if (m_ButtonHolderFadeComponent)
			m_ButtonHolderFadeComponent.FadeIn();
		if (m_OverlayBackgroundColorFadeUIComponent)
			m_OverlayBackgroundColorFadeUIComponent.FadeIn();
		
		Widget returnToMenuBtn = m_wRoot.FindAnyWidget(m_sBackButtonName);
		if (returnToMenuBtn)
		{
			SCR_InputButtonComponent returnToMenuButton = SCR_InputButtonComponent.Cast(returnToMenuBtn.FindHandler(SCR_InputButtonComponent));
			if (returnToMenuButton)
				returnToMenuButton.m_OnActivated.Insert(ReturnToMenu);
		}
		
		Widget debriefingWidgetButton = m_wRoot.FindAnyWidget(m_sDebriefingButtonName);
		if (debriefingWidgetButton)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
			{
				debriefingWidgetButton.SetVisible(false);
			}
			else 
			{
				SCR_DebriefingScreenComponent debriefingScreen = SCR_DebriefingScreenComponent.Cast(gameMode.FindComponent(SCR_DebriefingScreenComponent));
				if (debriefingScreen)
				{
					SCR_InputButtonComponent debriefingButton = SCR_InputButtonComponent.Cast(debriefingWidgetButton.FindHandler(SCR_InputButtonComponent));
					if (debriefingButton)
						debriefingButton.m_OnActivated.Insert(OpenDebriefingScreenMenu);
					}
				else
				{
					debriefingWidgetButton.SetVisible(false);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenDebriefingScreenMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.DebriefingScreenMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	void ReturnToMenu()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupConfirm);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void BackToMainMenuPopupConfirm()
	{
		OnGameEnd();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	/*!
	Sets the popup dialog text created in GameOverScreenInput
	\param popup dialog to set the text of
	*/
	void SetMainMenuPopUpTexts(DialogUI popup)
	{
		popup.SetMessage(m_sMainMenuPopUpMessage);
		popup.SetTitle(m_sMainMenuPopUpTitle);
		popup.SetTitleIcon(m_sMainMenuPopUpImageSet, m_sMainMenuPopUpImage);
	}
	
	//~ Removes itself from hierargy on game end just in cause
	protected void OnGameEnd()
	{
		m_wRoot.RemoveFromHierarchy();
	}
	
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_wRoot = w;
		
		ImageWidget colorOverlay =  ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundColorOverlayName));
		if (colorOverlay)
		{
			colorOverlay.SetOpacity(m_fBackgroundColorOverlayOpacity);
			m_OverlayBackgroundColorFadeUIComponent = SCR_FadeUIComponent.Cast(colorOverlay.FindHandler(SCR_FadeUIComponent));
			
			if (m_OverlayBackgroundColorFadeUIComponent)
			{
				m_OverlayBackgroundColorFadeUIComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				colorOverlay.SetVisible(false);
			}
		}
		
		Widget contentHolder = w.FindAnyWidget(m_sContentHolderName);
		if (contentHolder)
		{
			m_ContentFadeComponent = SCR_FadeUIComponent.Cast(contentHolder.FindHandler(SCR_FadeUIComponent));
			
			if (m_ContentFadeComponent)
			{
				m_ContentFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				contentHolder.SetVisible(false);
			}
		}
			
		Widget buttonHolder = w.FindAnyWidget(m_sButtonHolderName);
		if (buttonHolder)
		{
			m_ButtonHolderFadeComponent = SCR_FadeUIComponent.Cast(buttonHolder.FindHandler(SCR_FadeUIComponent));
			if (m_ButtonHolderFadeComponent)
			{
				m_ButtonHolderFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				buttonHolder.SetVisible(false);
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
		if (gameMode)
			gameMode.GetOnGameEnd().Insert(OnGameEnd);
		
		Widget tabView = w.FindAnyWidget(m_sTabViewName);
		
		//~ Only use tabview if it is visible in the layout
		if (tabView && tabView.IsVisible())
		{
			m_TabViewComponent = SCR_TabViewComponent.Cast(tabView.FindHandler(SCR_TabViewComponent));
			m_TabViewFadeComponent = SCR_FadeUIComponent.Cast(tabView.FindHandler(SCR_FadeUIComponent));
			
			if (m_TabViewFadeComponent)
			{
				m_TabViewFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				tabView.SetVisible(false);
				m_TabViewFadeComponent.GetOnFadeDone().Insert(OnTabViewFadeDone);
			}
		}
			
		if (m_TabViewComponent)
		{
			if (m_TabViewFadeComponent)
				m_TabViewComponent.EnableAllTabs(false);
			
			m_TabViewComponent.m_OnChanged.Insert(OnTabChanged);
			m_TabViewComponent.m_OnContentCreate.Insert(OnTabCreated);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
		if (gameMode)
			gameMode.GetOnGameEnd().Remove(OnGameEnd);
	}
};





