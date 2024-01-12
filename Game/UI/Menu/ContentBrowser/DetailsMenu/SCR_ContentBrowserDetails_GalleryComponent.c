/*
Component of a gallery in addon details menu.
*/

class SCR_ContentBrowserDetails_GalleryComponent : ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.Auto, "Names of tile widgets")]
	protected ref array<string> m_aTileWidgetNames;
	
	[Attribute("", UIWidgets.Auto, "See LoadImageTexture documentation")]
	protected bool m_bFromLocalStorage;
	
	// Array with image preview widgets
	protected ref array<Widget> m_aTiles = {};
	protected ref array<SCR_BackendImageComponent> m_aTileComponents = {};
	protected ref array<ref BackendImage> m_aImages = {};
	
	protected ref SCR_ContentBrowser_GalleryWidgets m_Widgets = new SCR_ContentBrowser_GalleryWidgets;
	
	protected int m_iCurrentItem = 0;
	
	
	// -------------------- Public ----------------------
	
	//------------------------------------------------------------------------------------------
	void SetImages(array<BackendImage> images)
	{
		m_aImages.Clear();
		foreach (auto img : images)
		{
			m_aImages.Insert(img);
			m_Widgets.m_SpinBoxComponent.AddItem(string.Empty);
		}
			
		UpdateAllWidgets();
	}
	
	
	// ----------- Protected -----------
	
	protected void Update()
	{
		// Update current highlighted dot
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		
		int tileId = m_aTiles.Find(focusedWidget);
		if (tileId != -1)
		{
			int spinBoxItemId = m_iCurrentItem + tileId;
			if (m_Widgets.m_SpinBoxComponent.GetCurrentIndex() != spinBoxItemId)
				m_Widgets.m_SpinBoxComponent.SetCurrentItem(spinBoxItemId);
		}
		else
		{
			int currentItem = m_Widgets.m_SpinBoxComponent.GetCurrentIndex();
			int spinBoxItemId = Math.ClampInt(currentItem, m_iCurrentItem, m_iCurrentItem + m_aTiles.Count() - 1);
			if (m_Widgets.m_SpinBoxComponent.GetCurrentIndex() != spinBoxItemId)
				m_Widgets.m_SpinBoxComponent.SetCurrentItem(spinBoxItemId);
		}
		
		GetGame().GetCallqueue().CallLater(Update, 10);
	}
	
	protected void UpdateAllWidgets()
	{
		UpdateTiles();
		
		m_Widgets.m_NextButtonComponent.SetEnabled(m_iCurrentItem < (m_aImages.Count() - m_aTiles.Count()));
		m_Widgets.m_PrevButtonComponent.SetEnabled(m_iCurrentItem > 0);
	}
	
	//------------------------------------------------------------------------------------------
	//! Updates images on tiles according to currently selected item
	protected void UpdateTiles()
	{
		int nTiles = m_aTiles.Count();
		int nImages = m_aImages.Count();
		
		for (int iTile = 0; iTile < nTiles; iTile++)
		{
			int iImage = m_iCurrentItem + iTile;
			
			if (iImage < nImages && iImage >= 0)
				ShowImageOnTile(iTile, m_aImages[iImage]);
			else
				ShowImageOnTile(iTile, null);
		}
	}
	
	
	//------------------------------------------------------------------------------------------
	//! Shows image on one of preview image boxes
	//! Preview image box is of this layout:
	//! {F7F8BD8128A6CEAF}UI/layouts/Menus/ContentBrowser/DetailsMenu/Gallery/ContentBrowser_Gallery_ImageWithBackground.layout
	protected void ShowImageOnTile(int id, BackendImage backendImage)
	{
		SCR_BackendImageComponent comp = m_aTileComponents[id];
		
		if (!comp)
			return;
		
		
		Widget tile = m_aTiles[id];
		ImageWidget wImage = ImageWidget.Cast(tile.FindAnyWidget("Image"));
		
		// If we want to show no image, hide image widget, disable the tile so it can't be focused
		wImage.SetVisible(backendImage != null);
		tile.SetEnabled(backendImage != null);
		
		comp.SetImage(backendImage);
	}
	
	//------------------------------------------------------------------------------------------
	//! Offset - how much to move current item ID right/left
	protected void OffsetCurrentItem(int offset)
	{
		m_iCurrentItem = m_iCurrentItem + offset;
		int maxItemId = m_aImages.Count() - m_aTiles.Count();
		if (maxItemId < 0)
			maxItemId = 0;
		m_iCurrentItem = Math.ClampInt(m_iCurrentItem, 0, maxItemId);
		UpdateAllWidgets();
	}
	
	
	//------------------------------------------------------------------------------------------
	//! Called when a tile is clicked
	protected void OnTileClick(SCR_ModularButtonComponent comp)
	{
		// Bail if we have no images at all
		if (m_aImages.IsEmpty())
			return;
		
		// Find what image was selected in this tile
		int tileId = m_aTiles.Find(comp.GetRootWidget());
		
		if (tileId == -1)
			return;
		
		int imageId = m_iCurrentItem + tileId;
		
		// Correct the image ID if a tile without an image was selected
		imageId = Math.Clamp(imageId, 0, m_aImages.Count() - 1);
		
		array<BackendImage> galleryDialogImages = {};
		foreach (auto img : m_aImages)
			galleryDialogImages.Insert(img);
		
		SCR_ContentBrowser_GalleryDialog.CreateForImages(galleryDialogImages, imageId);
	}
	
	
	
	
	
	
	// Action listeners. They are invoked by right/left keys.
	
	//------------------------------------------------------------------------------------------
	protected void OnNextAction()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_aTiles[m_aTiles.Count()-1])
			return;
		OffsetCurrentItem(1);		
	}
	
	
	//------------------------------------------------------------------------------------------
	protected void OnPrevAction()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_aTiles[0])
			return;
		OffsetCurrentItem(-1);		
	}
	
	
	
	
	
	
	// Event handlers for left/right buttons
	
	//------------------------------------------------------------------------------------------
	protected void OnNextButton()
	{
		OffsetCurrentItem(1);
		GetGame().GetWorkspace().SetFocusedWidget(m_aTiles[m_aTiles.Count()-1]);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnPrevButton()
	{
		OffsetCurrentItem(-1);
		GetGame().GetWorkspace().SetFocusedWidget(m_aTiles[0]);
	}
	
	
	
	
	
	//------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		if (!SCR_Global.IsEditMode())
			GetGame().GetCallqueue().CallLater(UpdateSize, 0);
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		if (m_aTiles.IsEmpty())
			return;
		
		// Resize the height of all images, we must keep same aspect ratio
		float sizex, sizey;
		m_aTiles[0].GetScreenSize(sizex, sizey);
		float sizexUnscaled = GetGame().GetWorkspace().DPIUnscale(sizex);
		m_Widgets.m_ImagesHeightSize.EnableHeightOverride(true);
		m_Widgets.m_ImagesHeightSize.SetHeightOverride(sizexUnscaled / SCR_WorkshopUiCommon.IMAGE_SIZE_RATIO);
	}
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widgets.Init(w);
	
		
		// Find image gallery widgets
		foreach (string widgetName : m_aTileWidgetNames)
		{
			Widget wImageTile = w.FindAnyWidget(widgetName);
			m_aTiles.Insert(wImageTile);
			
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.Cast(wImageTile.FindHandler(SCR_ModularButtonComponent));
			comp.m_OnClicked.Insert(OnTileClick);
			
			if (wImageTile)
			{
				SCR_BackendImageComponent backendImgComp = SCR_BackendImageComponent.Cast(wImageTile.FindHandler(SCR_BackendImageComponent));
				m_aTileComponents.Insert(backendImgComp);
			}
		}
		
		// Reset the state of dots
		if (!SCR_Global.IsEditMode())
			m_Widgets.m_SpinBoxComponent.ClearAll();
		
		// Listen to inputs
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().AddActionListener("MenuRight", EActionTrigger.DOWN, OnNextAction);
			GetGame().GetInputManager().AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPrevAction);
		}
		
		m_Widgets.m_NextButtonComponent.m_OnClicked.Insert(OnNextButton);
		m_Widgets.m_PrevButtonComponent.m_OnClicked.Insert(OnPrevButton);
		
		m_Widgets.m_SpinBoxComponent.SetCurrentItem(0);
		
		UpdateAllWidgets();
		
		GetGame().GetCallqueue().CallLater(Update, 50);
	}
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnNextAction);
			GetGame().GetInputManager().RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPrevAction);
		}
		
		GetGame().GetCallqueue().Remove(Update);
	}
};
