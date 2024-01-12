//------------------------------------------------------------------------------------------------
class SCR_NearbyContextDisplay : SCR_InfoDisplayExtended
{
	[Attribute("3", UIWidgets.Slider, "Maximum distance of actions that can be presented to player. (meters)", category: "NearbyContextDisplay", params: "0 100 0.1")]
	protected float m_fWidgetMaximumRange;
	[Attribute("0.7", UIWidgets.Slider, "Minimum assumed distance of actions (for scaling). (meters)", category: "NearbyContextDisplay", params: "0 100 0.1")]
	protected float m_fWidgetMinimumRange;

	[Attribute("128", UIWidgets.Slider, "Amount of widgets that are created on initialization and then re-used.", category: "NearbyContextDisplay", params: "0 256 1")]
	protected int m_iPrecachedWidgetCount;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout", category: "NearbyContextDisplay")]
	ResourceName m_wIconLayoutPath;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, category: "NearbyContextDisplay")]
	protected vector m_vVisibleWidgetColor;
	[Attribute("0.1 0.1 0.1 1", UIWidgets.ColorPicker, category: "NearbyContextDisplay")]
	protected vector m_vNotVisibleWidgetColor;
	
	
	/*!
		Defines horizontal opacity (y) based on distance from screen center (x).
	*/
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Horizontal opacity (y) based on screen center distance (x).", category: "NearbyContextDisplay")]
	private ref Curve m_pHorizontalOpacityCurve;
	/*!
		Defines vertical opacity (y) based on distance from screen center (x).
	*/
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Vertical opacity (y) based on screen center distance (x).", category: "NearbyContextDisplay")]
	private ref Curve m_pVerticalOpacityCurve;
	
	private ref Color m_pColorVisible;
	private ref Color m_pColorNotVisible;
	
	//! Interaction handler attached to parent entity.
	private InteractionHandlerComponent m_pInteractionHandlerComponent;	
	
	//! Array of widgets that are filled and re-used as deemed neccessary.
	private ref array<Widget> m_aWidgets = {};
	//! Currently visible widgets.
	private int m_iVisibleWidgets;
	//! Original widget width.
	private float m_fOriginalSizeX;
	//! Original widget height.
	private float m_fOriginalSizeY;
	
	//------------------------------------------------------------------------------------------------
	//! Performs cleanup.
	void ~SCR_NearbyContextDisplay()
	{
		int widgetsCount = m_aWidgets.Count();
		for (int i = widgetsCount-1; i >= 0; i--)
		{
			if (m_aWidgets[i])
				m_aWidgets[i].RemoveFromHierarchy();
		}
		
		m_aWidgets.Clear();
		
		m_pColorVisible = null;
		m_pColorNotVisible = null;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_aWidgets.IsEmpty())
		{
			Print("Pre-cached SCR_NearbyContextDisplay widgets cannot be detected; re-rooting to m_wRoot cannot happen!", LogLevel.ERROR);
			return;
		}

		if (!m_wRoot)
		{
			Print("The SCR_NearbyContextDisplay root is NULL, the pre-cached SCR_NearbyContextDisplay widgets cannot be re-rooted!", LogLevel.ERROR);
			return;
		}
		
			
		// Re-root the blips to the info display root
		foreach (Widget w : m_aWidgets)
			m_wRoot.AddChild(w);
	}	
		
	//------------------------------------------------------------------------------------------------
	override void DisplayInit(IEntity owner)
	{
		m_pInteractionHandlerComponent = InteractionHandlerComponent.Cast(owner.FindComponent(InteractionHandlerComponent));
		if (!m_pInteractionHandlerComponent)
			Print("Interaction handler component could not be found! SCR_NearbyContextDisplay will not work.", LogLevel.WARNING);
		
		
		m_pColorVisible = new Color(m_vVisibleWidgetColor[0], m_vVisibleWidgetColor[1], m_vVisibleWidgetColor[2], 1.0);
		m_pColorNotVisible = new Color(m_vNotVisibleWidgetColor[0], m_vNotVisibleWidgetColor[1], m_vNotVisibleWidgetColor[2], 1.0);
		
		// Pre-cache widgets
		for (int i = 0; i < m_iPrecachedWidgetCount; i++)
		{
			// Instantiate widgets
			Widget iconWidget = GetGame().GetWorkspace().CreateWidgets(m_wIconLayoutPath);
			if (!iconWidget)
			{
				if (m_wIconLayoutPath.IsEmpty())
					Print("SCR_NearbyContextDisplay could not create widgets! Verify that provided Icon Layout Path is valid!", LogLevel.ERROR);
				break;
			}
			
			m_aWidgets.Insert(iconWidget);
			// Disable their visibility by default
			iconWidget.SetVisible(false);
			// Prepare their transformation
			FrameSlot.SetAnchor(iconWidget, 0, 0);
			FrameSlot.SetAlignment(iconWidget, 0.5, 0.5);
			FrameSlot.SetSizeToContent(iconWidget, true);
		}
		
		// Store default value
		if (m_aWidgets[0])
		{
			Widget icon = FindIconWidget(m_aWidgets[0]);
			vector size = FrameSlot.GetSize(icon);
			m_fOriginalSizeX = size[0];
			m_fOriginalSizeY = size[1];
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		// Were we drawing something?
		// If yes, then disable all
		if (m_iVisibleWidgets > 0)
			SetVisibleWidgets(0);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// m_pInteractionHandlerComponent.SetNearbyCollectionEnabled(true);
		if (!m_pInteractionHandlerComponent.GetNearbyCollectionEnabled())
		{
			// Were we drawing something?
			// If yes, then disable all
			if (m_iVisibleWidgets > 0)
				SetVisibleWidgets(0);
		
			return;
		}
		
		// Fetch nearby contexts
		array<UserActionContext> contexts = {};
		int count = m_pInteractionHandlerComponent.GetNearbyAvailableContextList(contexts);
		
		// Prepare widgets
		int actualCount = 0;
		
		// Get required data
		BaseWorld world = owner.GetWorld();
		int cameraIndex = world.GetCurrentCameraId();
		vector mat[4];
		world.GetCamera(cameraIndex, mat);
		vector referencePos = mat[3];
		
		// Iterate through individual contexts,
		// validate that they are visible,
		// and update their widget representation
		const float threshold = 0.25;
		const float fovBase = 100; // whatever fits
		
		// Field of view and screen resolution is necessary to compute proper position and size of widgets
		float zoom = 1; // world.GetCameraVerticalFOV(cameraIndex) - missing crucial getter
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
				zoom = fovBase / Math.Max(camera.GetVerticalFOV(), 1);
		}
		float distanceLimit =  m_fWidgetMaximumRange * zoom;
		float distanceLimitSq = distanceLimit * distanceLimit;
		// Screen resolution is necessary to know how far away the widget is from screen center
		// or from cursor, if we ever allow player to use mouse cursor or eye tracking software to select the actions.
		float resX; float resY; 
		GetGame().GetWorkspace().GetScreenSize(resX, resY);
		
		UserActionContext currentContext = m_pInteractionHandlerComponent.GetCurrentContext();
		foreach (UserActionContext ctx : contexts)
		{
			// Do not draw currently select one
			if (currentContext == ctx)
				continue;
			
			// We already have too much
			if (actualCount >= m_iPrecachedWidgetCount)
				break;
			
			vector position = ctx.GetOrigin();
			float distanceSq = vector.DistanceSq(position, referencePos);
			
			// Just ignore actions out of reach, we will fade them out anyway
			if (distanceSq < distanceLimitSq && IsInLineOfSight(position, mat, threshold))
			{
				Widget widget =  m_aWidgets[actualCount];
				if (widget)
				{
					SetWidgetWorldPosition(world, cameraIndex, widget, position);
					
					// TODO@AS:
					// First child is an image.
					// We need more robust solution if not.
					Widget child = FindIconWidget(widget);
					if (child)
					{
						// distance^2 from context to camera origin
						float distance = Math.Sqrt(distanceSq);
						bool visible = ctx.IsInVisibilityAngle(referencePos);
						SetWidgetScale(child, distance, zoom);
						SetWidgetAlpha(child, distance, distanceLimit, resX, resY);
						SetWidgetColor(child, distance, visible);
					}
				}
				actualCount++;
			}
		}
		
		// Enable required amount of widgets
		SetVisibleWidgets(actualCount);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Iterates through available precached widgets and leaves only provided count enabled.
	//! \param count The amount of widgets to leave active.
	protected void SetVisibleWidgets(int count)
	{
		if (m_aWidgets.IsEmpty() || !m_wRoot)
			return;
		
		// Enable additional widgets
		if (count > m_iVisibleWidgets)
		{
			for (int i = m_iVisibleWidgets; i < count; i++)
				m_aWidgets[i].SetVisible(true);

			m_iVisibleWidgets = count;
		}
		// Disable exceeding widgets
		else if (count < m_iVisibleWidgets)
		{
			for (int i = count; i < m_iVisibleWidgets; i++)
				m_aWidgets[i].SetVisible(false);
			
			m_iVisibleWidgets = count;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetScale(Widget widget, float distance, float zoom)
	{
		// Scale is reversely proportionalto distance, but we don't want to make icons more than 2x bigger - this may depend on art resolution
		// Scale also depends on current camera FOV, the smaller FOV the bigger the icon
		float scale = zoom / Math.Max(distance, m_fWidgetMinimumRange);
		
		FrameSlot.SetSize(widget, scale * m_fOriginalSizeX, scale * m_fOriginalSizeY);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetAlpha(Widget widget, float distance, float limit, float resX, float resY)
	{
		// First stage of alpha depends on distance to camera
		float alpha = 0;
		if (limit != 0)
			alpha = Math.Clamp(limit - distance, 0, limit) / limit;
		
		// Second stage of alpha depends on distance from screen center
		float x; float y; 
		widget.GetScreenPos(x, y);
		
		// Get distance from center as 0,1
		// where 0 = center, 1 = edge
		float xDistance = Math.AbsFloat(x - resX * 0.5) / resX;
		float yDistance = Math.AbsFloat(y - resY * 0.5) / resY;
		
		// Sample curves
		x = Math3D.Curve(ECurveType.CurveProperty2D, xDistance, m_pHorizontalOpacityCurve)[1];
		y = Math3D.Curve(ECurveType.CurveProperty2D, yDistance, m_pVerticalOpacityCurve)[1];
		
		// Take the smaller value as priority,
		// average doesn't really work well here.
		float min = Math.Min(x, y);
		
		// Apply opacity
		widget.SetOpacity(alpha * min);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetColor(Widget widget, float distance, bool isInVisibilityRange)
	{
		if (isInVisibilityRange)
			widget.SetColor(m_pColorVisible);
		else
			widget.SetColor(m_pColorNotVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Recalculates worldPosition to screen space and applies it to widget.
	//! \param world The world we work in
	//! \param cameraIndex Index of currently active camera
	//! \param widget Target widget to be transformed
	//! \param worldPosition Position in world space
	protected void SetWidgetWorldPosition(BaseWorld world, int cameraIndex, Widget widget, vector worldPosition)
	{
		vector screenPosition = GetGame().GetWorkspace().ProjWorldToScreen(worldPosition, world, cameraIndex);
		float x =  screenPosition[0];
		float y =  screenPosition[1];
		FrameSlot.SetPos(widget, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Performs a dot product check against threshold whether point is in line of sight of provided transformation.
	//! \param point Point to perform check for
	//! \param transform Reference matrix checked against
	//! \param threshold01 Dot product result is compared against this value, higher values result in more narrow arc.
	protected bool IsInLineOfSight(vector point, vector transform[4], float val)
	{
		vector direction = point - transform[3];
		direction.Normalize();
		
		const float threshold = 0.25;
		if (vector.Dot(direction, transform[2]) > val)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! TODO@AS: We need a robust solution if the icon is not first child in the frame.
	//! \param layout The layout to find icon widget in
	//! \returns Returns the icon image widget or null if none.
	protected Widget FindIconWidget(Widget layout)
	{
		return layout.GetChildren();
	}
	
};
