[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "NotificationTestGeneratorEntity", dynamicBox: true, visible: false)]
class NotificationTestGeneratorEntityClass: SCR_GeneratorBaseEntityClass
{
};

class NotificationTestGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute("")]
	string Name;
	
	ref array<ref Shape> m_Shapes;
	
	void NotificationTestGeneratorEntity(IEntitySource src, IEntity parent)
	{
		m_Shapes = new ref array<ref Shape>();
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_Shapes = new ref array<ref Shape>();
	}
	
	#ifdef WORKBENCH
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		Print("Entity: " + Name + " intersected with " + mins.Count() + " boxes");
		m_Shapes = new ref array<ref Shape>();
		for (int i = 0; i < mins.Count(); ++i)
		{
			vector points[5];
			points[0] = mins[i];
			points[1][0] = maxes[i][0]; points[1][1] = 0; points[1][2] = mins[i][2];
			points[2] = maxes[i];
			points[3][0] = mins[i][0]; points[3][1] = 0; points[3][2] = maxes[i][2];
			points[4] = mins[i];
			m_Shapes.Insert(Shape.CreateLines(0xffdd55ff, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, points, 5));
		}
		
	}
	#endif
};
