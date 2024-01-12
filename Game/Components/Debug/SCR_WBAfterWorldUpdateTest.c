[ComponentEditorProps(category: "GameScripted/Debug", description: "debug")]
class SCR_WBAfterWorldUpdateTestClass: ScriptComponentClass
{
};

class SCR_WBAfterWorldUpdateTest : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	void SCR_WBAfterWorldUpdateTest(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		ent.SetFlags(EntityFlags.ACTIVE, false);
	}
	
	#ifdef WORKBENCH
		//------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
		{
			if (!owner)
				Print("Owner is NULL!!!");
			else
				Print(owner);
		}
	#endif
};
