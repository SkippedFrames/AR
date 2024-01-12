void On2DOpticsADSChange(SCR_2DOpticsComponent comp, bool inADS);
typedef func On2DOpticsADSChange;

void On2DOpticsIlluminationChange(Color color, Color glowColor);
typedef func On2DOpticsIlluminationChange;

class SCR_2DOpticsDisplay : SCR_InfoDisplayExtended
{
	protected SCR_2DOpticsComponent m_Optic;

	protected ResourceName m_sReticleTexture;
	protected ResourceName m_sReticleGlowTexture;
	protected ResourceName m_sFilterTexture;

	protected float m_fReticleAngularSize;
	protected float m_fReticlePortion;
	protected float m_fReticleBaseZoom;
	protected float m_fVignetteSize;
	protected float m_fObjectiveSize;
	protected float m_fRollOffset;

	protected vector m_vMisalignmentOffset;
	protected vector m_vObjectiveOffset; // Objective and reticle offset
	protected vector m_vVignetteOffset; // Vignette SetReticleOffset
	protected vector m_vRotation;
	protected vector m_vRotationSpeed;
	protected vector m_vRotationOffset;
	protected vector m_vMovement;
	protected vector m_vMovementSpeed;
	protected vector m_vMovementOffset;

	protected bool m_bInitialBlurOver;

	// Default widget names
	const string WIDGET_LAYOUT_REAR = "HRearEyePiece";
	const string WIDGET_LAYOUT_FRONT = "HFrontObjective";
	const string WIDGET_IMAGE_COVER = "ImgCover";

	const string WIDGET_SIZE_REAR = "SizeRear";
	const string WIDGET_SIZE_OBJECTIVE = "SizeFrontObjective";

	const string WIDGET_IMAGE_PADDING_LEFT = "ImgPaddingLeft";
	const string WIDGET_IMAGE_PADDING_RIGHT = "ImgPaddingRight";
	const string WIDGET_IMAGE_PADDING_TOP = "ImgPaddingTop";
	const string WIDGET_IMAGE_PADDING_BOTTOM = "ImgPaddingBottom";

	const string WIDGET_OVERLAY_RETICLES = "OverlayReticles";
	const string WIDGET_IMAGE_RETICLE = "ImgReticle";
	const string WIDGET_IMAGE_RETICLE_GLOW = "ImgReticleGlow";
	const string WIDGET_IMAGE_VIGNETTE = "ImgVignette";
	const string WIDGET_IMAGE_SCRATCHES = "ImgScratches";
	const string WIDGET_BLUR = "Blur";
	const string WIDGET_IMAGE_FILTER = "ImgFilter";

	const string WIDGET_SIZE_LEFT = "SizePaddingLeft";
	const string WIDGET_SIZE_TOP = "SizePaddingTop";

	const float OPACITY_INITIAL = 0.75;

	protected Widget m_wLayoutRear;
	protected Widget m_wLayoutFront;
	protected ImageWidget m_wImgCover;
	protected ImageWidget m_wImgScratches;

	protected Widget m_wOverlayReticles;
	protected ImageWidget m_wImgReticle;
	protected ImageWidget m_wImgReticleGlow;
	protected BlurWidget m_wBlur;
	protected ImageWidget m_wImgFilter;

	protected SizeLayoutWidget m_wRearFillLeft;
	protected SizeLayoutWidget m_wRearFillTop;
	protected SizeLayoutWidget m_wFrontFillLeft;
	protected SizeLayoutWidget m_wFrontFillTop;

	protected SizeLayoutWidget m_wSizeLayoutRear;
	protected SizeLayoutWidget m_wSizeLayoutObjective;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_wRoot.SetVisible(false);

		FindWidgets();

