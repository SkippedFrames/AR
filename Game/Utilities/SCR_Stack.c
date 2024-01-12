//-----------------------------------------------------------------------------------------------------------
// Temporal wrapper for array that works as a stack
// Items are stored as 'first in, last out'
// Items inside are always ref, so bear that in mind
// Might be replaced with cpp implementation with script api later on
class SCR_Stack<Class T>
{
	protected ref array<ref T> m_aArray;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Push an item to the end of the stack
	void Push(T item) 
	{
		m_aArray.Insert(item);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Pop an item from the end of the stack (or null if none)
	T Pop()
	{
		int length = m_aArray.Count();
		if (length >= 1)
		{
			int index = length-1;
			
			ref T poppedItem = m_aArray[index];
			m_aArray.Remove(index);
			
			return poppedItem;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns true if stack is empty.
	bool IsEmpty()
	{
		return (m_aArray.Count() == 0);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the count of elements stored in this stack.
	int Count()
	{
		return m_aArray.Count();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Create an empty stack, initialize variables
	void SCR_Stack()
	{
		m_aArray = new ref array<ref T>();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Cleanup, release variables
	void ~SCR_Stack()
	{
		if (m_aArray)
		{
			m_aArray.Clear();
			m_aArray = null;			
		}
	}
};
