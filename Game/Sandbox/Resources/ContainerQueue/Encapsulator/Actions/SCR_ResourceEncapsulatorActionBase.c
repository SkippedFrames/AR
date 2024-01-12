[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionBase
{
	void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionChangeResourceValue : SCR_ResourceEncapsulatorActionBase
{
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceValueCurrent;
	
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		float resourcesResult	= Math.Min(m_fResourceValueCurrent, representedContainer.GetMaxResourceValue());
		m_fResourceValueCurrent	-= resourcesResult;
		
		representedContainer.SetResourceValue(resourcesResult, false);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionDisableUserActions : SCR_ResourceEncapsulatorActionBase
{
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		IEntity owner = representedContainer.GetOwner();
		
		if (!owner)
			return;
		
		ActionsManagerComponent actionManagerComponent = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		
		if (!actionManagerComponent)
			return;
		
		actionManagerComponent.Deactivate(owner);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionDisableInventoryStorage : SCR_ResourceEncapsulatorActionBase
{
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		IEntity owner = representedContainer.GetOwner();
		
		if (!owner)
			return;
		
		InventoryItemComponent inventoryComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		
		if (!inventoryComponent)
			return;
		
		inventoryComponent.DisablePhysics();
		inventoryComponent.ActivateOwner(false);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionChangeRights : SCR_ResourceEncapsulatorActionBase
{
	[Attribute(defvalue: EResourceRights.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Limits the taking of resources to a specific group", enums: ParamEnumArray.FromEnum(EResourceRights))]
	protected EResourceRights m_eResourceRights;
	
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		representedContainer.SetResourceRights(m_eResourceRights);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionChangeDecay : SCR_ResourceEncapsulatorActionBase
{
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bEnableResourceDecay;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: "0.0 inf 1.0")]
	protected float m_fResourceDecay;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceDecayTickrate;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceDecayTimeout;
	
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		representedContainer.EnableDecay(m_bEnableResourceDecay, false);
		representedContainer.SetResourceDecay(m_fResourceDecay, false);
		representedContainer.SetResourceDecayTickrate(m_fResourceDecayTickrate);
		representedContainer.SetResourceDecayTimeout(m_fResourceDecayTimeout);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionChangeOnEmptyBehavior : SCR_ResourceEncapsulatorActionBase
{
	[Attribute(defvalue: EResourceContainerOnEmptyBehavior.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the behavior of when the container resource value reaches 0.", enums: ParamEnumArray.FromEnum(EResourceContainerOnEmptyBehavior))]
	protected EResourceContainerOnEmptyBehavior m_eOnEmptyBehavior;
	
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		representedContainer.SetOnEmptyBehavior(m_eOnEmptyBehavior);
	}
}

[BaseContainerProps()]
class SCR_ResourceEncapsulatorActionChangeGain : SCR_ResourceEncapsulatorActionBase
{
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bEnableResourceGain;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: "0.0 inf 1.0")]
	protected float m_fResourceGain;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceGainTickrate;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceGainTimeout;
	
	override void PerformAction(SCR_ResourceContainer representativeContainer, SCR_ResourceContainer representedContainer)
	{
		if (!representedContainer)
			return;
		
		representedContainer.EnableGain(m_bEnableResourceGain, false);
		representedContainer.SetResourceGain(m_fResourceGain, false);
		representedContainer.SetResourceGainTickrate(m_fResourceGainTickrate);
		representedContainer.SetResourceGainTimeout(m_fResourceGainTimeout);
	}
}

