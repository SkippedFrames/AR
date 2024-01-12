//------------------------------------------------------------------------------------------------
class SCR_MoveInCircleComponentClass: ScriptComponentClass
{
};

class SCR_MoveInCircleComponent : ScriptComponent
{
	protected ref TVectorArray m_CirclePoints = {};
	protected float m_fElapsedTime = 0;
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		super.EOnFixedFrame(owner, timeSlice);
		
		m_fElapsedTime += timeSlice / 10;
		if(m_fElapsedTime >= 1)
			m_fElapsedTime -= 1;
		
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		
		mat[3] = Math3D.Curve(ECurveType.NaturalCubic, m_fElapsedTime, m_CirclePoints);
		Math3D.AnglesToMatrix(Vector(Math.Lerp(0, 360, m_fElapsedTime), 0.0, 0.0), mat);
		owner.SetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FIXEDFRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		vector entityPos = owner.GetOrigin();
		for (float i = 0; i < Math.PI2; )
		{
			float x = Math.Cos(i) * 10;
			float y = Math.Sin(i) * 10;
			i = i + 0.03;
			m_CirclePoints.Insert(Vector(entityPos[0] + x, entityPos[1], entityPos[2] + y));
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MoveInCircleComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MoveInCircleComponent()
	{
	}

};
