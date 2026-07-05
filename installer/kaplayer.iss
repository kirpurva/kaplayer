; Kaplayer installer script (Inno Setup 6)
;
; Build:
;   1. Release build + windeployqt into a staging folder (see README)
;   2. ISCC.exe installer\kaplayer.iss /DSourceDir=<staging-folder>
;
; SourceDir must contain kaplayer.exe and its deployed DLLs/plugins.

#ifndef SourceDir
#define SourceDir "C:\Users\kongk\kaplayer-dist\kaplayer-v1.0.0"
#endif
#define AppVersion "1.0.0"

[Setup]
AppId={{9F1B7A52-3C64-4E0D-9B57-2B4A31D9E7C1}
AppName=Kaplayer
AppVersion={#AppVersion}
AppPublisher=Kirti Raj
AppPublisherURL=https://github.com/kirpurva/kaplayer
DefaultDirName={autopf}\Kaplayer
DefaultGroupName=Kaplayer
UninstallDisplayIcon={app}\kaplayer.exe
OutputBaseFilename=kaplayer-{#AppVersion}-setup
SetupIconFile=..\icons\kaplayer.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Tasks]
Name: desktopicon; Description: "Create a &desktop shortcut"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion

[Icons]
Name: "{group}\Kaplayer"; Filename: "{app}\kaplayer.exe"
Name: "{autodesktop}\Kaplayer"; Filename: "{app}\kaplayer.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\kaplayer.exe"; Description: "Launch Kaplayer"; Flags: nowait postinstall skipifsilent
