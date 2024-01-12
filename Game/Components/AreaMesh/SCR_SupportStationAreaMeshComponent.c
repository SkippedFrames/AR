[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_SupportStationAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
};

class SCR_SupportStationAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	[Attribute(desc: "Set NONE to get first found support station. Else it will find the specific support station of the same type. It will always search itself, the parent, compartments and the compartments of the parents to find the SupportStation", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationType), category: "Support Station")]
	protected ESupportStationType m_eSearchForType;
	
	[Attribute("{6FD8A355A2011A46}Assets/Editor/VirtualArea/VirtualArea_SupportStation_Disabled.emat", desc: "Material mapped on outside and inside of the mesh when support station is disabled. Inside mapping is mirrored.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat", category: "Virtual Area")]
	protected ResourceName m_sDisabledMaterial;
	
	[Attribute("1", desc: "Used when mesh area is a child of the Support Station. This will make sure that the preview displayed when placing is the correct size", category: "Support Station")]
	protected float m_fRadius;

	//~ Save ref of support station after getting it
	protected SCR_BaseSupportStationComponent m_SupportStationComponent;
	
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		if (!m_SupportStationComponent)
			return m_fRadius;

		//~ Range is set to Pow2 in runtime so make sure to Sqrt it
		if (SCR_Global.IsEditMode())
			return m_SupportStationComponent.GetRange();
		else
			return Math.Sqrt(m_SupportStationComponent.GetRange());
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get support station component from owner, parent or siblings
	protected SCR_BaseSupportStationComponent GetSupportStation(IEntity owner)
	{
		array<Managed> supportStationComponents = {};
		owner.FindComponents(SCR_BaseSupportStationComponent, supportStationComponents);
		SCR_BaseSupportStationComponent supportStation = GetValidSupportStation(supportStationComponents);
		if (supportStation)
			return supportStation;
		
		IEntity parent = owner.GetParent();
		
		//~ Not found on owner so look for support station on parent
		if (parent)
		{
			supportStationComponents.Clear();
			parent.FindComponents(SCR_BaseSupportStationComponent, supportStationComponents);

			supportStation = GetValidSupportStation(supportStationComponents);
			if (supportStation)
			{
				if (SCR_Global.IsEditMode() && m_fRadius != supportStation.GetRange())
					Print("'SCR_SupportStationAreaMeshComponent' is on a child entity yet the m_fRadius (" + m_fRadius + ") is not equal to the support Station range (" + supportStation.GetRange() + ") make sure it is equal else the preview will not show correctly", LogLevel.ERROR);
				else if (!SCR_Global.IsEditMode() && m_fRadius !=  Math.Sqrt(supportStation.GetRange()))
					Print("'SCR_SupportStationAreaMeshComponent' is on a child entity yet the m_fRadius (" + m_fRadius + ") is not equal to the support Station range (" + Math.Sqrt(supportStation.GetRange()) + ") make sure it is equal else the preview will not show correctly", LogLevel.ERROR);
				
				return supportStation;
			}
				
			
			//~ Not found on parent so look for support station on children of parent
			IEntity child = parent.GetChildren();
			while (child != null && m_SupportStationComponent == null)
			{
				supportStationComponents.Clear();
				child.FindComponents(SCR_BaseSupportStationComponent, supportStationComponents);

				supportStation = GetValidSupportStation(supportStationComponents);
				if (supportStation)
				{
					if (SCR_Global.IsEditMode() && m_fRadius != supportStation.GetRange())
					Print("'SCR_SupportStationAreaMeshComponent' is on a child entity yet the m_fRadius (" + m_fRadius + ") is not equal to the support Station range (" + supportStation.GetRange() + ") make sure it is equal else the preview will not show correctly", LogLevel.ERROR);
				else if (!SCR_Global.IsEditMode() && m_fRadius != Math.Sqrt(supportStation.GetRange()))
					Print("'SCR_SupportStationAreaMeshComponent' is on a child entity yet the m_fRadius (" + m_fRadius + ") is not equal to the support Station range (" + Math.Sqrt(supportStation.GetRange()) + ") make sure it is equal else the preview will not show correctly", LogLevel.ERROR);

					return supportStation;
				}
					
				child = child.GetSibling();
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Checks if there are any valid support stations in the given array
	protected SCR_BaseSupportStationComponent GetValidSupportStation(notnull array<Managed> supportStationComponents)
	{
		if (supportStationComponents.IsEmpty())
			return null;

		SCR_BaseSupportStationComponent supportStation;

		foreach (Managed station : supportStationComponents)
		{
			supportStation = SCR_BaseSupportStationComponent.Cast(station);
			if (!supportStation)
				continue;

			if (m_eSearchForType == ESupportStationType.NONE || supportStation.GetSupportStationType() == m_eSearchForType)
			{
				return supportStation;
			}
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get material used for area mesh. Enabled states sets the correct mesh area
	protected override ResourceName GetMaterial()
	{
		if (m_SupportStationComponent && !m_SupportStationComponent.IsEnabled())
			return m_sDisabledMaterial;
		else 
			return super.GetMaterial();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Regenerates mesh area if on enabled changed as material changed
	protected void OnEnabledChanged(bool enabled)
	{
		GenerateAreaMesh();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!owner)
			return;
		
		m_SupportStationComponent = GetSupportStation(owner);
			
		//~ Did not find support station
		if (!m_SupportStationComponent)
		{
			//~ Give warning in workbench
			#ifdef WORKBENCH
			Print("'SCR_SupportStationAreaMeshComponent' (" + owner + ") Component must be attached to enity with SCR_BaseSupportStationComponent! BasePrefab should not have a support Station component", LogLevel.WARNING);
			return;
			#endif
				
			//~ Call one frame later to make sure that Support Station component is known to system
			GetGame().GetCallqueue().CallLater(Init, param1: owner);
			return;
		}
		
		//~ Call actual init if Support Station was found
		Init(owner);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void Init(IEntity owner)
	{
		if (!m_SupportStationComponent)
		{
			m_SupportStationComponent = GetSupportStation(owner);
			
			if (!m_SupportStationComponent)
			{
				Print("'SCR_SupportStationAreaMeshComponent' (" + owner + ") Component must be attached to enity with SCR_BaseSupportStationComponent! BasePrefab should not have a support Station component", LogLevel.WARNING);
				return;
			}
		}
		
		//~ Ignore if does not use range
		if (!m_SupportStationComponent.UsesRange())
			return;
		
		vector offset = m_SupportStationComponent.GetOffset();
		if (offset != vector.Zero)
		{
			if (m_SupportStationComponent.GetOwner() != owner)
			{
				vector localTransform[4];
				owner.GetLocalTransform(localTransform);
				
				if (localTransform[3] != offset)
					Print("'SCR_SupportStationAreaMeshComponent' (" + owner + ") linked support station (" + typename.EnumToString(ESupportStationType, m_SupportStationComponent.GetSupportStationType()) + ") has an offset but the local position of the area mesh is: '" + localTransform[3] + "' were as the support station offset is '" + offset + "' this must be the same else the zone shows an incorrect area!", LogLevel.ERROR);
			}
			else 
			{
				Print("'SCR_SupportStationAreaMeshComponent' (" + owner + ") linked support station (" + typename.EnumToString(ESupportStationType, m_SupportStationComponent.GetSupportStationType()) + ") has an offset but the support station is on the same entity as the mesh so no offset could be given. Make the mesh a child entity of the support station else the zone shows an incorrect area!", LogLevel.ERROR);
			}
		}
		
		//~ Only show when enabled. If disabled it will only start showing the zone if enabled in runtime (including disabled zone)
		if (m_SupportStationComponent.IsEnabled())
			GenerateAreaMesh();
		
		//~ Listen to when support station is enabled/disabled
		m_SupportStationComponent.GetOnEnabledChanged().Insert(OnEnabledChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		if (m_SupportStationComponent && m_SupportStationComponent.UsesRange())
			m_SupportStationComponent.GetOnEnabledChanged().Remove(OnEnabledChanged);
	}
};


