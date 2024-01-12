[EntityEditorProps(category: "GameScripted/Gadgets", description: "Consumable gadget")]
class SCR_ConsumableItemComponentClass : SCR_GadgetComponentClass
{
};

//------------------------------------------------------------------------------------------------
// Consumable gadget component
class SCR_ConsumableItemComponent : SCR_GadgetComponent
{
	[Attribute("", UIWidgets.Object, category: "Consumable" )]
	protected ref SCR_ConsumableEffectBase m_ConsumableEffect;

	[Attribute("0", UIWidgets.CheckBox, "Switch model on use, f.e. from packaged version", category: "Consumable")]
	protected bool m_bAlternativeModelOnAction;	
	
	[Attribute("0", UIWidgets.CheckBox, "Show the model of the item on the character after using", category: "Consumable")]
	protected bool m_bVisibleEquipped;

	protected SCR_CharacterControllerComponent m_CharController;
	
	//Target character is the target set when it's not m_CharacterOwner
	protected IEntity m_TargetCharacter;
	
	protected int m_iStoredHealedGroup;
	
	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! \param target is the target entity
	void ApplyItemEffect(IEntity target, SCR_ConsumableEffectAnimationParameters animParams, IEntity item, bool deleteItem = true)
	{
		if (!m_ConsumableEffect)
			return;

		m_ConsumableEffect.ApplyEffect(target, target, item, animParams);

		ModeClear(EGadgetMode.IN_HAND);
		
		if (deleteItem)
			RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseBegan event from SCR_CharacterControllerComponent
	protected void OnUseBegan(IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{
		if (m_bAlternativeModelOnAction)
			SetAlternativeModel(true);

		if (!m_CharacterOwner || !animParams)
			return;
		
		SetHealedGroup(animParams.m_intParam, true);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseComplete event from SCR_CharacterControllerComponent
	protected void OnApplyToCharacter(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams)
	{
		ClearInvokers(animParams.m_intParam);
		
		if (!successful)
		{
			if (animParams.m_itemUseCommandId == -1)
			{
				Print("Consumable item OnItemUseComplete event called with empty SCR_ConsumableEffectAnimationParameters", LogLevel.ERROR);
				return;
			}

			if (m_bAlternativeModelOnAction)
				SetAlternativeModel(false);

			return;	
		}

		bool deleteItem = true;
		if (GetConsumableEffect())
			deleteItem = GetConsumableEffect().GetDeleteOnUse();
		
		IEntity target = GetTargetCharacter();
		if (!target)
			target = m_CharacterOwner;
		if (!target)
			return;

		ApplyItemEffect(target, animParams, item, deleteItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetHealedGroup(EBandagingAnimationBodyParts group, bool healed)
	{
		m_iStoredHealedGroup = group * healed;
		
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(m_CharacterOwner);
		if (!ownerChar)
			return;

		SCR_CharacterDamageManagerComponent targetDamageMan;
		IEntity target = GetTargetCharacter();
		ChimeraCharacter targetChar;
		if (target)
			targetChar = ChimeraCharacter.Cast(target);
		else
			targetChar = ChimeraCharacter.Cast(m_CharacterOwner);

		if (targetChar)
			targetDamageMan = SCR_CharacterDamageManagerComponent.Cast(targetChar.GetDamageManager());
		
		if (!targetDamageMan)
			return;

		targetDamageMan.SetGroupIsBeingHealed(targetDamageMan.FindAssociatedHitZoneGroup(group), healed);
	}
	
	//-----------------------------------------------------------------------------
	//! Switch item model
	//! \param useAlternative determines whether alternative or base model is used
	void SetAlternativeModel(bool useAlternative)
	{
		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast( GetOwner().FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return;

		ItemAttributeCollection attributeCollection = inventoryItemComp.GetAttributes();
		if (!attributeCollection)
			return;

		SCR_AlternativeModel additionalModels = SCR_AlternativeModel.Cast( attributeCollection.FindAttribute(SCR_AlternativeModel));
		if (!additionalModels)
			return;

		ResourceName consumableModel;
		if (useAlternative)
			consumableModel = additionalModels.GetAlternativeModel();
		else if (GetOwner())
			consumableModel = SCR_Global.GetPrefabAttributeResource(GetOwner(), "MeshObject", "Object");

		Resource resource = Resource.Load(consumableModel);
		if (!resource)
			return;

		BaseResourceObject baseResource = resource.GetResource();
		if (baseResource)
		{
			VObject asset = baseResource.ToVObject();
			if (asset)
				GetOwner().SetObject(asset, "");
		}
	}

	//-----------------------------------------------------------------------------
	//! Set visibility of item
	//! \Sets item visible on the outside of the physical character
	override bool IsVisibleEquipped()
	{
		return m_bVisibleEquipped;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode == EGadgetMode.IN_HAND)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(charOwner.FindComponent(SCR_CharacterControllerComponent));
			m_CharController.m_OnItemUseFinishedInvoker.Insert(OnApplyToCharacter);
			m_CharController.m_OnItemUseBeganInvoker.Insert(OnUseBegan);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_HAND)
		{
			// CallLater to ensure the invokers are removed after the m_OnItemUseFinishedInvoker is called. This is ensured because m_OnItemUseFinishedInvoker is called on end of fixedFrame
			if (m_CharController)
				GetGame().GetCallqueue().CallLater(ClearInvokers, param1: -1);
		}

		super.ModeClear(mode);
	}

	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		if (!m_ConsumableEffect || !m_ConsumableEffect.CanApplyEffect(m_CharacterOwner, GetOwner()))
			return;

		m_ConsumableEffect.ActivateEffect(m_CharacterOwner, m_CharacterOwner, GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearInvokers(int targetGroup = -1)
	{
		if (targetGroup == -1 && m_iStoredHealedGroup != 0)
			SetHealedGroup(m_iStoredHealedGroup, false);
		else
			SetHealedGroup(targetGroup, false);
	
		if (!m_CharController)
			return;
		
		m_CharController.m_OnItemUseFinishedInvoker.Remove(OnApplyToCharacter);
		m_CharController.m_OnItemUseBeganInvoker.Remove(OnUseBegan);
		
		m_CharController = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable type
	//! \return consumable type
	SCR_EConsumableType GetConsumableType()
	{
		if (m_ConsumableEffect)
			return m_ConsumableEffect.m_eConsumableType;

		return SCR_EConsumableType.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable effect
	//! \return consumable effect
	SCR_ConsumableEffectBase GetConsumableEffect()
	{
		return m_ConsumableEffect;
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.CONSUMABLE;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeToggled()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetCharacter(IEntity target)
	{
		m_TargetCharacter = target;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTargetCharacter()
	{
		return m_TargetCharacter;
	}
};
