[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_KickPlayerContextAction: SCR_SelectedEntitiesContextAction
{
	[Attribute("0", desc: "How long before kicked-out player can reconnect.\n-1 means permanent ban (at least until exe restart)")]
	protected int m_iKickTimeout;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Reason for kicking shown to kicked-out player.", enums: ParamEnumArray.FromEnum(SCR_PlayerManagerKickReason))]
	protected SCR_PlayerManagerKickReason m_KickReason;
	
	[Attribute(SCR_CommonDialogs.DIALOGS_CONFIG)]
	protected ResourceName m_DialogSource;
	
	[Attribute()]
	protected string m_sDialogSourceName;
	
	protected SCR_EditablePlayerDelegateComponent m_PlayerDelegate;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_EditablePlayerDelegateComponent playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(selectedEntity);
		return playerDelegate && !SCR_Global.IsAdmin(playerDelegate.GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	/*override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(selectedEntity, cursorWorldPosition, flags);
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			m_PlayerDelegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
			if (m_PlayerDelegate)
			{
				SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_DialogSource, m_sDialogSourceName);
				dialog.m_OnConfirm.Insert(OnKickDialogConfirm);
			}			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Kick player on confirm
	protected void OnKickDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		dialog.m_OnConfirm.Remove(OnKickDialogConfirm);
		
		if (m_PlayerDelegate)
			GetGame().GetPlayerManager().KickPlayer(m_PlayerDelegate.GetPlayerID(), m_KickReason, m_iKickTimeout);
	}
};
