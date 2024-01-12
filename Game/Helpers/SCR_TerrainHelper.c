class SCR_TerrainHelper
{
	protected static const float MAX_TRACE_LENGTH = 100; //--- How far to search for geometry below given position

	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain height at given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return Position above sea
	*/
	static float GetTerrainY(vector pos, BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		if (!world)
			world = GetGame().GetWorld();
		if (!world)
			return 0;

		float surfaceY;
		if (trace)
		{
			trace.Start = pos;
			trace.End = Vector(pos[0], pos[1] - MAX_TRACE_LENGTH, pos[2]);
			
			if (trace.Flags == 0)
				trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			
			float traceCoef = world.TraceMove(trace, null);
			if (traceCoef < 1)
				surfaceY = trace.Start[1] - (trace.Start[1] - trace.End[1]) * traceCoef;
		}
		
		if (surfaceY == 0)
			surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		
		if (noUnderwater)
			surfaceY = Math.Max(surfaceY, 0);
		
		return surfaceY;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get height above terrain of given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return Position above terrain
	*/
	static float GetHeightAboveTerrain(vector pos, BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		return pos[1] - GetTerrainY(pos, world, noUnderwater, trace);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain normal vector on given position.
	\param[inout] pos World position, its vertical axis will be modified to be the surface height
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return Normal vector
	*/
	static vector GetTerrainNormal(inout vector pos, BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return vector.Zero;
		
		//--- Trace defined, use it to calculate intersection
		if (trace)
		{
			//--- Make sure that trace does not start underground
			pos[1] = Math.Max(pos[1], world.GetSurfaceY(pos[0], pos[2]) + 0.01);
			
			trace.Start = pos;
			trace.End = Vector(pos[0], pos[1] - MAX_TRACE_LENGTH, pos[2]);
			
			if (trace.Flags == 0)
				trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			
			float traceCoef = world.TraceMove(trace, null);
			if (traceCoef < 1)
			{
				pos[1] = trace.Start[1] - (trace.Start[1] - trace.End[1]) * traceCoef;
				if (noUnderwater && pos[1] < world.GetOceanBaseHeight())
				{
					//--- Underwater, use ocean surface normal (always up)
					pos[1] = Math.Max(pos[1], world.GetOceanBaseHeight());
					return vector.Up;
				}
				return trace.TraceNorm;
			}
			return vector.Up; //--- Default is up, not zero, as that could break calculations like vector.Dot
		}
		
		//--- Simplified calculation without custom trace
		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater && surfaceY < world.GetOceanBaseHeight())
		{
			pos[1] = Math.Max(surfaceY, world.GetOceanBaseHeight());
			return vector.Up;
		}
		
		//--- Get surface normal
		pos[1] = surfaceY;
		TraceParam traceRef = new TraceParam();
		traceRef.Start = pos + vector.Up;
		traceRef.End = pos - vector.Up;
		traceRef.Flags = TraceFlags.WORLD;
		world.TraceMove(traceRef, null);

		return traceRef.TraceNorm;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain basis vectors on given position.
	\param pos World position
	\param[out] result Matrix to be filled with basis vectors
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return True if the basis was defined successfully
	*/
	static bool GetTerrainBasis(vector pos, out vector result[4], BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		vector normal = GetTerrainNormal(pos, world, noUnderwater, trace);
		if (normal == vector.Zero)
			return false;

		//--- Get basis matrix
		vector perpend = normal.Perpend();
		Math3D.DirectionAndUpMatrix(perpend, normal, result);

		//--- Rotate the matrix to always point North
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(-perpend.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(result, basis, result);

		//--- Set terrain position
		result[3] = pos;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Snap transformation to terrain position.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return True if the operation was performed successfully
	*/
	static bool SnapToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater, trace))
			return false;

		//--- Set position to surface
		transform[3] = surfaceBasis[3];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Orient transformation to terrain normal.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return True if the operation was performed successfully
	*/
	static bool OrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater, trace))
			return false;

		//--- Reset pitch and roll, but preserve yaw
		//vector angles = Math3D.MatrixToAngles(transform);
		//Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);

		//--- Combine surface and entity transformations
		Math3D.MatrixMultiply3(surfaceBasis, transform, transform);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Snap transformation to terrain position and orient it to terrain normal.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\trace When defined, use this trace to check surface intersection (useful for setting custom trace flags or ignored entities)
	\return True if the operation was performed successfully
	*/
	static bool SnapAndOrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false, TraceParam trace = null)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater, trace))
			return false;

		//--- Set position to surface
		transform[3] = surfaceBasis[3];

		//--- Reset pitch and roll, but preserve yaw
		//vector angles = Math3D.MatrixToAngles(transform);
		//Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);

		//--- Combine surface and entity transformations
		Math3D.MatrixMultiply3(surfaceBasis, transform, transform);

		return true;
	}
};
