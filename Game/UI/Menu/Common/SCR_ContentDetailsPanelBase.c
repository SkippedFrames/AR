/*
todo: refactor this.
Don't do init of widgets in HandlerAttached of Base class.
Instead make a new widgets class for each details panel layout type.
*/

class SCR_ContentDetailsPanelBase : ScriptedWidgetComponent
{
	protected ref SCR_ContentDetailsPanelBaseWidgets widgets = new SCR_ContentDetailsPanelBaseWidgets; // todo delete this. For now it's here for compatibility.
	
	Widget m_wRoot;
	
	[Attribute()]
	protected ref SCR_ContentBrowser_ColorScheme m_ColorScheme;
	
	[Attribute()]
	protected ref DetailsPanelContentPresetConfig m_FallbackContent;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Common image set", "imageset")]
	protected ResourceName m_IconImageSet; 
	
	[Attribute("500")]
	protected int m_iMaxDescriptionLenght;
	
	protected const int DESCRIPTION_LENGHT_OFFSET = 10;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		widgets.Init(w.FindWidget("ContentDetailsPanel")); // The layout is exported from base layout which is embedded into this layout.
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		// This will also get called on updates of children, ignore them
		if (w != m_wRoot)
			return true;
		
		GetGame().GetCallqueue().CallLater(UpdateSize, 0);
		
		return true;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		// Resize the height of the picture, it must keep a fixed aspect ratio
		float sizex, sizey;
		m_wRoot.GetScreenSize(sizex, sizey);
		float sizexUnscaled = GetGame().GetWorkspace().DPIUnscale(sizex);
		widgets.m_TopSize.EnableHeightOverride(true);
		widgets.m_TopSize.SetHeightOverride(sizexUnscaled / SCR_WorkshopUiCommon.IMAGE_SIZE_RATIO );
	}
	
	//-----------------------------------------------------------------------------------
	//! Get fallback content by tag 
	protected DetailsPanelContentPreset FallbackContentByTag(string contentTag)
	{	
		foreach (DetailsPanelContentPreset content : m_FallbackContent.m_aContent)
		{
			if (content.m_sTag == contentTag)
				return content;
		}
		
		return m_FallbackContent.m_DefaultContent;
	}
	
	//-----------------------------------------------------------------------------------
	//! Set description text with enforced text lenght limit
	protected void SetDescriptionText(string text)
	{
		// Edit text if it's too long
		if (text.Length() > m_iMaxDescriptionLenght + DESCRIPTION_LENGHT_OFFSET)
		{
			text = text.Substring(0, m_iMaxDescriptionLenght);
			text += "...";
		}
		
		widgets.m_DescriptionText.SetText(text);
	}
};


//-----------------------------------------------------------------------------------
[BaseContainerProps(configRoot : true)]
class DetailsPanelContentPresetConfig
{
	[Attribute()]
	ref DetailsPanelContentPreset m_DefaultContent;
	
	[Attribute()]
	ref array<ref DetailsPanelContentPreset> m_aContent;
};

//-----------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomTitleField("m_sTag")]
class DetailsPanelContentPreset
{
	[Attribute()]
	string m_sTag;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "edds")]
	ResourceName m_sImage;
	
	[Attribute()]
	string m_sTitle;
	
	[Attribute()]
	string m_sTitleImageName;
	
	[Attribute("255 255 255 255")]
	ref Color m_sTitleImageColor;
	
	[Attribute()]
	string m_sDescription;
};
