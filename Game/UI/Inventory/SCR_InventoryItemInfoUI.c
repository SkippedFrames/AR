[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Item Info UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventoryItemInfoUI : ScriptedWidgetComponent
{

	private Widget							m_infoWidget;
	private VerticalLayoutWidget			m_wHintWidget;
	private TextWidget						m_wTextName;
	private TextWidget						m_wTextDescription;
	private TextWidget						m_wTextWeight;
	private ImageWidget 					m_wItemIcon;
	protected SCR_SlotUIComponent			m_pFrameSlotUI;
	protected Widget 						m_wWidgetUnderCursor;
	protected bool 							m_bForceShow;
	
	private string 							m_sHintLayout = "{9996B50BE8DFED5F}UI/layouts/Menus/Inventory/InventoryItemHintElement.layout";
	
					
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	//------------------------------------------------------------------------------------------------
	protected void ShowInfoWidget( bool bShow )
	{
		if (m_bForceShow)
		{
			m_infoWidget.SetVisible( true );
			return;
		}

		if ( !m_wWidgetUnderCursor )
			return;
		if ( WidgetManager.GetWidgetUnderCursor() != m_wWidgetUnderCursor )
			return; //the cursor is on different position already
		m_infoWidget.SetVisible( true );
	}

	//------------------------------------------------------------------------------------------------
	void Show( float fDelay = 0.0, Widget w = null, bool forceShow = false )
	{
		m_bForceShow = forceShow;
		m_wWidgetUnderCursor = w;
		if ( fDelay == 0 )
			ShowInfoWidget( true );
		else
		{
			GetGame().GetCallqueue().Remove( ShowInfoWidget );
			GetGame().GetCallqueue().CallLater( ShowInfoWidget, fDelay*1000, false, true );
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName iconPath, Color color = null)
	{
		if (iconPath.IsEmpty())
			return;
		
		m_wItemIcon.SetVisible(m_wItemIcon.LoadImageTexture(0, iconPath));
		if (color)
			m_wItemIcon.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void ShowIcon(bool isVisible)
	{
		m_wItemIcon.SetVisible(isVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide( float fDelay = 1.0 )
	{
		m_infoWidget.SetVisible( false );
		m_infoWidget.SetEnabled( false );
		m_wItemIcon.SetVisible(false);
		
		Widget childWidget = m_wHintWidget.GetChildren();
		while (childWidget)
		{
			Widget next = childWidget.GetSibling();
			m_wHintWidget.RemoveChild(childWidget);
			childWidget = next;
		}
	}
		
	//------------------------------------------------------------------------------------------------
	void Move( float x, float y )
	{
		if ( !m_pFrameSlotUI )
			return;
		m_pFrameSlotUI.SetPosX( x );
		m_pFrameSlotUI.SetPosY( y );
	}

	//------------------------------------------------------------------------------------------------
	void SetName( string sName )
	{
		m_wTextName.SetText( sName );
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDescription( string sDescr )
	{
		if( sDescr != "" )
		{
			m_wTextDescription.SetEnabled( true );
			m_wTextDescription.SetVisible( true );
			m_wTextDescription.SetText( sDescr );
		}
		else
		{
			m_wTextDescription.SetEnabled( false );
			m_wTextDescription.SetVisible( false );
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetItemHints(InventoryItemComponent item, array<SCR_InventoryItemHintUIInfo> itemHintArray )
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		foreach (SCR_InventoryItemHintUIInfo hintUIInfo : itemHintArray)
		{
			if (!hintUIInfo.CanBeShown(item))
				continue;
			
			Widget createdWidget = workspace.CreateWidgets(m_sHintLayout, m_wHintWidget);
			if (!createdWidget)
				return;
			
			hintUIInfo.SetItemHintNameTo(item, RichTextWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintText")));
			hintUIInfo.SetIconTo(ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon")));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWeight( string sWeight )
	{
		if (!sWeight.IsEmpty())
		{
			m_wTextWeight.SetEnabled( true );
			m_wTextWeight.SetVisible( true );
			m_wTextWeight.SetTextFormat("#AR-ValueUnit_Short_Kilograms", sWeight);
		}
		else
		{
			m_wTextWeight.SetEnabled( false );
			m_wTextWeight.SetVisible( false );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWeight( float fWeight )
	{
		if( fWeight <= 0.0 )
			SetWeight( "" );						
		else
			SetWeight( fWeight.ToString() );
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		if( !w )
			return;
		m_infoWidget		= w;
		m_wHintWidget		= VerticalLayoutWidget.Cast( w.FindAnyWidget( "VerticalHintParent" ) );
		m_wTextName 		= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_name" ) );
		m_wTextDescription 	= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_description" ) );
		m_wTextWeight 		= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_weight" ) );
		m_wItemIcon 		= ImageWidget.Cast(w.FindAnyWidget("ItemInfo_icon"));
		Widget wItemInfo	= m_infoWidget.FindAnyWidget( "ItemInfo" );
		if ( !wItemInfo )
			return;
		m_pFrameSlotUI 		= SCR_SlotUIComponent.Cast( wItemInfo.FindHandler( SCR_SlotUIComponent ) );
	}
	
	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if ( m_infoWidget )
		{		
			GetGame().GetCallqueue().Remove( ShowInfoWidget );
			m_infoWidget.RemoveHandler( m_pFrameSlotUI );
			m_infoWidget.RemoveHandler( this );
			m_infoWidget.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetInfoWidget()
	{
		return m_infoWidget.FindAnyWidget("size");
	}

	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryItemInfoUI()
	{
	}	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryItemInfoUI()
	{
	}
};
