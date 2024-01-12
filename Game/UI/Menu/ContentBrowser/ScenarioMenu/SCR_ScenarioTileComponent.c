//------------------------------------------------------------------------------------------------
class SCR_ScenarioTileComponent : SCR_ButtonBaseComponent
{
	
	MissionWorkshopItem m_Scenario = null;
	SCR_MissionHeader m_Header = null;
	protected bool m_bIsLocal = true;
	protected ImageWidget m_ScenarioImage;
	protected TextWidget m_ScenarioName;
	
	protected TextWidget m_ScenarioSumary;
	protected TextWidget m_ScenarioGameMode;
	
	protected Widget m_ScenarioMod;
	protected SCR_ButtonComponent m_ModParentComp;
	protected Widget m_wPlayableButtom;
	protected Widget m_wWhiteFrame;

	protected SCR_InputButtonComponent m_PlayButtonComponent;
	protected SCR_InputButtonComponent m_HostButtonComponent;
	protected SCR_InputButtonComponent m_JoinButtonComponent;
	
	protected SCR_ButtonComponent m_LinkComp;
	protected Widget m_wGamepadModNameButton;
	protected SCR_ButtonComponent m_GamepadModNameButtonController
	protected Widget m_ScenarioModButton;
	protected Widget m_wFavouriteOverlay;
	SCR_SelectableTileButtonComponent m_FavComponent;
	private string m_sFavouriteOverlay = "FavouriteOverlayButton";
	
	private InputManager m_InputManager;
	
	ContentBrowserUI m_ParentContentBrowserMenu;
	
