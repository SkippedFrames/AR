[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableCommentComponentClass : SCR_EditableEntityComponentClass
{
}

/** @ingroup Editable_Entities
*/

/*!
*/
class SCR_EditableCommentComponent : SCR_EditableEntityComponent
{
	[Attribute(category: "Comment", defvalue: "1 1 1 1")]
	private ref Color m_sColor;

	[Attribute(category: "Comment", defvalue: "1", uiwidget: UIWidgets.Slider, params: "0.5 2 0.1")]
	private float m_fSizeCoef;

	[Attribute(category: "Comment")]
	private bool m_bBold;

	[Attribute(category: "Comment")]
	private bool m_bItalic;

	//------------------------------------------------------------------------------------------------
	/*!
	Apply custom attributes on given text widget.
	\param textWidget Modified text
	*/
	void ApplyTo(TextWidget textWidget)
	{
		textWidget.SetText(GetDisplayName());
		textWidget.SetColor(m_sColor);
		textWidget.SetBold(m_bBold);
		textWidget.SetItalic(m_bItalic);

		float size;
		textWidget.GetTextSize(size, size);
		textWidget.SetExactFontSize(size * m_fSizeCoef);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Apply custom attributes on given image widget.
	\param iconWidget Modified image
	*/
	void ApplyTo(ImageWidget iconWidget)
	{
		iconWidget.SetColor(m_sColor);

		SCR_UIInfo info = GetInfo();
		if (info)
			info.SetIconTo(iconWidget);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get comment color
	\return Color
	*/
	Color GetColor()
	{
		return m_sColor;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get font size coef
	\return Coef
	*/
	float GetSizeCoef()
	{
		return m_fSizeCoef;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the test should be bold.
	\return True if bold
	*/
	bool IsBold()
	{
		return m_bBold;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the test should be italic.
	\return True if italic
	*/
	bool IsItalic()
	{
		return m_bItalic;
	}

#ifdef WORKBENCH
	private const float _WB_DRAW_DISTANCE = 1500 * 1500; //--- Squared value

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		SCR_UIInfo info = GetInfo();
		if (!info)
			return;

		GenericEntity genericOwner = GenericEntity.Cast(owner);
		if (!genericOwner)
			return;

		WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(genericOwner);
		if (!src)
			return;

		if (!api.IsEntityVisible(genericOwner) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			return;

		vector cameraTransform[4];
		genericOwner.GetWorld().GetCurrentCamera(cameraTransform);

		vector pos;
		if (!GetPos(pos))
			return;

		int colorBackground = 0x00000000;
		if (api.IsEntitySelected(genericOwner))
		{
			colorBackground = ARGBF(1, 0, 0, 0);
		}
		else
		{
			//--- Draw only if near when not selected
			if (vector.DistanceSq(pos, cameraTransform[3]) > _WB_DRAW_DISTANCE)
				return;
		}

		float testSize = 16 * m_fSizeCoef;
		int color = m_sColor.PackToInt();

		string displayName = GetDisplayName();
		int underscore = displayName.LastIndexOf("_");
		if (underscore != -1)
		{
			underscore++;
			displayName = displayName.Substring(underscore, displayName.Length() - underscore);
		}

		DebugTextWorldSpace.Create(genericOwner.GetWorld(), displayName, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, pos[0], pos[1], pos[2], testSize, color, colorBackground);
	}
#endif
}
