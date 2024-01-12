[EntityEditorProps(category: "GameScripted/Utility", description: "Schedules a spherical region of the world for preload.", color: "100 255 255 255", style: "sphere" )]
class SCR_ShedulePreloadEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
sealed class SCR_ShedulePreloadEntity : GenericEntity
{
	[Attribute("500", UIWidgets.Slider, "The radius in metres from the center of this entity which should be scheduled to preload.", "0 1500 1")]
	private float m_fPreloadRadius;
	
	[Attribute("0", UIWidgets.CheckBox, "Positions first used camera to this preload location.")]
	private bool m_bPositionCamera;
	
	
	#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		ref auto pShape = Shape.CreateSphere(ARGB(100,0,255,0), ShapeFlags.ONCE | ShapeFlags.WIREFRAME, GetOrigin(), m_fPreloadRadius);
		
		if (m_bPositionCamera)
		{
			vector pos, rot;
			GetDesiredCameraTransform(pos, rot);
			
			auto dir = rot.AnglesToVector();
			auto  color = ARGB(100,0,255,64);
			auto flags = ShapeFlags.ONCE | ShapeFlags.WIREFRAME;
			ref auto pCamShape = Shape.CreateSphere(color, flags, pos, 0.01);
			ref auto pCamArrow = Shape.CreateArrow(pos, pos + dir, 0.01, color, flags);
		}
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	protected void GetDesiredCameraTransform(out vector position, out vector angles)
	{
		vector offset = 0.5 * m_fPreloadRadius * vector.Up;
		position = GetOrigin() + offset;
		angles = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ShedulePreloadEntity(IEntitySource src, IEntity parent)
	{
		auto pGame = GetGame();
		if (!pGame)
			return;
		
		auto pWorld = pGame.GetWorld();
		if (!pWorld)
			return;
		
		if (m_bPositionCamera)
		{
			int cameraId = pWorld.GetCurrentCameraId();
			if (cameraId >= 0)
			{
				vector pos, rot;
				GetDesiredCameraTransform(pos, rot);
				pWorld.SetCamera(cameraId, pos, rot);
			}
		}
		
		pWorld.SchedulePreload(GetOrigin(), m_fPreloadRadius);
	}

};
