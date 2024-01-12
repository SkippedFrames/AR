//------------------------------------------------------------------------------------------------
//! Map marker layout component
//! Attached to root of marker base layout
class SCR_MapMarkerWidgetComponent : SCR_ScriptedWidgetComponent
{
	protected const ResourceName IMAGE_SET = "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	protected const string PRIVATE_QUAD = "private";
	protected const string PUBLIC_QUAD = "public";
	
	protected bool m_bIsEventListening;	// whether this marker reacts to events
	protected bool m_bIsSymbolMode;		// app-6 symbol visualization mode
	protected bool m_bIsOwnerMode;		// player is the markers owner
	protected int m_iLayerID;			// map layer ID
	
	protected ImageWidget m_wMarkerIcon;
	protected ImageWidget m_wMarkerGlowIcon;
	protected ImageWidget m_wMarkerModeIcon;
	protected TextWidget m_wMarkerText;
	protected TextWidget m_wMarkerAuthor;
	protected Widget m_wSymbolRoot;
	protected Widget m_wSymbolOverlay;
	
	protected ref Color m_GlowDefault  = Color.FromSRGBA(21, 29, 32, 155);
	protected ref Color m_GlowSelected = Color.FromSRGBA(226, 168, 79, 155);
	
	protected ref Color m_TextColor = new Color(0.0, 0.0, 0.0, 1.0);
	protected ref Color m_CurrentImageColor = new Color(0.0, 0.0, 0.0, 1.0);
	protected SCR_MapMarkerBase m_MarkerObject;
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerObject(notnull SCR_MapMarkerBase marker)
	{
		m_MarkerObject = marker;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerID(int id)
	{
		m_iLayerID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRotation(float angle)
	{
		m_wMarkerIcon.SetRotation(angle);
		m_wMarkerGlowIcon.SetRotation(angle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Supports custom aspect ratio in case of non standard size imagesets
	void SetImage(ResourceName icon, string quad, float aspectRatio = 1)
	{
		m_wMarkerIcon.LoadImageFromSet(0, icon, quad);
		if (aspectRatio != 1 && aspectRatio != 0)
		{
			vector size = m_wMarkerIcon.GetSize();
			m_wMarkerIcon.SetSize(size[0] * 0.9, (size[1] * (1/aspectRatio)) * 0.9); // todo, temp size adjust before symbols group side are fixed
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGlowImage(ResourceName icon, string quad)
	{
		m_wMarkerGlowIcon.SetVisible(true);
		m_wMarkerGlowIcon.LoadImageFromSet(0, icon, quad);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visual mode for military symbol which is constructed through additional component
	void SetMilitarySymbolMode(bool state)
	{
		m_bIsSymbolMode = state;
		
		m_wSymbolRoot.SetEnabled(state);
		m_wSymbolRoot.SetVisible(state);
		
		m_wMarkerIcon.SetVisible(!state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEventListening(bool state)
	{
		m_bIsEventListening = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateMilitarySymbol(SCR_MilitarySymbol milSymbol)
	{
		SCR_MilitarySymbolUIComponent symbolComp = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolOverlay.FindHandler(SCR_MilitarySymbolUIComponent));
		if (symbolComp)
			symbolComp.Update(milSymbol);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_wMarkerText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTextVisible(bool state)
	{
		m_wMarkerText.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAuthor(string text)
	{
		m_bIsOwnerMode = false;
		
		m_wMarkerAuthor.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAuthorVisible(bool state)
	{
		if (m_bIsOwnerMode)
			m_wMarkerModeIcon.SetVisible(state);
		else 
			m_wMarkerAuthor.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetModeIcon(bool state, bool isPublic)
	{		
		m_bIsOwnerMode = true;
		
		if (isPublic)
			m_wMarkerModeIcon.LoadImageFromSet(0, IMAGE_SET, PUBLIC_QUAD);
		else 
			m_wMarkerModeIcon.LoadImageFromSet(0, IMAGE_SET, PRIVATE_QUAD);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetModeIconVisible(bool state)
	{
		m_wMarkerModeIcon.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetColor(Color color)
	{
		m_CurrentImageColor = color;
		
		if (m_bIsSymbolMode)
			m_wSymbolOverlay.SetColor(color);
		else 
			m_wMarkerIcon.SetColor(color);
			
		m_wMarkerText.SetColor(m_TextColor);
		m_wMarkerAuthor.SetColor(m_TextColor);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_bIsEventListening || !m_MarkerObject || !SCR_MapMarkersUI.IsOwnedMarker(m_MarkerObject))
			return false;
		
		m_wMarkerGlowIcon.SetColor(m_GlowSelected);
		
		m_MarkerObject.LayerChangeLogic(0);
				
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_bIsEventListening || !m_MarkerObject || !SCR_MapMarkersUI.IsOwnedMarker(m_MarkerObject))
			return false;
		
		m_wMarkerGlowIcon.SetColor(m_GlowDefault);
		
		m_MarkerObject.LayerChangeLogic(m_iLayerID);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wMarkerIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerIcon"));
		m_wMarkerGlowIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerIconGlow"));
		m_wMarkerText = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerText"));
		m_wMarkerAuthor = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerAuthor"));
		m_wMarkerModeIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerModeIcon"));
		m_wSymbolRoot = m_wRoot.FindAnyWidget("SymbolWidget");
		m_wSymbolOverlay = m_wRoot.FindAnyWidget("SymbolOverlay");
	}
};