	ref ScriptInvoker m_OnPlayClicked = new ref ScriptInvoker();
	ref ScriptInvoker m_OnHostClicked = new ref ScriptInvoker();
	ref ScriptInvoker m_OnJoinClicked = new ref ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_InputManager = GetGame().GetInputManager();
		m_ScenarioGameMode = TextWidget.Cast(m_wRoot.FindAnyWidget("ScenarioGameMode"));
		m_ScenarioImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("ScenarioImage"));
		m_ScenarioName = TextWidget.Cast(m_wRoot.FindAnyWidget("ScenarioName"));
		m_ScenarioMod = m_wRoot.FindAnyWidget("ScenarioMod");
		m_ScenarioModButton = m_wRoot.FindAnyWidget("ScenarioModButton");
		m_ScenarioSumary = TextWidget.Cast(m_wRoot.FindAnyWidget("ScenarioSumary"));
		m_wPlayableButtom = m_wRoot.FindAnyWidget("PlayablePC");
		m_wWhiteFrame = m_wRoot.FindAnyWidget("WhiteFrameOverlay");
		
		
		m_PlayButtonComponent = SCR_InputButtonComponent.GetInputButtonComponent("PlayButton", m_wRoot);
		if (m_PlayButtonComponent)
			m_PlayButtonComponent.m_OnActivated.Insert(OnPlay);
		
		m_HostButtonComponent = SCR_InputButtonComponent.GetInputButtonComponent("HostButton", m_wRoot);
		if (m_HostButtonComponent)
			m_HostButtonComponent.m_OnActivated.Insert(OnHost);
		
		m_JoinButtonComponent = SCR_InputButtonComponent.GetInputButtonComponent("JoinButton", m_wRoot);
		if (m_JoinButtonComponent)
			m_JoinButtonComponent.m_OnActivated.Insert(OnJoin);
		
		if (m_ScenarioModButton)
		{
			m_ModParentComp = SCR_ButtonComponent.Cast(m_ScenarioModButton.FindHandler(SCR_ButtonComponent));
			if (m_ModParentComp)
			{
				m_ModParentComp.m_OnClicked.Insert(OpenParentMod);
			}
		}
		
		m_wFavouriteOverlay = w.FindAnyWidget(m_sFavouriteOverlay);
		if (m_wFavouriteOverlay)
		{
			m_FavComponent = SCR_SelectableTileButtonComponent.Cast(m_wFavouriteOverlay.FindHandler(SCR_SelectableTileButtonComponent));
			if (m_FavComponent)	
			{	
				m_FavComponent.m_OnChanged.Insert(OnFavouriteClicked);
			}
		}
		
		m_wGamepadModNameButton = m_wRoot.FindAnyWidget("ModNavButton");
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		AnimateWidget.Opacity(m_wPlayableButtom, 0, m_fAnimationRate);
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}
	
	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetMission()
	{
		return m_Scenario;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MissionHeader GetHeader()
	{
		return m_Header;
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenParentModGamepad()
	{
		if (!m_ScenarioMod.IsVisible())
		return;
		if (m_wRoot == GetGame().GetWorkspace().GetFocusedWidget())
			OpenParentMod();
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenParentMod()
	{
		if (!m_Scenario)
		return;
		WorkshopItem owner = m_Scenario.GetOwner();
		if (!owner)
			return;
		ContentBrowserDetailsMenu baseMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowserDetailsMenu, 0, true, false));

		if (baseMenu)
		{
			// todo
			//if (m_ParentContentBrowserMenu)
			//	baseMenu.SetParentMenu(m_ParentContentBrowserMenu);
			//	baseMenu.SetWorkshopItem(owner, null, null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentMenu(ContentBrowserUI menu)
	{
		m_ParentContentBrowserMenu = menu;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioHeader(SCR_MissionHeader header)
	{
		m_Header = header;
		if (m_ScenarioImage)
		{
			if (header.m_sIcon)
			{
				m_ScenarioImage.LoadImageTexture(0, header.m_sIcon, false, true);
				m_ScenarioImage.SetImage(0);
			}
					
		}
		
		if (m_ScenarioName)
		{
			if (header.m_sName)
					m_ScenarioName.SetText(header.m_sName);
		}
		
		if (m_ScenarioSumary)
		{
			if (header.m_sDescription)
				m_ScenarioSumary.SetText(header.m_sDescription);
		}
		
		if (m_ScenarioGameMode)
		{
			if (m_ScenarioGameMode)
					m_ScenarioGameMode.SetText(header.m_sGameMode);
		}
		
		if (header.IsMultiplayer())
		{
			m_HostButtonComponent.GetRootWidget().SetVisible(true);
			m_JoinButtonComponent.GetRootWidget().SetVisible(true);
		}
		else
		{
			m_HostButtonComponent.GetRootWidget().SetVisible(false);
			m_JoinButtonComponent.GetRootWidget().SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario, bool bShowOwner = true, bool bShowFavourite = true)
	{
		m_Scenario = scenario;
		SCR_MissionHeader header = SCR_MissionHeader.Cast(scenario.GetHeader());
		
		if (bShowFavourite)
		{
			m_wFavouriteOverlay.SetVisible(true);
			if (scenario.IsFavorite())
				AnimateWidget.Opacity(m_wPlayableButtom, 1, UIConstants.FADE_RATE_DEFAULT);
			m_FavComponent.SetSelected(scenario.IsFavorite(), false);
		}
		
		if (header)
			SetScenarioHeader(header);
		else
		{
			if (m_ScenarioImage)
			{
				ImageScale img = scenario.Thumbnail().GetLocalScale(0);		//take any scale
				if (img)
				{
					m_ScenarioImage.LoadImageTexture(0, img.Path(), false, true);
					m_ScenarioImage.SetImage(0);
				}
			
			}
			
			if (m_ScenarioName)
			{
				if (scenario.Name())
					m_ScenarioName.SetText(scenario.Name());
			}
			
			if (m_ScenarioSumary)
			{
				if (scenario.Description())
					m_ScenarioSumary.SetText(scenario.Description());
			}
			
		}
		
			m_FavComponent.SetSelected(m_Scenario.IsFavorite(), false);
		
		
		WorkshopItem owner = scenario.GetOwner();
		if (owner)
		{
			
			if (bShowOwner)
			{
				if (m_ScenarioMod)
				{
					
					m_ScenarioMod.SetVisible(true);
					m_ModParentComp.SetContent(owner.Name());
				}
				else
					m_ScenarioName.SetColor(UIColors.CONTRAST_COLOR);
				
			}
			else
			{
				if (m_ScenarioMod)
					m_ScenarioMod.SetVisible(false);
			}
			int flags = owner.GetStateFlags();
			if (!(flags & EWorkshopItemState.EWSTATE_OFFLINE) || !owner.IsLoaded())
			{
				m_bIsLocal = false;
				m_wPlayableButtom.SetVisible(false);
			} 
		}
		

	}

	
	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Scenario;
	}
	
	
	//------------------------------------------------------------------------------------------------
	bool IsFocused()
	{
		return GetGame().GetWorkspace().GetFocusedWidget() == this.m_wRoot;
	}
		
	//------------------------------------------------------------------------------------------------
	void OnPlay()
	{
		if (!this.IsFocused())
			return;
		
		m_OnPlayClicked.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnHost()
	{
		if (!this.IsFocused())
			return;
		
		m_OnHostClicked.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnJoin()
	{
		if (!this.IsFocused())
			return;
		
		m_OnJoinClicked.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuSelect()
	{
			return;	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		//if (w != m_wRoot)
			//return false;
		m_OnFocus.Invoke(w);

		m_FavComponent.ShowButton();
		AnimateWidget.Opacity(m_wPlayableButtom, 1, m_fAnimationRate);
		AnimateWidget.Color(m_wWhiteFrame, m_BackgroundSelected, m_fAnimationRate);
		//AnimateWidget.Opacity(m_wGamepadModNameButton, 1, m_fAnimationRate);

		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			m_wGamepadModNameButton.SetOpacity(0);
		}
		else
		{
			m_InputManager.AddActionListener("MenuRefresh", EActionTrigger.PRESSED, OpenParentModGamepad);
			m_wGamepadModNameButton.SetOpacity(1);
		}
		
		return super.OnFocus( w,  x,  y);;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{	
		/*if (GetWidgetUnderCursor() == w)
			AnimateWidget.Color(m_wWhiteFrame, COLOR_BACKGROUND_HOVERED, m_fAnimationRate);
		else
		{*/
			AnimateWidget.Color(m_wWhiteFrame, UIColors.TRANSPARENT, m_fAnimationRate);
			AnimateWidget.Opacity(m_wPlayableButtom, 0, m_fAnimationRate);
			AnimateWidget.Opacity(m_wGamepadModNameButton, 0, m_fAnimationRate);
		//}
			m_InputManager.RemoveActionListener("MenuRefresh", EActionTrigger.PRESSED, OpenParentModGamepad);
			m_wGamepadModNameButton.SetOpacity(0);
		m_FavComponent.HideButton();
		return super.OnFocusLost( w,  x,  y);;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnButtonHovered()
	{
		OnMouseEnter(m_wRoot, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceIsGamepad(bool isGamepad)
	{

	}
	
	//------------------------------------------------------------------------------------------------
	void OnFavouriteClicked()
	{
		m_Scenario.SetFavorite(m_FavComponent.IsSelected());
	}
	
};
