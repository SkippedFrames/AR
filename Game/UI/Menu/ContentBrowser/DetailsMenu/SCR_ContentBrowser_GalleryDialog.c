/*
Dialog which shows big image gallery over whole screen.
*/

class SCR_ContentBrowser_GalleryDialog : MenuBase
{
	protected ref SCR_ContentBrowser_GalleryDialogWidgets m_Widgets = new SCR_ContentBrowser_GalleryDialogWidgets;
	
	protected ref array<ref BackendImage> m_aImages = {};
	
	protected int m_iCurrentItem;
	
	//----------------------------------------------------------------------------------
	static SCR_ContentBrowser_GalleryDialog CreateForImages(array<BackendImage> images, int selectedImageIndex)
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GalleryDialog);
		SCR_ContentBrowser_GalleryDialog galleryDialog = SCR_ContentBrowser_GalleryDialog.Cast(menuBase);
		galleryDialog.Init(images, selectedImageIndex);
		return galleryDialog;
	}
	
	
	//----------------------------------------------------------------------------------
	protected void Init(array<BackendImage> images, int selectedImageIndex)
	{
		m_Widgets.m_SpinBoxComponent.ClearAll();
		m_aImages.Clear();
		foreach (auto img : images)
		{
			m_Widgets.m_SpinBoxComponent.AddItem(string.Empty);
			m_aImages.Insert(img);		
		}
		
		if (!m_aImages.IsEmpty())
		{
			m_iCurrentItem = selectedImageIndex;
			ShowImage(selectedImageIndex);
		}
	}
	
	
	//----------------------------------------------------------------------------------
	protected void ShowImage(int id)
	{
		if (id < 0 || id >= m_aImages.Count())
			return;
		
		BackendImage backendImage = m_aImages[id];
		
		m_Widgets.m_BackendImageComponent.SetImage(backendImage);
		
		m_Widgets.m_SpinBoxComponent.SetCurrentItem(id);
		
		UpdateButtons();
	}
	
	
	//----------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		m_Widgets.m_NextButtonComponent.SetEnabled(m_iCurrentItem < (m_aImages.Count() - 1));
		m_Widgets.m_PrevButtonComponent.SetEnabled(m_iCurrentItem > 0);
	}
	
	
	//------------------------------------------------------------------------------------------
	//! Offset - how much to move current item ID right/left
	protected void OffsetCurrentItem(int offset)
	{
		m_iCurrentItem = m_iCurrentItem + offset;
		m_iCurrentItem = Math.ClampInt(m_iCurrentItem, 0, m_aImages.Count() - 1);
		ShowImage(m_iCurrentItem);
	}
	
	
	//----------------------------------------------------------------------------------
	protected void OnBackButton()
	{
		this.Close();
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnNextButton()
	{
		OffsetCurrentItem(1);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnPrevButton()
	{
		OffsetCurrentItem(-1);
	}
	
	//----------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_Widgets.Init(GetRootWidget());
		
		m_Widgets.m_BackButtonComponent.m_OnActivated.Insert(OnBackButton);
		m_Widgets.m_NextButtonComponent.m_OnClicked.Insert(OnNextButton);
		m_Widgets.m_PrevButtonComponent.m_OnClicked.Insert(OnPrevButton);
		
		// Close this dialog when user clicks outside
		SCR_ModularButtonComponent backgroundButtonComponent = SCR_ModularButtonComponent.Cast(GetRootWidget().FindHandler(SCR_ModularButtonComponent));
		backgroundButtonComponent.m_OnClicked.Insert(OnBackButton);
		
		// Also close this when user clicks on cross at top-right
		m_Widgets.m_CloseButtonComponent.m_OnClicked.Insert(OnBackButton);
		
		// Listen to inputs
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().AddActionListener("MenuRight", EActionTrigger.DOWN, OnNextButton);
			GetGame().GetInputManager().AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPrevButton);
		}
		
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}
	
	
	//----------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnNextButton);
			GetGame().GetInputManager().RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPrevButton);
		}
	}
};
