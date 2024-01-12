[ComponentEditorProps(category: "GameScripted/FactionManager/Components", description: "Base for FactionManager scripted component.")]
class SCR_BaseFactionManagerComponentClass: ScriptComponentClass
{
};

/*!
Base class for components attached to SCR_FactionManager.
*/
class SCR_BaseFactionManagerComponent : ScriptComponent
{
	/*!
	Function called on every machine when factions are initialized.
	To be overloaded by inherited components.
	\param factions Available factions
	*/
	void OnFactionsInit(array<Faction> factions)
	{
	}
	
	void SCR_BaseFactionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!ent.Type().IsInherited(SCR_FactionManager))
		{
			Print(string.Format("SCR_BaseFactionManagerComponent must be on SCR_FactionManager, it's on %1!", ent.Type()), LogLevel.WARNING);
		}
	}
};
