enum DepthOfFieldTypes
{
	NONE,
	SIMPLE,
	BOKEH,
};

class SCR_DepthOfFieldEffect : SCR_BaseScreenEffect
{
	// PP constants
	//DOFBokeh AddDOFBokehEffect()
	protected const int FOCUSDISTANCE_MULTIPIER 							= 15;
	protected const int FOCALLENGTH_MAX 									= 5000;
	[Attribute( defvalue: "225", uiwidget: UIWidgets.EditBox, desc: "Intensity of the nearby DOF" )]
	protected int m_iFocalLengthNearIntensity;
	
	//DOFNormal AddDOFEffect()
	protected const float FOCALDISTANCE_INTENSITY 						= 0.3;
	protected const float STANDARD_FOCALCHANGE_NEAR 					= 0.25;
	protected const int SIMPLEDOF_FOCALCHANGE_MAX			 			= 10000;
	protected const int SIMPLEDOF_EFFECT_MAX							= 1500;
	// Variables synced with DeathEffect 
	protected const float DOF_START_OPACITY								= 0.7;
	protected const float DOF_FADEIN_OPACITY_TARGET						= 1;

	// Variables connected to a material, need to be static
	static const int DEPTH_OF_FIELD_PRIORITY							= 8;
	
	// Play Animation of ClearDOFOutEffect()
	protected const float DOFOUT_OPACITY_FADEOUT_DURATION				= 2;
	protected const float DOFOUT_PROGRESSION_FADEOUT_DURATION			= 1;

	//DepthOfField
	private static float s_fFocalChange									= 10000;
	private static float s_fFocalDistance;
	private static float s_fFocalChangeNear;
	protected static bool s_bSkipFar;
	protected const string DOF_NORMAL_EMAT								= "{403795B9349EA61C}UI/Materials/ScreenEffects_DepthOfFieldPP.emat";

	//DepthOfFieldBOKEH
	private static float s_fFocalLength									= 0.1;	// Blur originates from horizon, higher is closer to camera
	private static float s_fFocusDistance;
	private static float s_fFocalLengthNear;									// Blur originates from camera, higher is further
	protected const string DOF_BOKEH_EMAT								= "{5CFBB3297D669D9C}UI/Materials/ScreenEffects_DepthOfFieldBokehPP.emat";
	
	private int m_iDesiredDofType;

	// Widgets
	private ImageWidget													m_wDOFOut;
	private ImageWidget													m_wDeath;

	// Enabling/Disabling of PP fx
	private static bool s_bNearDofEffect;
	private static bool s_bEnableDOFBokeh;
	private static bool s_bEnableDOF;
	private bool m_bDisplaySuspended;
	
