#ifdef WORKBENCH
/*!
	\brief A Basic Code Formatter - use Ctrl+Shift+K to trigger.
*/
// ###############
// It is recommended to be VERY CAREFUL with breakpoints here
// as GetCurrentFile, GetLinesCount, GetLineText, SETLineText etc may target this file
// Once a breakpoint is hit (without FileIO), switch back to the edited file and press F5
// ###############
[WorkbenchPluginAttribute(
	name: "Basic Code Formatter",
	shortcut: "Ctrl+Shift+K",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF036)]
class SCR_BasicCodeFormatterPlugin : WorkbenchPlugin
{
	/*
		Category: Formatting
	*/

	[Attribute(defvalue: "1", desc: "Fix e.g 'for( int' → 'for (int', ';;' → ';', ') ;' → ');', 'NULL' → 'null', ',' → ', ', etc.", category: "Formatting")]
	protected bool m_bGeneralFormatting;

	[Attribute(defvalue: "1", desc: "Remove empty spaces at the end of code lines", category: "Formatting")]
	protected bool m_bTrimLineEnds;

	[Attribute(defvalue: "1", desc: "Replace four-spaces tabs with the tab character and remove spaces mixed with tabs", category: "Formatting")]
	protected bool m_bFixTabs;

	[Attribute(defvalue: "1", desc: "Fix method separators (" + TWO_SLASHES + "---)", category: "Formatting")]
	protected bool m_bFixMethodSeparators;

//	[Attribute(defvalue: "1", desc: "Replace consecutive empty lines with only one", category: "Formatting")]
//	protected bool m_bRemoveMultiEmptyLines;

	[Attribute(defvalue: "1", desc: "Add one final line return if the file is missing one", category: "Formatting")]
	protected bool m_bAddFinalLineReturn;

	[Attribute(desc: "Accepted prefixes (e.g 'SCR_', 'TAG_', etc) note that an underscore is automatically added if missing. 'SCR_' is automatically whitelisted", category: "Formatting")]
	protected ref array<string> m_aAcceptedScriptPrefixes;

	/*
		Category: Batch Process
	*/