		SCR_2DOpticsComponent.s_On2DOpticADSChanged.Insert(OnADSChange);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStopDraw(IEntity owner)
	{
		SCR_2DOpticsComponent.s_On2DOpticADSChanged.Remove(OnADSChange);

		if (!m_Optic)
			return;

		m_Optic.OnSetupOpticImage().Remove(SetupOpticImage);
		m_Optic.OnIlluminationChange().Remove(ChangeReticleTint);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_Optic)
			return;

		if (!m_bInADS && !SCR_BinocularsComponent.IsZoomedView() && !m_Optic.GetIsZoomed())
			return;

		// Widget check
		if (!m_wLayoutRear || !m_wLayoutFront)
			return;

		MoveScopeWidgets(timeSlice);
		LowerBlurIntensity(timeSlice, m_Optic.GetAnimationSpeedBlur());
		SetReticleOffset(m_Optic.GetReticleOffsetX(), m_Optic.GetCurrentReticleOffsetY());
	}

	//------------------------------------------------------------------------------------------------
	//! Find widget refences
	protected void FindWidgets()
	{
		// Parts
		m_wLayoutRear = m_wRoot.FindAnyWidget(WIDGET_LAYOUT_REAR);
		m_wLayoutFront = m_wRoot.FindAnyWidget(WIDGET_LAYOUT_FRONT);
		m_wImgCover = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE_COVER));
		m_wImgScratches = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE_SCRATCHES));

		if (!m_wLayoutRear || !m_wLayoutFront)
		{
			Print("Scope vignette movement is not possible due to missing widget references!", LogLevel.WARNING);
			return;
		}

		// Rear part - eye piece
		m_wSizeLayoutRear = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_REAR));
		m_wRearFillLeft = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_LEFT));
		m_wRearFillTop = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_TOP));

		// Front part - objective
		m_wSizeLayoutObjective = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_OBJECTIVE));
		m_wFrontFillLeft = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_LEFT));
		m_wFrontFillTop = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_TOP));

		m_wOverlayReticles = m_wLayoutFront.FindAnyWidget(WIDGET_OVERLAY_RETICLES);
		m_wImgReticle = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE));
		m_wImgReticleGlow = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE_GLOW));
		m_wBlur = BlurWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_BLUR));
		m_wImgFilter = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_FILTER));
	}

	//------------------------------------------------------------------------------------------------
	//! Setup images and right texture sizing
	void SetupOpticImage()
	{
		// Setup reticle
		if (!m_sReticleTexture.IsEmpty())
			m_wImgReticle.LoadImageTexture(0, m_sReticleTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleTexture.IsEmpty());

		if (!m_sReticleGlowTexture.IsEmpty())
			m_wImgReticleGlow.LoadImageTexture(0, m_sReticleGlowTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleGlowTexture.IsEmpty());

		// Setup lens filter
		if (!m_sFilterTexture.IsEmpty())
			m_wImgFilter.LoadImageTexture(0, m_sFilterTexture);

		m_wImgFilter.SetVisible(!m_sFilterTexture.IsEmpty());

		// Compute reticle base FOV once
		if (m_fReticleBaseZoom > 0)
			m_fReticleBaseZoom = -m_Optic.CalculateZoomFOV(m_fReticleBaseZoom);

		float fovReticle;
		if (m_fReticleBaseZoom == 0)
			fovReticle = m_Optic.GetFovZoomed();
		else if (m_fReticleBaseZoom < 0)
			fovReticle = -m_fReticleBaseZoom;

		// Account for part that represents the measurable reticle
		float reticleAngularSize;
		if (m_fReticlePortion > 0)
			reticleAngularSize = m_fReticleAngularSize / m_fReticlePortion;

		float reticleSize;
		if (fovReticle > 0)
			reticleSize = reticleAngularSize / fovReticle;
		else if (m_Optic.GetFovZoomed() > 0)
			reticleSize = reticleAngularSize / m_Optic.GetFovZoomed();

		// Save basic data
		m_Optic.SetDefaultSize(reticleSize);
		m_Optic.SetCurrentReticleSize(m_Optic.GetDefaultSize());

		UpdateScale(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnADSChange(notnull SCR_2DOpticsComponent comp, bool inADS)
	{
		m_Optic = comp;

		m_Optic.GetReticleTextures(m_sReticleTexture, m_sReticleGlowTexture, m_sFilterTexture);
		m_Optic.GetReticleData(m_fReticleAngularSize, m_fReticlePortion, m_fReticleBaseZoom);

		m_Optic.OnSetupOpticImage().Insert(SetupOpticImage);
		m_Optic.OnIlluminationChange().Insert(ChangeReticleTint);

		if (!inADS)
		{
			m_Optic.OnSetupOpticImage().Remove(SetupOpticImage);
			m_Optic.OnIlluminationChange().Remove(ChangeReticleTint);
			Deactivate();
			m_Optic = null;
			return;
		}

		m_wRoot.SetZOrder(-1);
		m_wRoot.SetVisible(false);

		m_bInitialBlurOver = false;

		m_vMisalignmentOffset = vector.Zero;

		m_Optic.GetRotation(m_vRotation, 1);
		m_vRotationSpeed = vector.Zero;
		m_vRotationOffset = vector.Zero;

		m_Optic.GetMovement(m_vMovement, 1);

		m_vMovementSpeed = vector.Zero;
		m_vMovementOffset = vector.Zero;

		SetupOpticImage();

		Activate();
	}

	//------------------------------------------------------------------------------------------------
	protected void Activate()
	{
		// Prevent calnceling animation
		AnimateWidget.StopAnimation(m_wRoot, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(DeactivateWidget);

		// Activate and setup hud
		m_wRoot.SetVisible(true);
		m_wImgCover.SetVisible(false);
		m_wRoot.SetOpacity(0);

		// Blur
		m_wBlur.SetIntensity(OPACITY_INITIAL);
		m_wBlur.SetOpacity(1);

		//Play fadeIn animation after given delay
		GetGame().GetCallqueue().CallLater(PlayEntryAnimation, m_Optic.GetAnimationActivationDelay(), false, m_Optic.GetAnimationEnterTime());
	}

	//------------------------------------------------------------------------------------------------
	//! Stop any running animation for entering Scope and disable the layout after some delay
	protected void Deactivate()
	{
		// Prevent entering animation
		AnimateWidget.StopAnimation(m_wRoot, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(PlayEntryAnimation);

		//Deactivate the HUD after set delay
		GetGame().GetCallqueue().CallLater(DeactivateWidget, m_Optic.GetAnimationDeactivationDelay(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! Start optics entring animation
	protected void PlayEntryAnimation(float animationSpeed)
	{
		if (animationSpeed > 0)
			AnimateWidget.Opacity(m_wRoot, 1, animationSpeed);
		else
			m_wRoot.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Open optics hud with animation delay
	protected void DeactivateWidget()
	{
		if (m_wImgCover)
			m_wImgCover.SetOpacity(0);

		if (m_wRoot)
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Update widget sizes according to the screen height and DPI scale
	void UpdateScale(float reticleScale, float vignetteScale, float objectiveScale)
	{
		float uiScale = 1;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
			uiScale = workspace.DPIUnscale(workspace.GetHeight());

		// Reticle size setup
		if (m_wOverlayReticles)
		{
			float reticleSize = m_Optic.GetCurrentReticleSize() * uiScale * reticleScale;
			FrameSlot.SetSize(m_wOverlayReticles, reticleSize, reticleSize);
		}

		// Vignette size setup
		if (m_wSizeLayoutRear)
		{
			m_fVignetteSize = m_Optic.GetObjectiveFov() * m_Optic.GetVignetteScale() * uiScale * vignetteScale / m_Optic.GetFovZoomed();

			m_wSizeLayoutRear.SetWidthOverride(m_fVignetteSize);
			m_wSizeLayoutRear.SetHeightOverride(m_fVignetteSize);
		}

		// Ocular size setup
		if (m_wSizeLayoutObjective)
		{
			m_fObjectiveSize = m_Optic.GetObjectiveFov() * m_Optic.GetObjectiveScale() * uiScale * objectiveScale / m_Optic.GetFovZoomed();

			m_wSizeLayoutObjective.SetWidthOverride(m_fObjectiveSize);
			m_wSizeLayoutObjective.SetHeightOverride(m_fObjectiveSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Change reticle texture position with angular offset
	void SetReticleOffset(float x, float y)
	{
		if (!m_wOverlayReticles)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float fov = m_Optic.GetFovZoomed();
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
				fov = camera.GetVerticalFOV();
		}

		float uiHeight = workspace.DPIUnscale(workspace.GetHeight());
		float offsetX = x * uiHeight / fov;
		float offsetY = y * uiHeight / fov;

		FrameSlot.SetPos(m_wOverlayReticles, offsetX, offsetY);

		vector size = FrameSlot.GetSize(m_wOverlayReticles);
		if (m_wImgReticle)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticle.SetPivot(pivotX, pivotY);
		}

		if (m_wImgReticleGlow)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticleGlow.SetPivot(pivotX, pivotY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bring down lens blur intensity in steps
	protected void LowerBlurIntensity(float timeSlice, float speed)
	{
		if (!m_wBlur)
			return;

		float intensity = m_wBlur.GetIntensity();
		intensity = Math.Lerp(intensity, 0, timeSlice * speed);

		// Clear initial bluer
		if (intensity < 0.1 && !m_bInitialBlurOver)
		{
			m_bInitialBlurOver = true;
			intensity = 0;
		}

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	//! Change color of Reticle Image
	protected void ChangeReticleTint(Color color, Color colorGlow)
	{
		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetColor(colorGlow);

		if (m_wImgReticle)
			m_wImgReticle.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Apply result movement to scope widgets
	protected void MoveScopeWidgets(float timeSlice)
	{
		// Update vignette and reticle direction and roll
		vector sightMat[4];
		float fov;
		vector misalignment = m_Optic.GetMisalignment(sightMat, fov);

		// Stabilize roll
		float roll = misalignment[2] * m_Optic.GetRollScale();
		if (m_Optic.GetRollDampingSpeed() > 0)
		{
			float rollSmooth = Math.Min(1, timeSlice * m_Optic.GetRollDampingSpeed());
			m_fRollOffset = Math.Lerp(m_fRollOffset, roll, rollSmooth);
			roll -= m_fRollOffset;
		}

		misalignment[0] = fixAngle_180_180(misalignment[0]);
		misalignment[1] = fixAngle_180_180(misalignment[1]);

		// Stabilize binocular reticle over time
		if (m_Optic.GetMisalignmentDampingSpeed() > 0)
		{
			float offsetDamping = Math.Min(timeSlice * m_Optic.GetMisalignmentDampingSpeed(), 1);
			m_vMisalignmentOffset = vector.Lerp(m_vMisalignmentOffset, misalignment, offsetDamping);
			misalignment = misalignment - m_vMisalignmentOffset;
		}

		// Stabilize rotation speed over time
		vector rotation = m_Optic.GetRotation(m_vRotation, timeSlice);
		if (m_Optic.GetRotationDampingSpeed() > 0)
		{
			float rotationDamping = Math.Min(timeSlice * m_Optic.GetRotationDampingSpeed(), 1);
			m_vRotationOffset = vector.Lerp(m_vRotationOffset, rotation, rotationDamping);
			rotation = rotation - m_vRotationOffset;
		}

		// Movement is also stabilizable
		vector movement = m_Optic.GetMovement(m_vMovement, timeSlice);
		if (m_Optic.GetMovementDampingSpeed() > 0)
		{
			float movementDamping = Math.Min(timeSlice * m_Optic.GetMovementDampingSpeed(), 1);
			m_vMovementOffset = vector.Lerp(m_vMovementOffset, movement, movementDamping);
			movement = movement - m_vMovementOffset;
		}

		// Make sure lerp doesn't go out of bounds
		float vignetteMove = Math.Min(timeSlice * m_Optic.GetVignetteMoveSpeed(), 1);

		m_vVignetteOffset = vector.Lerp(m_vVignetteOffset, m_Optic.GetMisalignmentScale() * misalignment, vignetteMove);

		// Objective must be set instantly
		m_vObjectiveOffset = m_Optic.GetMisalignmentScale() * misalignment;

		if (m_Optic.GetRotationScale() > 0)
			m_vVignetteOffset = m_vVignetteOffset - rotation * m_Optic.GetRotationScale();

		if (m_Optic.GetMovementScale() > 0)
			m_vVignetteOffset = m_vVignetteOffset + movement * m_Optic.GetMovementScale();

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		float screenW, screenH;
		workspace.GetScreenSize(screenW, screenH);
		screenW = workspace.DPIUnscale(screenW);
		screenH = workspace.DPIUnscale(screenH);

		float pixelsPerDegree = screenH / fov;

		// Vignette movement
		float defaultRearLeft = VignetteDefaultPosition(screenW, m_fVignetteSize);
		float defaultRearTop = VignetteDefaultPosition(screenH, m_fVignetteSize);

		float rearPaddingLeft = defaultRearLeft + pixelsPerDegree * m_vVignetteOffset[0];
		float rearPaddingTop = defaultRearTop - pixelsPerDegree * m_vVignetteOffset[1];

		m_wRearFillLeft.SetWidthOverride(rearPaddingLeft);
		m_wRearFillTop.SetHeightOverride(rearPaddingTop);

		// Objective and reticle movement
		float defaultFrontLeft = VignetteDefaultPosition(screenW, m_fObjectiveSize);
		float defaultFrontTop = VignetteDefaultPosition(screenH, m_fObjectiveSize);

		float frontPaddingLeft = defaultFrontLeft + pixelsPerDegree * m_vObjectiveOffset[0];
		float frontPaddingTop = defaultFrontTop - pixelsPerDegree * m_vObjectiveOffset[1];

		m_wFrontFillLeft.SetWidthOverride(frontPaddingLeft);
		m_wFrontFillTop.SetHeightOverride(frontPaddingTop);

		// Reticle rotation
		if (m_wImgReticle)
			m_wImgReticle.SetRotation(roll);

		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetRotation(roll);

		if (m_wImgFilter)
			m_wImgFilter.SetRotation(roll);

		if (m_wImgCover)
			m_wImgCover.SetRotation(roll);

		if (m_wImgScratches)
			m_wImgScratches.SetRotation(roll + m_Optic.GetScratchesRoll());

		// Motion offset blur
		if (m_bInitialBlurOver)
			MotionBlur(m_vVignetteOffset[0], m_vVignetteOffset[1]);
	}

	//------------------------------------------------------------------------------------------------
	protected void MotionBlur(float posX, float posY)
	{
		if (!m_wBlur)
			return;

		// Check movement
		if (!m_Optic.GetIsMoving() || !m_Optic.GetIsRotating())
			return;

		// Pick axis
		float intensity = Math.AbsFloat(posX);
		if (intensity > Math.AbsFloat(posY))
			intensity = Math.AbsFloat(posY);

		// Scale and limit
		intensity = intensity * m_Optic.GetMotionBlurScale();
		if (intensity > m_Optic.GetMotionBlurMax())
			intensity = m_Optic.GetMotionBlurMax();

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	protected float VignetteDefaultPosition(float screenSize, float vignetteSize)
	{
		float defaultPos = (screenSize - vignetteSize) * 0.5;
		if (defaultPos < 0)
			defaultPos = 0;
		return defaultPos;
	}

}
