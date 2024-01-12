class SCR_WeightedArray<Class TValue>: Managed
{
	protected ref array<TValue> m_Values = new array<TValue>();
	protected ref array<float> m_Weights = new array<float>();
	protected float m_TotalWeight;
	
	int GetRandomValue(out TValue outValue)
	{
		float weightedValue = Math.RandomFloat(0, m_TotalWeight);
		return GetWeightedValue(outValue, weightedValue);
	}
	int GetRandomValue(out TValue outValue, RandomGenerator randomGenerator)
	{
		float weightedValue = randomGenerator.RandFloatXY(0, m_TotalWeight);
		return GetWeightedValue(outValue, weightedValue);
	}
	int GetWeightedValue(out TValue outValue, float weightedValue)
	{
		float weight;
		for (int i = 0, count = m_Values.Count(); i < count; i++)
		{
			weight += m_Weights[i];
			if (weightedValue < weight)
			{
				outValue = m_Values[i];
				return i;
			}
		}
		return -1;
	}
	float GetTotalWeight()
	{
		return m_TotalWeight;
	}
	TValue Get(int n)
	{
		return m_Values.Get(n);
	}
	void Set(int n, TValue value)
	{
		m_Values.Set(n, value);
	}
	int Insert(TValue value, float weight)
	{
		m_Weights.Insert(weight);
		m_TotalWeight += weight;
		return m_Values.Insert(value);
	}
	void Remove(int i)
	{
		m_TotalWeight -= m_Weights[i];
		m_Values.Remove(i);
		m_Weights.Remove(i);
	}
	void RemoveOrdered(int i)
	{
		m_TotalWeight -= m_Weights[i];
		m_Values.RemoveOrdered(i);
		m_Weights.RemoveOrdered(i);
	}
	float GetWeight(int i)
	{
		return m_Weights[i];
	}
	TValue GetValue(int i)
	{
		return m_Values[i];
	}
	int Count()
	{
		return m_Values.Count();
	}
	bool IsEmpty()
	{
		return m_Values.IsEmpty();
	}
	bool Contains(TValue value)
	{
		return m_Values.Contains(value);
	}
	int Find(TValue value)
	{
		return m_Values.Find(value);
	}
	
	int CopyFrom(notnull SCR_WeightedArray<TValue> from)
	{
		Clear();
		int count = from.Count();
		for (int i = 0; i < count; i++)
		{
			m_Weights.Insert(from.m_Weights[i]);
			m_Values.Insert(from.m_Values[i]);
		}
		return count;
	}
	void Clear()
	{
		m_Weights.Clear();
		m_Values.Clear();
	}
	int ToArray(out notnull array<TValue> outArray)
	{
		return outArray.Copy(m_Values);
	}
	void Debug()
	{
		PrintFormat("H_WeightedArray count: %1, total weight: %2", Count(), GetTotalWeight());
		for (int i, count = Count(); i < count; i++)
		{
			PrintFormat("[%1] => %2: %3", i, m_Weights[i], m_Values[i]);
		}
	}
}
