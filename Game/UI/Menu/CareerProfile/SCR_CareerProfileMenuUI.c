//------------------------------------------------------------------------------------------------
class SCR_CareerProfileMenuUI: SCR_SuperMenuBase
{
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_InputButtonComponent comp = SCR_InputButtonComponent.GetInputButtonComponent("Back",GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);

		bool showBlur = !GetGame().m_bIsMainMenuOpen;

		Widget img = GetRootWidget().FindAnyWidget("MenuBackground");
		if (img)
			img.SetVisible(!showBlur);

		BlurWidget blur = BlurWidget.Cast(GetRootWidget().FindAnyWidget("Blur0"));
		if (blur)
		{
			blur.SetVisible(showBlur);
			if (showBlur)
			{
				float w, h;
				GetGame().GetWorkspace().GetScreenSize(w, h);
				blur.SetSmoothBorder(0, 0, w * 0.8, 0);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		Close();
	}
};