	[Attribute(defvalue: "0", desc: "Process all script files in the addon defined below (only process the currently opened file if unchecked)", category: "Batch Process")]
	protected bool m_bBatchProcessAddon;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Addon in which script files will be batch-formatted (if \"Batch Process Addon\" above is selected)", enums: ParamEnumAddons.FromEnum(titleFormat: 2, hideCoreModules: 2), category: "Batch Process")]
	protected int m_iAddon;

	[Attribute(defvalue: "1", desc: "Only log reports where processing did something to the file", category: "Batch Process")]
	protected bool m_bOnlyLogChanges;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Max number of formatting reports to display - use -1 to display all reports", params: "-1 100 1", category: "Batch Process")]
	protected int m_iMaxLoggedReports;

	/*
		Category: Options
	*/

	[Attribute(defvalue: "0", desc: "Demo mode only logs (possible) fixes and does not modify the file at all", category: "Options")]
	protected bool m_bDemoMode;

	[Attribute(defvalue: "1", desc: "Display a dialog on execution if unchecked - dialog is always displayed when batch-processing is enabled", category: "Options")]
	protected bool m_bSilentExecution;

	[Attribute(defvalue: "1", desc: "Only format DiffCommand-detected modified lines instead of all of them", category: "Options")]
	protected bool m_bOnlyFormatModifiedLines;

	[Attribute(defvalue: "1", desc: "Write fixes in console log", category: "Options")]
	protected bool m_bLogFixes;

	[Attribute(defvalue: "1", desc: "Show potential improvements report", category: "Options")]
	protected bool m_bReportFindings;

	/*
		Category: Advanced
	*/

	[Attribute(defvalue: DEFAULT_DIFF_CMD, desc: "The command line used to generate the diff file", category: "Advanced")]
	protected string m_sDiffCommand;

	protected ScriptEditor m_ScriptEditor;

	protected ref array<ref array<string>> m_aGeneralFormatting_Start;
	protected ref array<ref array<string>> m_aGeneralFormatting_Middle;
	protected ref array<ref array<string>> m_aGeneralFormatting_End;

	protected ref array<string> m_aForbiddenDivisions;
	protected ref array<string> m_aForbiddenPrefixes;
	protected ref array<string> m_aPrefixLineChecks;
	protected ref array<string> m_aPrefixChecks;

	protected ref array<string> m_aForForEachWhileArray;
	protected ref array<string> m_aIfForForEachWhileArray;
	protected ref array<string> m_aEndBracketSemicolonArray;
	protected ref array<string> m_aNewArrayNewRefArray;
	protected ref array<string> m_aScriptInvokerArray;

	protected ref map<string, string> m_mVariableTypePrefixes;
	protected ref map<string, string> m_mVariableTypePrefixesStart;
	protected ref map<string, string> m_mVariableTypePrefixesEnd;

	protected static const int LINE_NUMBER_LIMIT = 10;	//!< used by JoinLineNumbers to limit the amount of shown line numbers

	protected static const string SPACE = SCR_StringHelper.SPACE;
	protected static const string TWO_SPACES = SPACE + SPACE;
	protected static const string FOUR_SPACES = SPACE + SPACE + SPACE + SPACE;
	protected static const string TAB = SCR_StringHelper.TAB;
	protected static const string BRACKET_OPEN = "{";			// avoids {} Script Editor indent shenanigans
	protected static const string BRACKET_CLOSE = "}";
	protected static const string TWO_SLASHES = "/" + "/";		// avoids Script Editor // highlight shenanigans

	// "varName' '= 42", "varName\t= 42", "varName, otherVarName", "varName= 42", "varName;"
	protected static const ref array<string> VARIABLE_NAME_ENDING = { SPACE, TAB, ",", "=", ";" }; // technically, "everything but [a-zA-Z0-9_]"

	protected static const string METHOD_SEPARATOR = TWO_SLASHES + "------------------------------------------------------------------------------------------------";
	protected static const string DIFF_FILENAME = "tempDiffFile.txt";
	protected static const string DEFAULT_DIFF_CMD = "cmd /c svn diff \"%1\" > \"%2\""; // %1 = absolute target filepath, %2 = absolute destination filepath
	protected static const ref array<string> FORMAT_IGNORE = { TWO_SLASHES, "/" + "*", "\"" };	// avoids Script Editor comment shenanigans

	//------------------------------------------------------------------------------------------------
	//! Running method
	override void Run()
	{
		if (m_bBatchProcessAddon || !m_bSilentExecution)
		{
			if (!Workbench.ScriptDialog("Configure 'Basic Code formatter' plugin", "Configure formatting options", this))
				return;
		}

		Initialise();

		if (m_bBatchProcessAddon)
			RunAddonFilesBatchProcess();
		else
			RunCurrentFile();
	}

	//------------------------------------------------------------------------------------------------
	//! Required initialisation (prefixes filter)
	protected void Initialise()
	{
		m_ScriptEditor = Workbench.GetModule(ScriptEditor);
		if (!m_ScriptEditor)
		{
			Print("Script Editor API is not available", LogLevel.ERROR);
			return;
		}

		if (m_aPrefixChecks)
			m_aPrefixChecks.Clear();
		else
			m_aPrefixChecks = {};

		for (int i = m_aAcceptedScriptPrefixes.Count() - 1; i >= 0; i--)
		{
			string prefix = m_aAcceptedScriptPrefixes[i].Trim();
			if (prefix.IsEmpty())
			{
				m_aAcceptedScriptPrefixes.RemoveOrdered(i);
				continue;
			}

			if (!prefix.EndsWith("_"))
				m_aAcceptedScriptPrefixes[i] = prefix + "_";
		}

		array<string> acceptedScriptPrefixes = {};
		acceptedScriptPrefixes.Copy(m_aAcceptedScriptPrefixes); // avoid having SCR_ added to the user UI

		if (!acceptedScriptPrefixes.Contains("SCR_"))
			acceptedScriptPrefixes.InsertAt("SCR_", 0);

		foreach (string prefix : acceptedScriptPrefixes)
		{
			foreach (string prefixLineCheck : m_aPrefixLineChecks)
			{
				m_aPrefixChecks.Insert(prefixLineCheck + prefix);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Run the current file
	protected void RunCurrentFile()
	{
		string currentFile;
		m_ScriptEditor.GetCurrentFile(currentFile);
		SCR_BasicCodeFormatterPluginFileReport report = ProcessFile(currentFile, false);
		if (report)
			PrintReport(report, m_bLogFixes, m_bReportFindings); // always print current file's report
	}

	//------------------------------------------------------------------------------------------------
	//! Show dialogs to treat all addon script files
	protected void RunAddonFilesBatchProcess()
	{
		string absoluteAddonDirectory;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, string.Empty, absoluteAddonDirectory, true))
		{
			Print("Wrong addon selected: " + SCR_AddonTool.GetAddonIndex(m_iAddon) + " (read-only?)", LogLevel.ERROR);
			return;
		}

		string addonName = SCR_AddonTool.GetAddonIndex(m_iAddon);
		array<ResourceName> scriptFiles = SCR_WorldEditorToolHelper.SearchWorkbenchResources({ "c" }, null, SCR_AddonTool.ToFileSystem(addonName));

		array<string> relativeFilePaths = {};
		foreach (ResourceName scriptFile : scriptFiles)
		{
			string scriptFilePath = scriptFile.GetPath();
			relativeFilePaths.Insert(scriptFilePath);
		}

		int editableScriptsCount = relativeFilePaths.Count();
		if (editableScriptsCount < 1)
		{
			Print("No script files found in " + addonName + " - leaving", LogLevel.WARNING);
			return;
		}

		SCR_BasicCodeFormatterPluginOKCancelDialog okCancelDialog = new SCR_BasicCodeFormatterPluginOKCancelDialog();
		if (!Workbench.ScriptDialog(
				"Warning",
				"You are about to format " + editableScriptsCount + " \"" + addonName + "\" addon script files.\n\n" + "Continue?",
				okCancelDialog)
		)
			return;

		array<ref SCR_BasicCodeFormatterPluginFileReport> reports = ProcessFiles(relativeFilePaths, true);

		if (m_iMaxLoggedReports == 0)
			return;

		int displayedReports;
		foreach (SCR_BasicCodeFormatterPluginFileReport report : reports)
		{
			if (!report)
				continue;

			if (m_bOnlyLogChanges && report.IsClean())
				continue;

			PrintReport(report, m_bLogFixes, m_bReportFindings);

			displayedReports++;
			if (m_iMaxLoggedReports > 0 && displayedReports >= m_iMaxLoggedReports)
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Process multiple files (using ProcessFile)
	//! \param relativeFilePaths multiple relative file paths
	//! \return array of reports (in the order of relativeFilePaths)
	protected array<ref SCR_BasicCodeFormatterPluginFileReport> ProcessFiles(array<string> relativeFilePaths, bool useFileIO)
	{
		string currentFile;
		m_ScriptEditor.GetCurrentFile(currentFile);

		array<ref SCR_BasicCodeFormatterPluginFileReport> result = {};
		SCR_BasicCodeFormatterPluginFileReport report;
		foreach (string relativeFilePath : relativeFilePaths)
		{
			report = ProcessFile(relativeFilePath, useFileIO && relativeFilePath != currentFile); // do NOT use FileIO on the current file - the Script Editor may lose edits track
			result.Insert(report);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Process a single file
	//! \param relativeFilePath the file's relative path
	//! \param useFileIO true = use FileIO's API (no Ctrl+Z available, but does not open a Script Editor tab),
	//!                  false = use ScriptEditor API method (opens a tab, allows for Ctrl+Z)
	//! \return provided file's report, null on error
	protected SCR_BasicCodeFormatterPluginFileReport ProcessFile(string relativeFilePath, bool useFileIO)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(relativeFilePath))
		{
			Print("Provided relative file path is empty", LogLevel.WARNING);
			return null;
		}

		bool isReadOnly = false;
		bool demoMode = m_bDemoMode;

		// checking the file is modifiable
		string absoluteFilePath;
		if (!Workbench.GetAbsolutePath(relativeFilePath, absoluteFilePath, true))
		{
			Print("Cannot find the absolute file path, switching to readonly mode - is file read-only? " + relativeFilePath, LogLevel.WARNING);
			demoMode = true;
			isReadOnly = true;
		}

		SCR_BasicCodeFormatterPluginFileReport report = new SCR_BasicCodeFormatterPluginFileReport(relativeFilePath, relativeFilePath == __FILE__);

		bool reportFindings = !report.m_bIsPluginFile && m_bReportFindings;
		bool doGeneralFormatting = !report.m_bIsPluginFile && m_bGeneralFormatting;

		bool isPreviousLineEmptyLine;
		bool emptyLineGroupLogged;

		if (m_bOnlyFormatModifiedLines && !isReadOnly)
		{
			report.m_iDiffTime = System.GetTickCount();
			report.m_aDiffLines = GetFileModifiedLineNumbers(absoluteFilePath);
			report.m_iDiffTime = System.GetTickCount(report.m_iDiffTime);

			if (report.m_aDiffLines && report.m_aDiffLines.IsEmpty()) // if no changes are done, check the whole file
				report.m_aDiffLines = null;
		}

		array<string> pieces;
		/*
			Start
		*/
		int startTick = System.GetTickCount();

		array<string> lines = ReadFileContent(relativeFilePath, useFileIO);
		int linesCount = lines.Count(); // get array count instead of incrementing a variable

		foreach (int lineNumber, string fullLine : lines)
		{
			int lineNumberPlus1 = lineNumber + 1;
			bool formatThisLine = !report.m_aDiffLines || report.m_aDiffLines.Contains(lineNumberPlus1); // null = format everything
			if (!formatThisLine && !reportFindings)
				continue;

			string indentation;
			GetIndentAndLineContentAsPieces(fullLine, indentation, pieces);
			int piecesCount = pieces.Count();
			string lastPiece;

			if (formatThisLine)
			{
				if (m_bTrimLineEnds)
				{
					int endSpacesRemoved;
					if (piecesCount < 1)
					{
						endSpacesRemoved = indentation.Length();
						indentation = string.Empty;
					}
					else
					{
						string endPiece = pieces[piecesCount - 1];
						endPiece = SCR_StringHelper.TrimRight(endPiece);
						endSpacesRemoved = pieces[piecesCount - 1].Length() - endPiece.Length();
						if (endSpacesRemoved > 0)
							pieces.Set(piecesCount - 1, endPiece);
					}

					if (endSpacesRemoved > 0)
					{
						report.m_iEndSpacesRemovedTotal += endSpacesRemoved;
						report.m_aTrimmings.Insert(lineNumberPlus1);
					}
				}

				if (m_bFixTabs)
				{
					int fourSpacesReplaced = indentation.Replace(FOUR_SPACES, TAB);
					int spaceInTabsReplaced = indentation.Replace(SPACE, string.Empty);

					if (fourSpacesReplaced > 0 || spaceInTabsReplaced > 0)
					{
						report.m_iFourSpacesReplacedTotal += fourSpacesReplaced;
						report.m_iSpaceInTabsReplacedTotal += spaceInTabsReplaced;
						report.m_aFixedIndentations.Insert(lineNumberPlus1);
					}
				}

				if (m_bFixMethodSeparators)
				{
					if (piecesCount == 1 && pieces[0].StartsWith(TWO_SLASHES + "---") && pieces[piecesCount - 1].EndsWith("---") && pieces[0] != METHOD_SEPARATOR)
					{
						pieces.Set(0, METHOD_SEPARATOR);
						report.m_iMethodSeparatorFixedTotal++;
					}
				}

				if (doGeneralFormatting)
					report.m_iGeneralFormattingTotal += GeneralFormatting(indentation, pieces);
			}

			string finalContent = SCR_StringHelper.Join("", pieces);

			if (reportFindings)
			{
				string findingsString = finalContent.Trim();

				bool isCurrentLineEmpty = findingsString.IsEmpty();
				if (isPreviousLineEmptyLine && isCurrentLineEmpty)
				{
					if (!emptyLineGroupLogged)
					{
						report.m_aDoubleEmptyLineFound.Insert(lineNumber /* -1 */);
						emptyLineGroupLogged = true;
					}
				}

				isPreviousLineEmptyLine = isCurrentLineEmpty;
				if (!emptyLineGroupLogged)
					emptyLineGroupLogged = false;

				findingsString = string.Empty;
				foreach (string piece : pieces)
				{
					if (!SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
						findingsString += piece;
				}

				if (!findingsString.IsEmpty())
				{
					if (SCR_StringHelper.ContainsAny(findingsString, m_aNewArrayNewRefArray))
						report.m_aNewArrayFound.Insert(lineNumberPlus1);
					else // to not have both 'new ref array' and 'new ref'
					if (findingsString.Contains("new ref "))
						report.m_aNewRefFound.Insert(lineNumberPlus1);

					if (findingsString.Contains("auto "))
						report.m_aAutoKeywordFound.Insert(lineNumberPlus1);

					if (findingsString.Contains("autoptr "))
						report.m_aAutoptrKeywordFound.Insert(lineNumberPlus1);

					foreach (string forbiddenDivision : m_aForbiddenDivisions)
					{
						if (findingsString.Contains(forbiddenDivision))
						{
							report.m_aDivideByXFound.Insert(lineNumberPlus1);
							break;
						}
					}

					if (SCR_StringHelper.StartsWithAny(findingsString, m_aForForEachWhileArray) && lineNumber < linesCount - 1)
					{
						string nextLine = lines[lineNumberPlus1];
						if (!nextLine.Trim().StartsWith(BRACKET_OPEN))
							report.m_aUnscopedLoopFound.Insert(lineNumberPlus1);
					}

					if (SCR_StringHelper.StartsWithAny(findingsString, m_aIfForForEachWhileArray) && SCR_StringHelper.EndsWithAny(findingsString, m_aEndBracketSemicolonArray))
						report.m_aOneLinerFound.Insert(lineNumberPlus1);

					if (HasBadVariableNaming(indentation + findingsString))
						report.m_aBadVariableNamingFound.Insert(lineNumberPlus1);

					if (SCR_StringHelper.ContainsAny(findingsString, m_aScriptInvokerArray))
						report.m_aBadScriptInvokerFound.Insert(lineNumberPlus1);

					if (
						findingsString.EndsWith(";") &&
						(findingsString.StartsWith("Print(") && !findingsString.Contains("LogLevel.")) || findingsString.StartsWith("PrintFormat"))
						report.m_aWildPrintFound.Insert(lineNumberPlus1);

					if (SCR_StringHelper.StartsWithAny(findingsString, m_aPrefixLineChecks) && !SCR_StringHelper.StartsWithAny(findingsString, m_aPrefixChecks))
						report.m_aNonPrefixedClassOrEnumFound.Insert(lineNumberPlus1);
				}
			}

			string finalLine = indentation + finalContent;
			if (finalLine == fullLine)
				continue;

			report.m_iLinesEdited++;

			if (demoMode)
				continue;

			if (useFileIO)
				lines[lineNumber] = finalLine;
			else
				m_ScriptEditor.SetLineText(finalLine, lineNumber); // directly use Script Editor to avoid going through lines twice
		}
		/*
			Finish
		*/

		if (!demoMode && m_bAddFinalLineReturn && lines[linesCount - 1] != string.Empty)
		{
			if (useFileIO)
				lines.Insert(string.Empty);
			else
				AddFinalLineReturnToCurrentFile();

			report.m_bHasAddedFinalLineReturn = true;
		}

		// later, with proper line insertion/deletion
//		if (m_bRemoveMultiEmptyLines)
//		{
//			if (useFileIO)
//				doSomething();
//			else
//				RemoveEmptyLinesInCurrentFile();
//		}

		// write time!
		if (!demoMode && useFileIO)
			WriteFileContent(relativeFilePath, lines);

		report.m_iFormatTime = System.GetTickCount(startTick);

		return report;
	}

	//------------------------------------------------------------------------------------------------
	//! Get relative file's content lines as array of strings
	//! \param relativeFilePath the file's relative path
	//! \param useFileIO use FileIO API, otherwise use Script Editor API (opening the file in a tab)
	//! \return file lines
	protected array<string> ReadFileContent(string relativeFilePath, bool useFileIO)
	{
		array<string> result = {};

		if (useFileIO)		// FileIO
		{
			FileHandle fileHandle = FileIO.OpenFile(relativeFilePath, FileMode.READ);
			if (!fileHandle)
			{
				Print("File could not be opened: " + relativeFilePath, LogLevel.ERROR);
				return null;
			}

			while (!fileHandle.IsEOF())
			{
				string line;
				fileHandle.ReadLine(line);
				result.Insert(line);
			}

			fileHandle.Close();
		}
		else				// Script Editor
		{
			m_ScriptEditor.SetOpenedResource(relativeFilePath); // open tab
			for (int lineNumber, linesCount = m_ScriptEditor.GetLinesCount(); lineNumber < linesCount; lineNumber++)
			{
				string line;
				m_ScriptEditor.GetLineText(line, lineNumber);
				result.Insert(line);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Overwrite file with provided lines - the file is cleared and content gets replaced, and is created if it does not exist
	//! The final line return is -not- added automatically (using FileHandle.Write for the last line)
	//! \param relativeFilePath
	//! \param lines the file content
	//! \return operation success
	// TODO: check what happens if there are less lines than in the original file
	protected bool WriteFileContent(string relativeFilePath, notnull array<string> lines)
	{
		// let's not delete for metadata sake
//		if (FileIO.FileExists(relativeFilePath))
//			FileIO.DeleteFile(relativeFilePath);

		FileHandle fileHandle = FileIO.OpenFile(relativeFilePath, FileMode.WRITE);
		if (!fileHandle)
		{
			Print("Could not write content in file \"" + relativeFilePath + "\"", LogLevel.ERROR);
			return false;
		}

		int linesToWriteWithWriteLine = lines.Count() - 1;
		for (int i = 0; i < linesToWriteWithWriteLine; i++)
		{
			fileHandle.WriteLine(lines[i]);
		}

		// avoid automatic line return due to WriteLine
		fileHandle.Write(lines[linesToWriteWithWriteLine]); // because of the earlier --

		fileHandle.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Output fixes and findings in the log console
	//! \param report the generated report
	//! \param printFixes print applied fixes (edited lines, trimmings, formatting etc)
	//! \param printFindings print findings that may require user attention (one-liners, bad variable naming, etc)
	protected void PrintReport(notnull SCR_BasicCodeFormatterPluginFileReport report, bool printFixes, bool printFindings)
	{
		if (printFixes)
		{
			array<string> reportArray = {};

			if (report.m_iLinesEdited > 0)
				reportArray.Insert(string.Format("%1 lines changed", report.m_iLinesEdited));
			if (!report.m_aTrimmings.IsEmpty())
				reportArray.Insert(report.m_aTrimmings.Count().ToString() + "× line trimming"); // at line(s) " + JoinLineNumbers(report.m_aTrimmings));
			if (!report.m_aFixedIndentations.IsEmpty())
				reportArray.Insert(report.m_aFixedIndentations.Count().ToString() + "× indent fix(es) at line(s) " + JoinLineNumbers(report.m_aFixedIndentations));
			if (report.m_iGeneralFormattingTotal > 0)
				reportArray.Insert(string.Format("%1 formattings", report.m_iGeneralFormattingTotal));
			if (report.m_iEndSpacesRemovedTotal > 0)
				reportArray.Insert(string.Format("%1 end spaces trimming", report.m_iEndSpacesRemovedTotal));
			if (report.m_iFourSpacesReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 4-spaces indent -> tabs replaced", report.m_iFourSpacesReplacedTotal));
			if (report.m_iSpaceInTabsReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 space(s) in indentation removed", report.m_iSpaceInTabsReplacedTotal));
			if (report.m_iMethodSeparatorFixedTotal > 0)
				reportArray.Insert(string.Format("%1 method separators fixed", report.m_iMethodSeparatorFixedTotal));

			if (report.m_bHasAddedFinalLineReturn)
				reportArray.Insert("added final newline");

			string reportString;
			if (m_bDemoMode && !reportArray.IsEmpty())
				reportString = "[DEMO] ";

			reportString += FilePath.StripPath(report.m_sRelativeFilePath) + " - ";

			if (reportArray.IsEmpty())
				reportString += "no fixes to report";
			else
				reportString += SCR_StringHelper.Join(", ", reportArray, false);

			Print("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -", LogLevel.NORMAL);

			string reportLine = reportString + " (formatting: " + report.m_iFormatTime + " ms";
			if (report.m_iDiffTime > 0)
				reportLine += ", diff: " + report.m_iDiffTime + " ms - total: " + (report.m_iFormatTime + report.m_iDiffTime) + " ms";
			reportLine += ")";

			if (report.m_bIsPluginFile)
				reportLine += " - [PLUGIN FILE]";

			Print("" + reportLine, LogLevel.NORMAL);

			if (report.m_aDiffLines && !report.m_aDiffLines.IsEmpty())
				Print("Checking (" + report.m_aDiffLines.Count() + "×) edited lines " + JoinLineNumbers(report.m_aDiffLines), LogLevel.NORMAL);
			else
				Print("Checking all lines", LogLevel.NORMAL);
		}

		if (printFindings && !report.m_bIsPluginFile)
		{
			if (!report.m_aNewRefFound.IsEmpty())
				PrintFinding("'new ref'", report.m_aNewRefFound, "no 'ref' is needed on creation, only on declaration");

			if (!report.m_aNewArrayFound.IsEmpty())
				PrintFinding("'new (ref) array<>'", report.m_aNewArrayFound, "replace by {} whenever possible");

			if (!report.m_aDoubleEmptyLineFound.IsEmpty())
				PrintFinding("multiple consecutive empty lines", report.m_aDoubleEmptyLineFound, "leave only one");

			if (!report.m_aAutoKeywordFound.IsEmpty())
				PrintFinding("'auto' keyword", report.m_aAutoKeywordFound, "use the direct type instead");

			if (!report.m_aAutoptrKeywordFound.IsEmpty())
				PrintFinding("'autoptr' keyword", report.m_aAutoptrKeywordFound, "remove if used on a standard variable, or replace with 'ref' if used for a member variable array");

			if (!report.m_aDivideByXFound.IsEmpty())
				PrintFinding("potentially avoidable division", report.m_aDivideByXFound, "use ' * 0.5' instead of ' / 2', ' * 0.1' instead of ' / 10' etc whenever possible");

			if (!report.m_aUnscopedLoopFound.IsEmpty())
				PrintFinding("loop without brackets", report.m_aUnscopedLoopFound, "always use brackets {} with for/foreach/while loops");

			if (!report.m_aOneLinerFound.IsEmpty())
				PrintFinding("one-liner", report.m_aOneLinerFound, "use a line return after an if/for/foreach/while condition");

			if (!report.m_aBadVariableNamingFound.IsEmpty())
				PrintFinding("badly-named variables", report.m_aBadVariableNamingFound, "use proper prefixes: m_s for ResourceName/string, m_v for vectors, NO m_p, CASED_CONSTS, etc");

			if (!report.m_aBadScriptInvokerFound.IsEmpty())
				PrintFinding("raw ScriptInvoker", report.m_aBadScriptInvokerFound, "use typed ScriptInvokerBase<> - see SCR_ScriptInvokerHelper.c for examples");

			if (!report.m_aWildPrintFound.IsEmpty())
				PrintFinding("wild Print()", report.m_aWildPrintFound, "use Print (not PrintFormat) with LogLevel.XXX to show this is not a debug print");

			if (!report.m_aNonPrefixedClassOrEnumFound.IsEmpty())
				PrintFinding("non-prefixed class/enum", report.m_aNonPrefixedClassOrEnumFound, "classes and enums should be prefixed; see the settings to setup accepted prefixes (current " + SCR_StringHelper.Join(", ", m_aAcceptedScriptPrefixes) + ")");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! General formatting like spacing, "NULL" → "null", ";;" → ";", etc
	//! \param indentation used to determine the scope level
	//! \param[inout] pieces text pieces input/output
	//! \return number of replacements that happened
	protected int GeneralFormatting(string indentation, inout notnull array<string> pieces)
	{
		int piecesCount = pieces.Count();
		if (piecesCount < 1)
			return 0;

		// a semicolon was needed at one point due to Doxygen, now not anymore (reverting such changes)
		if (indentation.IsEmpty() && pieces[0].StartsWith(BRACKET_CLOSE + ";"))
		{
			pieces[0] = SCR_StringHelper.ReplaceTimes(pieces[0], BRACKET_CLOSE + ";", BRACKET_CLOSE, 1);
			return 1;
		}

		if (pieces[0].StartsWith(TWO_SLASHES)) // don't format (inline) comments
			return 0;

		int replacements;
		array<ref array<string>> format;
		string toReplace;
		string replaceWith;
		bool hasChanged;

		// line start
		int startPieceIndex = -1;
		string startPiece;
		foreach (int pieceIndex, string piece : pieces)
		{
			if (!SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
			{
				startPiece = piece;
				startPieceIndex = pieceIndex;
				break;
			}
		}

		if (!startPiece.IsEmpty())
		{
			format = m_aGeneralFormatting_Start;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					Print(FilePath.StripPath(__FILE__) + ":" + __LINE__ + " - safety exit! " + toReplace + "/" + replaceWith, LogLevel.WARNING);
					continue;
				}

				while (startPiece.StartsWith(toReplace))
				{
					startPiece = SCR_StringHelper.ReplaceTimes(startPiece, toReplace, replaceWith, 1);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(startPieceIndex, startPiece);
					replacements++;
				}
			}

			// "class abc : parent" and "foreach x : y" colon spacing
			if (
				(startPiece.StartsWith("class ") || startPiece.StartsWith("foreach (")) &&
				startPiece.IndexOf(":") > -1
			)
			{
				hasChanged = false;
				int colonIndex = startPiece.IndexOf(":");
				if (startPiece[colonIndex - 1] != SPACE)
				{
					startPiece = SCR_StringHelper.InsertAt(startPiece, SPACE, colonIndex);
					hasChanged = true;
				}

				colonIndex = startPiece.IndexOf(":");
				if (colonIndex < startPiece.Length() -1 && startPiece[colonIndex + 1] != SPACE)
				{
					startPiece = SCR_StringHelper.InsertAt(startPiece, SPACE, colonIndex + 1);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(startPieceIndex, startPiece);
					replacements++;
				}
			}
		}

		// line middles
		for (int pieceIndex; pieceIndex < piecesCount; pieceIndex++)
		{
			string piece = pieces[pieceIndex];

			if (SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
				continue;

			format = m_aGeneralFormatting_Middle;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					Print(FilePath.StripPath(__FILE__) + ":" + __LINE__ + " - safety exit! " + toReplace + "/" + replaceWith, LogLevel.WARNING);
					continue;
				}

				while (piece.Contains(toReplace))
				{
					piece.Replace(toReplace, replaceWith);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(pieceIndex, piece);
					replacements++;
				}
			}
		}

		// now line ends
		int endPieceIndex = -1;
		string endPiece;
		for (int pieceIndex = piecesCount - 1; pieceIndex >= 0; pieceIndex--)
		{
			if (!SCR_StringHelper.StartsWithAny(pieces[pieceIndex], FORMAT_IGNORE))
			{
				endPiece = pieces[pieceIndex];
				endPieceIndex = pieceIndex;
				break;
			}
		}

		if (!endPiece.IsEmpty())
		{
			format = m_aGeneralFormatting_End;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					Print(FilePath.StripPath(__FILE__) + ":" + __LINE__ + " - safety exit! " + toReplace + "/" + replaceWith, LogLevel.WARNING);
					continue;
				}

				while (endPiece.EndsWith(toReplace))
				{
					endPiece = endPiece.Substring(0, endPiece.Length() - toReplace.Length()) + replaceWith;
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(endPieceIndex, endPiece);
					replacements++;
				}
			}
		}

		// replace a,b / a;b with a, b / a; b
		// replace 1/5 with 1 / 5 (+ - / *) - keep -1 but format 1-1 to 1 - 1
		for (int pieceIndex; pieceIndex < piecesCount; pieceIndex++)
		{
			string piece = pieces[pieceIndex];
			if (SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
				continue;

			for (int i, length = piece.Length(); i < length; i++)
			{
				// after this point is for "check after"
				if (i == length - 1)
					break;

				string prevChar;
				if (i > 0)
					prevChar = piece[i - 1];
				string character = piece[i];
				string nextChar;
				if (i < length - 1)
					nextChar = piece[i + 1];

				if ((character == "," || character == ";") && (nextChar != SPACE && nextChar != TAB && nextChar != "\"")) // tab = beware of line end + tab + comment
				{
					piece = SCR_StringHelper.InsertAt(piece, SPACE, i + 1);
					i++;
					length++;
					replacements++;
					continue;
				}

				// after this point is for "check before"
				if (i == 0)
					continue;

				if (character == "+" || character == "-" || character == "*" || character == "/")
				{
					if (SCR_StringHelper.DIGITS.Contains(prevChar))
					{
						piece = SCR_StringHelper.InsertAt(piece, SPACE, i);
						length++;
						replacements++;
						continue;
					}

					if (SCR_StringHelper.DIGITS.Contains(nextChar))
					{
						if (!(character == "-" && !(SCR_StringHelper.DIGITS.Contains(prevChar) || (i > 1 && SCR_StringHelper.DIGITS.Contains(piece[i - 2]))))) // exception for values like -1
						{
							piece = SCR_StringHelper.InsertAt(piece, SPACE, i + 1);
							i++;
							length++;
							replacements++;
							continue;
						}
					}
				}
			}

			pieces.Set(pieceIndex, piece);
		}

		return replacements;
	}

/*
	//------------------------------------------------------------------------------------------------
	protected void RemoveEmptyLinesInCurrentFile()
	{
		// remove double empty lines
		int actualLine;
		bool isCurrentLineEmpty;
		bool wasPreviousLineEmpty;
		for (int i, cnt = m_ScriptEditor.GetLinesCount(); i < cnt; i++)
		{
			m_ScriptEditor.GetLineText(fullLine, i);
			isCurrentLineEmpty = fullLine.IsEmpty();

			if (!isCurrentLineEmpty || !wasPreviousLineEmpty)
			{
				if (i != actualLine)
					m_ScriptEditor.SetLineText(fullLine, actualLine);
				actualLine++;
			}

			wasPreviousLineEmpty = isCurrentLineEmpty;
		}

		// Print("last line with actual content = " + actualLine, LogLevel.NORMAL);

		// delete trailing lines
		for (int cnt = m_ScriptEditor.GetLinesCount() - 1; actualLine <= cnt; cnt--)
		{
			m_ScriptEditor.RemoveLine(cnt);
		}

		// remove top empty lines
		m_ScriptEditor.GetLineText(fullLine, 0);
		for (int i = 0; i < 50; i++)
		{
			if (!fullLine.IsEmpty())
				break;

			m_ScriptEditor.RemoveLine(0);
			m_ScriptEditor.GetLineText(fullLine, 0);
		}

		// remove bottom empty lines
		for (int i; i < 500; i++)
		{
			m_ScriptEditor.RemoveLine(actualLine);
		}
	}
// */

	//------------------------------------------------------------------------------------------------
	//! Add the final line return to a file (to end with a line return instead of the usual closing bracket)
	//! \return true if an empty line has been added, false otherwise
	protected bool AddFinalLineReturnToCurrentFile()
	{
		int lastLineNumber = m_ScriptEditor.GetLinesCount() - 1;
		string fullLine;
		m_ScriptEditor.GetLineText(fullLine, lastLineNumber);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(fullLine))
			return false;

		if (m_bDemoMode)
			return true;

		// haxx
		m_ScriptEditor.InsertLine(string.Empty, lastLineNumber);
		m_ScriptEditor.SetLineText(fullLine, lastLineNumber);
		m_ScriptEditor.SetLineText(string.Empty, lastLineNumber + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Split line between indentation and content (trailing spaces included if any)
	//! \param fullLine the line's content
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] content gets the text
	protected static void GetIndentAndLineContent(string fullLine, out string indentation, out string content)
	{
		int lineLength = fullLine.Length();
		if (lineLength < 1)
		{
			indentation = string.Empty;
			content = string.Empty;
			return;
		}

		int firstCharIndex = -1;
		for (int i; i < lineLength; i++)
		{
			string character = fullLine[i];
			if (character != SCR_StringHelper.TAB && character != SCR_StringHelper.SPACE)
			{
				firstCharIndex = i;
				break;
			}
		}

		if (firstCharIndex < 0)
		{
			indentation = fullLine;
			content = string.Empty;
			return;
		}

		if (firstCharIndex == 0)
		{
			indentation = string.Empty;
			content = fullLine;
			return;
		}

		indentation = fullLine.Substring(0, firstCharIndex);
		content = fullLine.Substring(firstCharIndex, lineLength - firstCharIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Get line content as array of strings. Cannot return an empty array
	//! \param fullLine the line to analyse
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] pieces the line as split in code, string, comment parts - first element being indentation (empty when none). can be provided null, result is never null or empty
	static void GetIndentAndLineContentAsPieces(string fullLine, out string indentation, out array<string> pieces)
	{
		pieces = {};

		string content;
		GetIndentAndLineContent(fullLine, indentation, content);

		if (content.IsEmpty())
			return;

		bool isInCommentBlock;
		bool isInString;
		string currentContent;
		for (int i, contentLength = content.Length(); i < contentLength; i++)
		{
			string previousChar;
			if (i > 0)
				previousChar = content[i - 1];

			string currentChar = content[i];
			string nextChar;
			if (i < contentLength - 1)
				nextChar = content[i + 1];

			/*
				string management
			*/
			if (!isInCommentBlock && currentChar == "\"")
			{
				if (isInString)
				{
					if (previousChar != "\\") // not escaped
					{
						pieces.Insert(currentContent + currentChar);
						currentContent = string.Empty;
						isInString = false;
						continue;
					}
				}
				else
				{
					pieces.Insert(currentContent);
					currentContent = currentChar;
					isInString = true;
					continue;
				}
			}
			else
			/*
				comment management
			*/
			if (!isInString && currentChar == "/")
			{
				if (isInCommentBlock)
				{
					if (previousChar == "*")
					{
						pieces.Insert(currentContent + currentChar);
						currentContent = string.Empty;
						isInCommentBlock = false;
						continue;
					}
				}
				else
				{
					if (nextChar == "*")
					{
						pieces.Insert(currentContent);
						currentContent = currentChar;
						isInCommentBlock = true;
						continue;
					}
					else
					if (nextChar == "/" && !isInString) // the rest is comment
					{
						if (!currentContent.IsEmpty())
							pieces.Insert(currentContent);

						pieces.Insert(content.Substring(i, contentLength - i));
						return;
					}
				}
			}

			currentContent += currentChar;

			if (i == contentLength - 1 && !currentContent.IsEmpty())
				pieces.Insert(currentContent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks for bad prefixes and non-uppercased consts
	//! \param fullLine the line to check - indentation is used to determine the variable's level (1 tab = member variable)
	//! \return true if a badly-named variable has been spotted
	protected bool HasBadVariableNaming(string fullLine)
	{
		// only interested in one-tab variables :) (aka member variables)
		if (fullLine.Length() < 2 || fullLine[0] != TAB || fullLine[1] == TAB || fullLine.Contains("("))
			return false;

		bool isConst = fullLine.Contains("const ");
		if (isConst)
			fullLine = SCR_StringHelper.ReplaceTimes(fullLine, "const ", "", 1);

		string rest;
		if (fullLine.StartsWith("	protected ") || fullLine.StartsWith("	protected	")) // ending with space or tab
			rest = fullLine.Substring(11, fullLine.Length() - 11).Trim();
		else if (fullLine.StartsWith("	private ") || fullLine.StartsWith("	private	")) // ending with space or tab
			rest = fullLine.Substring(9, fullLine.Length() - 9).Trim();
		else
			rest = fullLine.Trim();

		// "protected static", not "static protected" you monsters
		string expectedPrefix = "m_";
		if (rest.StartsWith("static ") || rest.StartsWith("static	")) // ending with space or tab
		{
			rest = rest.Substring(7, rest.Length() - 7).Trim();
			expectedPrefix = "s_";
		}

		if (rest.StartsWith("ref ") || rest.StartsWith("ref	")) // ending with space or tab
			rest = rest.Substring(4, rest.Length() - 4).Trim();

		int index;
		if (isConst)
		{
			index = SCR_StringHelper.IndexOf(rest, VARIABLE_NAME_ENDING);
			if (index < 0)
				return false; // can't create a false positive on e.g protected string\n m_sValue;
			index++;
		}
		else
		{
			index = rest.IndexOf(expectedPrefix);
			if (index < 0)
				return false; // can't create a false positive on e.g protected string\n m_sValue;
		}

		if (index == 0)
			return false;

		string type = rest.Substring(0, index - 1);

		int indexEnd = SCR_StringHelper.IndexOfFrom(rest, index, VARIABLE_NAME_ENDING);
		if (indexEnd < 0)
			indexEnd = rest.Length(); // out of options

		string varName = rest.Substring(index, indexEnd - index);

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(varName))
			return false;

		if (isConst)
			return varName != SCR_StringHelper.Filter(varName, SCR_StringHelper.UPPERCASE + SCR_StringHelper.DIGITS + "_"); // varName is not in format [A-Z0-9_]+

		if (SCR_StringHelper.StartsWithAny(varName, m_aForbiddenPrefixes))
			return true;

		if (varName.Contains("[") && varName.Contains("]"))
			type = "array<x>"; // inner type is not important

		if (m_mVariableTypePrefixes.Contains(type))
		{
			expectedPrefix += m_mVariableTypePrefixes.Get(type);
			return !varName.StartsWith(expectedPrefix);
		}

		foreach (string key, string value : m_mVariableTypePrefixesStart)
		{
			if (type.StartsWith(key))
			{
				expectedPrefix += value;
				return !varName.StartsWith(expectedPrefix);
			}
		}

		// ends with?
		foreach (string key, string value : m_mVariableTypePrefixesEnd)
		{
			if (type.EndsWith(key))
			{
				expectedPrefix += value;
				return !varName.StartsWith(expectedPrefix);
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins the ints together with commas and ampersand for the last element
	//! examples:
	//! {} = ""
	//! { 1 } = "1"
	//! { 1, 2 } = "1 & 2"
	//! { 1, 2, 3 } = "1, 2 & 3"
	//! { 1, 2, 3, 4 } = "1, 2, 3 & 4"
	//! { 1, 2, ... , 10, 11 } = "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, ..."
	//! \param lineNumbers said line numbers
	//! \param maxNumbers maximum number of lines before ellipsis
	//! \return the joined numbers
	protected string JoinLineNumbers(notnull array<int> lineNumbers, int maxNumbers = LINE_NUMBER_LIMIT)
	{
		if (lineNumbers.IsEmpty())
			return string.Empty;

		string result = lineNumbers[0].ToString();
		int count = lineNumbers.Count();
		bool hitLimit = maxNumbers > -1 && count > maxNumbers;
		if (hitLimit)
			count = maxNumbers;

		for (int i = 1; i < count - 1; i++)
		{
			result += ", " + lineNumbers[i];
		}

		if (hitLimit)
			return result + ", " + lineNumbers[count - 1] + ", ...";

		if (count > 1)
			result += " & " + lineNumbers[count - 1];

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Print the finding with line numbers
	//! examples:
	//! "int(s)", {}, ""							= "No int(s) found"
	//! "int(s)", {}, "a tip"						= "No int(s) found"
	//! "int(s)", { 1, 2, 3 }, ""					= "3 int(s) found at line(s) 1, 2 & 3"
	//! "int(s)", { 1, 2, 3 }, "use longs instead"	= "3 int(s) found at line(s) 1, 2 & 3 - use longs instead"
	//! \param description the finding(s)' description
	//! \param lineNumbers the lines where the findings have been found
	//! \param tip a suggestion to fix the finding
	protected void PrintFinding(string description, notnull array<int> lineNumbers, string tip = string.Empty)
	{
		description.TrimInPlace();
		tip.TrimInPlace();
		if (lineNumbers.IsEmpty())
		{
			Print("No " + description + " found", LogLevel.NORMAL);
			return;
		}

		if (!tip.IsEmpty())
			tip = " - " + tip;

		Print(lineNumbers.Count().ToString() + "× " + description + " found at line(s) " + JoinLineNumbers(lineNumbers) + tip, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the 1-based line numbers of lines that were modified since the last edit (according to the local VCS' diff)
	//! This method creates a temporary txt file next to the analysed one containing the diff result
	//! The temporary txt file is deleted (if everything goes well) after its parsing
	//! \param absoluteFilePath the ABSOLUTE file path (e.g C:/ArmaReforger/Data/scripts/myFile.c)
	//! \return array of line numbers (first line = 1!), empty on no changes / new file, null on error
	protected array<int> GetFileModifiedLineNumbers(string absoluteFilePath)
	{
		if (!FileIO.FileExists(absoluteFilePath))
		{
			Print("Absolute file does not exist, skipping diff for \"" + absoluteFilePath + "\"", LogLevel.ERROR);
			return null;
		}

		string absoluteFileDirectory = FilePath.StripFileName(absoluteFilePath);
		if (!absoluteFileDirectory.EndsWith("/"))
			absoluteFileDirectory += "/";

		string absoluteCmdOutputFilePath = absoluteFileDirectory + DIFF_FILENAME;
		string commandLine = string.Format(m_sDiffCommand, absoluteFilePath, absoluteCmdOutputFilePath);

		int errorCode = Workbench.RunCmd(commandLine, true);
		if (errorCode != 0)
		{
			Print("Failed to obtain diff - the file may not be VCS-added yet or the command is wrong (error code " + errorCode + ")", LogLevel.ERROR);
			Print("command line = " + commandLine, LogLevel.ERROR);

			if (FileIO.FileExists(absoluteCmdOutputFilePath))
			{
				if (!FileIO.DeleteFile(absoluteCmdOutputFilePath))
					Print("Temp diff file could NOT be deleted", LogLevel.WARNING);
			}

			return null;
		}

		if (!FileIO.FileExists(absoluteCmdOutputFilePath))
		{
			Print("Temp diff file does not exist", LogLevel.ERROR);
			return null;
		}

		bool started;
		int lineNumber;
		array<int> result;
		FileHandle fileHandle = FileIO.OpenFile(absoluteCmdOutputFilePath, FileMode.READ);
		if (fileHandle)
		{
			result = {};

			while (!fileHandle.IsEOF())
			{
				string line;
				fileHandle.ReadLine(line);
				if (line.StartsWith("@@ -"))
				{
					started = true;
					int plusIndex = line.IndexOf("+") + 1; // note the +1
					int commaIndex = line.IndexOfFrom(plusIndex, ",");
					string diffLineNumber = line.Substring(plusIndex, commaIndex - plusIndex);
					lineNumber = diffLineNumber.ToInt();
					continue; // lineNumber--; // because it is announcing the *next* line
				}

				if (!started)
					continue;

				if (line.StartsWith("-"))
					continue;

				if (line.StartsWith("+"))
					result.Insert(lineNumber);

				lineNumber++;
			}

			fileHandle.Close();
		}
		else
		{
			Print("Could not open the temp diff file", LogLevel.ERROR);
		}

		if (!FileIO.DeleteFile(absoluteCmdOutputFilePath))
			Print("Temp diff file could NOT be deleted", LogLevel.WARNING); // we have our result, the file will unfortunately stay

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Basic Code formatter' plugin", "Formats code, basically.", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor
	protected void SCR_BasicCodeFormatterPlugin()
	{
		// line start
		m_aGeneralFormatting_Start = {};
		m_aGeneralFormatting_Start.Insert({ "if(",			"if (" });
		m_aGeneralFormatting_Start.Insert({ "else if(",		"else if (" });
		m_aGeneralFormatting_Start.Insert({ "for(",			"for (" });
		m_aGeneralFormatting_Start.Insert({ "foreach(",		"foreach (" });
		m_aGeneralFormatting_Start.Insert({ "while(",		"while (" });
		m_aGeneralFormatting_Start.Insert({ "switch(",		"switch (" });

		// line middle
		m_aGeneralFormatting_Middle = {};
		m_aGeneralFormatting_Middle.Insert({ TWO_SPACES,	SPACE });			// double spaces
		m_aGeneralFormatting_Middle.Insert({ ";;",			";" });				// double semi-colon (so, colon?)
		// m_aGeneralFormatting_Middle.Insert({ " , ",			", " });
		// m_aGeneralFormatting_Middle.Insert({ " ; ",			"; " });
		m_aGeneralFormatting_Middle.Insert({ " ,",			"," });
		m_aGeneralFormatting_Middle.Insert({ " ;",			";" });
		m_aGeneralFormatting_Middle.Insert({ "NULL",		"null" });
		m_aGeneralFormatting_Middle.Insert({ "{ }",			"{}" });
		m_aGeneralFormatting_Middle.Insert({ "array <",		"array<" });
		// m_aGeneralFormatting_Middle.Insert({ "autoptr ",	string.Empty });	// useless as all classes inherit from Managed and are therefore managed by ARC - WARNING - autoptr can be used as a substitute to ref!!
		m_aGeneralFormatting_Middle.Insert({ "( ",			"(" });
		m_aGeneralFormatting_Middle.Insert({ " )",			")" });

		// line end
		m_aGeneralFormatting_End = {};
		// m_aGeneralFormatting_End.Insert({ ")];",	")]" });
		m_aGeneralFormatting_End.Insert({ ", )]",	")]" });
		m_aGeneralFormatting_End.Insert({ ",)]",	")]" });
		// m_aGeneralFormatting_End.Insert({ "++i)",	"i++)" }); // veeery tempted
		// m_aGeneralFormatting_End.Insert({ "--i)",	"i--)" }); // to fix both :D

		m_aForbiddenDivisions = {};
		array<int> forbiddenDivisors = { 2, 4, 5, 8, 10, 20, 50, 100, 1000, 10000, 100000, 1000000 };
		foreach (int forbiddenDivisor : forbiddenDivisors)
		{
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + ";");
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + ")");
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + SPACE);
		}

		m_aForbiddenPrefixes = { "m_p", "s_p", "Event_" };	// :D
		m_aPrefixLineChecks = { "class ", "enum " };		//!< what should be followed by a TAG_ prefix

		m_aForForEachWhileArray = { "for ", "foreach ", "while " };
		m_aIfForForEachWhileArray = { "if ", "for ", "foreach ", "while " };
		m_aNewArrayNewRefArray = { "new array<", "new ref array<" };
		m_aScriptInvokerArray = { "ScriptInvoker()", "ScriptInvoker<", "ScriptInvoker\t", "ScriptInvoker " };
		m_aEndBracketSemicolonArray = { BRACKET_CLOSE, ";" };

		m_mVariableTypePrefixes = new map<string, string>();
		m_mVariableTypePrefixes.Insert("bool", "b");
		m_mVariableTypePrefixes.Insert("float", "f");
		m_mVariableTypePrefixes.Insert("int", "i");
		m_mVariableTypePrefixes.Insert("string", "s");
		m_mVariableTypePrefixes.Insert("ResourceName", "s");
		m_mVariableTypePrefixes.Insert("LocalizedString", "s");
		m_mVariableTypePrefixes.Insert("vector", "v");

		m_mVariableTypePrefixesStart = new map<string, string>();
		m_mVariableTypePrefixesStart.Insert("array<", "a");
		m_mVariableTypePrefixesStart.Insert("map<", "m");
		// enums - Q&D
		for (int i, count = SCR_StringHelper.UPPERCASE.Length(); i < count; i++)
		{
			m_mVariableTypePrefixesStart.Insert("E" + SCR_StringHelper.UPPERCASE[i], "e");
			m_mVariableTypePrefixesStart.Insert("SCR_E" + SCR_StringHelper.UPPERCASE[i], "e");
		}

		m_mVariableTypePrefixesEnd = new map<string, string>();
		m_mVariableTypePrefixesEnd.Insert("Widget", "w"); // will need more advanced checks (TextWidget, etc)
	}
}

//! databag
class SCR_BasicCodeFormatterPluginFileReport
{
	string m_sRelativeFilePath;
	bool m_bIsPluginFile;

	int m_iFormatTime = -1;
	int m_iDiffTime = -1;

	int m_iEndSpacesRemovedTotal;
	int m_iSpaceInTabsReplacedTotal;
	int m_iFourSpacesReplacedTotal;
	int m_iMethodSeparatorFixedTotal;
	int m_iGeneralFormattingTotal;
	int m_iLinesEdited;

	// fixes
	ref array<int> m_aDiffLines;		// CAN be null
	ref array<int> m_aTrimmings = {};	// content is actually not used, only count... for now?
	ref array<int> m_aFixedIndentations = {};
	bool m_bHasAddedFinalLineReturn;

	// reports
	ref array<int> m_aDoubleEmptyLineFound = {};
	ref array<int> m_aNewRefFound = {};
	ref array<int> m_aNewArrayFound = {};
	ref array<int> m_aAutoKeywordFound = {};
	ref array<int> m_aAutoptrKeywordFound = {};
	ref array<int> m_aDivideByXFound = {};
	ref array<int> m_aUnscopedLoopFound = {};
	ref array<int> m_aOneLinerFound = {};
	ref array<int> m_aBadVariableNamingFound = {};
	ref array<int> m_aBadScriptInvokerFound = {};
	ref array<int> m_aWildPrintFound = {};
	ref array<int> m_aNonPrefixedClassOrEnumFound = {};

	//------------------------------------------------------------------------------------------------
	//! does the report have anything... to report
	bool IsClean()
	{
		return
			m_iLinesEdited == 0 &&
			m_aDoubleEmptyLineFound.IsEmpty() &&
			m_aNewRefFound.IsEmpty() &&
			m_aNewArrayFound.IsEmpty() &&
			m_aAutoKeywordFound.IsEmpty() &&
			m_aAutoptrKeywordFound.IsEmpty() &&
			m_aDivideByXFound.IsEmpty() &&
			m_aUnscopedLoopFound.IsEmpty() &&
			m_aOneLinerFound.IsEmpty() &&
			m_aBadVariableNamingFound.IsEmpty() &&
			m_aBadScriptInvokerFound.IsEmpty() &&
			m_aWildPrintFound.IsEmpty() &&
			m_aNonPrefixedClassOrEnumFound.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_BasicCodeFormatterPluginFileReport(string relativeFilePath, bool isPluginFile)
	{
		m_sRelativeFilePath = relativeFilePath;
		m_bIsPluginFile = isPluginFile; // check not done here in case this class gets moved away from the plugin file
	}
}

class SCR_BasicCodeFormatterPluginOKCancelDialog
{
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif
