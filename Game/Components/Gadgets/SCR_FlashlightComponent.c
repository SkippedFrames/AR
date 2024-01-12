[EntityEditorProps(category: "GameScripted/Gadgets", description: "Flashlight", color: "0 0 255 255")]
class SCR_FlashlightComponentClass: SCR_GadgetComponentClass
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDescription")]
class SCR_LenseColor
{
	[Attribute("", UIWidgets.EditBox, desc: "Lense type", category: "Flashlight")]
	string m_sDescription;
	
	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Lense color", category: "Flashlight")]
	vector m_vLenseColor;
	
	[Attribute("5.0", UIWidgets.Slider, desc: "Light Value", "-8.0 20 1", category: "Flashlight")]
	float m_fLightValue;
};

//------------------------------------------------------------------------------------------------
class SCR_FlashlightComponent : SCR_GadgetComponent
{		
	[Attribute("50", UIWidgets.EditBox, desc: "Intensity of the emmissive texture", params: "0 1000", category: "Flashlight")]
	protected float m_fEmissiveIntensity;
			
	[Attribute("", UIWidgets.Object, desc: "Array of lense colors", category: "Flashlight")]
	protected ref array<ref SCR_LenseColor> m_LenseArray; 
	
	[Attribute("0.5", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneHand;
	
	[Attribute("1.2", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneStrapped;
	
	[Attribute("0.1", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneVehicle;
	
	[Attribute("0 0 -0.03", UIWidgets.EditBox, desc: "When strapped light entity is being aligned with weapon to adjust light angle, this entity is offset to avoid it visibly floating", category: "Flashlight")]
	protected vector m_vFlashlightAdjustOffset;
		
	protected bool m_bLastLightState;	// remember the last light state for cases where you put your flashlight into storage but there is no slot available -> true = active
	protected bool m_bIsSlottedVehicle;	// char owner is in vehicle while flashlight is slotted
	protected int m_iCurrentLenseColor;	
	protected vector m_vAnglesCurrent;	// current local angles
	protected vector m_vAnglesTarget;	// target local angles used to align flashlight with aiming angles while strapped
	protected vector m_ItemMat[4];		// owner transformation matrix
	protected LightEntity m_Light;
	protected ParametricMaterialInstanceComponent m_EmissiveMaterial;
	protected SCR_CompartmentAccessComponent m_CompartmentComp;
	protected CharacterControllerComponent m_CharController;
					
	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{		
		m_bActivated = state;

		// Play sound
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();		
		if (soundManagerEntity)
		{
			if (m_bActivated)
				soundManagerEntity.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.SOUND_FLASHLIGHT_ON);
			else
				soundManagerEntity.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.SOUND_FLASHLIGHT_OFF);
		}
				
		UpdateLightState();
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update state of light ON/OFF
	protected void UpdateLightState()
	{
		if (m_bActivated)
			EnableLight();
		else 
			DisableLight();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch flashlight lenses
	protected void SwitchLenses(int filter)
	{
		m_iCurrentLenseColor = filter;
		
		m_Light.SetColor(Color.FromVector(m_LenseArray[m_iCurrentLenseColor].m_vLenseColor), m_LenseArray[m_iCurrentLenseColor].m_fLightValue);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn light entity
	protected void EnableLight()
	{	
		if (m_Light)	
			m_Light.SetEnabled(true);
		
		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(m_fEmissiveIntensity);
		
		SwitchLenses(0);	// TODO lens switch support
		
		if (m_iMode == EGadgetMode.IN_SLOT)
			SetShadowNearPlane(true);
		else if (m_iMode == EGadgetMode.IN_HAND)
			SetShadowNearPlane(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove light
	protected void DisableLight()
	{
		if (m_Light)
			m_Light.SetEnabled(false);
		
		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(0);
		
		DeactivateGadgetFlag();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set near plane of light shadow to avoid blocking the light with arm/weapon
	//! \param state determines mode: false - in hand / true - strapped
	//! \param isLeavingVehicle is signal for removing vehicle near plane mode
	protected void SetShadowNearPlane(bool state, bool isLeavingVehicle = false)
	{				
		float nearPlane;
		m_bIsSlottedVehicle = false;
		
		if (state)	// strapped mode
		{
			nearPlane = m_fLightNearPlaneStrapped;
			if (m_CompartmentComp && m_CompartmentComp.IsInCompartment() && !isLeavingVehicle)
			{
					nearPlane = m_fLightNearPlaneVehicle;
					m_bIsSlottedVehicle = true;
				DeactivateGadgetFlag();
				}
			else if (m_bActivated)
			{
				ActivateGadgetFlag();
				AdjustTransform(m_vFlashlightAdjustOffset);
		}
		}
		else		// in hand mode
		{
			nearPlane = m_fLightNearPlaneHand;
		}
		
		if (m_Light)
		{
			m_Light.SetNearPlane(nearPlane);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void AdjustTransform(vector offset = vector.Zero)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
		{
		
			InventoryStorageSlot slot = itemComponent.GetParentSlot();
			ItemAnimationAttributes animAttr = ItemAnimationAttributes.Cast(itemComponent.FindAttribute(ItemAnimationAttributes));
			if (slot && animAttr)
			{
				vector matLS[4];
				animAttr.GetAdditiveTransformLS(matLS);
				matLS[3] = matLS[3] + m_vEquipmentSlotOffset + offset;
				slot.SetAdditiveTransformLS(matLS); 
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
		{
			SetShadowNearPlane(true);
			
			if (PilotCompartmentSlot.Cast(compSlot))
				ToggleActive(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true, true);
	}
			
	//------------------------------------------------------------------------------------------------
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{		
		super.ModeSwitch(mode, charOwner);

		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return;

		if (mode == EGadgetMode.ON_GROUND || !charOwner)
			m_CharController = null;
		else
			m_CharController = CharacterControllerComponent.Cast( m_CharacterOwner.FindComponent(CharacterControllerComponent) );
		
		if (mode == EGadgetMode.ON_GROUND || mode == EGadgetMode.IN_STORAGE)
		{
			inventoryItemComp.SetTraceable(true);

			m_bLastLightState = m_bActivated;
			
			if (m_bActivated)
				ToggleActive(false);
			
			if (mode == EGadgetMode.ON_GROUND)	// always set the last state as false when removing from inventory
				m_bLastLightState = false;
				
		}
		else if (mode == EGadgetMode.IN_SLOT)
		{
			inventoryItemComp.SetTraceable(false);

			m_CompartmentComp = SCR_CompartmentAccessComponent.Cast(m_CharacterOwner.FindComponent(SCR_CompartmentAccessComponent));
			if (m_CompartmentComp)
			{				
				m_CompartmentComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			}
			
			SetShadowNearPlane(true);
		}
		else if (mode == EGadgetMode.IN_HAND)
		{			
			inventoryItemComp.SetTraceable(true);

			SetShadowNearPlane(false);
			
			if (m_bLastLightState)
				ToggleActive(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_SLOT)
		{					
			if (m_CompartmentComp)
			{				
				m_CompartmentComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}
			
			SetShadowNearPlane(false);
		}
		
		super.ModeClear(mode);
	}
			
	//------------------------------------------------------------------------------------------------
	override protected void ToggleActive(bool state)
	{
		if (m_iMode == EGadgetMode.IN_STORAGE && !m_bActivated)	// trying to activate flashlight hidden in inventory
			return;	
		
		// trying to activate flashlight while driving
		if (!m_bActivated && m_CompartmentComp && PilotCompartmentSlot.Cast(m_CompartmentComp.GetCompartment()))
			return;
		
		super.ToggleActive(state);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateGadgetFlag()
	{
		super.ActivateGadgetFlag();

		ConnectToGadgetsSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	override void DeactivateGadgetFlag()
	{
		super.DeactivateGadgetFlag();
		
		DisconnectFromGadgetsSystem();
		
		if (m_iMode == EGadgetMode.IN_SLOT )	// reset slot position of the gadget back to its default
			AdjustTransform();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		SCR_CharacterControllerComponent scrCharacterController = SCR_CharacterControllerComponent.Cast(m_CharController);
		if (scrCharacterController && scrCharacterController.GetLifeState() != ECharacterLifeState.ALIVE)	// if in chars inventory && char is dead/uncon
			return;
		
		ToggleActive(!m_bActivated);
	}
		
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.FLASHLIGHT;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeToggled()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool RplSave(ScriptBitWriter writer)
    {
		if (!super.RplSave(writer))
			return false;
		
        writer.WriteBool(m_bActivated);
       
		return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		if (!super.RplLoad(reader))
			return false;
		
        reader.ReadBool(m_bActivated);
				
		UpdateLightState();
		
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool OnTicksOnRemoteProxy() 
	{ 
		return true; // proxies will only tick on owners without this
	};
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{		
		if (m_iMode != EGadgetMode.IN_SLOT || !m_CharController || m_CharController.IsDead())	// deactivate frame on death without turning the light off
		{
			DeactivateGadgetFlag();
			return;
		}
			
		SCR_CharacterControllerComponent scrCharacterController = SCR_CharacterControllerComponent.Cast(m_CharController);	
		if (scrCharacterController && scrCharacterController.GetLifeState() != ECharacterLifeState.ALIVE)	// not needed while uncon + can get quite jittery there
			return;
		
		// adjust angle of the flashlight to provide usable angle within various poses
		GetOwner().GetTransform(m_ItemMat);
		m_vAnglesCurrent = GetOwner().GetLocalYawPitchRoll();
		m_vAnglesTarget = ( Math.RAD2DEG * m_CharController.GetInputContext().GetAimingAngles() - Math3D.MatrixToAngles(m_ItemMat) ) + m_vAnglesCurrent;	 // diff between WS aiming and item angles, add local to the result
		m_vAnglesTarget[0] = fixAngle_180_180(m_vAnglesTarget[0]);		
		m_vAnglesTarget[1] = Math.Clamp(m_vAnglesTarget[1], 0, 30);	// only need too offset upwards, also avoid glitches with some stances
		m_vAnglesTarget[2] = 0;	// no roll		
		
		m_vAnglesTarget[0] = Math.Lerp(m_vAnglesCurrent[0], m_vAnglesTarget[0], timeSlice*4);
		m_vAnglesTarget[1] = Math.Lerp(m_vAnglesCurrent[1], m_vAnglesTarget[1], timeSlice*4);
		GetOwner().SetYawPitchRoll(m_vAnglesTarget); // sets local angles
	}
					
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		m_Light =  LightEntity.Cast( owner.GetChildren() );
		m_EmissiveMaterial = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));

		// Insert default entry if none are set
		if (m_LenseArray.Count() == 0)
		{
			SCR_LenseColor defColor = new SCR_LenseColor();
			defColor.m_sDescription = "CONFIGURE ME";
			m_LenseArray.Insert(defColor);
		}
	}
};
