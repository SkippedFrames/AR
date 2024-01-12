[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableEntityComponentClass: ScriptComponentClass
{
	[Attribute("0", UIWidgets.ComboBox, category: "Editable Entity", desc: "System type of the entity.", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_EntityType;
	
	[Attribute(category: "Visualization", desc: "GUI representation")]
	protected ref SCR_UIInfo m_UIInfo;
	
	[Attribute("", UIWidgets.Auto, category: "Visualization", desc: "Bone to which the icon is attached to")]
	protected string m_sIconBoneName;
	
	[Attribute(category: "Editable Entity")]
	protected ref SCR_EditableEntityInteraction m_EntityInteraction;
	
	/*!
	Get entity type.
	\return Type
	*/
	EEditableEntityType GetEntityType()
	{
		return m_EntityType;
	}
	/*!
	Get information about the entity. When none exist, create a dummy one.
	\return Info class
	*/
	SCR_UIInfo GetInfo()
	{
		return m_UIInfo;
	}
	/*!
	Get bone name on which entity icon will be rendered.
	\return Bone name
	*/
	string GetIconBoneName()
	{
		return m_sIconBoneName;
	}
	/*!
	Get entity interaction rules of this entity. If it doesn't contain any custom rules, those for its type will be used.
	\return Interaction rules
	*/
	SCR_EditableEntityInteraction GetEntityInteraction()
	{
		if (m_EntityInteraction)
			return m_EntityInteraction;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			return core.GetEntityInteraction(GetEntityType());
		else
			return null;
	}
	
	/*!
	Get component source from prefab resource.
	\param prefab Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	\return Component source
	*/
	static IEntityComponentSource GetEditableEntitySource(Resource entityResource)
	{
		if (!entityResource) return null;
		
		BaseResourceObject entityBase = entityResource.GetResource();
		if (!entityBase) return null;
			
		IEntitySource entitySource = entityBase.ToEntitySource();
		if (!entitySource) return null;
		
		return GetEditableEntitySource(entitySource);
	}
	/*!
	Get component source from entity source.
	\param entitySource Entity source
	\return Component source
	*/
	static IEntityComponentSource GetEditableEntitySource(IEntitySource entitySource)
	{
		if (!entitySource) return null;
		
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(SCR_EditableEntityComponent))
				return componentSource;
		}
		return null;
	}
	/*!
	Get UI info from SCR_EditableEntityComponent source
	\param componentSource Component source
	\return UI info
	*/
	static SCR_EditableEntityUIInfo GetInfo(IEntityComponentSource componentSource)
	{		
		BaseContainer infoSource = componentSource.GetObject("m_UIInfo");
		if (!infoSource) return null;
		
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(infoSource));
		info.InitFromSource(componentSource);
		return info;
	}
	/*!
	Get entity system type
	\param componentSource Component source
	\return Type
	*/
	static EEditableEntityType GetEntityType(IEntityComponentSource componentSource)
	{
		EEditableEntityType type;
		componentSource.Get("m_EntityType", type);
		return type;
	}
	/*!
	Get entity interaction rules of this entity prefab. If it doesn't contain any custom rules, those for its type will be used.
	\param componentSource Component source
	\return Interaction rules
	*/
	static SCR_EditableEntityInteraction GetEntityInteraction(IEntityComponentSource componentSource)
	{
		BaseContainer container = componentSource.GetObject("m_EntityInteraction");
		if (container)
			return SCR_EditableEntityInteraction.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			return core.GetEntityInteraction(GetEntityType(componentSource));
		else
			return null;
	}
	/*!
	Get slot prefab from SCR_EditableEntityComponent source
	\param componentSource Component source
	\return Slot prefab
	*/
	static ResourceName GetSlotPrefab(IEntityComponentSource componentSource)
	{
		BaseContainer info = componentSource.GetObject("m_UIInfo");
		if (!info) return ResourceName.Empty;
		
		ResourceName slotPrefab;
		info.Get("m_SlotPrefab", slotPrefab);
		return slotPrefab;
	}
	/*!
	Get entity flags
	\param componentSource Component source
	\return Flags
	*/
	static EEditableEntityFlag GetEntityFlags(IEntityComponentSource componentSource)
	{
		EEditableEntityFlag flags;
		componentSource.Get("m_Flags", flags);
		return flags;
	}
	/*!
	Check if the SCR_EditableEntityComponent source has given flag
	\param componentSource Component source
	\param flag Queried flag
	\returnTrue if it has the flag
	*/
	static bool HasFlag(IEntityComponentSource componentSource, EEditableEntityFlag flag)
	{
		EEditableEntityFlag flags;
		componentSource.Get("m_Flags", flags);
		return flags & flag;
	}
	
	/*!
	Gets entity budget cost values from EditableEntity component source
	\param editableEntitySource Component source of the SCR_EditableEntityComponent
	\param budgetValues Output array filled with budget cost values
	\return True if the component source is valid and component source has budget costs defined 
	*/
	static bool GetEntitySourceBudgetCost(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		if (!editableEntitySource) return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (editableEntityUIInfo)
		{
			return editableEntityUIInfo.GetEntityBudgetCost(budgetValues);
		}
		
		return !budgetValues.IsEmpty();
	}
	
	/*!
	Gets entity *+ entity children* budget cost values from EditableEntity component source
	\param editableEntitySource Component source of the SCR_EditableEntityComponent
	\param budgetValues Output array filled with budget cost values
	\return True if the component source is valid and component source has budget costs defined 
	*/
	static bool GetEntitySourceChildrenBudgetCosts(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		if (!editableEntitySource) return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (editableEntityUIInfo)
		{
			editableEntityUIInfo.GetEntityChildrenBudgetCost(budgetValues);
		}
		
		return !budgetValues.IsEmpty();
	}
};
