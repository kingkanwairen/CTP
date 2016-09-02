; �ű��� Inno Setup �ű��� ���ɣ�
; �йش��� Inno Setup �ű��ļ�����ϸ��������İ����ĵ���
;#include "compiler:FlashLib.iss"

#define MyAppName "ArbitrageTrading"
#define MyInstallApp "����ArbitrageTrading"
#define MyUnInstallApp "ж��ArbitrageTrading"
#define AboutArbitrageTrading "����ArbitrageTrading"
#define MyAppVersion "1.0"
#define MyAppPublisher "KING"
#define MyAppURL "http://www.nanhua.net/"
#define MyAppExeName "ArbitrageTrading.exe"

[Setup]
; ע: AppId��ֵΪ������ʶ��Ӧ�ó���
; ��ҪΪ������װ����ʹ����ͬ��AppIdֵ��
; (�����µ�GUID����� ����|��IDE������GUID��)
AppId={{9E45CA97-F90D-4E42-B635-C73DDB37E887}
AppName={#MyAppName}
AppVersion=1.0.0.1
AppCopyright=?1996-2015 NANHUA
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
;����ļ���
OutputDir=ArbitrageTrading
;��װ����ļ���
OutputBaseFilename=setup
;��װͼ��
;SetupIconFile=D:\Testfg\GPM20101029\systex.ico
;��װ��������
;Password=111111
;Encryption=yes

;ѹ�����
Compression=lzma
SolidCompression=yes
;��װǰ�鿴���ı��ļ�
InfoBeforeFile=E:\ArbitrageTrading\readme.txt
 ; ��ע�汾��Ϣ
VersionInfoCompany={#MyAppURL}
VersionInfoDescription=ArbitrageTrading������ǿ��
VersionInfoVersion=1.0.0.1
VersionInfoCopyright=Copyright(C) 1996-2015 NANHUA

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]

;��ִ���ļ�
Source: "E:\ArbitrageTrading\bin\ArbitrageTrading.exe"; DestDir: "{app}\bin"; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\Microsoft.VC80.CRT.manifest"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\msvcm80.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\msvcp80.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\msvcr80.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\QtCore4.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\QtGui4.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion

Source: "E:\ArbitrageTrading\bin\QtXml4.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\USTPmduserapi.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\bin\USTPtraderapi.dll"; DestDir: "{app}\bin" ; Flags:ignoreversion


;ͼƬ
Source: "E:\ArbitrageTrading\image\background.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\save.png"; DestDir: "{app}\image"; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\event.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\exit.png"; DestDir: "{app}\image"; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\green.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\md.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\red.png"; DestDir: "{app}\image"  ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\subpic.png"; DestDir: "{app}\image"; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\title.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\subtab.png"; DestDir: "{app}\image" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\image\tab.png"; DestDir: "{app}\image" ; Flags:ignoreversion

;����
Source: "E:\ArbitrageTrading\config\config.xml"; DestDir: "{app}\config" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\config\param.txt"; DestDir: "{app}\config" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\config\license.txt"; DestDir: "{app}\config" ; Flags:ignoreversion
Source: "E:\ArbitrageTrading\log\trade.txt"; DestDir: "{app}\log" ; Flags:ignoreversion

[Icons]
Name: "{group}\{#MyInstallApp}"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{group}\{#MyUnInstallApp}"; Filename: "{app}\unins000.exe"
Name: "{group}\{#AboutArbitrageTrading}"; Filename: "{app}\readme.txt"
Name: "{commondesktop}\{#MyInstallApp}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon
;Name: "{commondesktop}\{#MyUnInstallApp}"; Filename: "{app}\unins000.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent


