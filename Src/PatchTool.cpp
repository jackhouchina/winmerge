/////////////////////////////////////////////////////////////////////////////
//    License (GPLv2+):
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or (at
//    your option) any later version.
//    
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
/////////////////////////////////////////////////////////////////////////////
/** 
 * @file  files.cpp
 *
 * @brief Code file routines
 */
// RCS ID line follows -- this is updated by CVS
// $Id$

#include "stdafx.h"
#include "DiffWrapper.h"
#include "patchDlg.h"
#include "patchtool.h"

/** 
 * @brief Adds files to list for patching
 */
void CPatchTool::AddFiles(CString file1, CString file2)
{
	PATCHFILES files;
	files.lfile = file1;
	files.rfile = file2;

	// TODO: Read and add file's timestamps
	m_fileList.AddTail(files);
}

/** 
 * @brief Create patch from files given
 * @note Files can be given using AddFiles() or selecting using
 * CPatchDlg.
 */
int CPatchTool::CreatePatch()
{
	DIFFSTATUS status = {0};
	BOOL bResult = TRUE;
	BOOL bDiffSuccess;

	// If files already inserted, add them to dialog
	int count = m_fileList.GetCount();
	POSITION pos = m_fileList.GetHeadPosition();

	for (int i = 0; i < count; i++)
	{
		PATCHFILES files = m_fileList.GetNext(pos);
		m_dlgPatch.AddItem(files);
	}

	if (ShowDialog())
	{
		// Select patch create -mode
		m_diffWrapper.SetUseDiffList(FALSE);
		m_diffWrapper.SetCreatePatchFile(TRUE);
		m_diffWrapper.SetAppendFiles(m_dlgPatch.m_appendFile);

		int fileCount = m_dlgPatch.GetItemCount();
		POSITION pos = m_dlgPatch.GetFirstItem();

		for (int i = 0; i < fileCount; i++)
		{
			PATCHFILES files = m_dlgPatch.GetNextItem(pos);
			CString filename1 = files.lfile;
			CString filename2 = files.rfile;
			
			// Set up DiffWrapper
			m_diffWrapper.SetCompareFiles(filename1, filename2);
			bDiffSuccess = m_diffWrapper.RunFileDiff();
			m_diffWrapper.GetDiffStatus(&status);

			if (!bDiffSuccess || status.bPatchFileFailed)
			{
				bResult = FALSE;
				CString errMsg;
				AfxFormatString1(errMsg, IDS_FILEWRITE_ERROR, m_dlgPatch.m_fileResult);
				AfxMessageBox(errMsg, MB_ICONSTOP);
				break;
			}

			// Append next files...
			m_diffWrapper.SetAppendFiles(TRUE);
		}
		if (bResult && fileCount > 0)
			AfxMessageBox(IDS_DIFF_SUCCEEDED, MB_ICONINFORMATION);
	}
	m_dlgPatch.ClearItems();
	return 1;
}

/** 
 * @brief Show patch options dialog and check options selected
 */
BOOL CPatchTool::ShowDialog()
{
	DIFFOPTIONS diffOptions = {0};
	PATCHOPTIONS patchOptions;
	BOOL bRetVal = TRUE;

	if (m_dlgPatch.DoModal() == IDOK)
	{
		// There must be one filepair
		if (m_dlgPatch.GetItemCount() < 1)
			bRetVal = FALSE;

		// Result file must be given
		if (m_dlgPatch.m_fileResult.IsEmpty())
			bRetVal = FALSE;
		else
			m_diffWrapper.SetPatchFile(m_dlgPatch.m_fileResult);

		// These two are from dropdown list - can't be wrong
		patchOptions.outputStyle = m_dlgPatch.m_outputStyle;
		patchOptions.context = m_dlgPatch.m_contextLines;
		m_diffWrapper.SetPatchOptions(&patchOptions);

		// These are from checkboxes and radiobuttons - can't be wrong
		diffOptions.nIgnoreWhitespace = m_dlgPatch.m_whitespaceCompare;
		diffOptions.bIgnoreBlankLines = m_dlgPatch.m_ignoreBlanks;
		m_diffWrapper.SetAppendFiles(m_dlgPatch.m_appendFile);

		// Use this because non-sensitive setting can't write
		// patch file EOLs correctly
		diffOptions.bEolSensitive = TRUE;
		
		diffOptions.bIgnoreCase = !m_dlgPatch.m_caseSensitive;
		m_diffWrapper.SetOptions(&diffOptions);
	}
	else
		return FALSE;

	return bRetVal;
}