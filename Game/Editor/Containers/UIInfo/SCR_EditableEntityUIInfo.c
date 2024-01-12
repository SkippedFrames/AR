[BaseContainerProps()]
class SCR_EditableEntityUIInfo: SCR_UIInfo
{
	[Attribute(params: "edds imageset", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	private ResourceName m_Image;
	
	[Attribute()]
	private FactionKey m_sFaction;
	
	[Attribute("1", UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), desc: "Entity labels configured by developers, changes will be applied when updating editable entities")]
	private ref array<EEditableEntityLabel> m_aAuthoredLabels;
	
	[Attribute("1", UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), desc: "Autoconfigured labels, for manual changes edit AuthoredLabels array instead")]
	private ref array<EEditableEntityLabel> m_aAutoLabels;
	
	[Attribute("", category: "Editable Entity", desc: "Define budget costs")]
	private ref array<ref SCR_EntityBudgetValue> m_EntityBudgetCost;
	
	[Attribute("", category: "Editable Entity", desc: "Autoconfigured budget costs of child entities (group/composition entities)")]
	private ref array<ref SCR_EntityBudgetValue> m_EntityChildrenBudgetCost;
	
	[Attribute(category: "Editable Entity", params: "et", desc: "Prefab this entity extends.")]
	protected ResourceName m_SlotPrefab;
	
	//--- Hidden vars, used only in content browser where they're filled from component source
	protected EEditableEntityType m_EntityType;
	protected EEditableEntityFlag m_EntityFlags;
	
	ResourceName GetImage()
	{
		return m_Image;
	}
	EEditableEntityType GetEntityTypex()
	{
		return 0;
	}
	FactionKey GetFactionKey()
	{
		return m_sFaction;
	}
	Faction GetFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			return factionManager.GetFactionByKey(m_sFaction);
		else
			return null;
	}
	int GetEntityLabels(out notnull array<EEditableEntityLabel> entityLabels)
	{
		entityLabels.InsertAll(m_aAutoLabels);
		entityLabels.InsertAll(m_aAuthoredLabels);
		return entityLabels.Count();
	}
	
	/*!
	Check if entity has label in either Auto or Authored labels
	\param label Label to search
	\return true if entity has the label
	*/
	bool HasEntityLabel(EEditableEntityLabel label)
	{
		if (m_aAutoLabels.Contains(label))
			return true;
		if (m_aAuthoredLabels.Contains(label))
			return true;
		
		return false;
	}
		
	/*!
	Get Entity budget costs
	\return false if budget cost should be determined by entitytype, or return true with an empty cost array to avoid any budget cost
	*/
	bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets)
	{
		SCR_EntityBudgetValue.MergeBudgetCosts(outBudgets, m_EntityBudgetCost);
		return !outBudgets.IsEmpty();
	}
	/*!
	Get only Entity's children budget costs, i.e. cost of entities inside a composition entitiy
	*/
	void GetEntityChildrenBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets)
	{
		SCR_EntityBudgetValue.MergeBudgetCosts(outBudgets, m_EntityChildrenBudgetCost);
	}
	
	/*!
	Get prefab this entity prefab extends.
	\return Type
	*/
	ResourceName GetSlotPrefab()
	{
		return m_SlotPrefab;
	}
	
	/*!
	Get entity type.
	\return Type
	*/
	EEditableEntityType GetEntityType()
	{
		return m_EntityType;
	}
	/*!
	Check if a flag is active.
	\param flag
	\return True if the flag is active
	*/
	bool HasEntityFlag(EEditableEntityFlag flag)
	{
		return (m_EntityFlags & flag) == flag;
	}
	
	/*!
	Set asset image to given image widget.
	Use this function instead of retrieving the texture using GetIconPath() and setting it manually!
	When the texture is an image set, manual setting would not work.
	\param imageWidget Target image widget
	\return True when the image was set
	*/
	bool SetAssetImageTo(ImageWidget imageWidget)
	{
		if (!imageWidget || m_Image.IsEmpty())
			return false;
		
		imageWidget.LoadImageTexture(0, m_Image);
		return true;
	}
	
	/*!
	Initialize UI info from component source
	\param componentSource
	*/
	void InitFromSource(IEntityComponentSource componentSource)
	{
		m_EntityType = SCR_EditableEntityComponentClass.GetEntityType(componentSource);
		m_EntityFlags = SCR_EditableEntityComponentClass.GetEntityFlags(componentSource);
	}
	
	protected override void CopyFrom(SCR_UIName source)
	{
		SCR_EditableEntityUIInfo editableEntitySource = SCR_EditableEntityUIInfo.Cast(source);
		if (editableEntitySource)
		{
			m_Image = editableEntitySource.m_Image;
			m_sFaction = editableEntitySource.m_sFaction;
			m_aAuthoredLabels = editableEntitySource.m_aAuthoredLabels;
			m_aAutoLabels = editableEntitySource.m_aAutoLabels;
			m_EntityBudgetCost = editableEntitySource.m_EntityBudgetCost;
			m_EntityChildrenBudgetCost = editableEntitySource.m_EntityChildrenBudgetCost;
			m_SlotPrefab = editableEntitySource.m_SlotPrefab;
		}
		
		super.CopyFrom(source);
	}
};
