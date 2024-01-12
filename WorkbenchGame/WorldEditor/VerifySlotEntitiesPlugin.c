[WorkbenchPluginAttribute(name: "Verify Slot Entities", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf560)]
class VerifySlotEntitiesPlugin: WorkbenchPlugin
{
	override void Run()
	{
#ifdef WORKBENCH
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		WBProgressDialog progress = new WBProgressDialog("Processing world entities...", worldEditor);
		
		BaseContainer container;
		SCR_SiteSlotEntity entity;
		int n = 0;
		
		Print("Verify Slot Entities: Start", LogLevel.DEBUG);
		
		api.BeginEntityAction(Type().ToString());
		for (int i = 0, count = worldEditor.GetNumContainers(); i < count; i++)
		{
			container = worldEditor.GetContainer(i);
			if (container.GetClassName().ToType().IsInherited(SCR_SiteSlotEntity))
			{
				entity = SCR_SiteSlotEntity.Cast(api.SourceToEntity(container));
				entity._WB_SnapToTerrain(container);
				entity._WB_OrientToTerrain(container);
				n++;
			}
			progress.SetProgress(i / count);
		}
		api.EndEntityAction();
		
		Print(string.Format("Verify Slot Entities: End, %1 slots processed", n), LogLevel.DEBUG);
#endif
	}
};
