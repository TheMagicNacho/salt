#ifndef SALT_LIB_CONTEXT_H_
#define SALT_LIB_CONTEXT_H_

#include <windows.h>

/// @file context.h
/// @brief Command identifiers and child control constants for the Salt editor.

/// @class Context
/// @brief Scoped container for application command and control IDs.
class Context {
   public:
    /// @enum Command
    /// @brief Menu command IDs and accelerator action identifiers.
    enum Command : WORD {
        // File commands
        FileNew = 1001,     ///< Create a new document.
        FileOpen = 1002,    ///< Open an existing file.
        FileSave = 1003,    ///< Save current document.
        FileSaveAs = 1004,  ///< Save current document with a new name/location.
        FilePrint = 1005,   ///< Print document.
        FileExit = 1006,    ///< Close and exit application.

        // Edit commands
        EditUndo = 1011,       ///< Undo last edit.
        EditCut = 1012,        ///< Cut selection to clipboard.
        EditCopy = 1013,       ///< Copy selection to clipboard.
        EditPaste = 1014,      ///< Paste clipboard text.
        EditSelectAll = 1015,  ///< Select all text.

        // View commands
        ViewWordWrap = 1021,     ///< Toggle word wrapping.
        ViewFont = 1022,         ///< Open font chooser dialog.
        ViewZoomIn = 1023,       ///< Increase font size.
        ViewZoomOut = 1024,      ///< Decrease font size.
        ViewZoomReset = 1025,    ///< Reset zoom to default.
        ViewThemeToggle = 1026,  ///< Toggle between dark and light themes.
        ViewStatusBar = 1027,    ///< Toggle status bar visibility.

        // Help commands
        HelpAbout = 1031,  ///< Show About dialog.
    };

    /// @enum ControlId
    /// @brief Control IDs for child Win32 windows.
    enum ControlId : WORD {
        MainEdit = 2001,    ///< Primary multiline edit control ID.
        MainStatus = 2002,  ///< Bottom status bar control ID.
    };
};

#endif  // SALT_LIB_CONTEXT_H_
