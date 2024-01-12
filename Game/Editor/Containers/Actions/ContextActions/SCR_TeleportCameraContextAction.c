// Script File
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TeleportCameraContextAction : SCR_GeneralContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBePerformed(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (hoveredEntity)
		{
			vector pos;
			return hoveredEntity.GetPos(pos);
		}
		else
		{
			return cursorWorldPosition != vector.Zero;
		}
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (cameraManager)
		{
			SCR_ManualCamera camera = cameraManager.GetCamera();
			if (camera)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					if (hoveredEntity)
						hoveredEntity.GetPos(cursorWorldPosition);
					
					teleportComponent.TeleportCamera(cursorWorldPosition, true, false);
				}
			}
		}
	}
};
