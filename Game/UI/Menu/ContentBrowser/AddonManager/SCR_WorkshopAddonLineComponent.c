/*
Component to be attached to addon lines.
*/

//----------------------------------------------------------------------------------------------
class SCR_WorkshopAddonLineComponent : SCR_AddonLineBaseComponent
{
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	//----------------------------------------------------------------------------------------------
	bool IsWorkshopItemEnabled()
	{
		if (!m_Item)
			return false;
		
		return m_Item.GetEnabled();
	}
	
	//----------------------------------------------------------------------------------------------
	override void Init(SCR_WorkshopItem item)
	{
		super.Init(item);
		
		// OnChanged is called whenever something happens with the workshop item.
		// Simplest approach is to refresh whole line associated with the workshop item on this event.
		item.m_OnChanged.Insert(UpdateAllWidgets);
	}
	
	
	//----------------------------------------------------------------------------------------------
	override protected void UpdateAllWidgets()
	{	
		if (!m_bCanUpdate)
			return;
		
		if (!m_Item)
			return;
	
		bool mouseOver = m_bMouseOver;
			
		// Update name
		m_Widgets.m_NameText.SetText(m_Item.GetName());
		
		// Enable buttons
		bool enabled = m_Item.GetEnabled();
		
		HandleEnableButtons(enabled);
		
		m_Widgets.m_SizeMoveRight.SetVisible(!enabled);
		m_Widgets.m_MoveRightButton.SetVisible(!enabled && m_bMouseOver || m_bFocused);
		
		m_Widgets.m_SizeMoveLeft.SetVisible(enabled);
		m_Widgets.m_MoveLeftButton.SetVisible(enabled && m_bMouseOver || m_bFocused);
		
		// Update state text
		string stateText;
		bool downloading = m_Item.GetDownloadAction() || m_Item.GetDependencyCompositeAction();
		bool problemCritical;
		string problemDescription;
		//bool problem = SCR_WorkshopUiCommon.GetHighestPriorityProblemDescription(m_Item, problemDescription, problemCritical);
		if (downloading)
		{
			float progress = SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);
			stateText = string.Format("%1%%", Math.Round(100.0*progress));
			m_Widgets.m_HorizontalState.SetVisible(true);
			m_Widgets.m_UpdateButton.SetVisible(false);
		}
		/*else if (problem)
		{
			stateText = problemDescription;
			m_Widgets.m_StateText.SetVisible(mouseOver); // Those texts are too big, had to disable
		}*/
		else
			m_Widgets.m_HorizontalState.SetVisible(false);
		
		m_Widgets.m_StateText.SetText(stateText);
		
		EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();
		
		
		m_Widgets.m_FixButton.SetVisible(
			problem == EWorkshopItemProblem.BROKEN ||
			problem == EWorkshopItemProblem.DEPENDENCY_MISSING ||
			problem == EWorkshopItemProblem.DEPENDENCY_OUTDATED
		);
		m_Widgets.m_UpdateButton.SetVisible(problem == EWorkshopItemProblem.UPDATE_AVAILABLE);
		
		// Update buttons 
		if (m_ActionButtons) 
			m_ActionButtons.ShowAllButtons(m_bMouseOver);
	}
	
	//----------------------------------------------------------------------------------------------
	// Callbacks 
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	override void OnDeleteButton()
	{
		if (!m_Item)
			return;
		
		SCR_DeleteAddonDialog.CreateDeleteAddon(m_Item);
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnActionButton()
	{
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnUpdateButton()
	{
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//----------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		if (m_Item)
			m_Item.m_OnChanged.Remove(UpdateAllWidgets); // Remember to unsubscribe from OnChanged event when this UI component is destroyed!
	}
}
