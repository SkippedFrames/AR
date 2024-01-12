[EntityEditorProps(category: "GameScripted/GameMode")]
class SCR_PositionClass : GenericEntityClass
{
};

class SCR_Position : GenericEntity
{
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	int m_iColor = Color.WHITE;
	string m_sText = string.Empty;

	//------------------------------------------------------------------------------------------------
	void SetColorAndText()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		SetColorAndText();

		vector mat[4];
		GetWorldTransform(mat);

		// Draw point and arrow
		vector position = mat[3];
		ref Shape pointShape = Shape.CreateSphere(m_iColor, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		ref Shape arrowShape = Shape.CreateArrow(position, position + mat[2], 0.2, m_iColor, ShapeFlags.ONCE);

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api)
		{
			IEntity selectEntity = api.GetSelectedEntity();
			if (!SCR_Position.Cast(selectEntity))
				return;
		}

		if (!m_sText.IsEmpty())
		{
			ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
				GetWorld(),
				m_sText,
				DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
				position[0],
				position[1] + 0.4,
				position[2],
				20.0,
				m_iColor);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	void SCR_Position(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);
	}	
};
