class SCR_StringHelper
{
	static const string LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
	static const string UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const string LETTERS = LOWERCASE + UPPERCASE;
	static const string DIGITS = "0123456789";
	static const string SPACE = " ";
	static const string TAB = "\t";
	protected static const string TRANSLATION_KEY_CHARS = "_-";

	//------------------------------------------------------------------------------------------------
	static bool ContainsDigit(string input)
	{
		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (asciiValue >= 48 && asciiValue <= 57)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the amount of times a needle is found in the haystack.
	//! e.g looking for "AA" in "AAAAA" will find it twice.
	//! \param haystack the string in which to search
	//! \param needle the string to find
	//! \param caseInsensitive if set to true, the search will be case-insensitive (e.g "A" will match "a" and vice-versa)
	//! \return the amount of needle occurrences in haystack, 0 if haystack or needle is empty
	static int CountOccurrences(string haystack, string needle, bool caseInsensitive = false)
	{
		if (needle.IsEmpty() || haystack.IsEmpty())
			return 0;

		if (caseInsensitive)
		{
			needle.ToLower();
			haystack.ToLower();
		}

		int needleLength = needle.Length();
		int haystackLength = haystack.Length();

		if (needleLength > haystackLength)
			return 0;

		int result;
		int searchIndex;
		while (searchIndex < haystackLength)
		{
			int resultIndex = haystack.IndexOfFrom(searchIndex, needle);
			if (resultIndex < 0)
				break;

			result++;
			searchIndex = resultIndex + needleLength;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param input the input string
	//! \param characters the characters to either respect or remove depending on useCharactersAsBlacklist
	//! \param useCharactersAsBlacklist false for characters to be a whitelist, true for a blacklist
	//! \return the resulting string
	static string Filter(string input, string characters, bool useCharactersAsBlacklist = false)
	{
		if (input.IsEmpty() || (!useCharactersAsBlacklist && characters.IsEmpty()))
			return string.Empty;

		string result;
		for (int i, length = input.Length(); i < length; i++)
		{
			string letter = input[i];
			if (characters.Contains(letter) != useCharactersAsBlacklist)
				result += letter;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param format
	//! \param input must be ASCII
	static bool IsFormat(SCR_EStringFormat format, string input)
	{
		switch (format)
		{
			case SCR_EStringFormat.ALPHABETICAL_UC:		return CheckCharacters(input, false, true, false);
			case SCR_EStringFormat.ALPHABETICAL_LC:		return CheckCharacters(input, true, false, false);
			case SCR_EStringFormat.ALPHABETICAL_I:		return CheckCharacters(input, true, true, false);
			case SCR_EStringFormat.ALPHANUMERICAL_UC:	return CheckCharacters(input, false, true, true);
			case SCR_EStringFormat.ALPHANUMERICAL_LC:	return CheckCharacters(input, true, false, true);
			case SCR_EStringFormat.ALPHANUMERICAL_I:	return CheckCharacters(input, true, true, true);
			case SCR_EStringFormat.DIGITS_ONLY:			return CheckCharacters(input, false, false, true);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided string respects all the limitations
	//! \param allowLC allow LowerCase characters (abc..xyz)
	//! \param allowUC allow UpperCase characters (ABC..XYZ)
	//! \param allowDigits allow all numbers (012..789)
	protected static bool CheckCharacters(string input, bool allowLC, bool allowUC, bool allowDigits)
	{
		if (input.IsEmpty())
			return false;

		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (!(
				(allowLC && asciiValue >= 97 && asciiValue <= 122) ||
				(allowUC && asciiValue >= 65 && asciiValue <= 90) ||
				(allowDigits && asciiValue >= 48 && asciiValue <= 57)
			))
				return false;
		}

		return true;
	}

	/*
	//------------------------------------------------------------------------------------------------
	//! old method, 4-5× slower but allows for non-ASCII values, not useful for now
	protected static bool CheckCharactersOld(string input, bool allowLC, bool allowUC, bool allowDigits)
	{
		string filter;
		if (allowLC)
			filter += LOWERCASE;
		if (allowUC)
			filter += S_UPPERCASE;
		if (allowDigits)
			filter += S_DIGITS;

		if (filter.IsEmpty())
			return false;

		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}

		return true;
	}
	// */

	//------------------------------------------------------------------------------------------------
	//! format with string arguments in the form of an array
	//! \param format with %1, %2 etc
	//! \param[notnull] arguments array
	//! \return string.Format'ted string (with max 9 arguments)
	static string Format(string input, notnull array<string> arguments)
	{
		if (input.IsEmpty())
			return string.Empty;

		if (!input.Contains("%"))
			return input;

		switch (arguments.Count())
		{
			case 0: return string.Format(input);
			case 1: return string.Format(input, arguments[0]);
			case 2: return string.Format(input, arguments[0], arguments[1]);
			case 3: return string.Format(input, arguments[0], arguments[1], arguments[2]);
			case 4: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3]);
			case 5: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
			case 6: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
			case 7: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
			case 8: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		}

		// 9 and more
		return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets int values from a string (e.g { 0, 3, 5, 7, 9 } from "0 3 5 abc 7 9)
	//! numbers should be separated by spaces
	static array<int> GetIntsFromString(string input)
	{
		array<int> result = {};
		array<string> splits = {};
		input.Split(SPACE, splits, true);

		foreach (string split : splits)
		{
			int value = split.ToInt();
			if (value != 0 || split == "0")
				result.Insert(value);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the first occurrence of the provided samples
	//! see string.IndexOf
	static int IndexOf(string input, notnull array<string> samples)
	{
		if (input.IsEmpty() || samples.IsEmpty())
			return -1;

		int result = -1;
		foreach (string sample : samples)
		{
			int index = input.IndexOf(sample);
			if (index != -1 && index < result)
				result = index;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the first occurrence of the provided samples from a position
	//! see string.IndexOfFrom
	static int IndexOfFrom(string input, int start, notnull array<string> samples)
	{
		if (start < 0 || start > input.Length() || input.IsEmpty() || samples.IsEmpty())
			return -1;

		int result = -1;
		foreach (string sample : samples)
		{
			int index = input.IndexOfFrom(start, sample);
			if (index != -1 && index < result)
				result = index;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets float values from a string (e.g { 0.3, 5.0, 7.9 } from "0.3 5.0 abc 7.9")
	//! numbers should be separated by spaces
	static array<float> GetFloatsFromString(string input)
	{
		array<float> result = {};
		array<string> splits = {};
		input.Split(SPACE, splits, true);

		foreach (string split : splits)
		{
			float value = split.ToFloat();
			if (value != 0 || split.StartsWith("0"))
				result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Insert a string into another string
	//! \return a new string instance with the result
	static string InsertAt(string input, string insertion, int insertionIndex = 0)
	{
		if (input.IsEmpty() || insertion.IsEmpty() || insertionIndex < 0 || insertionIndex > input.Length())
			return input;

		if (insertionIndex == 0)
			return insertion + input;

		return input.Substring(0, insertionIndex) + insertion + input.Substring(insertionIndex, input.Length() - insertionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if input is empty or only made of spaces or tabs
	static bool IsEmptyOrWhiteSpace(string input)
	{
		return input.Trim().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if the provided string is in the translation key format (e.g #AR-Translation_Value_3) - the pound sign (#) must be present!
	//! \param input
	//! \return true if input is in the translation key format
	static bool IsTranslationKey(string input)
	{
		if (IsEmptyOrWhiteSpace(input))
			return false;

		if (input != input.Trim())
			return false;

		if (input.Length() < 2 || !input.StartsWith("#"))
			return false;

		for (int i, count = TRANSLATION_KEY_CHARS.Length(); i < count; i++)
		{
			if (input.EndsWith(TRANSLATION_KEY_CHARS[i]))
				return false;
		}

		string filter = LOWERCASE + UPPERCASE;
		if (!filter.Contains(input[1])) // \#[a-zA-Z].*
			return false;

		filter += DIGITS + TRANSLATION_KEY_CHARS;
		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins strings together (reverse operation of string.Split)
	static string Join(string separator, notnull array<string> pieces, bool joinEmptyEntries = true)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result = pieces[0];
		for (int i = 1, cnt = pieces.Count(); i < cnt; i++)
		{
			if (joinEmptyEntries || !pieces[i].IsEmpty())
				result += separator + pieces[i];
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Pads the provided string's left end (start) with the provided padding
	static string PadLeft(string input, int length, string padding = SPACE)
	{
		while (input.Length() < length)
		{
			input = padding + input;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Pads the provided string's right end (end) with the provided padding
	static string PadRight(string input, int length, string padding = SPACE)
	{
		while (input.Length() < length)
		{
			input += padding;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace until there is no trace of search
	//! @code
	//! string input = "AAAAAABAAA";
	//! input.Replace("AA", "A");										// input is now "AAABAA"
	//! SCR_StringHelper.ReplaceRecursive("AAAAAABAAA", "AA", "A");	// returns "ABA"
	//! SCR_StringHelper.ReplaceRecursive("AAAAAABAAA", "AA", "AAX");	// returns "AAAAAABAAA"
	//! @endcode
	//! \param input the input in which to search and replace
	//! \param what to replace - an empty sample will do nothing
	//! \param replace CANNOT contain sample for an obvious reason
	//! \return the modified input, or the original one on wrong arguments
	static string ReplaceRecursive(string input, string sample, string replace)
	{
		if (input.IsEmpty() || sample.IsEmpty() || sample == replace || replace.Contains(sample))
			return input;

		while (input.IndexOf(sample) > -1)
		{
			input.Replace(sample, replace);
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace X times a string from within a string from left to right
	//! the next occurrence is searched after the previous replacement, there is no overlap
	//! @code
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya");		// returns "Heya Hallo Yellow"
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya", 2);	// returns "Heya Haya Yellow"
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya", 4);	// returns "Heya Haya Yeyaw"
	//! SCR_StringHelper.ReplaceTimes("A A A", "A", "BA", 2);		// returns "BA BA A"
	//! SCR_StringHelper.ReplaceTimes("A A A", "A", "B", 1, 1);	// returns "A B A"
	//! @endcode
	//! \param input the input in which to search and replace
	//! \param sample what to replace - an empty sample will do nothing
	//! \param replace the replacement string
	//! \param howMany times the string must be replaced
	//! \param skip first occurrences -not- to be replaced
	//! \return input with 'sample' replaced by 'replace' 'howMany' times after skipptin 'skip' occurrences
	static string ReplaceTimes(string input, string sample, string replace, int howMany = 1, int skip = 0)
	{
		if (howMany < 1 || input.IsEmpty() || sample.IsEmpty() || sample == replace)
			return input;

		int sampleLength = sample.Length();
		int replaceLength = replace.Length();

		int index;
		while (howMany > 0)
		{
			index = input.IndexOfFrom(index, sample);
			if (index < 0)
				break;

			if (skip > 0)
			{
				skip--;
				index += sampleLength;
				continue;
			}

			if (index == 0)
				input = replace + input.Substring(sampleLength, input.Length() - sampleLength);
			else
				input = input.Substring(0, index) + replace + input.Substring(index + sampleLength, input.Length() - (index + sampleLength));

			// no overlap
			index += replaceLength;

			howMany--;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the provided input string reversed
	static string Reverse(string input)
	{
		string result;
		for (int i = input.Length() - 1; i >= 0; i--)
		{
			result += input[i];
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if input contains any needles
	static bool ContainsAny(string input, array<string> needles)
	{
		if (input.IsEmpty() || !needles)
			return false;

		foreach (string needle : needles)
		{
			if (input.Contains(needle))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if input contains every needles
	static bool ContainsEvery(string input, array<string> needles)
	{
		if (input.IsEmpty())
			return false;

		if (!needles)
			return true;

		foreach (string needle : needles)
		{
			if (!input.Contains(needle))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool StartsWithAny(string input, array<string> lineStarts)
	{
		if (input.IsEmpty() || !lineStarts)
			return false;

		foreach (string lineStart : lineStarts)
		{
			if (input.StartsWith(lineStart))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool EndsWithAny(string input, array<string> lineEnds)
	{
		if (input.IsEmpty())
			return false;

		foreach (string lineEnd : lineEnds)
		{
			if (input.EndsWith(lineEnd))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the actual translation from the translation key
	//! If not a translation key, the provided input is returned
	//! It is NOT recommended to manipulate a potentially non-ASCII string (multibyte UTF-8), use at your own risk!
	//! \return translated string, multibyte UTF-8 format
	static string Translate(
		string input,
		string param1 = string.Empty,
		string param2 = string.Empty,
		string param3 = string.Empty,
		string param4 = string.Empty,
		string param5 = string.Empty,
		string param6 = string.Empty,
		string param7 = string.Empty,
		string param8 = string.Empty,
		string param9 = string.Empty)
	{
		return WidgetManager.Translate(input, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the actual translation from the translation key
	//! If not a translation key, the provided input is returned
	//! It is NOT recommended to manipulate a potentially non-ASCII string (multibyte UTF-8), use at your own risk!
	//! \return translated string, multibyte UTF-8 format
	static string Translate(string input, notnull array<string> arguments)
	{
		if (input.IsEmpty())
			return string.Empty;

		if (!input.Contains("%"))
			return input;

		switch (arguments.Count())
		{
			case 0: return WidgetManager.Translate(input);
			case 1: return WidgetManager.Translate(input, arguments[0]);
			case 2: return WidgetManager.Translate(input, arguments[0], arguments[1]);
			case 3: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2]);
			case 4: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3]);
			case 5: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
			case 6: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
			case 7: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
			case 8: return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		}

		// 9 and more
		return WidgetManager.Translate(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove spaces and tabs on the left end of the provided string
	//! Vertical tabs and other characters are ignored and considered as normal characters (for now?)
	//! \param
	static string TrimLeft(string input)
	{
		if (input.IsEmpty())
			return string.Empty;

		for (int i, count = input.Length(); i < count; i++)
		{
			string character = input[i];
			if (character == SPACE || character == TAB)
				continue;

			return input.Substring(i, count - i);
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	static string TrimRight(string input)
	{
		if (input.IsEmpty())
			return string.Empty;

		for (int i = input.Length() - 1; i >= 0; i--)
		{
			string character = input[i];
			if (character == SPACE || character == TAB)
				continue;

			return input.Substring(0, i + 1);
		}

		return string.Empty;
	}
}

enum SCR_EStringFormat
{
	ALPHABETICAL_UC,	//!< [A-Z]+
	ALPHABETICAL_LC,	//!< [a-z]+
	ALPHABETICAL_I,		//!< [a-zA-Z]+ (I = insensitive)
	ALPHANUMERICAL_UC,	//!< [A-Z0-9]+
	ALPHANUMERICAL_LC,	//!< [a-z0-9]+
	ALPHANUMERICAL_I,	//!< [a-zA-Z0-9]+ (I = insensitive)
	DIGITS_ONLY,		//!< [0-9]+
	// FLOATING_POINT,		//!< [0-9][0-9\.]+[0-9]
}

// typedef?
class SCR_StringArray : array<string>
{
	//------------------------------------------------------------------------------------------------
	int CountEmptyEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				result++;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	int CountEmptyOrWhiteSpaceEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				result++;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	string Join(string separator = string.Empty, bool joinEmptyEntries = true)
	{
		return SCR_StringHelper.Join(separator, this, joinEmptyEntries);
	}

	//------------------------------------------------------------------------------------------------
	bool HasEmptyEntry()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool HasEmptyOrWhiteSpaceEntry()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEmptyEntries()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEmptyOrWhiteSpaceEntries()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ToLower()
	{
		string tmp;
		for (int i = Count() - 1; i >= 0; i--)
		{
			tmp = this[i];
			tmp.ToLower();
			this[i] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ToUpper()
	{
		string tmp;
		for (int i = Count() - 1; i >= 0; i--)
		{
			tmp = this[i];
			tmp.ToUpper();
			this[i] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	void Trim()
	{
		for (int i = Count(); i >= 0; i--)
		{
			this[i] = this[i].Trim();
		}
	}
}
