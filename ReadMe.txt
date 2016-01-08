BrowseFolder (C) res2001, 2016

Displays a standard browse folder dialog.
If successful full path to the selected folder is output to stdout and exit with
a return code of 0. If the user select Cancel - nothing is output and exit with 
a return code 14.

BrpwseFolder.exe [/Help] [/Description:<description>] [/Title:<title>] 
                 [/Flag:<flags>] [/Block:<block window>] 
                 [/Center:<window for centering>] [/CurPath:<folder>] 
                 [/RootPath:<root folder>]

Parameters:
/Help                          - help screen.
/Description:<description>     - dialog description. Default - empty.
/Title:<title>                 - dialog title. Default - "Browse folder"
/Block:<block window>          - indicates which window is blocked while 
                                 displaying the browse folder dialog.
                                 Default - window.
  <block window>               - possible values:
                                   none   - do not block the windows.
                                   window - block the console window from
                                            which you launched BrowseFolder.exe.
                                   <window title> - blocked the window with the 
                                            given title.
/Center:<window for centering> - specifies the window about which centered the 
                                 BrowseFolder dialog box.
                                 Default - window.
  <window for centering>       - possible values:
                                   none	  - not to be centered. The BrowseFolder
                                            dialog window that appears is shown
                                            in the coordinates of the specified
                                            OS.
                                   window - center on the console window from 
                                            which you launched BrowseFolder.exe.
                                   screen - center on desktop Windows.
                                   <window title> - to be centered relative to 
                                            the window with the specified title.
/CurPath:<folder>              - the path to the folder to be selected when 
                                 initializing the dialog.
                                 Default - "My computer".
  <folder>                     - possible values: are the same as for the /RootPath.
/RootPath:<root folder>        - the folder above which will not be possible to
                                 climb when selecting folders.
                                 Default - empty.
  <root folder>                - possible values:
                                   desktop      - %USERPROFILE%\Desktop
                                   computer     - "My computer"
                                   favorites    - %USERPROFILE%\Favorites
                                   appdata      - %USERPROFILE%\AppData\Roaming
                                   localappdata - %USERPROFILE%\AppData\Local
                                   documents    - %USERPROFILE%\Documents
                                   music        - %USERPROFILE%\Music
                                   pictures     - %USERPROFILE%\Pictures
                                   video        - %USERPROFILE%\Videos
                                   network      - "Network neighbourhood"
                                   profile      - %USERPROFILE%
                                   programfiles - %ProgramFiles%
                                   windows      - %SystemRoot%
                                   system       - %SystemRoot%\System32
                                   <folder>     - any file system folder
/Flag:<flags>                  - standard flags of the dialog BrowseFolder.
                                 Default - BIF_RETURNONLYFSDIRS + BIF_USENEWUI 
                                           + BIF_NONEWFOLDERBUTTON.
                                 Detailed description of the flags look (field
                                 ulFlag):
                https://msdn.microsoft.com/library/windows/desktop/bb773205.aspx
  <flags>                     - possible values (in parentheses are the decimal
                                value of the flag):

                                BIF_RETURNONLYFSDIRS (1) - Only return file
                                system directories.

                                BIF_STATUSTEXT (4) - Include a status area in
                                the dialog box. The callback function can set
                                the status text by sending messages to the
                                dialog box. This flag is not supported when

                                BIF_NEWDIALOGSTYLE is specified.

                                BIF_EDITBOX (16) - Include an edit control in
                                the browse dialog box that allows the user to
                                type the name of an item.

                                BIF_NEWDIALOGSTYLE (64)	- Use the new user 
                                interface. Setting this flag provides the user
                                with a larger dialog box that can be resized. 
                                The dialog box has several new capabilities,
                                including: drag-and-drop capability within the
                                dialog box, reordering, shortcut menus, new 
                                folders, delete, and other shortcut menu 
                                commands.

                                BIF_USENEWUI (80) - BIF_EDITBOX 
                                                    + BIF_NEWDIALOGSTYLE

                                BIF_NONEWFOLDERBUTTON (512) - Do not include
                                the New Folder button in the browse dialog box.

                                BIF_BROWSEINCLUDEFILES (16384) - The browse 
                                dialog box displays files as well as folders.

                                BIF_SHAREABLE (32768) - The browse dialog box
                                can display sharable resources on remote
                                systems. This is intended for applications that
                                want to expose remote shares on a local system.
                                The BIF_NEWDIALOGSTYLE flag must also be set.

                                Also available flags: BIF_DONTGOBELOWDOMAIN (2), 
                                  BIF_RETURNFSANCESTORS (8), BIF_VALIDATE (32), 
				  BIF_BROWSEINCLUDEURLS (128), BIF_UAHINT (256),
                                  BIF_NOTRANSLATETARGETS (1024), 
                                  BIF_BROWSEFORCOMPUTER (4096), 
                                  BIF_BROWSEFORPRINTER (8192), 
                                  BIF_BROWSEFILEJUNCTIONS (65536). 
                                A description of these flags,see the link above.
                                Specify the /Flag parameter value is the sum of
                                the decimal values of the flags.

                                For example, the default combinations of flags:
                                  BIF_RETURNONLYFSDIRS + BIF_USENEWUI 
                                  + BIF_NONEWFOLDERBUTTON
                                equal 1 + 16 + 64 + 512 = 593. 
                                To specify this value, use the following 
                                recording parameters: /Flag:593

Error codes:
0 - successful completion of the program. Output to stdout the full path of the
    selected folder.
1 - it is not possible to obtain command line parameters.
2 - it is not possible to create an object BrowseFolderDialog.
3-8 - not the correct parameter values.
10 - not the right parameter.
12 - error OleInitialize.
14 - the user refused to select a folder.
15 - conversion error of the selected folder.
The error codes are accompanied by text messages that appear in stderr.

Versions:
05.01.2016	- 1.0.0