	//Character
	protected ChimeraCharacter 											m_pCharacterEntity;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wDOFOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("DOFOut"));	
		m_wDeath = ImageWidget.Cast(m_wRoot.FindAnyWidget("DeathOverlay"));
	}

	//------------------------------------------------------------------------------------------------
	override void SettingsChanged()
	{
		ClearEffects();
		
		// Get desired type of DOF
		BaseContainer m_VideoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (m_VideoSettings)
		{
			m_VideoSettings.Get("m_iDofType", m_iDesiredDofType);
			m_VideoSettings.Get("m_bNearDofEffect", s_bNearDofEffect);
		}

		if (!m_pCharacterEntity)
			return;
		
		if (m_iDesiredDofType == DepthOfFieldTypes.SIMPLE)
		{
			m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(),DEPTH_OF_FIELD_PRIORITY,PostProcessEffectType.DepthOfFieldBokeh, "");
			m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(),DEPTH_OF_FIELD_PRIORITY,PostProcessEffectType.DepthOfField, DOF_NORMAL_EMAT);
		}
		else if (m_iDesiredDofType == DepthOfFieldTypes.BOKEH)
		{
			m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(),DEPTH_OF_FIELD_PRIORITY,PostProcessEffectType.DepthOfField, "");
			m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(),DEPTH_OF_FIELD_PRIORITY,PostProcessEffectType.DepthOfFieldBokeh, DOF_BOKEH_EMAT);
		}
	}

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		SettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnSuspended()
	{
		m_bDisplaySuspended = true;
		ClearEffects();
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnResumed()
	{
		m_bDisplaySuspended = false;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateEffect(float timeSlice)
	{
		if (m_bDisplaySuspended)
			return;

		if (!m_wDeath)
			return;
		
		if (m_iDesiredDofType == DepthOfFieldTypes.NONE)
			return;
		
		bool addNear = IsNearDOFAllowed();
		if (m_iDesiredDofType == DepthOfFieldTypes.SIMPLE)
			AddDOFEffect(timeSlice, addNear);
		else if (m_iDesiredDofType == DepthOfFieldTypes.BOKEH)
			AddDOFBokehEffect(addNear);
	}

	//------------------------------------------------------------------------------------------------
	bool IsNearDOFAllowed()
	{
		if (!s_bNearDofEffect || !m_pCharacterEntity)
			return false;

		CharacterControllerComponent controller = m_pCharacterEntity.GetCharacterController();

		if (!controller.IsWeaponRaised() || controller.IsGadgetInHands() || controller.GetInspect())
			return false;

		if (!controller.IsWeaponADS())
			return true;

		//When ADS'sing and current sights are using PIP, disable nearDOF
		BaseWeaponManagerComponent weaponManager = m_pCharacterEntity.GetCharacterController().GetWeaponManagerComponent();;
		if (weaponManager)
		{
			SCR_2DPIPSightsComponent sights = SCR_2DPIPSightsComponent.Cast(weaponManager.GetCurrentSights());
			if (sights && sights.IsPIPEnabled())
				return false;
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void AddDOFBokehEffect(bool nearDofAllowed)
	{
		if (!m_pCharacterEntity)
			return;
		
		if (m_wDeath.GetOpacity() > 0.1 && !m_bDisplaySuspended)
			s_fFocalLength = s_fFocalLengthNear + (FOCALLENGTH_MAX * (m_wDeath.GetOpacity() - DOF_START_OPACITY) / (DOF_FADEIN_OPACITY_TARGET - DOF_START_OPACITY));
		else
			s_fFocalLength = 0.1; //If no death/unconsciousness blur is desired, set focallength to defaultvalue
		
		s_fFocusDistance = FOCUSDISTANCE_MULTIPIER;

		//s_fFocalLength cannot be 0, so it is disabled when it is within a 0.1 margin of the lowest permitted value
		if (s_fFocalLength > 0.2 || nearDofAllowed)
		{
			s_bEnableDOFBokeh = true;
			s_fFocalLengthNear = m_iFocalLengthNearIntensity;
		}
		else
			s_bEnableDOFBokeh = false;
	}

	//------------------------------------------------------------------------------------------------
	void AddDOFEffect(float timeslice, bool nearDofAllowed)
	{
		if (!m_pCharacterEntity)
			return;
		
		s_fFocalDistance = FOCALDISTANCE_INTENSITY;

		// if s_bNearDofEffect is allowed, simpleDOF always must be enabled. If not, it must be disabled when inactive (i.e. focalchange > max)
		if (s_fFocalChange < SIMPLEDOF_FOCALCHANGE_MAX)
		{
			s_bEnableDOF = true;
			s_bSkipFar = false;
			s_fFocalChangeNear = SIMPLEDOF_FOCALCHANGE_MAX;
		}
		else if (nearDofAllowed)
		{
			s_fFocalChangeNear = STANDARD_FOCALCHANGE_NEAR;
			s_bEnableDOF = true;
			s_bSkipFar = true;
		}
		else
			s_bEnableDOF = false;

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (damageMan && damageMan.GetState() == EDamageState.DESTROYED && !m_bDisplaySuspended)
			s_fFocalChange = 100 - 100 * (m_wDeath.GetOpacity() - DOF_START_OPACITY) / (DOF_FADEIN_OPACITY_TARGET - DOF_START_OPACITY);
		else
			s_fFocalChange = SIMPLEDOF_FOCALCHANGE_MAX;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		s_fFocalLength 						= 0.1;
		s_bEnableDOF						= false;
		s_bEnableDOFBokeh					= false;
	}
};
