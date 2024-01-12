class SCR_AISelectFireMode: AITaskScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Try set Firemode", "", ParamEnumArray.FromEnum(EWeaponFiremodeType) )]
	protected EWeaponFiremodeType m_FiremodeType;
	
    override bool VisibleInPalette() {return true;}
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		GenericEntity controlledEntity = GenericEntity.Cast(owner.GetControlledEntity());
		if (!controlledEntity)
			return ENodeResult.FAIL;
		
		// Resolve which weapon manager to use
		
		BaseWeaponManagerComponent wpnManagerComponent;
		CharacterControllerComponent controller;
		
		
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(controlledEntity.FindComponent(CompartmentAccessComponent));
		if (!compAccess)
			return ENodeResult.FAIL;
		
		BaseCompartmentSlot compartment = compAccess.GetCompartment();
		if (compartment)
		{
			wpnManagerComponent = BaseWeaponManagerComponent.Cast(compartment.GetOwner().FindComponent(BaseWeaponManagerComponent));
			if (wpnManagerComponent)
			{
				// Return success if in turret. Turret weapons have no fire modes anyway.
				return ENodeResult.SUCCESS;
			}
		}
		else
		{
			controller = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
			if (controller)
				wpnManagerComponent = controller.GetWeaponManagerComponent();
		}
		
		if (!wpnManagerComponent || !controller)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent wpnComponent = wpnManagerComponent.GetCurrent();
		if (!wpnComponent)
			return ENodeResult.FAIL;
		
		if (wpnComponent.GetCurrentFireModeType() == m_FiremodeType)
			return ENodeResult.SUCCESS;
		
		BaseMuzzleComponent muzzle = wpnComponent.GetCurrentMuzzle();
		
		// Fail if there is no muzzle, it is probably a grenade
		if (!muzzle)
			return ENodeResult.FAIL;
		
		array<BaseFireMode> fireModes = {};
		muzzle.GetFireModesList(fireModes);
		
		int fireModesCount = fireModes.Count();
		int semiAytoIndex = -1, autoIndex = -1, burstIndex = -1, safetyIndex = -1;
		for (int i = 0; i<fireModesCount; i++)
		{
			switch (fireModes[i].GetFiremodeType())
			{
				case EWeaponFiremodeType.Semiauto : 
				{		 
					semiAytoIndex = i;
					break;
				}
				case EWeaponFiremodeType.Auto :
				{
					autoIndex = i;
					break;
				}
				case EWeaponFiremodeType.Burst :
				{
					burstIndex = i;
					break;
				}
				case EWeaponFiremodeType.Safety :
				{
					safetyIndex = i;
					break;
				}
			}
		}
		
		if (m_FiremodeType == EWeaponFiremodeType.Auto && autoIndex != -1)
		{
			controller.SetFireMode(autoIndex);
			return ENodeResult.SUCCESS;
		}
		else if (m_FiremodeType == EWeaponFiremodeType.Burst || m_FiremodeType == EWeaponFiremodeType.Auto && burstIndex != -1)
		{
			controller.SetFireMode(burstIndex);
			return ENodeResult.SUCCESS;
		}
		else if (semiAytoIndex != -1)
		{
			controller.SetFireMode(semiAytoIndex);
			return ENodeResult.SUCCESS;
		}
		else if (safetyIndex != -1)
		{
			controller.SetFireMode(safetyIndex);
			return ENodeResult.SUCCESS;
		}
		
		return ENodeResult.FAIL;
    }
		
	override protected string GetNodeMiddleText()
	{
		return "Automatically select the fire rate. Auto > burst > single";
	}
};
