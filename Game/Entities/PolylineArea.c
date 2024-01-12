[EntityEditorProps(category: "GameLib/Scripted", description: "Script model", dynamicBox: true)]
class PolylineAreaClass: GenericEntityClass
{

};

class PolylineArea: GenericEntity
{
	[Attribute( "0", UIWidgets.CheckBox, "" )]
	bool m_bDrawDebug;
	
	//[Attribute( "", UIWidgets.None, "" )]
	ref array<float> m_Points;
	
	private ref array<float> m_PointsWorld = new array<float>;
	private ref array<vector> m_PositionsWorld = new array<vector>;
	private vector m_PosWorldDbg[256];
	private vector m_BBoxDbg[5];
	
	private bool m_bIsInsideDebug;
	
	//bbox in world space
	private float m_fBBoxMinX;
	private float m_fBBoxMaxX;
	private float m_fBBoxMinY;
	private float m_fBBoxMaxY;
	
	//private float m_f
	
	
	//-------------------------------------------------------------------------------
	//TODO: MOVE TO MATH LIB
	//!Checks if a point is within a 2d axis alligned bounding box (AABB)
	//!'minX','minY' are its bottom-left BBox coordinates, maxX,maxY its upper-right BBox coordinates, (x,y) the point's coordinates.
	private bool IsPointInsideBBox(float minX, float maxY, float maxX, float minY,float  x,float y)
	{
		return(x >= minX && x <= maxX && y >= minY && y <= maxY);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Performs a check on a given 'entity' and returns 'true' when it's inside the area, 'false' otherwise
	bool IsEntityInside(IEntity entity)
	{
		vector entPos = entity.GetOrigin();
		if ( !m_Points || !IsPointInsideBBox(m_fBBoxMinX, m_fBBoxMaxY, m_fBBoxMaxX, m_fBBoxMinY, entPos[0], entPos[2] ) )
		{
			//Print("out");
			return false;
		}
		//Print("in");
		return Math2D.IsPointInPolygon(m_PointsWorld, entPos[0], entPos[2]);
	}
	//------------------------------------------------------------------------------------------------
	//! Performs a check on a given map position 'pos' and returns 'true' when it's inside the area, 'false' otherwise
	bool IsPosInside(vector pos)
	{
		if ( !m_Points || !IsPointInsideBBox(m_fBBoxMinX, m_fBBoxMaxY, m_fBBoxMaxX, m_fBBoxMinY, pos[0], pos[2] ) )
			return false;
		
		return Math2D.IsPointInPolygon(m_PointsWorld, pos[0], pos[2]);
	}

	//------------------------------------------------------------------------------------------------
	//! For Debug purposes only, needs to have EntityEvent.FRAME enabled
	override void EOnFrame(IEntity owner, float timeSlice)
	{	
		if (!m_bDrawDebug)
			return;

		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		vector plrMat[4];
		player.GetTransform(plrMat);
		int color = ARGB(255, 0, 255, 0);
		m_bIsInsideDebug = IsEntityInside(player);
		if(m_bIsInsideDebug) 
			color = ARGB(255, 255, 0, 0);
		Shape.CreateSphere(color, ShapeFlags.WIREFRAME|ShapeFlags.NOZBUFFER|ShapeFlags.ONCE, plrMat[3], 0.1);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!LoadPolyline())
			return;
		vector matWrld[4];
		GetWorldTransform(matWrld);
		
		//transform local pos to world pos
		for(int i = 0; i < m_Points.Count();)
		{
			vector point;
			point[0] = m_Points[i];
			i++;
			point[2] = m_Points[i];
			i++;
			
			m_PositionsWorld.Insert(point.Multiply4(matWrld));
		}
		
		//set a default value to bbox coresponding with an existing point(default can't be a plain 0)
		m_fBBoxMinX = m_PositionsWorld[0][0];
		m_fBBoxMaxX = m_PositionsWorld[0][0];
		m_fBBoxMinY = m_PositionsWorld[0][2];
		m_fBBoxMaxY = m_PositionsWorld[0][2];
		// convert array of vectors into array of points, build debug lines and calculate bbox
		foreach (int i, vector pos: m_PositionsWorld)
		{
			m_PosWorldDbg[i] = pos;
			m_PointsWorld.Insert(pos[0]);
			m_PointsWorld.Insert(pos[2]);
			
			// establish BBox
			if (pos[0] < m_fBBoxMinX)
			{
				m_fBBoxMinX = pos[0];
			}
			if (pos[0] > m_fBBoxMaxX)
			{
				m_fBBoxMaxX = pos[0];
			}
			if (pos[2] < m_fBBoxMinY)
			{
				m_fBBoxMinY = pos[2];
			}
			if (pos[2] > m_fBBoxMaxY)
			{
				m_fBBoxMaxY = pos[2];
			}
			//BBox shape
			if (i == m_PositionsWorld.Count() - 1)
			{
				m_PosWorldDbg[i+1] = m_PosWorldDbg[0];
			}
		}
		
		// construct debug bbox
		vector pointLowerLeft, pointLowerRight, pointUpperLeft, pointUpperRight;
		float elevation = matWrld[3][1];
		pointLowerLeft[0] = m_fBBoxMinX;
		pointLowerLeft[1] = elevation;
		pointLowerLeft[2] = m_fBBoxMinY;
		
		pointLowerRight[0] = m_fBBoxMaxX;
		pointLowerRight[1] = elevation;
		pointLowerRight[2] = m_fBBoxMinY;
		
		pointUpperLeft[0] = m_fBBoxMinX;
		pointUpperLeft[1] = elevation;
		pointUpperLeft[2] = m_fBBoxMaxY;
		
		pointUpperRight[0] = m_fBBoxMaxX;
		pointUpperRight[1] = elevation;
		pointUpperRight[2] = m_fBBoxMaxY;
		
		m_BBoxDbg[0] = pointLowerLeft;
		m_BBoxDbg[1] = pointLowerRight;
		m_BBoxDbg[2] = pointUpperRight;
		m_BBoxDbg[3] = pointUpperLeft;
		m_BBoxDbg[4] = pointLowerLeft;
		//-----------
		
		
		#ifdef WORKBENCH
		if (m_bDrawDebug)
		{
			Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, m_PosWorldDbg, m_PositionsWorld.Count()+1);
			Shape.CreateLines(ARGB(255, 0, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, m_BBoxDbg, 5);
		}
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load Polyline from child
	bool LoadPolyline()
	{
		ShapeEntity child = ShapeEntity.Cast(GetChildren());
		if (!child)
			return false;
		
		m_Points = new array<float>;
		auto points = new array<vector>;
		child.GetPointsPositions(points);
		
		for (int i = 0; i < points.Count(); i++)
		{
			vector pos = points.Get(i);
			
			float posX = pos[0];
			float posY = pos[2];
			
			m_Points.Insert(posX);
			m_Points.Insert(posY);
		}
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void PolylineArea(IEntitySource src, IEntity parent)
	{
		if(!GetGame().GetWorldEntity())
			return;

		SetFlags(EntityFlags.TRACEABLE, false);
		SetEventMask(EntityEvent.INIT);
		#ifdef WORKBENCH
		SetEventMask(EntityEvent.FRAME);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void ~PolylineArea()
	{
		
	}
	
	
};
