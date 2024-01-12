//------------------------------------------------------------------------------------------------
class SCR_ProfileEditSubMenu : SCR_SubMenuBase
{
	[Attribute("EditName")]
	protected string m_sNameButtonName;
	
	[Attribute("EditPicture")]
	protected string m_sPictureButtonName;
	
	protected DialogUI m_OpenedDialog;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu) 
	{
		super.OnMenuOpen(parentMenu);
		
		SCR_ButtonBaseComponent name = SCR_ButtonBaseComponent.GetButtonBase(m_sNameButtonName, m_wRoot);
		if (name)
			name.m_OnClicked.Insert(OnNameClicked);
		
		SCR_ButtonBaseComponent picture = SCR_ButtonBaseComponent.GetButtonBase(m_sPictureButtonName, m_wRoot);
		if (picture)
			picture.m_OnClicked.Insert(OnPictureClicked);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNameClicked()
	{
		m_OpenedDialog = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ProfileDialog));
		if (!m_OpenedDialog)
			return;
		
		//m_OpenedDialog.m_OnConfirm.Insert(OnProfileChanged);

		SCR_EditBoxComponent edit = SCR_EditBoxComponent.GetEditBoxComponent("ProfileName", m_OpenedDialog.GetRootWidget());
		if (!edit)
			return;
		
		string name = SCR_Global.GetProfileName();
		edit.SetValue(name);
	}

	//------------------------------------------------------------------------------------------------
	void OnProfileChanged()
	{
		if (!m_OpenedDialog)
			return;
		
		SCR_EditBoxComponent edit = SCR_EditBoxComponent.GetEditBoxComponent("ProfileName", m_OpenedDialog.GetRootWidget());
		if (edit)
		{
			string name = edit.GetValue();
			// TODO: Future feature of custom name
		}
		
		m_OpenedDialog.CloseAnimated();
	}
		
	//------------------------------------------------------------------------------------------------
	void OnPictureClicked()
	{
		// TODO: Implement when image change is possible
	}
};
