[EntityEditorProps()]
class SCR_PIPCameraClass : CameraBaseClass
{

};

class SCR_PIPCamera : CameraBase
{

	void SCR_PIPCamera(IEntitySource src, IEntity parent)
	{
		/// Ooofgly hack.
		/// We want to prevent auto-registration.
		/// TODO@AS: Proper camera manager support for "overlay" camera
		ClearEventMask(EntityEvent.INIT);
	}
	
	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		
		// CameraBase is activate by default, we don't want that here because UpdatePIPCamera is called from sight POSTFRAME event
		Deactivate();
	}
	
	/// Apply camera base props to camera in world
	void ApplyProps(int index)
	{
		BaseWorld world = GetWorld();
		world.SetCameraVerticalFOV(index, GetVerticalFOV());
		world.SetCameraFarPlane(index, GetFarPlane());
		world.SetCameraNearPlane(index, GetNearPlane());
		world.SetCameraType(index, CameraType.PERSPECTIVE);
		UpdatePIPCamera(0.0);
	}

	/// Apply entity transformation to camera in world
	void UpdatePIPCamera(float timeSlice)
	{
		ApplyTransform(timeSlice);
		//vector camMat[4];
		//GetWorldTransform(camMat);
		//GetWorld().SetCameraEx(index, camMat);
	}
};
