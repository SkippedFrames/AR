[EntityEditorProps(category: "GameScripted/Components", description: "ScriptWizard generated script file.")]
class SCR_PlaceableItemComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlaceableItemComponent : ScriptComponent
{
	[Attribute(params: "xob")]
	protected ResourceName m_sPreviewObject;
	
	[Attribute("0.5084", desc: "Max placement distance in meters.")]
	protected float m_fMaxPlacementDistance;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EPlacementType))]
	protected SCR_EPlacementType m_ePlacementType;
	
	//------------------------------------------------------------------------------------------------
	SCR_EPlacementType GetPlacementType()
	{
		return m_ePlacementType;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxPlacementDistance()
	{
		return m_fMaxPlacementDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	VObject GetPreviewVobject()
	{
		if (m_sPreviewObject.IsEmpty())
			return GetOwner().GetVObject();
		
		Resource resource = Resource.Load(m_sPreviewObject);
		if (!resource.IsValid())
			return GetOwner().GetVObject();
		
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return GetOwner().GetVObject();
		
		return resourceObject.ToVObject();
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_EPlacementType
{
	XZ_FIXED,
	XYZ
}
