//! This class can add multiple measurements - useful to measure specific parts of a loop
//! \code
//! SCR_TimeMeasurementHelper benchmark = new SCR_TimeMeasurementHelper();
//! Debug.BeginTimeMeasure();
//! foreach (SCR_Object obj : m_aObjects)
//! {
//! 	obj.OperationA();
//! 	benchmark.BeginMeasure("opB");
//! 	obj.OperationB();
//! 	benchmark.EndMeasure("opB"); // names must match
//! 	obj.OperationC();
//! }
//! Debug.EndTimeMeasure("Operation A, B and C total benchmark");
//! Print("Operation B only: " + benchmark.GetMeasure("opB") + " ms", LogLevel.NORMAL);
//! \code
class SCR_TimeMeasurementHelper
{
	protected ref map<string, ref SCR_TimeMeasurementHelper_Info> m_mData;

	//------------------------------------------------------------------------------------------------
	void BeginMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			info = new SCR_TimeMeasurementHelper_Info();
			m_mData.Insert(name, info);
		}

		info.m_fStart = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	void EndMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			Print("No measurement named \"" + name + "\" was found", LogLevel.WARNING);
			return;
		}

		info.m_fTotal += System.GetTickCount(info.m_fStart);
	}

	//------------------------------------------------------------------------------------------------
	float GetMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			Print("No measurement named \"" + name + "\" was found", LogLevel.WARNING);
			return -1;
		}

		return info.m_fTotal;
	}

	//------------------------------------------------------------------------------------------------
	void PrintAllMeasures()
	{
		foreach (string name, SCR_TimeMeasurementHelper_Info info : m_mData)
		{
			Print("Measure \"" + name + "\": " + info.m_fTotal + " ms", LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_mData.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_TimeMeasurementHelper()
	{
		m_mData = new map<string, ref SCR_TimeMeasurementHelper_Info>();
	}
}

// databag
class SCR_TimeMeasurementHelper_Info
{
	float m_fStart;
	float m_fTotal;
}
