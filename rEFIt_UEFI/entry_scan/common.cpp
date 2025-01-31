/*
 * refit/scan/common.c
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "entry_scan.h"
#include "../refit/menu.h"
#include "../Platform/guid.h"
#include "../Platform/APFS.h"
#include "../Platform/cpu.h"
#include "../gui/REFIT_MENU_SCREEN.h"
#include "../include/OSTypes.h"
#include "../libeg/XTheme.h"

#ifndef DEBUG_ALL
#define DEBUG_COMMON_MENU 1
#else
#define DEBUG_COMMON_MENU DEBUG_ALL
#endif

#if DEBUG_COMMON_MENU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_COMMON_MENU, __VA_ARGS__)
#endif

extern CONST CHAR8* IconsNames[];

const XIcon& ScanVolumeDefaultIcon(REFIT_VOLUME *Volume, IN UINT8 OSType, const EFI_DEVICE_PATH_PROTOCOL *DevicePath)

{
  UINTN IconNum = 0;
  const XIcon* IconX;
  // default volume icon based on disk kind
  switch (Volume->DiskKind) {
    case DISK_KIND_INTERNAL:
      switch (OSType) {
        case OSTYPE_OSX:
        case OSTYPE_OSX_INSTALLER:
          while (!IsDevicePathEndType(DevicePath) &&
            !(DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP)) {
            DevicePath = NextDevicePathNode(DevicePath);
          }
          if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP) {
            if ( ApfsSignatureUUID == *(EFI_GUID *)((UINT8 *)DevicePath+0x04) ) {
              IconNum = BUILTIN_ICON_VOL_INTERNAL_APFS;
            }
          } else {
            IconNum = BUILTIN_ICON_VOL_INTERNAL_HFS;
          }
          break;
        case OSTYPE_RECOVERY:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_REC;
          break;
        case OSTYPE_LIN:
        case OSTYPE_LINEFI:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_EXT3;
          break;
        case OSTYPE_WIN:
        case OSTYPE_WINEFI:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_NTFS;
          break;
        default:
          IconNum = BUILTIN_ICON_VOL_INTERNAL;
          break;
      }
      break;
    case DISK_KIND_EXTERNAL:
      IconNum = BUILTIN_ICON_VOL_EXTERNAL;
      break;
    case DISK_KIND_OPTICAL:
      IconNum = BUILTIN_ICON_VOL_OPTICAL;
      break;
    case DISK_KIND_FIREWIRE:
      IconNum = BUILTIN_ICON_VOL_FIREWIRE;
      break;
    case DISK_KIND_BOOTER:
      IconNum = BUILTIN_ICON_VOL_BOOTER;
      break;
    default:
      IconNum = BUILTIN_ICON_VOL_INTERNAL;
      break;
  }
//  DBG("asked IconNum = %llu Volume->DiskKind=%d OSType=%d\n", IconNum, Volume->DiskKind, OSType);
  IconX = &ThemeX->GetIcon(IconNum); //asked IconNum = BUILTIN_ICON_VOL_INTERNAL_HFS, got day icon
  if (IconX->Image.isEmpty()) {
    DBG("asked Icon %s not found, took internal\n", IconsNames[IconNum]);
    IconX = &ThemeX->GetIcon(BUILTIN_ICON_VOL_INTERNAL); //including embedded which is really present
  }

  return *IconX;
}

#define TO_LOWER(ch) (((ch >= L'A') && (ch <= L'Z')) ? ((ch - L'A') + L'a') : ch)
INTN StrniCmp(IN CONST CHAR16 *Str1,
              IN CONST CHAR16 *Str2,
              IN UINTN   Count)
{
  CHAR16 Ch1, Ch2;
  if (Count == 0) {
    return 0;
  }
  if (Str1 == NULL) {
    if (Str2 == NULL) {
      return 0;
    } else {
      return -1;
    }
  } else  if (Str2 == NULL) {
    return 1;
  }
  do {
    Ch1 = TO_LOWER(*Str1);
    Ch2 = TO_LOWER(*Str2);
    Str1++;
    Str2++;
    if (Ch1 != Ch2) {
      return (Ch1 - Ch2);
    }
    if (Ch1 == 0) {
      return 0;
    }
  } while (--Count > 0);
  return 0;
}

CONST CHAR16 *StriStr(IN CONST CHAR16 *Str,
                IN CONST CHAR16 *SearchFor)
{
  CONST CHAR16 *End;
  UINTN   Length;
  UINTN   SearchLength;
  if ((Str == NULL) || (SearchFor == NULL)) {
    return NULL;
  }
  Length = StrLen(Str);
  if (Length == 0) {
    return NULL;
  }
  SearchLength = StrLen(SearchFor);
  if (SearchLength > Length) {
    return NULL;
  }
  End = Str + (Length - SearchLength) + 1;
  while (Str < End) {
    if (StrniCmp(Str, SearchFor, SearchLength) == 0) {
      return Str;
    }
    ++Str;
  }
  return NULL;
}

void StrToLower(IN CHAR16 *Str)
{
   while (*Str) {
     *Str = TO_LOWER(*Str);
     ++Str;
   }
}

// TODO remove that and AlertMessage with a printf-like format

STATIC void CreateInfoLines(IN CONST XStringW& Message, OUT XStringWArray* Information)
{
  if (Message.isEmpty()) {
    return;
  }
  Information->setEmpty();
  //TODO will fill later
}

#if 0 //not needed?
STATIC void CreateInfoLines(IN CONST CHAR16 *Message, OUT XStringWArray* Information)
{
  CONST CHAR16 *Ptr;
//  CHAR16 **Information;
  UINTN   Index = 0, Total = 0;
  UINTN   Length = ((Message == NULL) ? 0 : StrLen(Message));
  // Check parameters
  if (Length == 0) {
    return;
  }
  // Count how many new lines
  Ptr = Message - 1;
  while (Ptr != NULL) {
    ++Total;
    Ptr = StrStr(++Ptr, L"\n");
  }
//  // Create information
//  Information = (CHAR16 **)AllocatePool((Total * sizeof(CHAR16 *)) + ((Length + 1) * sizeof(CHAR16)));
//  if (Information == NULL) {
//    return NULL;
//  }
  Information->Empty();
  // Copy strings
  CHAR16* Ptr2 = NULL; // VS2017 complains about uninitialized var.
//  CHAR16* Ptr2 = Information[Index++] = (CHAR16 *)(Information + Total);
//  StrCpyS(Ptr2, Length + 1, Message);
  while ((Index < Total) &&
         ((Ptr2 = (CHAR16*)StrStr(Ptr2, L"\n")) != NULL)) { // cast is ok because FilePath is not const, and we know that StrStr returns a pointer in FilePath. Will disappear when using a string object instead of CHAR16*
    *Ptr2++ = 0;
    XStringW* s = new XStringW;
    s->takeValueFrom(Ptr2);
    Information->AddReference(s, true);
//    Information[Index++] = Ptr2;
  }
//  // Return the info lines
//  if (Count != NULL) {
//    *Count = Total;
//  }
//  return Information;
}
#endif
extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;


// it is not good to use Options menu style for messages and one line dialogs
// it can be a semitransparent rectangular at the screen centre as it was in Clover v1.0
STATIC REFIT_MENU_SCREEN  AlertMessageMenu(0, XStringW(), XStringW(), &MenuEntryReturn, NULL);
void AlertMessage(const XStringW& Title, const XStringW& Message)
{
  CreateInfoLines(Message, &AlertMessageMenu.InfoLines);
  AlertMessageMenu.Title = Title;
  AlertMessageMenu.RunMenu(NULL);
  AlertMessageMenu.InfoLines.setEmpty();
}

#define TAG_YES 1
#define TAG_NO  2

//REFIT_SIMPLE_MENU_ENTRY_TAG(CONST CHAR16 *Title_, UINTN Tag_, ACTION AtClick_)
STATIC REFIT_SIMPLE_MENU_ENTRY_TAG   YesMessageEntry(XStringW().takeValueFrom(L"Yes"), TAG_YES, ActionEnter);
STATIC REFIT_SIMPLE_MENU_ENTRY_TAG   NoMessageEntry(XStringW().takeValueFrom(L"No"), TAG_NO, ActionEnter);

//REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2)
STATIC REFIT_MENU_SCREEN  YesNoMessageMenu(0, XStringW(), XStringW(), &YesMessageEntry, &NoMessageEntry);
// Display a yes/no prompt
XBool YesNoMessage(IN XStringW& Title, IN CONST XStringW& Message)
{
  XBool              Result = false;
  UINTN              MenuExit;
  CreateInfoLines(Message, &YesNoMessageMenu.InfoLines);
  YesNoMessageMenu.Title = Title;
  do
  {
    REFIT_ABSTRACT_MENU_ENTRY  *ChosenEntry = NULL;
    MenuExit = YesNoMessageMenu.RunMenu(&ChosenEntry);
    if ( ChosenEntry != NULL  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag == TAG_YES  &&
        ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
      Result = true;
      MenuExit = MENU_EXIT_ENTER;
    }
  } while (MenuExit != MENU_EXIT_ENTER);
  YesNoMessageMenu.InfoLines.setEmpty();
  return Result;
}
// Ask user for file path from directory menu
XBool AskUserForFilePathFromDir(const CHAR16 *Title OPTIONAL, IN REFIT_VOLUME *Volume,
                                  const CHAR16 *ParentPath OPTIONAL, const EFI_FILE *Dir,
                                  OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  //REFIT_MENU_SCREEN   Menu = { 0, L"Please Select File...", NULL, 0, NULL, 0, NULL,
  //                             0, NULL, false, false, 0, 0, 0, 0, { 0, 0, 0, 0 }, NULL};
  // Check parameters
  if ((Volume == NULL) || (Dir == NULL) || (Result == NULL)) {
    return false;
  }
  // TODO: Generate directory menu
  return false;
}

#define TAG_OFFSET 1000

//STATIC REFIT_MENU_SCREEN InitialMenu = {0, L"Please Select File...", NULL, 0, NULL,
//  0, NULL, NULL, false, false, 0, 0, 0, 0,
//  { 0, 0, 0, 0 }, NULL};
//STATIC REFIT_MENU_SCREEN  InitialMenu(0, L"Please Select File..."_XSW, XStringW());
// Ask user for file path from volumes menu
XBool AskUserForFilePathFromVolumes(const CHAR16 *Title OPTIONAL, OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  REFIT_MENU_SCREEN   Menu(0, L"Please Select File..."_XSW, XStringW());
  UINTN               Index = 0, /*Count = 0,*/ MenuExit;
  XBool               Responded = false;
  if (Result == NULL) {
    return false;
  }
  // Create volume entries
  for (Index = 0; Index < Volumes.size(); ++Index) {
    REFIT_VOLUME     *Volume = &Volumes[Index];
    if ((Volume == NULL) || (Volume->RootDir == NULL) ||
        ((Volume->DevicePathString.isEmpty()) && (Volume->VolName.isEmpty()))) {
      continue;
    }
	  REFIT_SIMPLE_MENU_ENTRY_TAG *Entry = new REFIT_SIMPLE_MENU_ENTRY_TAG(SWPrintf("%ls", (Volume->VolName.isEmpty()) ? Volume->DevicePathString.wc_str() : Volume->VolName.wc_str()), TAG_OFFSET + Index, MENU_EXIT_ENTER);
    Menu.Entries.AddReference(Entry, true);
  }
  // Setup menu
  Menu.Entries.AddReference(&MenuEntryReturn, false);
  Menu.Title.takeValueFrom(Title);

  do
  {
    REFIT_ABSTRACT_MENU_ENTRY *ChosenEntry = NULL;
    // Run the volume chooser menu
    MenuExit = Menu.RunMenu(&ChosenEntry);
    if ((ChosenEntry != NULL) && ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG() &&
        ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
      if (ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag >= TAG_OFFSET) {
        Index = (ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag - TAG_OFFSET);
        if (Index < Volumes.size()) {
          // Run directory chooser menu
          if (!AskUserForFilePathFromDir(Title, &Volumes[Index], NULL, Volumes[Index].RootDir, Result)) {
            continue;
          }
          Responded = true;
        }
      }
      break;
    }
  } while (MenuExit != MENU_EXIT_ESCAPE);
