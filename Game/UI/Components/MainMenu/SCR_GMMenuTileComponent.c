//------------------------------------------------------------------------------------------------
class SCR_GMMenuTileComponent : SCR_TileBaseComponent
{
	MissionWorkshopItem m_Item;
	ref SCR_MissionHeader m_Header;
	string m_sScenarioPath;
	Widget m_wFeatured;
	TextWidget m_wTypeText;
	TextWidget m_wName;
	TextWidget m_wDescription;
	SCR_InputButtonComponent m_Play;
	SCR_InputButtonComponent m_Continue;
	SCR_InputButtonComponent m_Restart;
	SCR_InputButtonComponent m_FindServer;
	Widget m_wFooter;

	// All return SCR_GMMenuTileComponent as script invoker
	ref ScriptInvoker m_OnPlay = new ScriptInvoker();
	ref ScriptInvoker m_OnContinue = new ScriptInvoker();
	ref ScriptInvoker m_OnRestart = new ScriptInvoker();
	ref ScriptInvoker m_OnFindServer = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wTypeText = TextWidget.Cast(w.FindAnyWidget("TypeText"));
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wFooter = w.FindAnyWidget("Footer");
		m_wFeatured = w.FindAnyWidget("Featured");
		m_Play = SCR_InputButtonComponent.GetInputButtonComponent("Play", w);
		if (m_Play)
			m_Play.m_OnActivated.Insert(OnPlay);
		
		m_Continue = SCR_InputButtonComponent.GetInputButtonComponent("Continue", w);
		if (m_Continue)
			m_Continue.m_OnActivated.Insert(OnContinue);

		m_FindServer = SCR_InputButtonComponent.GetInputButtonComponent("FindServer", w);
		if (m_FindServer)
			m_FindServer.m_OnActivated.Insert(OnFindServer);
		
		m_Restart = SCR_InputButtonComponent.GetInputButtonComponent("Restart", w);
		if (m_Restart)
			m_Restart.m_OnActivated.Insert(OnRestart);
		
		m_wFooter.SetOpacity(0);
		m_wFooter.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);

		if (button == 0)
			m_OnPlay.Invoke(this);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		//AnimateWidget.Opacity(m_wFooter, 1, m_fAnimationRate);
		m_wFooter.SetOpacity(1);
		m_wFooter.SetEnabled(true);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		//AnimateWidget.Opacity(m_wFooter, 0, m_fAnimationRate);
		m_wFooter.SetOpacity(0);
		m_wFooter.SetEnabled(false);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowMission(notnull MissionWorkshopItem item, bool isFeatured = false, bool isRecent = false, bool isRecommended = false)
	{
		m_Item = item;

		if (isFeatured)
			m_wTypeText.SetText("#AR-MainMenu_Featured");
		else if (isRecent)
			m_wTypeText.SetText("#AR-MainMenu_Recent");
		else if (isRecommended)
			m_wTypeText.SetText("#AR-MainMenu_Recommended");
		else
			m_wTypeText.SetText(" "); // Hide the widget, keep space

		m_wName.SetText(item.Name());
		m_wDescription.SetText(item.Description());
		
		// Set image through SCR_ButtonImageComponent
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(item.Id()));
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			ResourceName texture = GetTexture();
			if (!texture.IsEmpty())
				comp.SetImage(texture);
		}
		
		m_wFeatured.SetVisible(isFeatured);

		bool canContinue = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
		m_Play.SetVisible(!canContinue);
		m_Continue.SetVisible(canContinue);
		
		m_Restart.SetVisible(canContinue);
		m_FindServer.SetVisible(item.GetPlayerCount() > 1);
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetTexture()
	{
		if (!m_Header)
			return m_Item.Thumbnail().GetLocalScale(1920).Path();
		if (!m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;
		if (!m_Header.m_sIcon.IsEmpty())
			return m_Header.m_sIcon;

		return m_Header.m_sLoadingScreen;
	}

	//------------------------------------------------------------------------------------------------
	void OnPlay()
	{
		m_OnPlay.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnContinue()
	{
		m_OnContinue.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnRestart()
	{
		m_OnRestart.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnFindServer()
	{
		m_OnFindServer.Invoke(this);
	}
};
