[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DSightsComponentClass : SCR_2DOpticsComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_2DSightsComponent : SCR_2DOpticsComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Should hide parent of parent object when using 2D sights", category: "2DSights")]
	protected bool m_bShouldHideParentParentObject;

	[Attribute("0", UIWidgets.CheckBox, "Should reticle be scaled with current zoom", category: "2DSights")]
	protected bool m_bDynamicReticle;

	[Attribute("1.4", UIWidgets.Slider, "Scale mult. of scope when target recoil is applied.", category: "2DSights", params: "1 3 0.01")]
	protected float m_fRecoilScaleMax;

	[Attribute("0.12", UIWidgets.Slider, "The amount of linear translation of recoil in meters that is deemed as target and applies the most scale, no scale is applied beyond this value.", category: "2DSights", params: "0.001 0.5 0.001")]
	protected float m_fRecoilTranslationTarget;

	[Attribute("10", UIWidgets.Slider, "Scale mult. of scope movement when target recoil is applied.", category: "2DSights", params: "1 50 0.01")]
	protected float m_fRecoilScaleTranslation;

	[Attribute("1", UIWidgets.EditBox, desc: "", params: "1 2000", category: "Behavior")]
	protected bool m_bAllowRecoil;

	// sway, taken from binocs
	protected vector m_vLastCameraDir;

	// sway
	protected vector m_vLastPos;

	// recoil
	protected CharacterAimingComponent m_CharacterAiming;
	protected TurretControllerComponent m_TurretController;

	protected SCR_SightsZoomFOVInfo m_SightsFovInfo;

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		super.OnSightADSActivated();

		// Set fov
		if (!m_SightsFovInfo)
		{
			m_SightsFovInfo = SCR_SightsZoomFOVInfo.Cast(GetFOVInfo());
			SetupFovInfo();
		}

		SelectZoomLevel(m_iSelectedZoomLevel);

		// Set parent
		IEntity owner = GetOwner();
		if (owner)
		{
			IEntity parent = owner.GetParent();
			if (m_bShouldHideParentObject)
			{
				owner.ClearFlags(EntityFlags.VISIBLE, false);
				if (parent && m_bShouldHideParentParentObject)
				{
					parent.ClearFlags(EntityFlags.VISIBLE, false);
				}
			}

			m_TurretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
			if (m_TurretController)
			{
				BaseCompartmentSlot slot = m_TurretController.GetCompartmentSlot();
				if (slot)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(slot.GetOccupant());
					if (character)
					{
						m_ParentCharacter = character;
					}
				}
			}
			else
			{
				IEntity parentParent = parent;
				while (parentParent)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(parentParent);
					if (character)
					{
						m_ParentCharacter = character;
						m_CharacterAiming = CharacterAimingComponent.Cast(character.FindComponent(CharacterAimingComponent));
						break;
					}

					parentParent = parentParent.GetParent();
				}
			}
		}

		if (m_CharacterAiming)
		{
			// due to sway
			vector m[4];
			GetGame().GetWorld().GetCurrentCamera(m);
			m_vLastCameraDir = Math3D.MatrixToAngles(m);
			m_vLastPos = GetSightsRelPosition();
		}

		// Switching input
		if (m_SightsFovInfo.GetStepsCount() > 1)
			GetGame().GetInputManager().AddActionListener(ACTION_WHEEL, EActionTrigger.VALUE, SelectNextZoomLevel);

		// Setup illumination
		if (m_bHasIllumination)
			GetGame().GetInputManager().AddActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSDeactivated()
	{
		IEntity owner = GetOwner();
		if (owner && m_bShouldHideParentObject)
		{
			owner.SetFlags(EntityFlags.VISIBLE, true);
			IEntity parent = owner.GetParent();
			if (parent && m_bShouldHideParentParentObject)
			{
				parent.SetFlags(EntityFlags.VISIBLE, true);
			}
		}

		if (m_bShouldHideParentCharacter && m_ParentCharacter)
		{
			m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);
		}

		m_ParentCharacter = null;

		super.OnSightADSDeactivated();

		// Removing switching input
		GetGame().GetInputManager().RemoveActionListener(ACTION_WHEEL, EActionTrigger.VALUE, SelectNextZoomLevel);

		// Removing illumination
		if (m_bHasIllumination)
			GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanFreelook()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles)
	{
		vector weaponAnglesMat[3];
		Math3D.AnglesToMatrix(aimModAngles, weaponAnglesMat);
		Math3D.MatrixMultiply3(pOutCameraTransform, weaponAnglesMat, pOutCameraTransform);
	}

	//------------------------------------------------------------------------------------------------
	//! Update vignettes scale based on weapon recoil
	void UpdateRecoil(float timeSlice)
	{

		float zScale = 1;

		if (m_ParentCharacter)
		{
			AimingComponent aimingComponent;
			if (m_ParentCharacter.IsInVehicle() && m_TurretController)
			{
				aimingComponent = m_TurretController.GetTurretComponent();
			}
			else
			{
				aimingComponent = m_CharacterAiming;
			}

			if (aimingComponent)
			{
				vector aimingTranslation = aimingComponent.GetRawAimingTranslation();
				float zAmount = aimingTranslation[2];
				float zMag = Math.AbsFloat(zAmount / m_fRecoilTranslationTarget);
				// kick back only, TODO@AS: make sure we work both directions
				zScale = Math.Clamp(1 + zMag, 1 / m_fRecoilScaleMax, m_fRecoilScaleMax);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{
		super.OnSightADSPostFrame(owner, timeSlice);

		if (!m_bZoomed)
			return;

		if (m_bAllowRecoil)
			UpdateRecoil(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupFovInfo()
	{
		if (!m_SightsFovInfo)
			return;

		// Set zooms
		//m_SightsFovInfo.SetupZooms();
		array<float> zooms = m_SightsFovInfo.ZoomsToArray();

		// Get zooms
		//int zoomCount = m_SightsFovInfo.GetStepsCount();
		int zoomCount = zooms.Count();

		for (int i = 0; i < zoomCount; i++)
		{
			// Calculate fov for each zoom
			float zoom = zooms[i];
			float fov = CalculateZoomFOV(zoom);
			m_SightsFovInfo.InsertFov(fov);

			// Reticle sizes
			float initalZoom = zooms[0];

			if (i == 0)
			{
				// Initial setup
				m_aReticleSizes.Insert(m_fDefaultSize);
				m_SightsFovInfo.SetCurrentFov(fov);
			}
			else
			{
				// Calulate size
				float zoomProgress = zooms[i] / initalZoom;
				float size = m_fDefaultSize * zoomProgress;
				m_aReticleSizes.Insert(size);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Switch betweem zoom levels from sights
	protected void SelectZoomLevel(int id)
	{
		// Is zoomed
		if (!m_bZoomed)
			return;

		// Check sights and id
		if (!m_SightsFovInfo)
			return;

		if (id < 0 || id > m_SightsFovInfo.GetStepsCount() - 1)
			return;

		// Set zoom
		m_SightsFovInfo.SetIndex(id);
		m_fMagnification = m_SightsFovInfo.GetCurrentZoom();

		// Reticle size - dynamic reticle size
		if (m_bDynamicReticle)
			m_fCurrentReticleSize = m_aReticleSizes[id];
	}

	//------------------------------------------------------------------------------------------------
	//! Move zoom level up and down based on mouse wheel value
	protected void SelectNextZoomLevel(float value)
	{
		if (!m_SightsFovInfo || value == 0)
			return;

		// TEMPORARY SOLUTION
		// There is a 'mistake' in inheritance, m_pParentCharacter is never set because ADS changes are not propagated (super is not called in SCR_2DPIPSightsComponent)
		SCR_ChimeraCharacter pParentCharacter = null;
		IEntity parent = null;
		IEntity owner = GetOwner();
		if (owner)
		{
			parent = owner.GetParent();

			IEntity parentParent = parent;
			while (parentParent)
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(parentParent);
				if (character)
				{
					pParentCharacter = character;
					break;
				}

				parentParent = parentParent.GetParent();
			}
		}

		// Down
		if (value > 0 && m_iSelectedZoomLevel < m_SightsFovInfo.GetStepsCount())
		{
			m_iSelectedZoomLevel++;
			SelectZoomLevel(m_iSelectedZoomLevel);

			if (parent && pParentCharacter)
				pParentCharacter.SetNewZoomLevel(m_iSelectedZoomLevel, true, Replication.FindId(parent.FindComponent(WeaponComponent)));
		}

		// Up
		if (value < 0 && m_iSelectedZoomLevel > 0)
		{
			m_iSelectedZoomLevel--;
			SelectZoomLevel(m_iSelectedZoomLevel);

			if (parent && pParentCharacter)
				pParentCharacter.SetNewZoomLevel(m_iSelectedZoomLevel, false, Replication.FindId(parent.FindComponent(WeaponComponent)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Action for toggling illumination
	protected void ToggleIllumination(float value, EActionTrigger trigger)
	{
		m_bIsIlluminationOn = !m_bIsIlluminationOn;
		EnableReticleIllumination(m_bIsIlluminationOn);

		// TEMPORARY SOLUTION
		// There is a 'mistake' in inheritance, m_pParentCharacter is never set because ADS changes are not propagated (super is not called in SCR_2DPIPSightsComponent)
		IEntity owner = GetOwner();
		if (owner)
		{
			IEntity parent = owner.GetParent();
			SCR_ChimeraCharacter pParentCharacter = null;
			IEntity parentParent = parent;
			while (parentParent)
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(parentParent);
				if (character)
				{
					pParentCharacter = character;
					break;
				}

				parentParent = parentParent.GetParent();
			}

			if (pParentCharacter)
			{
				pParentCharacter.SetIllumination(m_bIsIlluminationOn, Replication.FindId(WeaponComponent.Cast(parent.FindComponent(WeaponComponent))));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override float GetReticleYOffsetTarget()
	{
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_RETICLE_OFFSET)
			return m_fReticleOffsetY + GetCurrentSightsRange()[0];

		return m_fReticleOffsetY;
	}

	//------------------------------------------------------------------------------------------------
	// Get Set API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	SCR_SightsZoomFOVInfo GetSightsFovInfo()
	{
		return m_SightsFovInfo;
	}
}
