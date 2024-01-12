class SCR_ResourceEntityRefundAction : SCR_ScriptedUserAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	[Attribute(defvalue: EResourceGeneratorID.DEFAULT_STORAGE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Identifier for the generator used for storage", enums: ParamEnumArray.FromEnum(EResourceGeneratorID))]
	protected EResourceGeneratorID m_eGeneratorIdentifier;
	
	[Attribute(desc: "Invalid reason shown when there is no valid refund point")]
	protected LocalizedString m_sInvalidNoValidRefundPoint;
	
	protected SCR_Faction m_Faction;
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	protected SCR_CatalogEntitySpawnerComponent m_CatalogEntitySpawnerComponent;
	protected RplComponent m_ReplicationComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected float m_fResourceCost;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_ReplicationComponent || !m_ReplicationComponent.IsMaster())
			return;
		
		vector mins, maxs;
		vector transform[4];
		IEntity owner = GetOwner();
		
		owner.GetBounds(mins, maxs);
		owner.GetWorldTransform(transform);
		array<SCR_ResourceGenerator> generators = GetGame().GetResourceGrid().QueryGenerators(mins, maxs, transform);
		
		foreach (SCR_ResourceGenerator generatorTemp: generators)
		{
			if (!generatorTemp)
				continue;
			
			m_CatalogEntitySpawnerComponent = SCR_CatalogEntitySpawnerComponent.Cast(generatorTemp.GetOwner().FindComponent(SCR_CatalogEntitySpawnerComponent));
			
			if (m_CatalogEntitySpawnerComponent 
			&&	m_CatalogEntitySpawnerComponent.CanRefund(GetOwner(), pUserEntity) 
			&&	generatorTemp.IsAllowed(pUserEntity, m_eResourceType))
			{
				m_ResourceGenerator = generatorTemp;
				break;
			}
		}
		
		if (!m_ResourceGenerator || !m_CatalogEntitySpawnerComponent)
			return;
		
		if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(GetOwner()))
			m_ResourceGenerator.RequestGeneration(m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier());
		else
			m_ResourceGenerator.RequestGeneration(m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier());
		
		RplComponent.DeleteRplEntity(GetOwner(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		/*
		if (!m_CatalogEntitySpawnerComponent)
			return true;
		
		if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(GetOwner()))
			outName = WidgetManager.Translate("Refund (%1)", m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier());
		else
			outName = WidgetManager.Translate("Refund (%1)", m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier());
		
		return true;
		*/
		if (!m_CatalogEntitySpawnerComponent)
			return false;
		
		if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(GetOwner()))
			ActionNameParams[0] = (m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier()).ToString();
		else
			ActionNameParams[0] = (m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier()).ToString();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{
		
		return null;
		m_ResourceGenerator = null;
		vector mins, maxs;
		vector transform[4];
		IEntity owner = GetOwner();
		
		owner.GetBounds(mins, maxs);
		owner.GetWorldTransform(transform);
		array<SCR_ResourceGenerator> generators = GetGame().GetResourceGrid().QueryGenerators(mins, maxs, transform);

		foreach (SCR_ResourceGenerator generatorTemp: generators)
		{
			if (!generatorTemp)
				continue;
			
			m_CatalogEntitySpawnerComponent = SCR_CatalogEntitySpawnerComponent.Cast(generatorTemp.GetOwner().FindComponent(SCR_CatalogEntitySpawnerComponent));
			
			if (m_CatalogEntitySpawnerComponent 
			&&	m_CatalogEntitySpawnerComponent.CanRefund(GetOwner(), user) 
			&&	generatorTemp.IsAllowed(user, m_eResourceType))
			{
				m_ResourceGenerator = generatorTemp;
				break;
			}
		}
		
		if (!m_ResourceGenerator)
		{
			m_sCannotPerformReason = m_sInvalidNoValidRefundPoint;
			return false;
		}
			
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBeShownScript(IEntity user)
	{
		if (!m_ReplicationComponent)
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ReplicationComponent = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		m_EntityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		
		if (!m_EntityCatalogManager)
			return;
		
		SCR_VehicleFactionAffiliationComponent factionComponent = SCR_VehicleFactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_VehicleFactionAffiliationComponent));
		
		if (!factionComponent)
			return;
		
		m_Faction = SCR_Faction.Cast(factionComponent.GetDefaultAffiliatedFaction());
		
		if (!m_Faction)
			return;
		
		SCR_EntityCatalogEntry entry = m_EntityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.VEHICLE, GetOwner().GetPrefabData().GetPrefabName(), m_Faction);
		
		if (!entry)
			return;
		
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		
		if (!data)
			return;
		
		m_fResourceCost = data.GetSupplyCost();
	}
}
