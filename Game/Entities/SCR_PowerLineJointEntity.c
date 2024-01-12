[EntityEditorProps(category: "GameScripted/Spectating", description: "Spectator camera, controlled by arrows and numpad keys", color: "0 0 255 255")]
class SCR_PowerLineJointEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PowerLineJointEntity: GenericEntity
{
	[Attribute("1.0")]
	protected float m_fSagInMeters;
	
	[Attribute("0.1")]
	protected float m_AngleRad;
	
	[Attribute("0.5")]
	protected float m_MovementSpeed;
	
	#ifdef WORKBENCH
	protected static const int SEGMENTS = 64;
	protected const int POINTS = SEGMENTS + 1;

	protected vector m_DebugLine[POINTS];
	protected vector m_DebugLinePivots[POINTS];
	protected vector m_vDirectionLevelNorm;
	protected vector m_vConnectedJointPos;
	protected ref array<vector> m_Curve = {};
	protected ref array<vector> m_CurvePivots = {};
	protected ref array<ref Shape> m_aShapes = {};
	
	private float m_fTimeAccu;
	
	
	//------------------------------------------------------------------------------------------------
	bool GetJoint()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource entSrc = api.EntityToSource(this);
		int childCount = entSrc.GetNumChildren();
		
		if (childCount != 0)
		{
			for (int i = 0; i < childCount; i++)
			{
				IEntitySource childSrc = entSrc.GetChild(i);
				IEntity child = api.SourceToEntity(childSrc);
				m_vConnectedJointPos = child.GetOrigin();
				//Print(m_vConnectedJointPos);
			}
			return true;
		}
		else
		{
			IEntitySource parent = entSrc.GetParent();
			SCR_PowerLineJointEntity parentEnt = SCR_PowerLineJointEntity.Cast(api.SourceToEntity(parent));
			//parentEnt.OnChildMoved();
			return false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnChildMoved()
	{
		//DrawDebug();
	}
	
	//------------------------------------------------------------------------------------------------
	void GenerateCurve()
	{
		m_Curve.Clear();
		m_CurvePivots.Clear();
		if(m_fSagInMeters <= 0)
			m_fSagInMeters = 0.0001;
		vector posThis = GetOrigin();
		//m_vConnectedJointPos = posThis + "50 10 0";//remove
		
		vector jointPos1, jointPos2;
		if(posThis[1] <= m_vConnectedJointPos[1])
		{
			jointPos1 = posThis;
			jointPos2 = m_vConnectedJointPos;
		}
		else
		{
			jointPos1 = m_vConnectedJointPos;
			jointPos2 = posThis;
		}
		 
		m_vDirectionLevelNorm = jointPos2 - jointPos1;
		m_vDirectionLevelNorm[1] = 0;
		m_vDirectionLevelNorm.Normalize();
		
		float heightDifferenceM = jointPos2[1] - jointPos1[1];
		
		vector jointPos1Adj = jointPos1;//this pos adjusted to height of the connected joint
		jointPos1Adj[1] = jointPos2[1];
		float distanceLevel = vector.Distance(jointPos1Adj, jointPos2);
		
		float s1 = m_fSagInMeters / distanceLevel;//step 1
		
		float c = (0.5 * 0.5) / s1;//step 2
		
		float s2 = (heightDifferenceM / distanceLevel) + s1;//step 3
		
		float x2 = Math.Sqrt(s2 * c);// step 4
		
		float xc = 0.5 + x2; //step 5
		
		float x1f = (0.5 / xc) * -1;//step 6
		float x2f = x2 / xc;//step 6
		
		
		float segmentSize = distanceLevel / SEGMENTS;
		
		int segment = 0;
		array<float> curveYValues = {};
		
		float step = 1 / SEGMENTS;
		float value = 0;
		
		for(int i = 0; i < POINTS;i++)
		{
			//Print(i);
			float x = Math.Lerp(x1f, x2f, value) * xc;
			value += step;
			//Print(value);
			//Print(x);
			float y = (x*x) / c;
			//Print(y);
			curveYValues.Insert(y);
		}

		
		for(int i = 0; i < POINTS; i++)
		{
			float curveY = curveYValues.Get(i);
			float y = curveY * distanceLevel;
			y -= m_fSagInMeters;
			
			
			vector pos = m_vDirectionLevelNorm * (segmentSize * i);
			pos[1] = pos[1] + y;
			pos = pos + jointPos1;
			m_Curve.Insert(pos);
			//Print(pos);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void GetPivots()
	{
		vector firstPoint = m_Curve[0];
		vector lastPoint = m_Curve[SEGMENTS];
		
		for(int i = 0; i <= POINTS; i++)
		{
			vector pivot = vector.Lerp(firstPoint,lastPoint,(float)i/SEGMENTS);
			//Print((float)i/POINTS);
			m_CurvePivots.Insert(pivot);
			//Print(pos);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawDebugShape()
	{
		for(int i = 0; i < POINTS; i++)
		{
			m_DebugLine[i] = m_Curve[i];
			//Print(m_DebugLine[i]);
		}
		//m_aShapes.Insert(Shape.CreateLines(ARGB(255, 255, 255, 0), 0, m_DebugLine, POINTS));
		Shape.CreateLines(ARGB(255, 5, 5, 5), ShapeFlags.ONCE, m_DebugLine, POINTS);
		//m_aShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_vConnectedJointPos, 0.1));
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawDebugShapePivots()
	{
		for(int i = 0; i < POINTS; i++)
		{
			m_DebugLinePivots[i] = m_CurvePivots[i];
		}
		m_aShapes.Insert(Shape.CreateLines(ARGB(123, 0, 0, 0), 0, m_DebugLinePivots, POINTS));
		//m_aShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_vConnectedJointPos, 0.1));
	}
	
	//------------------------------------------------------------------------------------------------
	void ModifyShape(float timeSlice)
	{
		float speed = m_MovementSpeed;
		m_fTimeAccu += speed * timeSlice;
		//Print(m_fTimeAccu);
		float val1 = Math.Sin(m_fTimeAccu) * 0.5 + 0.5;
		float val2 = Math.Lerp(-m_AngleRad, m_AngleRad, val1);
		
		float sin = Math.Sin(val2);
		float cos = Math.Cos(val2);
		
		//rotation matrix start
		vector rotMat[4];
		rotMat[0][0] = cos;
		rotMat[0][1] = sin;
		rotMat[0][2] = 0;
		
		rotMat[1][0] = -sin;
		rotMat[1][1] = cos;
		rotMat[1][2] = 0;
		
		rotMat[2][0] = 0;
		rotMat[2][1] = 0;
		rotMat[2][2] = 1;		
		
		rotMat[3][0] = 0;
		rotMat[3][1] = 0;
		rotMat[3][2] = 0;
		//rotation matrix end
		
		vector rightDir = m_vDirectionLevelNorm * "0 1 0";
		
		vector pointMat[4];
		pointMat[0] = rightDir.Normalized();
		pointMat[1] = "0 -1 0";
		pointMat[2] = m_vDirectionLevelNorm;
		//Print(pointMat);
		vector resultMat[4];
		Math3D.MatrixMultiply4(pointMat, rotMat, resultMat);
		//Print(resultMat);
		for(int i = 0; i < POINTS; i++)
		{
			float length = vector.Distance(m_Curve[i], m_CurvePivots[i]);
			vector v = resultMat[1] * length + m_CurvePivots[i];
			//Print(v);
			m_Curve[i] = v;
		}
		
		
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawDebug(float timeSlice)
	{
		if(m_vConnectedJointPos != "0 0 0")
		{
			ModifyShape(timeSlice);
			DrawDebugShape();
		}

	}
	
	//------------------------------------------------------------------------------------------------
	
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_fSagInMeters")
		{
			src.Get(key, m_fSagInMeters);
		}	
		
		if (key == "m_AngleRad")
		{
			src.Get(key, m_AngleRad);
		}
		
		if (key == "m_MovementSpeed")
		{
			src.Get(key, m_MovementSpeed);
		}
		
		if( GetJoint() )
		{
			if(m_vConnectedJointPos != "0 0 0")
			{
				m_aShapes.Clear();
				GenerateCurve();
				GetPivots();
				//DrawDebugShapePivots();
			}
		}
		m_MovementSpeed = Math.RandomFloat(m_MovementSpeed * 0.9, m_MovementSpeed * 1.1);
		m_fTimeAccu = Math.RandomFloat(0, 100);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if(m_vConnectedJointPos == "0 0 0")
		{
			if( GetJoint() )
			{
				if(m_vConnectedJointPos != "0 0 0")
				{
					m_aShapes.Clear();
					GenerateCurve();
					GetPivots();
					//DrawDebugShapePivots();
				}
			}
		}
		
		DrawDebug(timeSlice);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_PowerLineJointEntity(IEntitySource src, IEntity parent)
	{
		m_fTimeAccu = Math.RandomFloat(0, 100);
		m_MovementSpeed = Math.RandomFloat(m_MovementSpeed * 0.9, m_MovementSpeed * 1.1);
		//Print(m_AngleRad);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PowerLineJointEntity()
	{
		m_aShapes.Clear();
	}
	#endif
};
