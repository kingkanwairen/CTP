; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！
;#include "compiler:FlashLib.iss"

#define MyAppName "ArbitrageTrading"
#define MyInstallApp "启动ArbitrageTrading"
#define MyUnInstallApp "卸载ArbitrageTrading"
#define AboutArbitrageTrading "关于ArbitrageTrading"
#define MyAppVersion "1.0"
#define MyAppPublisher "KING"
#define MyAppURL "http://www.nanhua.net/"
#define MyAppExeName "ArbitrageTrading.exe"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (生成新的GUID，点击 工具|在IDE中生成GUID。)
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
;输出文件夹
OutputDir=ArbitrageTrading
;安装输出文件名
OutputBaseFilename=setup
;安装图标
;SetupIconFile=D:\Testfg\GPM20101029\systex.ico
;安装输入密码
;Password=111111
;Encryption=yes

;压缩相关
Compression=lzma
SolidCompression=yes
;安装前查看的文本文件
InfoBeforeFile=E:\ArbitrageTrading\readme.txt
 ; 备注版本信息
VersionInfoCompany={#MyAppURL}
VersionInfoDescription=ArbitrageTrading汉化增强版
VersionInfoVersion=1.0.0.1
VersionInfoCopyright=Copyright(C) 1996-2015 NANHUA

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]

;可执行文件
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


;图片
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

;配置
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


