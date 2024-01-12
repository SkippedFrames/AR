[WorkbenchPluginAttribute(name: "Snap and Orient Entities to Terrain", shortcut: "Ctrl+Page Down", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF2D1)]
class SnapAndOrientToTerrainPlugin : WorkbenchPlugin
{
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		BaseWorld world = api.GetWorld();
		if (!world)
			return;

		api.BeginEntityAction();
		IEntity entity;
		IEntity parent;
		vector transform[4], parentTransform[4];
		vector angles, pos;
		IEntitySource entitySource;
		EntityFlags flags;
		for (int i, count = api.GetSelectedEntitiesCount(); i < count; i++)
		{
			entity = api.GetSelectedEntity(i);
			entity.GetWorldTransform(transform);
			if (SCR_TerrainHelper.SnapAndOrientToTerrain(transform, world))
			{
				parent = api.SourceToEntity(api.EntityToSource(entity).GetParent());
				if (parent)
				{
					parent.GetWorldTransform(parentTransform);
					Math3D.MatrixInvMultiply4(parentTransform, transform, transform);
				}

				angles = Math3D.MatrixToAngles(transform);
				api.ModifyEntityKey(entity, "angleX", angles[1].ToString());
				api.ModifyEntityKey(entity, "angleY", angles[0].ToString());
				api.ModifyEntityKey(entity, "angleZ", angles[2].ToString());

				pos = transform[3];
				//--- Y height relative to ground, use 0 instead of ASL height
				entitySource = api.EntityToSource(entity);
				entitySource.Get("Flags", flags);
				if (flags & EntityFlags.RELATIVE_Y)
					pos[1] = 0;

				api.ModifyEntityKey(entity, "coords", string.Format("%1 %2 %3", pos[0], pos[1], pos[2]));
			}
		}
		api.EndEntityAction();
	}
};