//  FreePool(Entries);
  return Responded;
}

// Ask user for file path
XBool AskUserForFilePath(IN CHAR16 *Title OPTIONAL, IN EFI_DEVICE_PATH_PROTOCOL *Root OPTIONAL, OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  EFI_FILE *Dir = NULL;
  if (Result == NULL) {
    return false;
  }
  if (Root != NULL) {
    // Get the file path
    XStringW DevicePathStr = FileDevicePathToXStringW(Root);
    if (DevicePathStr.notEmpty()) {
      UINTN Index = 0;
      // Check the volumes for a match
      for (Index = 0; Index < Volumes.size(); ++Index) {
        REFIT_VOLUME *Volume = &Volumes[Index];
        if ((Volume == NULL) || (Volume->RootDir == NULL) ||
            (Volume->DevicePathString.isEmpty())) {
          continue;
        }
        if (Volume->DevicePathString.length() == 0) {
          continue;
        }
        // If the path begins with this volumes path it matches
        if (StrniCmp(DevicePathStr.wc_str(), Volume->DevicePathString.wc_str(), Volume->DevicePathString.length())) {
          // Need to
          const CHAR16 *FilePath = DevicePathStr.data(Volume->DevicePathString.length());
          UINTN   FileLength = StrLen(FilePath);
          if (FileLength == 0) {
            // If there is no path left then open the root
            return AskUserForFilePathFromDir(Title, Volume, NULL, Volume->RootDir, Result);
          } else {
            // Check to make sure this is directory
            if (!EFI_ERROR(Volume->RootDir->Open(Volume->RootDir, &Dir, FilePath, EFI_FILE_MODE_READ, 0)) &&
                (Dir != NULL)) {
              // Get file information
              EFI_FILE_INFO *Info = EfiLibFileInfo(Dir);
              if (Info != NULL) {
                // Check if the file is a directory
                if ((Info->Attribute & EFI_FILE_DIRECTORY) == 0) {
                  // Return the passed device path if it is a file
                  FreePool(Info);
                  Dir->Close(Dir);
                  *Result = Root;
                  return true;
                } else {
                  // Ask user other wise
                  XBool Success = AskUserForFilePathFromDir(Title, Volume, FilePath, Dir, Result);
                  FreePool(Info);
                  Dir->Close(Dir);
                  return Success;
                }
                //FreePool(Info);
              }
              Dir->Close(Dir);
            }
          }
        }
      }
    }
  }
  return AskUserForFilePathFromVolumes(Title, Result);
}

// input - tsc
// output - milliseconds
// the caller is responsible for t1 > t0
UINT64 TimeDiff(UINT64 t0, UINT64 t1)
{
  return DivU64x64Remainder((t1 - t0), DivU64x32(gCPUStructure.TSCFrequency, 1000), 0);
}


