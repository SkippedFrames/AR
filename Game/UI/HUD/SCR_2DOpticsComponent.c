[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DOpticsComponentClass : ScriptedSightsComponentClass
{
}

void OnSightsADSChanged(bool inADS, float fov);
typedef func OnSightsADSChanged;

//------------------------------------------------------------------------------------------------
//! Base class for 2D optics
//! Unifiying binoculars and optic sight
class SCR_2DOpticsComponent : ScriptedSightsComponent
{
	// Action names
	const string ACTION_WHEEL = "WeaponChangeMagnification";
	const string ACTION_ILLUMINATION = "WeaponToggleSightsIllumination";

	const float REFERENCE_FOV = 38;
	const float OPACITY_INITIAL = 0.75;

	const float NEAR_PLANE_DEFAULT = 0.05;
	const float NEAR_PLANE_ZOOMED = 0.05;

	// Optics setup
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Layout used for 2D sights HUD\n", params: "layout", category: "Resources")]
	protected ResourceName m_sLayoutResource;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleTexture;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle under glow\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleGlowTexture;

	[Attribute("0", category: "Resources")]
	protected bool m_bHasIllumination;

	[Attribute("0 0 0", UIWidgets.ColorPicker, category: "Resources")]
	protected ref Color m_cReticleTextureIllumination;

	[Attribute("0.05", UIWidgets.Slider, desc: "Reticle glow texture alpha\n[ % ]", params: "0 1 0.001", category: "Resources", precision: 3)]
	protected float m_fReticleTextureGlowAlpha;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of lens filter\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sFilterTexture;

	[Attribute("10", UIWidgets.EditBox, desc: "Optic magnification\n[ x ]", params: "0.1 200 0.001", category: "Sights", precision: 3)]
	protected float m_fMagnification;

	[Attribute("0", UIWidgets.Slider, "Angular size of reticle\n[ ° ]", params: "0 60 0.001", category: "Sights", precision: 3)]
	protected float m_fReticleAngularSize;

	[Attribute("1", UIWidgets.Slider, "Portion of reticle with specified angular size\n[ % ]", params: "0 10 0.00001", category: "Sights", precision: 5)]
	protected float m_fReticlePortion;

	[Attribute("0", UIWidgets.Slider, "POSITIVE:\n Rear focal plane - magnification at which reticle markings are valid\n\nZERO:\n Front focal plane reticle scaling with magnification.\n\nNEGATIVE:\n Enforced base FOV in degrees\n", params: "-60 200 0.001", category: "Sights", precision: 5)]
	protected float m_fReticleBaseZoom;

	[Attribute("0", UIWidgets.CheckBox, "Check if the optic is attached to a turret\n", category: "Sights")]
	protected bool m_bIsTurretOptic;

	[Attribute("10", UIWidgets.EditBox, desc: "Field of view of the objective\n[ ° ]", params: "0.001 60 0.001", category: "2DSights", precision: 3)]
	protected float m_fObjectiveFov;

	[Attribute("1", UIWidgets.EditBox, desc: "Ocular texture scale\n", params: "0.001 10 0.001", category: "2DSights", precision: 3)]
	protected float m_fObjectiveScale;

	[Attribute("1.1", UIWidgets.EditBox, desc: "Vignette texture scale\n", params: "0.001 10 0.001", category: "2DSights", precision: 3)]
	protected float m_fVignetteScale;

	[Attribute("1", UIWidgets.CheckBox, "Should hide parent object when using 2D sights\n", category: "2DSights")]
	protected bool m_bShouldHideParentObject;

	[Attribute("0", UIWidgets.CheckBox, "Should hide parent character when using 2D sights\n", category: "2DSights")]
	protected bool m_bShouldHideParentCharacter;

	// Animation attributes
	[Attribute("0", UIWidgets.Slider, "Time before entering animation starts", params: "0 2000 1", category: "Animations")]
	protected int m_iAnimationActivationDelay;

	[Attribute("0", UIWidgets.Slider, "Animation time for canceling optic.", params: "0 2000 1", category: "Animations")]
	protected int m_iAnimationDeactivationDelay;

	[Attribute("0", UIWidgets.Slider, "Animation time for optic animation", params: "0 5 0.01", category: "Animations")]
	protected float m_fAnimationEnterTime;

	[Attribute("2", UIWidgets.Slider, "Speed of blur fade out on optic entering", params: "0.01 5 0.01", category: "Animations")]
	protected float m_fAnimationSpeedBlur;

	// Effect attributes
	[Attribute("0", UIWidgets.EditBox, desc: "Optic misalignment damping speed - intended for binoculars, the higher, the faster the misalignment gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fMisalignmentDampingSpeed;

	[Attribute("0.1", UIWidgets.EditBox, desc: "Rotation effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fRotationScale;

	[Attribute("4", UIWidgets.EditBox, desc: "Rotation damping speed - the higher, the faster the rotation speed gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fRotationDampingSpeed;

	[Attribute("0.01", UIWidgets.EditBox, desc: "Movement effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fMovementScale;

	[Attribute("2", UIWidgets.EditBox, desc: "Movement damping speed - the higher, the faster the movement speed gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fMovementDampingSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Roll effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fRollScale;

	[Attribute("0", UIWidgets.EditBox, desc: "Roll damping speed - the higher, the faster the optic gets uprighted", params: "0 60 0.1", category: "Effects")]
	protected float m_fRollDampingSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Optic misalignment scale - for turrets and binoculars", params: "-60 60 0.1", category: "Effects")]
	protected float m_fMisalignmentScale;

	[Attribute("1", UIWidgets.EditBox, desc: "Vignette adjustment speed - the faster, the less inertia there is", params: "0 60 0.1", category: "Effects")]
	protected float m_fVignetteMoveSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Motion blur effect scaling", params: "0 100", category: "Effects")]
	protected float m_fMotionBlurScale;

	[Attribute("0.1", UIWidgets.EditBox, desc: "Max level of motion blur intensity", params: "0 1 0.01", category: "Effects")]
	protected float m_fMotionBlurMax;

	// 0 = immediately when entering ADS camera, 1.0 = only after full blend
	[Attribute("1", UIWidgets.Slider, "Percentage of camera transition at which sights activate.", params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSActivationPercentage;

	// 0 = immediately when leaving ADS camera, 1.0 = only after full blend
	[Attribute("0", UIWidgets.Slider, "Percentage of camera transition at which sights deactivate.", params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSDeactivationPercentage;

	[Attribute("0", UIWidgets.ComboBox, "Type of zeroing for this sights.", "", ParamEnumArray.FromEnum(SCR_EPIPZeroingType), category: "Sights")]
	protected SCR_EPIPZeroingType m_eZeroingType;

	[Attribute("0", UIWidgets.Slider, "Reticle Offset of scope center in X", params: "-1 1 0.0001", precision: 5, category: "Sights")]
	protected float m_fReticleOffsetX;

	[Attribute("0", UIWidgets.Slider, "Reticle Offset of scope center in Y", params: "-1 1 0.0001", precision: 5, category: "Sights")]
	protected float m_fReticleOffsetY;

	[Attribute("25", UIWidgets.Slider, "Interpolation speed for zeroing interpolation", params: "1 100.0 0.1", category: "Sights")]
	protected float m_fReticleOffsetInterpSpeed;

	[Attribute("0 0 0", UIWidgets.EditBox, "Camera offset when looking through scope", category: "Sights", params: "inf inf 0 purpose=coords space=entity coordsVar=m_vCameraPoint")]
	protected vector m_vCameraOffset;

	[Attribute("0 0 0", UIWidgets.EditBox, "Camera angles when looking through scope", category: "Sights", params: "inf inf 0 purpose=angles space=entity anglesVar=m_vCameraAngles")]
	protected vector m_vCameraAngles;

	protected float m_fCurrentReticleOffsetY;
	protected float m_fCurrentCameraPitch;

	//------------------------------------------------------------------------------------------------
	override float GetADSActivationPercentageScript()
	{
		return m_fADSActivationPercentage;
	}

	//------------------------------------------------------------------------------------------------
	override float GetADSDeactivationPercentageScript()
	{
		return m_fADSDeactivationPercentage;
	}

	//------------------------------------------------------------------------------------------------
	// Getters for attributes
	float GetReticleOffsetX()
	{
		return m_fReticleOffsetX;
	}

	float GetCurrentReticleOffsetY()
	{
		return m_fCurrentReticleOffsetY;
	}

	void GetReticleTextures(out ResourceName reticleTexture, out ResourceName reticleTextureGlow, out ResourceName filterTexture)
	{
		reticleTexture = m_sReticleTexture;
		reticleTextureGlow = m_sReticleGlowTexture;
		filterTexture = m_sFilterTexture;
	}

	//------------------------------------------------------------------------------------------------
	void GetReticleData(out float reticleAngularSize, out float reticlePortion, out float reticleBaseZoom)
	{
		reticleAngularSize = m_fReticleAngularSize;
		reticlePortion = m_fReticlePortion;
		reticleBaseZoom = m_fReticleBaseZoom;
	}

	//------------------------------------------------------------------------------------------------
	float GetObjectiveFov()
	{
		return m_fObjectiveFov;
	}

	//------------------------------------------------------------------------------------------------
	float GetObjectiveScale()
	{
		return m_fObjectiveScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetVignetteScale()
	{
		return m_fVignetteScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetMisalignmentDampingSpeed()
	{
		return m_fMisalignmentDampingSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetRotationScale()
	{
		return m_fRotationScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetRotationDampingSpeed()
	{
		return m_fRotationDampingSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetMovementScale()
	{
		return m_fMovementScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetMovementDampingSpeed()
	{
		return m_fMovementDampingSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetRollScale()
	{
		return m_fRollScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetRollDampingSpeed()
	{
		return m_fRollDampingSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetMisalignmentScale()
	{
		return m_fMisalignmentScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetVignetteMoveSpeed()
	{
		return m_fVignetteMoveSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetMotionBlurScale()
	{
		return m_fMotionBlurScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetMotionBlurMax()
	{
		return m_fMotionBlurMax;
	}

	//------------------------------------------------------------------------------------------------
	// Needed for zeroing
	SCR_EPIPZeroingType GetZeroType()
	{
		return m_eZeroingType;
	}

	//------------------------------------------------------------------------------------------------
	float GetMagnification()
	{
		return m_fMagnification;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsOpticsHidden()
	{
		return m_bIsOpticsHidden;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsZoomed()
	{
		return m_bZoomed;
	}

	protected ref array<float> m_aReticleSizes = {};
	protected int m_iSelectedZoomLevel = 0;

	// Overall movement
	protected float m_fScratchesRoll;

	// FOVs
	protected float m_fFovZoomed;
	protected float m_fDefaultSize;
	protected float m_fCurrentReticleSize;
	protected float m_fNearPlaneCurrent;

	// Movement check
	protected bool m_bIsMoving;
	protected bool m_bIsRotating;
	protected bool m_bZoomed = false;
	protected bool m_bWasEntityHidden = false;
	protected int m_iHeadBoneId = -1;

	protected bool m_bIsOpticsHidden;
	protected bool m_bIsIlluminationOn;

	// Owner and character references
	protected ChimeraCharacter m_ParentCharacter = null;

	static ref ScriptInvokerBase<OnSightsADSChanged> s_OnSightsADSChanged = new ScriptInvokerBase<OnSightsADSChanged>();
	static ref ScriptInvokerBase<On2DOpticsADSChange> s_On2DOpticADSChanged = new ScriptInvokerBase<On2DOpticsADSChange>();
	protected ref ScriptInvokerVoid s_OnSetupOpticImage = new ScriptInvokerVoid();
	protected ref ScriptInvokerBase<On2DOpticsIlluminationChange> s_OnIlluminationChange = new ScriptInvokerBase<On2DOpticsIlluminationChange>();

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid OnSetupOpticImage()
	{
		return s_OnSetupOpticImage;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<On2DOpticsIlluminationChange> OnIlluminationChange()
	{
		return s_OnIlluminationChange;
	}

	//------------------------------------------------------------------------------------------------
	//! Get base FOV of the optic, used to determine maximum FOV
	float GetFovZoomed()
	{
		// m_fMagnification value may change during gameplay
		// But player camera may not exist at the moment, so at first we store magnification to compute it from and then compute it on first use
		if (m_fFovZoomed < 0)
		 	m_fFovZoomed = CalculateZoomFOV(-m_fFovZoomed);

		return m_fFovZoomed;
	}

	//------------------------------------------------------------------------------------------------
	//! Set base FOV of the optic. Negative value means magnification to compute new m_fFovZoomed with
	void SetFovZoomed(float value)
	{
		m_fFovZoomed = value;
	}

	//------------------------------------------------------------------------------------------------
	float GetNearPlane()
	{
		return m_fNearPlaneCurrent;
	}

	//------------------------------------------------------------------------------------------------
	int GetAnimationActivationDelay()
	{
		return m_iAnimationActivationDelay;
	}

	//------------------------------------------------------------------------------------------------
	int GetAnimationDeactivationDelay()
	{
		return m_iAnimationDeactivationDelay;
	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationEnterTime()
	{
		return m_fAnimationEnterTime;
	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationSpeedBlur()
	{
		return m_fAnimationSpeedBlur;
	}

	//------------------------------------------------------------------------------------------------
	float GetDefaultSize()
	{
		return m_fDefaultSize;
	}

	//------------------------------------------------------------------------------------------------
	void SetDefaultSize(float value)
	{
		m_fDefaultSize = value;
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentReticleSize()
	{
		return m_fCurrentReticleSize;
	}

	//------------------------------------------------------------------------------------------------
	void SetCurrentReticleSize(float value)
	{
		m_fCurrentReticleSize = value;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsMoving()
	{
		return m_bIsMoving;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsRotating()
	{
		return m_bIsRotating;
	}

	//------------------------------------------------------------------------------------------------
	float GetScratchesRoll()
	{
		return m_fScratchesRoll;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	override void OnInit(IEntity owner)
	{
		m_fScratchesRoll = Math.RandomFloat(0, 360);

		// m_fMagnification value may change during gameplay
		// But player camera may not exist at the moment, so at first we store magnification to compute it from and then compute it on first use
		m_fFovZoomed = -m_fMagnification;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSActivated()
	{
		m_ParentCharacter = ChimeraCharacter.Cast(GetOwner().GetParent());

		super.OnSightADSActivated();

		s_On2DOpticADSChanged.Invoke(this, true);
		s_OnSightsADSChanged.Invoke(true, GetFovZoomed());

		// Play cover fade in animaiton
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;
		GetGame().GetCallqueue().CallLater(HideObjects, m_iAnimationActivationDelay, false);

		m_bZoomed = true;

		m_bIsOpticsHidden = false;

		// Setup illumination
		EnableReticleIllumination(m_bIsIlluminationOn);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSDeactivated()
	{
		super.OnSightADSDeactivated();
		s_On2DOpticADSChanged.Invoke(this, false);
		s_OnSightsADSChanged.Invoke(false, 0);

		// Initialize to current zero value
		m_fCurrentReticleOffsetY = GetReticleYOffsetTarget();

		if (m_bWasEntityHidden)
		{
			IEntity owner = GetOwner();
			if (owner)
				owner.SetFlags(EntityFlags.VISIBLE, false);

			if (m_ParentCharacter)
				m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);

			m_bWasEntityHidden = false;
		}

		GetGame().GetCallqueue().Remove(HideObjects); // in case this is called quickly after OnSightADSActivated, HideObjects needs to be cleared to prevent conflict

		// Leaving animation
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;

		m_bZoomed = false;
		m_bIsOpticsHidden = true;

		// Clean up reticle illumination
		EnableReticleIllumination(false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{
		float interp = Math.Min(1, timeSlice * m_fReticleOffsetInterpSpeed);
		// Interpolate zeroing values
		float reticleTarget = GetReticleYOffsetTarget();
		m_fCurrentReticleOffsetY = Math.Lerp(m_fCurrentReticleOffsetY, reticleTarget, interp);
		float pitchTarget = GetCameraPitchTarget();
		m_fCurrentCameraPitch = Math.Lerp(m_fCurrentCameraPitch, pitchTarget, interp);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS))
			DebugOptics();
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Hides objects to prevent clipping when entering ADS
	protected void HideObjects()
	{
		// Near plane
		m_fNearPlaneCurrent = NEAR_PLANE_ZOOMED;

		// Hide rendering
		if (m_bShouldHideParentObject)
		{
			IEntity owner = GetOwner();
			if (owner)
				owner.ClearFlags(EntityFlags.VISIBLE, false);

			// Set flag as otherwise we can make visible something that we didn't make invisible
			m_bWasEntityHidden = true;
		}

		// Hide parent character
		if (m_bShouldHideParentCharacter)
		{
			if (m_ParentCharacter)
				m_ParentCharacter.ClearFlags(EntityFlags.VISIBLE, false);

			// Set flag as otherwise we can make visible something that we didn't make invisible
			m_bWasEntityHidden = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return camera FOV for given magnification. Focus FOV is used as reference 1x magnification
	static float CalculateZoomFOV(float magnification)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 74;

		float referenceFOV;
		PlayerCamera camera = PlayerCamera.Cast(cameraManager.CurrentCamera());
		if (camera)
			referenceFOV = camera.GetFocusFOV();
		else
			referenceFOV = cameraManager.GetFirstPersonFOV();

		float referenceTan = Math.Tan(Math.DEG2RAD * (referenceFOV * 0.5));
		return Math.RAD2DEG * 2 * Math.Atan2(referenceTan, magnification);
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if move X a Y is below limit
	bool IsMoveCloseToLimit(vector move, float limitMin)
	{
		float x = move[0];
		float y = move[1];

		return (float.AlmostEqual(x, 0, limitMin) && float.AlmostEqual(y, 0, limitMin));
	}

	//------------------------------------------------------------------------------------------------
	/*! Get sight angles in camera space and vertical FOV of the camera for further use
	\param fov field of view of main camera
	\param sightM sight orientation in camera space
	\return Math3D.MatrixToAngles(cameraMat)
	*/
	vector GetMisalignment(out vector sightMat[4], out float fov)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		// Get FOV since we are already having camera
		fov = camera.GetVerticalFOV();

		// Get sight orientation
		// Turrets require local sight transform for reliability
		GetSightsTransform(sightMat, true);

		// Add optic orientation
		vector opticMat[4];
		GetCameraLocalTransform(opticMat);
		Math3D.MatrixMultiply3(opticMat, sightMat, sightMat);

		// Convert to world orientation
		vector ownerMat[3];
		GetOwner().GetWorldTransform(ownerMat);
		Math3D.MatrixMultiply3(ownerMat, sightMat, sightMat);

		vector cameraMat[3];
		camera.GetWorldTransform(cameraMat);
		Math3D.MatrixInvMultiply3(cameraMat, sightMat, cameraMat);

		//! Fix for PGO-7, however it breaks other types of optics.
		//! Different solution is needed, because this does not handle leaning gracefully.
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_NONE)
			Math3D.MatrixInvMultiply3(opticMat, cameraMat, cameraMat);

		vector angles = Math3D.MatrixToAngles(cameraMat);

		return angles;
	}

	//------------------------------------------------------------------------------------------------
	vector GetRotation(out vector previousDir, float timeSlice)
	{
		if (timeSlice <= 0)
			return vector.Zero;

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		vector currentDir = camera.GetWorldTransformAxis(2);

		bool rotated = !float.AlmostEqual(previousDir[0], currentDir[0]);
		rotated |= !float.AlmostEqual(previousDir[1], currentDir[1]);

		if (rotated)
		{
			m_bIsRotating = true;
			vector rotation = (currentDir - previousDir) / timeSlice;
			previousDir = currentDir;
			return camera.VectorToLocal(rotation);
		}

		m_bIsRotating = false;

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	vector GetMovement(out vector previousPos, float timeSlice)
	{
		if (timeSlice <= 0)
			return vector.Zero;

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		vector currentPos = camera.GetWorldTransformAxis(3);

		bool moved = !float.AlmostEqual(previousPos[0], currentPos[0]);
		moved &= !float.AlmostEqual(previousPos[1], currentPos[1]);
		moved &= !float.AlmostEqual(previousPos[2], currentPos[2]);

		if (moved)
		{
			m_bIsMoving = true;
			vector movement = (currentPos - previousPos) / timeSlice;
			previousPos = currentPos;
			return camera.VectorToLocal(movement);
		}

		m_bIsMoving = false;
		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetSightsRelPosition()
	{
		IEntity owner = GetOwner();
		if (m_ParentCharacter && owner)
			return m_ParentCharacter.CoordToLocal(owner.GetOrigin());

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	bool HasIllumination()
	{
		return m_bHasIllumination;
	}

	//------------------------------------------------------------------------------------------------
	Color GetReticleTextureIllumination()
	{
		return Color.FromInt(m_cReticleTextureIllumination.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle between illumination modes
	protected void EnableReticleIllumination(bool enable)
	{
		Color reticleTint;
		Color glowTint;

		if (m_bHasIllumination && enable)
		{
			reticleTint = GetReticleTextureIllumination();
			glowTint = GetReticleTextureIllumination();
		}
		else
		{
			reticleTint = Color.FromInt(Color.BLACK);
			glowTint = Color.FromInt(Color.WHITE);
		}

		glowTint.SetA(m_fReticleTextureGlowAlpha);
		s_OnIlluminationChange.Invoke(reticleTint, glowTint);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetReticleYOffsetTarget()
	{
		return 0.0;
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentReticleYOffset()
	{
		return m_fReticleOffsetY;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCameraPitchTarget()
	{
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_CAMERA_TURN)
			return -GetCurrentSightsRange()[0];

		return 0.0;
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentCameraPitchOffset()
	{
		return m_fCurrentCameraPitch;
	}

	//------------------------------------------------------------------------------------------------
	vector GetCameraOffset()
	{
		return m_vCameraOffset;
	}

	//------------------------------------------------------------------------------------------------
	vector GetCameraAngles()
	{
		return m_vCameraAngles;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		\param matrix Out transformation from character's model space to our local space.
	*/
	void GetCharacterToLocalTransform(out vector result[4])
	{
		vector temp[4];
		Math3D.MatrixIdentity4(temp);
		
		IEntity parent = GetOwner();
		IEntity lastNode = parent;
		
		while (lastNode)
		{
			// Stop on character, that is final node
			ChimeraCharacter chara = ChimeraCharacter.Cast(lastNode);
			if (chara)
				break;
			
			vector localTransform[4];
			lastNode.GetLocalTransform(localTransform);
			
			// If using a pivot, we need to apply the pivot transformation
			TNodeId pivotIndex = lastNode.GetPivot();
			if (pivotIndex != -1)
			{
				// Multiply pivot * local TM = model TM
				IEntity parentNode = lastNode.GetParent();
				if (parentNode)
				{
					vector pivotTM[4];
					parentNode.GetAnimation().GetBoneMatrix(pivotIndex, pivotTM);
					
					// This should not be happening - there should
					// rather be no pivot, yet it triggers at times.
					// ??? TODO@AS: See if we can have better API for pivoting like this
					if (!SCR_Math3D.IsMatrixEmpty(pivotTM))
						Math3D.MatrixMultiply4(pivotTM, localTransform, localTransform);
				}
			}
			
			Math3D.MatrixMultiply4(localTransform, temp, result);
			Math3D.MatrixCopy(result, temp);
			
			lastNode = lastNode.GetParent();
		}
		
		Math3D.MatrixGetInverse4(temp, result);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Return actual parent (weapon) with calculated transformation in relation to it.
		\param matrix Out rotation in relation to new parent
		\return Returns new parent to use, usually the weapon
	*/
	IEntity GetCameraLocalTransform(out vector matrix[4])
	{
		// Apply offset and angles as configured against front sight position
		vector position = m_vCameraOffset + GetSightsFrontPosition(true);
		vector yawPitchRoll = Vector(m_vCameraAngles[1], m_vCameraAngles[0] + m_fCurrentCameraPitch, m_vCameraAngles[2]);

		// Construct the local matrix in relation to original parent
		Math3D.AnglesToMatrix(yawPitchRoll, matrix);
		matrix[3] = position;

		// Now additionally we will browse the hierarchy until we hit our weapon
		vector temp[4];
		Math3D.MatrixCopy(matrix, temp);

		IEntity parent = GetOwner();
		IEntity lastNode = parent;

		BaseWeaponComponent weaponRoot;
		while (lastNode)
		{
			// Stop on weapon, that is final node
			weaponRoot = BaseWeaponComponent.Cast(lastNode.FindComponent(BaseWeaponComponent));
			if (weaponRoot)
				break;

			/// These multiplications will run even if we have no weapon,
			/// but it shouldn't be that big of a deal for now, weapon should exist regardless
			vector localTransform[4];
			lastNode.GetLocalTransform(localTransform);

			// If using a pivot, we need to apply the pivot transformation
			TNodeId pivotIndex = lastNode.GetPivot();
			if (pivotIndex != -1)
			{
				// Multiply pivot * local TM = model TM
				IEntity parentNode = lastNode.GetParent();
				if (parentNode)
				{
					vector pivotTM[4];
					parentNode.GetAnimation().GetBoneMatrix(pivotIndex, pivotTM);

					// This should not be happening - there should
					// rather be no pivot, yet it triggers at times.
					// ??? TODO@AS: See if we can have better API for pivoting like this
					if (!SCR_Math3D.IsMatrixEmpty(pivotTM))
						Math3D.MatrixMultiply4(pivotTM, localTransform, localTransform);
				}
			}
			Math3D.MatrixMultiply4(temp, localTransform, temp);
			lastNode = lastNode.GetParent();
		}

		// Only use calculated matrix+parent if its valid
		if (weaponRoot)
		{
			Math3D.MatrixCopy(temp, matrix);
			parent = lastNode;
		}

		return parent;
	}

#ifdef ENABLE_DIAG
	protected static bool s_bOpticDiagRegistered;
	protected float m_fDebugAngle = 6;
	protected float m_fDebugUnit = 1;

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_2DOpticsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_bOpticDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS, "", "Show optics diag", "Weapons");
			s_bOpticDiagRegistered = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InputFloatClamped(inout float value, string label, float min, float max, int pxWidth = 100)
	{
		DbgUI.InputFloat(label, value, pxWidth);
		value = Math.Clamp(value, min, max);
	}

	//------------------------------------------------------------------------------------------------
	void DebugOptics()
	{
		DbgUI.Begin("Optics");
		{
			InputFloatClamped(m_fDebugUnit, "m_fDebugUnit", -100, 100);
			InputFloatClamped(m_fDebugAngle, "m_fDebugAngle", -1000, 1000);
			InputFloatClamped(m_fReticleAngularSize, "m_fReticleAngularSize", 0, 100);
			InputFloatClamped(m_fReticlePortion, "m_fReticlePortion", 0, 100);
			InputFloatClamped(m_fObjectiveFov, "m_fObjectiveFov", 0, 100);
			InputFloatClamped(m_fObjectiveScale, "m_fObjectiveScale", -100, 100);
			InputFloatClamped(m_fVignetteScale, "m_fVignetteScale", -100, 100.0);
			InputFloatClamped(m_fVignetteMoveSpeed, "m_fVignetteMoveSpeed", -100, 100.0);
			InputFloatClamped(m_fMisalignmentDampingSpeed, "m_fMisalignmentDampingSpeed", -100, 100);
			InputFloatClamped(m_fRotationScale, "m_fRotationScale", -100, 100);
			InputFloatClamped(m_fRotationDampingSpeed, "m_fRotationDampingSpeed", -100, 100);
			InputFloatClamped(m_fMovementScale, "m_fMovementScale", -100, 100);
			InputFloatClamped(m_fMovementDampingSpeed, "m_fMovementDampingSpeed", -100, 100);
			InputFloatClamped(m_fRollScale, "m_fRollScale", -100, 100);
			InputFloatClamped(m_fRollDampingSpeed, "m_fRollDampingSpeed", -100, 100);
			InputFloatClamped(m_fMisalignmentScale, "m_fMisalignmentScale", -100, 100);
			InputFloatClamped(m_fReticleTextureGlowAlpha, "m_fReticleTextureGlowAlpha", 0, 1);
		}
		DbgUI.End();

		SCR_2DPIPSightsComponent pip = SCR_2DPIPSightsComponent.Cast(this);
		if (!pip || !pip.IsPIPActive())
			s_OnSetupOpticImage.Invoke();

		float targetSize = 0.5 * Math.Tan(Math.DEG2RAD * m_fDebugAngle * m_fDebugUnit);
		float range = 1000;

		vector sightMat[4];
		GetSightsTransform(sightMat, true);
		vector ownerMat[4];
		GetOwner().GetWorldTransform(ownerMat);
		Math3D.MatrixMultiply4(ownerMat, sightMat, sightMat);

		vector opticSide = sightMat[0];
		vector opticUp = sightMat[1];
		vector opticDir = sightMat[2];
		vector opticPos = sightMat[3];

		vector targetPos = opticPos + opticDir * range;
		vector targetSide = opticSide * targetSize * range;
		vector targetUp = opticUp * targetSize * range;

		vector target[8];
		target[0] = targetPos - targetSide + targetUp;
		target[1] = targetPos + targetSide + targetUp;
		target[2] = targetPos + targetSide - targetUp;
		target[3] = targetPos - targetSide - targetUp;
		target[4] = targetPos - targetSide + targetUp;
		target[5] = targetPos + targetSide - targetUp;
		target[6] = targetPos + targetSide + targetUp;
		target[7] = targetPos - targetSide - targetUp;

		vector target2[6];
		target2[0] = targetPos + targetSide;
		target2[1] = targetPos - targetSide;
		target2[2] = targetPos - targetUp;
		target2[3] = targetPos + targetSide;
		target2[4] = targetPos + targetUp;
		target2[5] = targetPos - targetSide;

		Shape.CreateLines(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, target, 8);
		Shape.CreateLines(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, target2, 6);
		Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.NOZWRITE, targetPos, targetSize * range);
		}
#endif
}
