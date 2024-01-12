[ComponentEditorProps(category: "GameScripted/Camera", description: "")]
class SCR_BaseCameraComponentClass: ScriptComponentClass
{
};

/*!
Base component for SCR_CameraBase.
*/
class SCR_BaseCameraComponent : ScriptComponent
{
	void SCR_BaseCameraComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (SCR_CameraBase.Cast(ent) == null)
		{
			Print(string.Format("Component %1 must be attached to SCR_CameraBase!", Type()), LogLevel.ERROR);
			Deactivate(ent);
		}
	}
};
