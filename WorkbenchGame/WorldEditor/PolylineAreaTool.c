[WorkbenchToolAttribute("Polyline Area Tool", "Turns polylines into polyline areas, select a polyline, then click 'Convert'", "", awesomeFontCode: 0xf5ee)]
class PolylineAreaTool: WorldEditorTool
{
	[Attribute( "5", UIWidgets.EditBox, "" )]
	float m_fRectangleWidth;
	
	[Attribute( "3", UIWidgets.EditBox, "" )]
	float m_fRectangleHeight;
	
	[Attribute( "3", UIWidgets.EditBox, "" )]
	float m_fCircleRadius;
	
	[Attribute( "16", UIWidgets.EditBox, "" )]
	float m_fCircleSegments;
	
	//---------------------------
	void ModifyPolyline(IEntitySource entSrc, array<vector> points)
	{
		BaseContainerList pts = entSrc.GetObjectArray("Points");
		
		while( pts.Count() > 0 )
		{
			m_API.RemoveObjectArrayVariableMember(entSrc, null, "Points", 0);
		}
		
		for(int i = 0; i < points.Count(); i++)
		{
			m_API.CreateObjectArrayVariableMember(entSrc, null, "Points", "ShapePoint", i);

			auto containerPath = new array<ref ContainerIdPathEntry>();
			auto entry = new ContainerIdPathEntry("Points", i);
			containerPath.Insert(entry);
			vector pointPos = points.Get(i);
			string posStr = "";
			posStr += pointPos[0].ToString();
			posStr += " ";
			posStr += pointPos[1].ToString();
			posStr += " ";
			posStr += pointPos[2].ToString();
			m_API.SetVariableValue(entSrc, containerPath, "Position", posStr);
		}
	}
	
	//---------------------------
	[ButtonAttribute("Turn to Rect")]
	void MakeRec()
	{
		//TODO: do once somewhere ?
		IEntity entity = m_API.GetSelectedEntity();
		if (!entity || !entity.IsInherited(ShapeEntity))
			return;

		vector lowerLeft, lowerRight, upperLeft, upperRight;
		
		lowerLeft[0] = 0 - m_fRectangleWidth / 2;
		lowerLeft[2] = 0 - m_fRectangleHeight / 2;
		
		lowerRight[0] = m_fRectangleWidth / 2;
		lowerRight[2] = 0 - m_fRectangleHeight / 2;
		
		upperLeft[0] = 0 - m_fRectangleWidth / 2;
		upperLeft[2] = m_fRectangleHeight / 2;
		
		upperRight[0] = m_fRectangleWidth / 2;
		upperRight[2] = m_fRectangleHeight / 2;
		
		auto positions = new array<vector>;
		positions.Clear();
		positions.Insert(lowerLeft);
		positions.Insert(lowerRight);
		positions.Insert(upperRight);
		positions.Insert(upperLeft);
		
		m_API.BeginEntityAction("","");
		IEntitySource entSrc = m_API.EntityToSource(entity);
		ModifyPolyline(entSrc, positions);
		m_API.EndEntityAction("");
	}
	
	//---------------------------
	[ButtonAttribute("Turn to Circle")]
	void MakeCircle()
	{
		//TODO: do once somewhere ?
		IEntity entity = m_API.GetSelectedEntity();
		if (!entity || !entity.IsInherited(ShapeEntity))
			return;
		auto positions = new array<vector>;
		positions.Clear();
		float segmentSize = Math.PI2 / m_fCircleSegments;
		
		for (float i = 0; i < Math.PI2; )
		{
			float x = Math.Cos(i) * m_fCircleRadius;
			float y = Math.Sin(i) * m_fCircleRadius;
			i += segmentSize;
			
			vector point;
			point[0] = x;
			point[1] = 0;
			point[2] = y;
			
			positions.Insert(point);
		}

		m_API.BeginEntityAction("","");
		IEntitySource entSrc = m_API.EntityToSource(entity);
		ModifyPolyline(entSrc, positions);
		m_API.EndEntityAction("");
	}
	
	
	[ButtonAttribute("Convert")]
	void Convert()
	{
		IEntity entity = m_API.GetSelectedEntity();
		if (!entity || !entity.IsInherited(ShapeEntity))
			return;

		IEntitySource src = m_API.EntityToSource(entity);

		m_API.BeginEntityAction("","");

		IEntity parentEnt;
		IEntitySource parentSrc = src.GetParent();
		if (parentSrc)
		{
			parentEnt = m_API.SourceToEntity(parentSrc);
		}
		else
		{
			parentEnt = m_API.CreateEntity("PolylineArea","", src.GetLayerID(), null, entity.GetOrigin(), "0 0 0");
			m_API.ParentEntity(parentEnt, entity, true);
		}
		
		int count = src.GetComponentCount();
		bool foundComp;
		for(int i = 0; i < count; i++)//workaround for nonfunctional FindComponent(Hierarchy)
		{
			IEntityComponentSource comp = src.GetComponent(i);

			if(comp.GetClassName() == "Hierarchy")
			{
				foundComp = true;
			}
		}
		
		if (!foundComp)
		{
			m_API.CreateComponent(src, "Hierarchy");
			
		}
		m_API.EndEntityAction("");
		
/*
		
		proto native IEntityComponentSource GetComponentAt(IEntitySource owner, int index);
		
		if (!entity.FindComponent(HierarchyComponent))
		{
			m_API.CreateComponent(src, "Hierarchy");
			m_API.EndEntityAction("");
		}*/
	}
};
