# Storm
SPH (Smoothed Particle Hydrodynamics) reimplementation for fluid simulation of air



# Setup
From the root folder containing this README. Execute Build\Script\Setup.bat once, then modify the newly created Build\Script\UserSettings.bat to match the path of each dependency in your workstation. And execute again Build\Script\Setup.bat.
Note :
- You should have downloaded all dependencies beforehand and should have done all setup steps that were specified (see next sections).
- We rely on junctions. It means that all your dependencies folder and Storm project should be put on compatible disks format (i.e. NTFS). But if junctions don’t work, we advise to copy your dependencies under the Storm’s root “Dependencies” folder after calling the setup script (then, you should also change how the setup script works unless you want to delete your dependencies every time).



# Prerequisite
- **Visual Studio Community 2019 v16.8.1 with C++20** (or latest draft). Later version of visual studio are not tested.
- **Python 2.7.6 or later** (to build PhysX, see “Dependencies list” section). To know your current installed version if you have one, type python in a cmd console.
- **Visual Studio 2017 toolsets (v141) and 2019 toolsets (v142)**
- **cmake_gui 3.15.0-rc1 or later**. (We used 3.15.0-rc1. We can’t guarantee an earlier version would work)...
- Because we rely a lot on junctions and hardlinks to setup the project, we advise you to use an NTFS disk (or at least a disk format that supports it).
- **.NET Framework 4.7.2** If you want to compile and use the “log viewer” or “script sender” tools.



# Dependencies list
- **Boost 1.75** built with Visual Studio 2019 (link: https://sourceforge.net/projects/boost/files/boost-binaries/1.75.0/ ). Choose the right and execute the downloaded executable (as for me, since I’m using VS 2019 with a x64 architecture, Therefore I downloaded “boost_1_75_0-msvc-14.2-64.exe”).
 + Extract boost into a new folder named “boost_1_75_0__2019” inside your dependencies folder (if not, you must change the path to boost dependencies inside UserSettings.bat).
    + Rename the folder “lib[xx]-msvc-[yy.y]” to “lib” with xx your platform architecture (as for me, it was 64 because I’m working on a x64 architecture) and yy.y your toolset version (likewise, it was 14.2).
    + Create a new folder inside boost_1_75_0__2019 named “include”.
    + Move the folder “boost” ([YourDependenciesFolder]/boost_1_75_0__2019/boost) to a new include folder ([YourDependenciesFolder]/boost_1_75_0__2019/include). You should have your boost folder at path ([YourDependenciesFolder]/boost_1_75_0__2019/include/boost).
- **Eigen 3.3.7** (link: https://gitlab.com/libeigen/eigen/-/tree/3.3.7).
- **OIS v1.5** built with Visual Studio 2019 (link: https://github.com/wgois/OIS/tree/v1.5 ). Follow the instructions on their site.
 + Generate the Visual studio file inside a folder named “bin” at OIS root folder (i.e if OIS path is C:/dep/OIS, then generate the vs project file into C:/dep/OIS/bin).
 + generate with default settings
 + build OIS into Debug and Release configurations.
- **Assimp v5.0.1** (link: https://github.com/assimp/assimp/releases/tag/v5.0.1 ). Download the .zip file (and the pdf if you want some documentation on how to use Assimp)...
 + Use CMake. Keep all settings as they come except for “LIBRARY_SUFFIX” and “ASSIMP_LIBRARY_SUFFIX” that should be kept but set to an empty field value (don’t forget to validate).
 You should also name the folder where to build the binary “bin” directly inside the root (the folder containing the README).
 + Build the generated sln both in debug and release configurations.
- **PhysX v4.0.0** built with Visual Studio 2017 (link: https://github.com/NVIDIAGameWorks/PhysX/tree/4.0.0 ). (You can use Visual Studio 2019 but the toolset installed should be 2017 (v141), or not... see the first point).
 + Follow instructions on their website. It is not said on their website, but the generated solution output the build under physx/compiler/[the setting you’ve chosen]. <br>
Note that I chose “vc15win64” settings. If you use another Visual Studio, be aware of Binary compatibility (Visual Studio 2015, 2017 and 2019 are binary compatible, but I don’t know about the future and they weren’t in the past). Be also aware the result file won’t be in the same folder than what we said, so adapt a little.
 + Build the library on “release”, “debug” and “profile” configurations.
 + Go to “physx/(Your output directory)/win.x86_64.vc141.mt” (the name could change if you have chosen another build setting) and copy “profile”, “debug” and “release” folders inside a new folder named “physx/lib”
- **Lua v5.4.0 release 2 (24/08/2020)** (link: http://luabinaries.sourceforge.net/). Take the static binaries for Windows 64 platform, or you are free to build them from scratch. Normally, it should be named “lua-5.4.0_Win64_vc16_lib”. I took the vc16 library since I’m using Visual Studio 2019, and Win64 because I’m working with a x64 workstation and project.
 + Ensure that the .lib file is inside a folder named “bin” beside “include” folder. Move it inside if not.
- **Sol2 v3.2.2** (link: https://github.com/ThePhD/sol2/tree/v3.2.2). Just download the zip and extract it.
- **Catch2 v2.12.2**. Not mandatory, but it is used for unit testing. (link: https://github.com/catchorg/catch2/releases ). If you choose to skip it, then unload all Automation projects from Storm.sln...



# Test Setup
We use catch2 as our unit test library. If you decided to download it, you must :
- Install Visual studio extension named “Test Adapter for Catch2” (Extensions -> Manage Extensions -> Online).
- Configure your run settings : Go to Test -> Configure Run Settings -> Select Solution Wide runsettings File. Navigate from Storm root directory to Source\Automation\StormAutomation-Base and select catch2.runsettings... 
- Select your processor architecture : Go to Test -> Processor Architecture for AnyCPU Projects and select “x64”. Though we aren’t sure of the usefulness of this step...
- Maybe you’ll have to disable Boost test adapter. Go to Extensions -> Manage Extensions -> Installed and search for “Test Adapter for Boost.Test”.



# Modules
- **Storm**: This is the SPH simulation executable entry point.
- **Storm-Animation**: This module is here to manage animations within the simulation (without being bound to a specific physics engine).
- **Storm-Config**: This is the configuration module.
- **Storm-Graphics**: This is the module responsible to display and render the simulation. It uses DirectX.
- **Storm-Helper**: This module contains all helpers that should be shared among all projects.
- **Storm-Input**: This module is for managing inputs, bindings and their callback.
- **Storm-Loader**: This module’s purpose is to load external objects like meshes.
- **Storm-Logger**: This module is for logging.
- **Storm-Misc**: This module is here to gather miscellaneous objects that cannot make a module by their own. For example, the RandomManager (that should be deterministic), the TimeManager (that should be compliant with Storm simulator), the ThreadManager (that is intended to be executed in the main Simulation loop like Unreal’s Async method), ... .
- **Storm-Network**: This module is intended to be the crossroad with the outside. It contains everything that could be used to manage the network and web.
- **Storm-ModelBase**: This module is the base for each module and provides an abstraction layer with interfaces and input/output objects. This allows modules to work together without directly referencing each other.
- **Storm-Physics**: This module is responsible for initializing and managing Physics computations.
- **Storm-Profiler**: This module provides some profiling data. It does not intend to replace Visual Studio built-in tool nor any other external library. But it provides a convenient way to register times, speed, ... and display it directly inside Storm application UI (or logging).
- **Storm-Safety**: This module is to provide some watchers to improve the safety of the application like freeze and memory watching.
- **Storm-Script**: This module is to abstract the scripting language used by the engine. For now, the scripting language is lua.
- **Storm-Serializer**: This module is to handle serialization of data to make real-time replay, for example.
- **Storm-Space**: This module is responsible to optimize and process the space partitioning of any data existing inside the virtual world.
- **Storm-Simulator**: This module is where the core of the simulation is done. It is the core “mastermind” module.
- **Storm-Windows**: This module is responsible for creating and managing the UI Window and everything related to it (like the title bar, menu, ... and their callbacks). It is the module that is the most platform dependent because it relies heavily on how Windows build its UI.



# Tools
Some tools were developed to ease our life. The main tools are :
- **Storm-LogViewer**: It is a little UI tool made with C# and WPF in an afternoon (sorry for the dirty code inside). Its purpose is to make the log easier to read, to search into and to sort.
- **Storm-Restarter**: This tool isn’t made to be manipulated directly by the user. It is used by the application itself to restart from the menu, therefore you need to build it if you want to use this feature.
- **Storm-ScriptSender**: Like the log viewer, it is a UI C#/WPF tool made in a weekend. It was made to send, save and sort easily scripts. It is an alternative to the shared script file.
- **Storm-Packager**: This tool packages the minimum needed to run Storm and/or any other tools. Then you can deploy the resulting zip folder and run any Storm application on another station. It is also able to request to package automatically for another branch (it will switch branches, build the solution, package it and revert to the old branch afterward).
- **Storm-MaterialAvailability**: This tool prints the availabilities of the materials found on the current station it is runned on.



# Launcher
“Launcher” folder is present directly inside the Storm root folder. You’ll find inside some utility scripts (batch) to easily start applications from executables you’ve built.
- **Storm**: This folder contains script to start the simulator (Storm.exe).
   - **Storm_Release.bat**: Start Release version of the simulator with default settings.
   - **Storm_Profile.bat**: Start Profile version of the simulator with default settings. The profile version is akin to the Release version, except that PhysX PVD (PhysX Visual Debugger) will be able to watch the simulator application. Note that it can impact performances so run it only if you want to use the PVD.
   - **Storm_Debug.bat**: Start Debug version of the simulator with default settings. 
- **Storm-LogViewer**: This folder contains script to start the log viewer (Storm-LogViewer.exe).
   - **Storm-LogViewer.bat**: Start Storm-LogViewer.exe with the default setting.
   - **Storm-LogViewer_NoInitRead.bat**: Start Storm-LogViewer.exe without initial read... The Storm-LogViewer will begin reading the last wrote log file from the moment when it was launched.



# Configuration

## Command line
Below are command lines for the different executables. Unless explicited, command line keys are case insensitive.



### Storm.exe
It is the main simulation application. Command lines should be used like this : --key=value or --key.
- **help (no value, facultative)**: Display the help. No simulation will be run and any other command line argument will be ignored.
- **scene (string, facultative, accept macros)**: This is the scene config file path to use. If left unset, we will pop the default installed file explorer and ask the user to choose one. Closing this popup without choosing a scene will exit the application.
- **macroConfig (string, facultative, accept built-in only macros)**: It is the macro config file path to use. If left unset, we will select the one inside the default Config folder (the one inside Custom/General takes priority over the one inside Custom/General/Original).
- **generalConfig (string, facultative, accept macros)**: This is the general config file path to use. If left unset, we will select the one inside the default Config folder (the one inside Custom/General takes priority over the one inside Custom/General/Original).
- **tempPath (string, facultative, accept macros)**: This is the temporary path to use (to a folder). If left unset, we will select the default temporary path folder.
- **mode (string, facultative)**: Specify the record/replay mode of the simulator. Accepted values (case insensitive) are “Record” or “Replay”. If the simulator is in record mode, then the simulation played will be recorded for a future replay. Default is unset, which means the application will just simulate without doing anything (not recording or replaying).
- **recordFile (string, facultative)**: Specify the path to output or read the recording. It must remain unset if no record mode was specified. Otherwise, it should reference a valid record file in case we’re in Replay mode (Note that this setting can be left Unset if there is a path inside the loaded scene config).
- **regenPCache (no value, facultative)**: Use this flag to regenerate the rigid body particle cache data.
- **noUI (no value, facultative)**: Specify we don’t want to visualize the simulation (this saves precious CPU resources to speed up the simulation). It must be used with record mode. Default is unset and the simulation will run naturally on a UI.
- **clearLogs (no value, facultative)**: Specify that we should empty the log folder before proceeding. Warning note: we mustn’t use this flag if we intend to run multiple Storm processes at the same time. Default is unset (we won’t clear the log folder).
- **threadPriority (string, facultative)**: Specify the priority of the simulation thread. If it is left unset, default OS priority will be applied. Accepted values (case insensitive) are “Below”, “Normal” or “High”.
- **stateFile (string, facultative, accept macros)**: Specify the simulation state file to load from. If left unset (default), we won’t load any. Note that it doesn’t make sense to start from a state file when we’re replaying, therefore this setting isn’t available if the mode is set to “Replay”.
- **noPhysicsTimeLoad (no value, facultative)**: Specify we shouldn’t load the physics time recorded in the state file. We’ll load it by default. Note that this setting should only be used if a state file were specified.
- **noVelocityLoad (no value, facultative)**: Specify we shouldn’t load the velocity part of the simulation state file. We’ll load it by default. Note that the setting should only be used if a state file were specified.
- **noForceLoad (no value, facultative)**: Specify we shouldn’t load the force part of the simulation state file. We’ll load it by default. Note that the setting should only be used if a state file were specified.



### Storm-LogViewer.exe
It is the application to see logs in a more friendly and clear manner. Command lines are exposed like this : key=value. The accepted command line arguments are :
- **MacroConfigFilePath (string, facultative, accept built-in only macros)**: This is the macro config file path to use. If left unset, we will select the one inside the default Config folder (the one inside Custom/General takes priority over the one inside Custom/General/Original).
- **LogFilePath (string, facultative, accept macros)**: This is the log file to display. If left unset, we will select the latest inside the default Log folder (located inside the default temporary folder). By not setting it, we’ll also allow the LogViewer to parse the next log when the day change...
- **NoInitialRead (no value, facultative)**: We’ll just read and display the log after the time the log viewer started... Otherwise, we would display all logs.
- **ReadLast (no value, facultative)**: Read and display the last log file found. This flag is ignored if we specified a log file to read with LogFilePath...



### Storm-ScriptSender.exe
This application provides a UI to easily send scripts to a connected Storm application without using a shared file with commands. Like Storm-LogViewer, command lines are exposed like this : key=value.
- **MacroConfigFilePath (string, facultative, accept built-in only macros)**: This is the macro config file path to use. If left unset, we will select the one inside the default Config folder (the one inside Custom/General takes priority over the one inside Custom/General/Original).
- **Port (unsigned integer, facultative)**: Manually set a port to listen to instead of 5007 by default. This integer should be in the ranges 1500 to 65535.
- **IntermediateSenderFolder (string, facultative, accept macros)**: Specify the folder where the script sender will save and load its xml script file. Default is equivalent to “$[StormIntermediate]/ScriptSender”.
- **ScriptXmlFileName (string, facultative, accept macros)**: The script file name to save/load into/from. Default is “SaveScriptCached.xml”.



### Storm-Packager.exe
This application was made to package easily a running version of Storm applications and Co. into a zip. It can also change and package another Storm’s git branch version.
It is not a professional application though. It does not intend to replace huge integration tool we could find on the market. But it has the advantage to be custom-made for Storm application (it is like a build script, but made with c++ that I’m so much more familiar with)...
When you start it, wait for it to finish or some nasty bugs could happen. The allowed commands are :
- **build (string, facultative)**: When specifying it with the branch name to build (i.e --build=develop), the packager will build the specified branch before packaging it (in the example, it will checkout develop, then build Storm.sln, package the result. And finally, it will revert to the branch we were before checking out develop).



## Config file

### Generality
In this section, all xml tags are described like this :
- **tagName (type, modality:[mandatory/facultative/semi-facultative], ...)** : description<br>

♦ <ins>tagName</ins> is the name of the xml tag.<br>
♦ <ins>type</ins> is the value type of the xml value.<br>
♦ <ins>modality</ins> is the importance of the tag :
- “mandatory” means the application will abort if the tag is left unset.
- “faculative” means the default value is used if the value is left unset.
- “semi-facultative” means the importance will depend on another setting : it could become mandatory or should remain unset.<br>
<br>

Except some exceptions described below, you should define the xml value for tagName like this : <br>
> \<tagName>value<\tagName>


<ins>Exceptions</ins>
- If the type is “vector3”, the xml should be defined like this : \<tagName x="xValue" y="yValue" z="zValue" \\>
- If the type is “rotation”, the xml should be defined like this : \<tagName angle="angleValue" x="xAxisValue" y="yAxisValue" z="zAxisValue" \\>
- If the type is “RGBAcolor”, the xml should be defined like this : \<tagName r="rValue" g="gValue" b="bValue" a="aValue" \\>. All “r”, “g”, “b” and “a” values are float values between 0.0 and 1.0 included.
- If the type is “article”, “misc”, see the section “Internal”.
- If the type is “SocketSetting”, the xml should be defined like this : \<tagName ip="yyy.yyy.yyy.yyy" port="portValue" enabled="boolean" \\>. With yyy a number between 0 and 255 included. Note that some ip and ports are invalid to use.


### Macro Configs

Macro are runtime substituted text defined by the user. In some text, we’ll try to find a key and substitute it in place with a value. The key is a text element inside $[...]. This feature is useful to slimmer down a path or some texts.<br>
e.g. : If we have a macro defined with a key equals “toto” and a value equals “titi”. If, inside a text accepting macros, we see $[toto], then we will replace it by titi).
Macro settings are stored in a xml file named “Macro.xml” located inside “Config\Custom\General”.

It is shared by all scenes and if there is no Macro.xml defined by the user, we will use the one inside the folder “Config\Custom\General\Original” which is a copy of the committed “Config\Template\General\Macro.xml”. Note that it is advised to make your own Macro.xml file and put it inside “Config\Custom\General”. Or you could specify the path to the Macro.xml from the command lines (see the specific section).<br>

A macro element is defined by a tag “macro” inside “macros” and has 2 attributes :
+ “key” (string, mandatory) : a token used to identify a macro. Do not add $[].
+ “value” (string, mandatory) : a text value to substitute the key.

Besides, you can reference a macro into another macro, in any kind of order you want (define a macro after a macro that will use it). But note that we will solve the macro iteratively and it cannot solve circular macro references (we’ll detect it and exit after complaining).

There are some pre-built-in macros that aren’t defined inside the macro file. Those can be used anywhere accepting macros (even inside command lines) :
+ **$[StormExe]** will refer to the Storm executable.
+ **$[StormFolderExe]** will refer to the folder containing the running storm executable.
+ **$[StormRoot]** will refer in case the storm application folders remained consistent, to the Storm root folder.
+ **$[StormConfig]** will refer in case StormRoot macro is valid, to where Config files are set.
+ **$[StormResource]** will refer in case StormRoot macro is valid, to where the resource folder is.
+ **$[StormIntermediate]** will refer in case StormRoot macro is valid, to where the Output folder is.
+ **$[StormRecord]** will refer, in case StormRoot macro is valid, to where the “Record” folder is.
+ **$[StormScripts]** will refer, in case StormRoot macro is valid, to where the “Scripts” folder is (where scripts will be put by default).
+ **$[StormStates]** will refer, in case StormRoot macro is valid, to where the “States” folder is.
+ **$[StormDebug]** will refer, in case StormRoot macro is valid, to where the “Debug” folder is.
+ **$[StormArchive]** will refer, in case StormRoot macro is valid, to where the “Archive” folder is.
+ **$[StormTmp]** will refer to the StormIntermediate if StormRoot macro is valid, or to OS defined temporary location.
+ **$[DateTime]** will refer to the current date at the time the application is started (in filesystem compatible format : Weekday_Year_Month_Day_Hour_Minute_Second ).
+ **$[Date]**, like DateTime, will refer to the current date at the time the application is started, but without hours and lesser time division (in filesystem compatible format : Weekday_Year_Month_Day ).
+ **$[PID]** will refer to the process unique ID (PID).
+ **$[ComputerName]** will refer to the computer name (aka hostname).
+ **$[SceneName]** will refer to the chosen scene name. This is an exception to the pre-build-in macros which can be used anywhere. It can only be used after selecting a scene. Therefore, be cautious when using this prebuilt macro. This settigs is only available for Storm application.
+ **$[SceneStateFolder]** will refer to the default state folder path. Since this macro is made from SceneName macros, it is also an exception to the pre-build-in macros which can be used anywhere. This settigs is only available for Storm application.


Note : macros are also applied to command lines except for the path to the macro configuration. But you’re still safe to use the pre-built-in macros except for the exceptional one.



### General config

General config (named Global.xml) is the global configuration of the application. It is the same for all scenes.
Like the Macro config, you can either specify one to use with command line, or it will search by default one inside the default config folder (“Config\Custom\General” or “Config\Custom\General\Original” if it doesn’t find it). It is advised to create and make your changes to the one inside “Config\Custom\General”.


Unless explicited, the following settings do not accept macros.


#### Application

- **displayBranch (boolean, facultative)**: If true, the branch the application was built upon will be displayed as part as its title when starting the application. It is useful for when we want to compare runs built from many branches and we want to know which is which. Default is false (because it is not really graceful if I want the application to be a proper application in the future).
- **beepOnFinish (boolean, facultative)**: If true, we’ll signal the user the end of the application with some Beep noise in case the simulation should stop automatically (i.e has an end time). Default is false.
- **empacketRecord (boolean, facultative)**: If true, we’ll try to empacket the recording in a new folder when the application finishes recording. Default is false. Records will be archived inside "$[StormArchive]/Records" folder.
- **language (string, facultative)**: Specify manually the language of the operating system. This is needed for some feature like the generation of Csv data since excel keyword formulas are bound to the OS locality language. We retrieve automatically the current user OS language by default. Accepted values are "English" and "French".


#### Debug (facultative)

##### - Log (facultative)
- **logFolderPath (string, facultative, accept macros)** : The folder where to gather the log files. The default is the temporary path (StormTmp macro). If it is empty, default is considered.
- **logFileName (string, facultative, accept macros)** : The log file name of the current run. The default is empty. If it is empty, we won’t log into a file (but the log will still be output to the console). Be aware to give unique name using the $[PID] macro or unexpected logging behaviour could occur because the file could be written by many processes at the same time (if you decide to start multiple Storm processes).
- **logLevel (string, facultative)** : The threshold level under which we ignore the log. Accepted values are in that importance order : Debug, DebugWarning, DebugError, Comment, Warning, Error, Fatal.
Note that the maximum value you can set is Fatal, it means that no matter what level you set, we would still log “Fatal” and “Always” logs. The default is Debug.
- **override (boolean, facultative)** : If the log file specified should have its content overridden each time. If “false”, the content will be appended instead. Default is “true”.
- **removeOlderThanDays (integer, facultative)** : Specify the number of day Storm will keep the log file. Log files older than the current date minus this day count will be removed. Disable this feature by setting a number <= 0 (therefore, we will keep everything). Default is -1.
- **fpsWatching (boolean, facultative)** : Our time keeper can watch fps and log if the fps of a thread is too low compared to what was expected. Setting it to "false" prevent it to watch the fps, and therefore to log if fps is under 20% of its expected frame rate. Default is "false".
- **logGraphicDeviceMessage (boolean, facultative)** : If we should print the graphic device log message. Warning : there is a lot ! Default is "false" but enable it only if it is necessary.
- **logPhysics (boolean, facultative)** : If we should print the physics messages. Warning : there is a lot ! Default is "false" but enable it only if it is necessary.


##### - Exception (facultative)
- **displayVectoredException (string, facultative)**: Specify how we should hook vectored exception happenning (could catch SE exception as well and non std exception catched at the end (the ...) and provide better information like the stack trace where the issue happened). Accepted values (non case sensitive) are "None", "FatalOnly" and "All". Default is FatalOnly. 
  + **"None"**: We won't hook any vectored exception handler. This is better performance wise, but you'll risk not knowing what happenend if you have silent or hard to track crashes.
  + **"FatalOnly"**: Display fatal errors information such as what is the fatal code and the stack trace. Note that fatal doesn't mean unhandled exception or "..." exceptions, but more like hardware or OS related killer exceptions that we cannot/shouldn't/mustn't recover from.
  + **"All"**: This is the verbose setting. Enable it only if you want to troubleshoot "..." issues (along with a lot of other irrelevant ones)


##### - Profile (faculative)
- **profileSimulationSpeed (boolean, faculative)** : Specify that we should enable Simulation speed profile. Default is false.


##### - PhysX (facultative)
- **physXPvdSocket (SocketSetting, facultative)**: The socket settings to use to communicate with PhysX visual debugger. Default values of ip and port are 127.0.0.1 and 5425 respectively. If the tag is unset (default), we won't create a way to communicate with PhysX debugger. Besides, Only Debug and Profile configuration are able to use the PVD. PVD cannot be used with Release. 
- **pvdConnectTimeout (unsigned integer, facultative)**: Positive integer defined in milliseconds specifying the conncection timeout to the PVD. Default is 33 (=> 33 ms).
- **pvdTransmitContacts (boolean, facultative)**: Specify we’ll send contact data to the PVD. Default is true.
- **pvdTransmitConstraints (boolean, facultative)**: Specify we’ll send constraints data to the PVD. Default is true.
- **pvdTransmitSceneQueries (boolean, facultative)**: Specify we’ll send scene query data to the PVD. Default is false.


##### - Graphics (facultative)
- **keepUnsupported (boolean, facultative)**: True if we should cycle on unsupported force when selecting a particle (legacy behavior), otherwise we'll skip all unsupported. Default is false.


#### Network (facultative)
- **enable (boolean, facultative)**: If false, the network module communication with Storm tools will be disabled. Default is false. Note that even if it is disabled, processes start/stop and web logic are still enabled.
- **scriptSender (SocketSetting, facultative)**: The socket settings referring to the communication with the Script sender tool. Ignored if the network module is disabled. The default used port is 5007.


#### Web (facultative)
- **browser (string, facultative)**: Specify the browser to use when opening a URL. This is not case sensitive. Accepted values are “Chrome”, “Firefox”, “Edge”, “InternetExplorer” or you can leave the field empty. Default is no browser. Note that if there is no browser set, then opening a URL won’t work and an error will be issued instead.
- **incognito (boolean, facultative)**: Specify we want to open a new browser window in incognito mode when we’ll browse through a URL. Default is false.


#### Graphics (facultative)
- **screenWidth (unsigned integer, facultative)** : Set the expected windows width at startup. Note that it could be something else near this value... Default is 1200.
- **screenHeight (unsigned integer, facultative)** : Set the expected windows height at startup. Note that it could be something else near this value... Default is 800.
- **screenX (integer, facultative)** : Set the expected windows x position at startup. If it is unset (default), then a default value will be chosen from the OS...
- **screenY (integer, facultative)** : Set the expected windows y position at startup. If it is unset (default), then a default value will be chosen from the OS...
- **fontSize (unsigned integer, facultative)** : the font size of any written information displayed in the HUD... Default is 16.
- **nearFarPlaneFixed (boolean, facultative)**: Specify if the normal behaviour of the near and far plane should be to NOT translate along the camera moves. Default is true.
- **selectedParticleAlwaysOnTop (boolean, facultative)**: Specify if the selected particle should be displayed on top of all particles (on the near plane). Default is false.
- **selectedParticleForceAlwaysOnTop (boolean, facultative)**: Specify if the selected particle force should be displayed on top of all particles (on the near plane). Default is true.
- **spinCameraToGravity (boolean, facultative)**: If true, the camera will spin for the gravity to go down. Default is false.
- **smoothCameraTransition (boolean, facultative)**: If true, the camera will move smoothly to its final destination. Not that it only affects translation, not rotations. Default is true.
- **showGravityArrow (boolean, facultative)**: If true, gravity arrow will be displayed by default on the HUD. Default is true.


#### Safety (facultative)

##### Memory (facultative)
- **memoryThreshold (positive float, facultative)**: The memory coefficient threshold applied to the total installed memory the workstation has. If the application take more than this threhold of RAM, then we'll kill the application as a safety measure. This value should be between ]0.0, 1.0]. Default is 0.95 (We allow the application to take 95% of the available RAM).
- **enableWatcher (boolean, facultative)**: If true, memory watcher is enabled. Default is true.


##### Freeze (facultative)
- **refreshDuration (unsigned integer, facultative)** : Set the refresh duration of the simulation freeze watcher in seconds. If the simultation is frozen more than this threshold, then we'll kill the application as a safety measure. This value should be greater than 30 seconds. Default is 1200 seconds (20 minutes). Note that this safety watcher is temporily disabled when a debugger is attached.


#### Simulation (facultative)
- **allowNoFluid (boolean, facultative)**: If true, we will allow the scene config file to not have any fluid (useful for testing rigid body features without minding particles while developing). Default is false.
- **stateRefreshFrameCount (positive integer, facultative)**: Specify how many frames before the next system state refresh. This value must be a positive integer. 0 means the state refreshes is disabled. Default is 0.


### Scene Config

Scene configuration files contain all the data for running a simulation, therefore it is mandatory to specify one. If it was not set from the command line, Storm application will open an explorer windows to allow you to choose one.
Unlike the other config files, it can be named as you want. Here the xml tags you can set :


#### General
- **startPaused (boolean, facultative)**: If true, the simulation will be paused when the application starts. Default is false.
- **gravity (vector3, facultative)**: This is the gravity vector, in meter per second squared. Default value is { x=0.0, y=-9.81, z=0.0 }
- **particleRadius (float, facultative)**: This is the particle radius in meter. It is also what is used for display but has also a physical value. Default value is 0.05.
- **kernelCoeff (positive float, facultative)**: This is the kernel multiplicator coefficient (without units). If this value is equal to 1.0, then the kernel length would be equal to the particle radius. Default value is 4.0.
- **maxKernelIncrementCoeff (positive float, facultative)**: This is the runtime end to the increase of value of the kernel coefficient (without units). It is made to increase the kernel coefficient at runtime with the elapsed physics time. Default value is 0.0 (no increase).
- **kernelIncrementCoeffEndTime (positive float, facultative)**: This is the end time in seconds when we stop increasing the kernel coefficient. After this time, the kernel length would be at its final value. Default value is the feature is disabled.
- **CFLCoeff (positive float, facultative)**: This is the coefficient lambda for the CFL (Courant-Friedrich-Levy conditions). It is used only if CFL is enabled. Default value is 0.4.
- **MaxCFLTime (positive float, facultative)**: This is the max time the CFL (Courant-Friedrich-Levy conditions) could take in seconds. It is used only if CFL is enabled. Default value is 0.5 seconds (500 ms).
- **CFLIteration (positive integer, facultative)**: This is the max time the CFL (Courant-Friedrich-Levy conditions) Should be run within one iteration. Default value is 2.
- **kernel (string, facultative)**: This is the kernel we would use. It isn’t case sensitive and the accepted values are for now “CubicSpline” (default) and “SplishSplashCubicSpline”.
- **physicsTime (float, facultative)**: This is the iteration loop physics time in seconds. If this value is less or equal to 0, then we would adapt it automatically using CFL condition. Default value is -1.
- **fps (float, facultative)**: This is the expected frame rate (it has nothing to do with the physics time, this is the refresh rate of many loop inside the engine to not consume too much CPU). If this value is less or equal to 0, then we would set it to the default value which is 60 fps.
- **simulationNoWait (boolean, facultative)**: Set this flag to true to run the simulation as fast as possible (disabling the framerate binding on the simulation thread, therefore removing the synchronisation wait that bind it to a specific framerate). Default is false.
- **simulation (string, mandatory)**: This is the simulation mode we will run. It isn’t case sensitive. Accepted values (for now) are “DFSPH”, “IISPH”, “PCISPH” and “WCSPH”.
- **neighborCheckStep (positive integer, facultative)**: This is a char between 1 and 255. This specifies that we will recompute the neighbourhood every neighborCheckStep step. Default is 1 (we recompute each step of the simulation).
- **startFixRigidBodies (boolean, facultative)**: If true, dynamic rigid bodies will be fixated in place at simulation start. See input keys to unfix them. Default is false.
- **freeRbAtTime (positive float, facultative)**: If set, rigid bodies will automatically be unfixed at the specified simulation physics time (in seconds). This value should be a valid positive floating point number. Default is unset.
- **midUpdateViscosity (boolean, facultative)**: If true, We’ll update rigid bodies with viscosity forces before solving the pressure force. Default is false. Note this setting is used only for DFSPH.
- **stateFileRemoveRbCollide (boolean, facultative)**: If true, we’ll remove colliding particles when we’ll load the state file, otherwise we’ll skip it. Default is true.
- **stateFileConsiderRbWallCollide (boolean, facultative)**: If true, we’ll also remove particle colliding with the wall when we load a state file, otherwise we’ll skip the wall (other rigid bodies are still processed though). Default is true.
- **removeFluidForVolumeConsistency (boolean, facultative)**: If true, we’ll remove particles to obtain the domain volumes if possible. This removal pass comes at the end of all removal pass. Default is true.
- **endPhysicsTime (float, facultative)**: This is the end time (physics time) in seconds the simulation should stop. After this time, the simulator will exit... The value should be greater than zero. Default is unset (the simulator will continue indefinitely).
- **fluidViscosityMethod (string, facultative)**: Specify what method to use when computing viscosity force of a fluid particle on another fluid particle. This setting is case insensitive. Allowed values are “Standard” (default) and “XSPH”.
- **rbViscosityMethod (string, facultative)**: Specify what method to use when computing viscosity force of a fluid particle on a rigid body particle. This setting is case insensitive. Allowed values are “Standard” (default) and “XSPH”.
- **fluidParticleRemovalMode (string, facultative)**: Specify what method formula to use to detect a fluid particle overlaps a rigid body particle, therefore should be eliminated at initialization time. This setting is case insensitive. Allowed values are “Sphere” (default) and “Cube”.
- **noStickConstraint (boolean, facultative)**: Enforce the no-stick constraint as explicited in R. Bridson book. Since it is experimental, default is false. For now, the setting is only implemented for DFSPH.
- **useCoendaEffect (boolean, facultative)**: Use Coenda effect penalty forces. Since it is experimental, default is false. For now, the setting is only implemented for DFSPH.


#### Graphics
- **cameraPosition (vector3, facultative)**: This is the camera (viewpoint) initial position. Each coordinate is meters. Default value is { x=0.0, y=0.0, z=-10.0 }.
- **cameraLookAt (vector3, facultative)**: This is the initial position of the target the camera look at. Each coordinate is meters. Default value is { x=0.0, y=0.0, z=0.0 }.
- **zNear (float, facultative)**: This is the initial distance in meter from the camera which everything that is nearer than this distance won’t be rendered. Default value is 0.01.
- **zFar (float, facultative)**: Same as zNear except that we skip displaying all objects farther than this distance value. Default value is 20.0.
- **watchRbId (positive integer, facultative)**: Specify which rigid body we should lock the near plane on. Default value is unset (no rigid body locked).
- **minVelocityColorValue (float, facultative)**: Set the minimum velocity value for the watched value to display the coldest colour. This is to be expressed as a squared norm. Default value is 0.01.
- **maxVelocityColorValue (float, facultative)**: Set the maximum velocity value for the watched value to display the hottest colour. This is to be expressed as a squared norm. Default value is 100.
- **minPressureColorValue (float, facultative)**: Set the minimum pressure value for the watched value to display the coldest colour. Default value is 0.
- **maxPressureColorValue (float, facultative)**: Set the maximum pressure value for the watched value to display the hottest colour. Default value is 10000.
- **minDensityColorValue (float, facultative)**: Set the minimum density value for the watched value to display the coldest colour. Default value is 0.
- **maxDensityColorValue (float, facultative)**: Set the maximum density value for the watched value to display the hottest colour. Default value is 2000.
- **blowerAlpha (float, facultative)**: Set the blower visualization alpha channel. Must be between 0 and 1 included. Default value is 0.25.
- **grid (vector3, facultative)**: Set the grid dimension. X coordinate will be the grid width, Z its depth and Y will be the height where the grid will be drawn. Note that X and Z will be ceiled. Default value is { x=10.0, y=0.0, z=10.0 }
- **particleDisplay (boolean, facultative)**: Specify if Solids should be displayed as particles on start. If not, they will be displayed as meshes. “false” by default.
- **constraintThickness (positive float, facultative)**: Specify the thickness of the line when visualizing the constraint. It should be a positive non-zero value. Default is “General.particleRadius / 3.0”.
- **constraintColor (RGBAcolor, facultative)**: Specify the colour of the line when visualizing the constraint. Default is { r=1.0, g=0.1, b=0.1, a=0.8 }.
- **forceThickness (positive float, facultative)**: Specify the thickness of the line when visualizing the selected particle force. It should be a positive non-zero value. Default is “General.particleRadius / 3.0”.
- **forceColor (RGBAcolor, facultative)**: Specify the colour of the line when visualizing the selected particle force. Default is { r=0.0, g=1.0, b=1.0, a=0.8 }.
- **normalsColor (RGBAcolor, facultative)**: Specify the colour of the line when visualizing the selected rigidbody normals. Default is { r=1.0, g=0.0, b=1.0, a=0.8 }.


#### Physics
- **enablePCM (boolean, facultative)**: Enable GJK-based distance collision detection system. Default is true.
- **enableAdaptiveForce (boolean, facultative)**: Enable adaptive forces to accelerate convergence of the solver. Default is true. Note this setting is incompatible with enableStabilization.
- **enableFrictionEveryIteration (boolean, facultative)**: Controls processing friction in all solver iterations. If false, PhysX processes friction only in the final 3 position iterations as well as all velocity iterations. Default is true.
- **enableStabilization (boolean, facultative)**: Enables additional stabilization pass in solvers. When set to true, this enables additional stabilization processing to improve that stability of complex interactions between large numbers of bodies. Note PhysX still consider it an experimental feature which does result in some loss of momentum. Default is false. Note this setting is incompatible with enableAdaptiveForce.
- **enableKinematicPairs (boolean, facultative)**: Enable contact pair filtering between kinematic rigid bodies. Seems deprecated but default is true.
- **enableKinematicStaticPairs (boolean, facultative)**: Enable contact pair filtering between kinematic and static rigid bodies. Seems deprecated but default is true.
- **enableAveragePoint (boolean, facultative)**: Enables average points in contact manifolds. When set to true, this enables additional contacts to be generated per manifold to represent the average point in a manifold. This can stabilize stacking when only a small number of solver iterations is used. Default is true.
- **enableEnhancedDeterminism (boolean, facultative)**: Provides improved determinism at the expense of performance. Default is false.
- **enableCCD (boolean, facultative)**: Enables a second broad phase check after integration that makes it possible to prevent objects from tunnelling through each other. Default is true.
- **noBuiltinDamping (boolean, facultative)**: Remove PhysX builtin angular damping (even though it does not works). Default is false.


#### Record
- **recordFps (float, semi-facultative)**: This is the record fps. It becomes mandatory if the simulator is started in Record mode.
- **recordFile (string, facultative, accept macros)**: Specify the path the recording will be. This path will be used in case it wasn’t set from the command line.
- **replayRealTime (boolean, facultative)**: Fix the replay to be the nearest possible from a real-time replay. It means that the simulation speed will try to be as near as possible to 1.0. Default is true.


#### Script
- **enabled (bool, facultative)**: Setting it to false prevent the engine to watch the scripting file. Default is true. Note that the scripting API will still be functional, it is that the engine won’t be able to answer to outside commands.
- **Init (tag)**: This is the section to control the initialization script call. For further details, please refer to “Scripting API” section.
 + **initScriptFile (string, facultative, accept macros)**: The path to the script init file containing the initialization script. Default is empty, we won’t use any script to initialize the engine. Note that the path should either remain empty, be a valid path to a plain text file, or shouldn’t exist (we’ll create one for you).
- **Runtime (tag)**: This is the section to control the script used as a way to communicate with the engine at runtime. For further details, please refer to “Scripting API” section.
 + **watchedScriptFile (string, facultative, accept macros)**: The path to the watched script file. Default is equivalent to “$[StormScripts]/RuntimeScript.txt”. This path should either remain empty, point to a valid text file, or not exist (we’ll create one for you).
 + **refreshTime (unsigned int, facultative)**: The refresh time in milliseconds of the watched script. Default is 100 ms. Cannot be equal to 0. Besides, note that even if it is below the framerate time of the engine, it won’t be watched faster (in another word, the real value is the maximum between this “value” and “1000 / the expected engine framerate”)


#### Fluid
This element is all setting appertaining to a fluid. Here the tag you can set inside :
- **id (positive integer, mandatory)**: This is the unique id of the fluid. It should be unique (note that it should not be equal to any rigid body id and blower id too).
- **fluidBlock (tag, semi-facultative)**: This is the fluid generator settings. There should be at least one if there is not unitParticles declared.
 + **firstPoint (vector3, facultative)**: This is one of the corners of the box where fluid particle should be generated. It cannot have the same value than secondPoint, default value is { x=0.0, y=0.0, z=0.0 }.
 + **secondPoint (vector3, facultative)**: This is the opposite corner from firstPoint where fluid particles should be generated. It cannot have the same value than firstPoint, default value is { x=0.0, y=0.0, z=0.0 }.
 + **denseMode (string, facultative)**: This is the load mode impacting how we will generate and set the particle positions inside the block. It isn’t case sensitive. Accepted values are : “Normal” (default) or “SplishSplash” (to use SPlisHSPlasH algorithm to generate the particles).
- **UnitParticles (tag, semi-facultative)**: Inside this tag should be listed particles we want to spawn manually at a specified particle. They are spawned one by one.
 + **position (vector3, facultative)**: This is the position of one spawned particle. Each position tag will represent one particle.
- **density (positive float, facultative)**: This is the rest density of the fluid in kg.m^-3. Default is 1.2754 kg.m^-3 which is the density of Dry air at 0 °C and normal ATM pressure.
- **smoothDensity0 (boolean, facultative)**: Enable the smoothing of the rest density from the actual average density. This is experimental. Default is false (we'll use the rest density as-is).
- **particleVolume (positive float, facultative)**: This is the particle volume of the fluid in m^3. This is a constant throughout the simulation. If left unset (default), we’ll compute it automatically. It should be a positive non-zero floating point number.
- **pressureK1 (positive zero-able float, facultative)**: This is the pressure stiffness constant coefficient used when initializing the pressure using state equation. In formulas, it is often found as k1. Default is set automatically from WCSPH formula.
- **pressureK2 (positive zero-able float, facultative)**: This is the pressure exponent constant coefficient used when initializing the pressure using state equation. In formulas, it is often found as k2. Default is 7.
- **relaxationCoeff (positive zero-able float, facultative)**: This is the relaxation coefficient (alias omega) used inside some simulation methods when computing prediction pressures. It should be between 0.0 and 1.0 included. Default is 0.5.
- **initRelaxationCoeff (positive zero-able float, facultative)**: This is the pressure initial relaxation coefficient used inside some simulation methods (like IISPH) when initializing pressure fields. It should be greater or equal to 0.0. Default is 0.5.
- **viscosity (positive float, facultative)**: This is the dynamic viscosity of the fluid in N.s/m² (or Pa.s). Default is 0.00001715 N.s/m² which is the dynamic viscosity of Dry air at 0 °C and normal ATM pressure.
- **soundSpeed (positive float, facultative)**: This is the speed of sound inside the given fluid in m/s. Default is 331.4 m/s which is the speed of sound of Dry air at 0 °C) and normal ATM pressure (340 is for 15 °C).
- **uniformDragCoeff (positive zero-able float, facultative)**: This is a user constant used to compute drag effect. It is typically equal to 1/2 * Cd * A where Cd is the drag coefficient and A is the cross-sectional area. But since this is particle to particle drag, the cross-sectional area is meaningless and can be tweaked by the user. The value should be greater or equal to zero. Default is 0 (no drag effect). Note that this coefficient is only applied for fluid to fluid drag effect. For fluid<->solid drag effect, use the dragCoeff set on a per rigid body basis.
- **reducedMassCoeff (positive float, facultative)**: This is the coefficient for mass initialization to prevent preessure spikes at the start of the simulation (cf SPlisHSPlasH). Default is 0.8.
- **enableGravity (boolean, facultative)**: Enable the gravity for the associated fluid particle system. Default is true.
- **removeCollidingParticles (boolean, facultative)**: If true, any fluid particle that collides with any rigid bodies will be forcefully removed when it is spawned. This does not reproduce the behaviour of SplishSplash since it does not remove insider particles. If false, fluid particles can spawn inside rigid bodies, possibly leading to some explosion, instabilities and bad physical states. Default is true.
- **removeOutDomainParticles (boolean, facultative)**: If true, any fluid particle outside the domain will be removed. Otherwise, fluid particles can spawn outside the domain. Default is true.
- **DFSPH (tag, facultative)**: This tag is for gathering all settings that are relevant to DFSPH. Those will be ignored if the simulation method is not DFSPH. If DFSPH was selected, but the tag isn’t present, then default value will be used.
 + **pressurePredictKCoeff (positive zero-able float, facultative)**: This is a multiplication coefficient for the predicted pressure coefficient we compute inside DFSPH. This parameter doesn’t exist in the real formula (is 1.0) but was introduced to balance pressure forces against viscosity forces. Default is 1.0.
 + **neighborThresholdDensity (positive integer, facultative)**: Neighbour count threshold used for DFSPH. If the neighbour count is below this value, then we won’t advect density for the current particle. It should be a non-zero positive integer. Default is 20.
 + **enableThresholdDensity (boolean, facultative)**: Enable the neighbour threshold density. Maybe for simulation that is with a filled domain (no void). Default is true.
 + **useRotationFix (boolean, facultative)**: Specify if we should use the last rotation fix. Default is true.
 + **enableDensitySolve (boolean, facultative)**: Specify if we should enable the invariant PPE (Pressure Poisson Equation) density solver. Default is true.
 + **useBernouilliPrinciple (boolean, facultative)**: Compute dynamic pressure along the fluid to produce additional pressure forces in the fluid using Bernoulli's principle. This is experimental. Default is false.
 + **minPredictIteration (positive integer, facultative)**: This is the minimum iteration we need to make inside one simulation loop when doing prediction iterations. The more iteration loop before returning a result there is, the stabler and slower the simulation would be. Default value is 2 (0 is ignored, we should at least make one iteration).
 + **maxPredictIteration (positive integer, facultative)**: This is the max iteration we’re allowed to make inside one simulation loop when doing prediction iterations. It is to avoid an infinite loop. It should be an integer strictly greater than 0 and should be greater or equal than minimum prediction iterations. Default value is 150.
 + **maxDensityError (positive float, facultative)**: This is the max density error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.
 + **maxPressureError (positive float, facultative)**: This is the max pressure error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.
- **IISPH (tag, facultative)**: This tag is for gathering all settings that are relevant to IISPH. Those will be ignored if the simulation method is not IISPH. If IISPH was selected, but the tag isn’t present, then default value will be used.
 + **minPredictIteration (positive integer, facultative)**: This is the minimum iteration we need to make inside one simulation loop when doing prediction iterations. The more iteration loop before returning a result there is, the stabler and slower the simulation would be. Default value is 2 (0 is ignored, we should at least make one iteration).
 + **maxPredictIteration (positive integer, facultative)**: This is the max iteration we’re allowed to make inside one simulation loop when doing prediction iterations. It is to avoid an infinite loop. It should be an integer strictly greater than 0 and should be greater or equal than minimum prediction iterations. Default value is 150.
 + **maxError (positive float, facultative)**: This is the max density error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.
- **PCISPH (tag, facultative)**: This tag is for gathering all settings that are relevant to PCISPH. Those will be ignored if the simulation method is not PCISPH. If PCISPH was selected, but the tag isn’t present, then default value will be used.
 + **minPredictIteration (positive integer, facultative)**: This is the minimum iteration we need to make inside one simulation loop when doing prediction iterations. The more iteration loop before returning a result there is, the stabler and slower the simulation would be. Default value is 2 (0 is ignored, we should at least make one iteration).
 + **maxPredictIteration (positive integer, facultative)**: This is the max iteration we’re allowed to make inside one simulation loop when doing prediction iterations. It is to avoid an infinite loop. It should be an integer strictly greater than 0 and should be greater or equal than minimum prediction iterations. Default value is 150.
 + **maxError (positive float, facultative)**: This is the max density error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.


#### RigidBodies
Inside this element should be put all rigid bodies. Each rigid body should be specified under a tag named “RigidBody”.

##### RigidBody
- **id (positive integer, mandatory)**: This is the unique id of the rigid body.  It should be unique (note that it should not be equal to any fluid id or blower id).
- **meshFile (string, mandatory, accept macros)**: This is a mesh file path this rigid body is bound to.
- **isStatic (boolean, facultative)**: Specify this to tell the simulation that this object is fixed (won’t move throughout the simulation). Default value is “true”.
- **wall (boolean, facultative)**: Specify that this object is the wall (considered to be a wall). By definition, a wall is static so if the value is true, the object will be considered static no matter the value set to isStatic. Default value is “false”.
- **collisionType (string, facultative)**: Specify what is the collision shape and how should it be handled. This is not case sensitive. Possible values are “None” (default value), “Sphere”, “Cube”, “Particle” and “Custom”. Note that if it is a particle, some settings won’t be taken into account (like the scale, the rotation or the layer count) and the engine will consider this rigid body as a single particle (only one particle will be made).
- **animation (string, facultative, accept macros)**: The path to the animation xml file that describes how to animate the rigid body.
- **animationName (string, facultative, accept macros)**: The name of the animation to use. If there is none (default), then the animation to use will be the one with the same id than the rigid body.
- **translation (vector3, facultative)**: The initial position in meters of the object. Default value is { x=0.0, y=0.0, z=0.0 }.
- **rotation (rotation, facultative)**: The initial rotation in degrees of the object. Default value is { angle=0.0, x=1.0, y=0.0, z=0.0 }.
- **scale (vector3, facultative)**: The initial scale of the object. Default value is { x=1.0, y=1.0, z=1.0 }.
- **color (RGBAColor, facultative)**: The colour of the rigid body. Default value is { r=0.3, g=0.5, b=0.5 a=1.0 }.
- **staticFrictionCoeff (float, facultative)**: The static friction coefficient of the object, it should be larger than 0.0. PhysX needs it but physically speaking I don’t know what to set. This is the minimum force norm threshold that makes our object move. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **dynamicFrictionCoeff (float, facultative)**: The dynamic friction coefficient of the object, it should be positive. PhysX needs it but physically speaking I don’t know what to set. This is the velocity reduction when a rigid body moves with a contact with another. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **restitutionCoeff (float, facultative)**: The restitution friction coefficient of the object (the bounciness of the object), it should be positive but close or above 1.0 may cause instabilities. PhysX needs it but physically speaking I don’t know what to set. Closer it is to 0.0, less the object will bounce and more it will lose energy when being in contact with another rib. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **angularDamping (float, facultative)**: PhysX angular velocity damping of 0.05 is a lie, therefore we need to expose a tweakable setting to do it ourself. The value shouldn’t be greater than 1.0. 0.0 means no damping, 1.0 means full damping (the object won’t rotate anymore). If the value is negative (then the velocity will be increased instead, but beware because there is a high chance the simulation could become unstable). Besides, this setting is only for dynamic rigid bodies. Default is 0.05 (as PhysX sets).
- **fixTranslation (boolean, facultative)**: Specify if the rigid body is allowed to translate. If true, then the rigid body will only be able to rotate in place. Default is false.
- **fixedVolume (boolean, facultative)**: Specify if the rigid body volume is allowed to change throughout the simulation. If true, then the volume won’t change. Note that if volumeComputation tag is set, then we would use it, otherwise we’ll fix the value to the one computed once at initialization time. This setting will be ignored in replay mode. Default is false.
- **mass (positive float, mandatory)**: The mass of the rigid body in kg. This has to be strictly positive (> 0.0). 
- **reducedVolumeCoeff (positive float, facultative)**: A coefficient applied much like the reduced mass coeff from fluid (see reducedMassCoeff), but to the rigidbodies particle volumes. It should be strictly greater than 0. Default is the same that was set for the reducedMassCoeff.
- **viscosity (positive float, facultative)**: The viscosity of the rigid body in Pa.s (???). This has to be strictly positive (> 0.0).
- **noStickCoeff (positive zero-able float, facultative)**: This is a coefficient made to modulate the norm of the force produced to ensure the no-stick constraint. Default is 1.0, set it to 0.0 to disable the no-stick constraint for the specific rigidbody.
- **dragCoeff (positive zero-able float, facultative)**: This is the drag coefficient of the rigid body object with the fluid. Default is 0.0 (drag effect disabled for the current rigid body). See uniformDragCoeff on fluids.
- **coendaCoeff (positive zero-able float, facultative)**: This is the coenda penalty force coefficient of the rigid body object with the fluid. Because it is experimental, default is 0.0 (coenda effect disabled for the current rigid body). See uniformDragCoeff on fluids.
- **normalsCoherency (boolean, facultative)**: if true, a second pass will be made to ensure rigid bodies particles normals are coherents (point to the same way (to the interior if it is a wall, or to the exterior if it is an non-wall object)). It is better to keep this setting to false for concave or miscellaneous objects. Default is true. Note that this is to correct normals from object that are correctly made, we cannot fix objects those triangles are in a mess. Besides, this settings ensure normals point to the inside for a wall and outside for any other rigid body.
- **sampleCountMDeserno (positive integer, semi-facultative)**: This is the input sample count of M. Deserno sampling algorithm of a sphere. It should be greater than 0. It is mandatory if specified geometry is "EquiSphereMDeserno" and the layeringGeneration is "Uniform".
- **layerCount (positive integer, facultative)**: The boundary particles layer to generate along the rigid body surface. Should be a non-zero positive integer. Default is 1.
- **layeringGeneration (string, facultative)**: The technique used to generate boundary particles layer on the rigid body surface. It isn’t case sensitive and the accepted values are “Scaling” (default), “Dissociated” and “Uniform”.
  * “Scaling” technique generates the layer as if the layer was generated by scaling the rigid body.
  * “Dissociated” technique generates the layer by extruding each triangle along its normal.
  * “Uniform” technique generates only one layer of particle with uniform generation. Note that the geometry tag is mandatory in case this mode was chosen.
- **geometry (string, facultative)**: Geometry type of the rigidbody. It isn’t case sensitive and the accepted values are “Sphere”, “Cube” and "EquiSphereMDeserno". Only available if we chose Uniform as layering generation technique. "EquiSphereMDeserno" is also a uniform sphere, but the sampling is done using M. Deserno algorithm given in its paper.
- **pInsideRemovalTechnique (string, facultative)**: Specify the technique used to detect and remove fluid particles inside the rigid body. It isn’t case sensitive and the accepted values are “None” (default) and “Normals”. Note that it is illegal to set something else other than “None” for wall rigid bodies.
  * “None” means no technique will be used; therefore we won’t check nor remove inside particles.
  * “Normals” means we will use rigid body normals to detect a particle inside the rigid bodies... We suppose the rigid body has normals those directions point outside and that the rigid body has no holes.
- **volumeComputation (string, facultative)**: Specify the technique used to compute the volume of the rigid body. It isn’t case sensitive and the accepted values are “None” (default), “Auto” and “TriangleIntegration”. Note that no matter the choice, volume computation will be disabled for individual particles.
  * “None” means we won’t compute a volume for the rigid body (it does not affect the simulation but some methods will be disabled). In another hand, this will speed up the simulation start.
  * “TriangleIntegration” will use mesh’s triangles to compute the volume by integrating the volume of each tetrahedron defined by the triangle to the center of the rigid body. This does not work for concave geometries (only convex geometries are handled correctly).
  * “Auto” means we’ll use the collision type of the object to fast compute the volume (this suppose the Collision Type was set and is correct). For Custom geometry, “TriangleIntegration” will be used.


#### Constraints

##### Constraint
- **type (string, mandatory)**: This is the type of the constraint. It is a mandatory setting. Accepted values (case insensitive) and are “PhysicsJoint” (to simulate a Joint purely with PhysX (beware : it can be springy)) or “Hardjoint” (we’ll simulate the joint ourself by blocking the position of the 2nd actor (we suppose the 1st actor is static and the second should be dynamic))
- **rbId1 (positive integer, mandatory)**: This is the id of the first rigid body the constraint is attached to. It is a mandatory setting and the rigid body id must exist.
- **rbId2 (positive integer, mandatory)**: This is the id of the second rigid body the constraint is attached to. It is a mandatory setting and the rigid body id must exist.
- **length (positive float, facultative)**: This is the max length (in meter) of the constraint separating both rigid bodies (+ the initial distance between those rigid bodies). It should be a positive value and default is 0.0.
- **rb1LinkOffset (vector3, facultative)**: This is the translation offset of the link endpoint on the rigid body 1 relative from its center. All coordinates are expressed in meters. Default is { x=0.0, y=0.0, z=0.0 } (the link end point is located at the center of the rigid body 1).
- **rb2LinkOffset (vector3, facultative)**: This is the translation offset of the link endpoint on the rigid body 2 relative from its center. All coordinates are expressed in meters. Default is { x=0.0, y=0.0, z=0.0 } (the link end point is located at the center of the rigid body 2).
- **noRotation (boolean, facultative)**: If true, the constraint won’t rotate along the constraint normal axis (rotation along the constraint axis will still be allowed). Default is “true”.
- **visualize (boolean, facultative)**: If true, the constraint will be displayed. Default is “true”.


#### Blowers

##### Blower
- **id (positive integer, mandatory)**: This is the blower id. This setting is mandatory. Note that this will also be the id of the blower rigid body that will be generated along your blower so it should also be unique with rigid bodies id and fluids id.
- **type (string, mandatory)**: This is the blower type. This setting is case insensitive and mandatory. It defines what your underlying blower would be. For now, the accepted values are “Cube”, “CubeGradualDirectional”, “Sphere”, “SpherePlanarGradual”, “Cylinder”, “CylinderGradualMidPlanar”, “Cone”, “RepulsionSphere”, “PulseExplosion” and “Explosion”.
- **startTime (positive float, facultative)**: This defines the time in simulation seconds time your blower start working. Before this value, your blower will remain disabled. This value + fadeInTime shouldn’t be greater than the endTime - fadeOutTime. Default value is 0.0 (the blower start right away).
- **endTime (positive float, facultative)**: This defines the time in simulation seconds time your blower stop completely. After this value, your blower will remain disabled. This value minus fadeOutTime shouldn’t be lesser than the start time + fadeInTime. If -1.0 is specified, the endTime will be ignored and the blower will continue indefinitely. Default value is -1.0.
- **fadeInTime (positive float, facultative)**: This defines the time in simulation seconds time the blower will take to attain its peak force. 0.0 means that it will be instant. The increase rate is linear between startTime and startTime + fadeInTime.
- **fadeOutTime (positive float, facultative)**: This defines the time in simulation seconds time the blower will take to stop from its peak force to a null force. 0.0 means that it will be instant. The decrease rate is linear between endTime - fadeOutTime and endTime.
- **radius (positive float, semi-mandatory)**: This defines the radius in meters of the blower. This setting is mandatory for Sphere type blowers and should be a positive non-zero floating point number. Any uses for another blower type than sphere derived blowers aren’t allowed.
- **upRadius (positive float, semi-mandatory)**: This defines the radius in meters of the Cone blower up disk. This setting is mandatory for Cone type blowers and should be a positive floating point number. Any uses for another blower type than Cone derived blowers aren’t allowed.
- **downRadius (positive float, semi-mandatory)**: This defines the radius in meters of the Cone blower down disk. This setting is mandatory for Cone type blowers and should be a positive floating point number. Any uses for another blower type than Cone derived blowers aren’t allowed.
- **height (positive float, semi-mandatory)**: This defines the height in meters of the blower. This setting is mandatory for Cylinder type blowers and should be a positive non-zero floating point number. Any uses for another blower type than cylinder derived blowers aren’t allowed.
- **dimension (vector3, semi-mandatory)**: This defines the dimension of the blower. This setting is mandatory for Cube type blowers and all coordinates should be positive non-zero values. This is the scaling of a 1m x 1m x 1m cube... Any uses for another blower type than cubes derived blowers aren’t allowed.
- **position (vector3, facultative)**: This defines the position (all coordinates are meters) of the origin center of the blower. Default is { x=0.0, y=0.0, z=0.0 }...
- **force (vector3, facultative)**: This defines the force applied by the blower to each particle in range (inside its effect area defined by the blower’s dimension). Default is { x=0.0, y=0.0, z=0.0 }...


#### Cage
This element is completely optional. Not declaring it prevents to create a cage inside the simulation (a domain where no particle is allowed to leave (but not physically meaningful)). Cage is a box.
- **boxMin (vector3, mandatory)**: The lower point of the cage spatial domain. All its components must be lower than boxMax.
- **boxMax (vector3, mandatory)**: The higher point of the cage spatial domain. All its components must be greater than boxMin.
- **infiniteDomain (bool, facultative)**: Specify if the cage should allow particle to pass through its borders. If true, then particle will be moved to the opposite side of the cage when they pass one side of the border. Default is false.
- **passthroughVelReduceCoeff (vector3, facultative)**: When infinite domain is true. This coefficient is the velocity reduction coefficient happening for all particles that crosses the border on each axis. All values should be positive. Default is { x=1.0, y=1.0, z=1.0 }.



### Animation

Animation config is where we define a rigid body animation.
Each rigid body animation should begin by a tag “RigidBody”. Inside, as an attribute, you must define either an id (which is the same than the id of the rigid body from your scene config), and/or a name which would be the name of the animation.
We advise you to use the animation “name” attribute instead of the id since it is more scalable.
Then inside the RigidBody must define a list of xml children tags those purposes are to define the animation.
Note that you should be careful of the order of each child since they must be defined in chronological order.
- **Keyframe (tag, facultative)**: This tag defines a keyframe. Frame in between are interpolated. Defined attributes are :
 + **time (float, mandatory)**: This is the keyframe time in seconds.
 + **posX (float, facultative)**: This is the x position coordinate of your rigid body at the keyframe time. If it isn’t set, then the x position of the last keyframe will be used. If there is no keyframe before, the initial x position of the rigid body will be used.
 + **posY (float, facultative)**: This is the y position coordinate of your rigid body at the keyframe time. If it isn’t set, then the y position of the last keyframe will be used. If there is no keyframe before, the initial y position of the rigid body will be used.
 + **posZ (float, facultative)**: This is the z position coordinate of your rigid body at the keyframe time. If it isn’t set, then the z position of the last keyframe will be used. If there is no keyframe before, the initial z position of the rigid body will be used.
 + **rotX (float, facultative)**: This is the x rotation in degree of your rigid body at the keyframe time. If it isn’t set, then the x rotation of the last keyframe will be used. If there is no keyframe before, the initial x rotation of the rigid body will be used.
 + **rotY (float, facultative)**: This is the y rotation in degree of your rigid body at the keyframe time. If it isn’t set, then the y rotation of the last keyframe will be used. If there is no keyframe before, the initial y rotation of the rigid body will be used.
 + **rotZ (float, facultative)**: This is the z rotation in degree of your rigid body at the keyframe time. If it isn’t set, then the z rotation of the last keyframe will be used. If there is no keyframe before, the initial z rotation of the rigid body will be used.
- **Motor (tag, facultative)**: This tag should be placed last if you need it. Specifying one will create a rotation motion around a fixed point.
 + **pivotX (float, facultative)**: This is the x position coordinate of your motor hinge point. The circular motion will orbit around this specific point. The default value is the location of your rigid body.
 + **pivotY (float, facultative)**: This is the y position coordinate of your motor hinge point. The circular motion will orbit around this specific point. The default value is the location of your rigid body.
 + **pivotZ (float, facultative)**: This is the z position coordinate of your motor hinge point. The circular motion will orbit around this specific point. The default value is the location of your rigid body.
 + **rotAxisX (float, semi-facultative)**: This is the z rotation axis coordinate of your motor. Default is 0.f but the final vector defined by x, y and z should be non-null.
 + **rotAxisY (float, semi-facultative)**: This is the y rotation axis coordinate of your motor. Default is 0.f but the final vector defined by x, y and z should be non-null.
 + **rotAxisZ (float, semi-facultative)**: This is the z rotation axis coordinate of your motor. Default is 0.f but the final vector defined by x, y and z should be non-null.
 + **speed (float, mandatory)**: This is your motor angular speed defined in turn per second. A positive value makes the motor rotate clockwise. Negative would make it rotate counterclockwise. This value shouldn’t be 0.
- **Loop (tag, facultative)**: This tag should be placed last if you need it. Specifying one will make the animation loop to the beginning when it has finished playing.


### Internal

Internal Config is a versioned config that mustn’t change much. It contains data that I didn’t want to hard-code.<br><br>
Since this configuration file shouldn’t be exposed a lot (but I’ll document it nevertheless), we wouldn’t be permissive in case an error happens and immediately exit the application.<br>
The location of the Internal config named “InternalConfig.xml” is inside under the folder “$[StormConfig]/Internal”.
So now, you are warned to not modify this config file unless it is truly necessary.


#### References
References contains a list of all references to articles, papers, books, websites, ... I used :
- **article (tag, facultative)**: Specify the reference is an article/paper/book. The exposed attributes are :
 + **authors (string, mandatory)**: Specify the authors of the reference. If there is more than one author, they should all be separated by a comma “,”.
 + **name (string, mandatory)**: Specify the name of the reference. Note that the “\n” will be interpreted as a line break.
 + **date (string, mandatory)**
 + **serialNumber (string, mandatory)**: ISBN, SN, DOI, ... It can be anything that allows to reference this article uniquely.
 + **URL (string, facultative)**: A URL to where we can retrieve the article easily. (Though we don’t specify if it will be free of charge.)
 + **bibTex (string, facultative)**: The relative path from BibTex folder (folder besides the InternalConfig.xml) to a text file containing the BibTex reference of the article. Note that we don’t validate the BibTex string is valid.
- **misc (tag, facultative)**: Specify a miscellaneous reference. It can be anything like a tutorial, a website, another engine source code, ...
 + **authors (string, mandatory)** Specify the authors of the reference. If there is more than one author, they should all be separated by a comma “,”.
 + **name (string, mandatory)**: Specify the name of the reference. Note that the “\n” will be interpreted as a line break.
 + **date (string, facultative)**
 + **serialNumber (string, facultative)**: Anything that allows to reference this reference uniquely.
 + **URL (string, facultative)**: An URL to where we can retrieve the reference easily. (Though we don’t specify if it will be free of charge.)
 + **bibTex (string, facultative)**: The relative path from BibTex folder (folder besides the InternalConfig.xml) to a text file containing the BibTex reference of the article. Note that we don’t validate the BibTex string is valid.



# Input bindings

## Key bindings

Note : If the term in the parenthesis is “Numpad”, then the keybinding is the value inside the Numpad only. If it is “Key”, then the value is not the one present on the Numpad.

- **Escape key (ESC)**: Leave the application.
- **Space bar**: Pause application if it is running. Unpause if it is paused.
- **+ (Numpad)**: Move near clipping plane forward. This input is not allowed if the near plane is locked on a rigid body to watch.
- **- (Numpad)**: Move near clipping plane backward. This input is not allowed if the near plane is locked on a rigid body to watch.
- **/ (Numpad)**: Move far clipping plane backward.
- **\* (Numpad)**: Move far clipping plane forward.
- **Up Arrow**: Move camera up.
- **Down Arrow**: Move camera down.
- **Left Arrow**: Move camera left.
- **Right Arrow**: Move camera right.
- **8 (Numpad)**: Move camera forward.
- **2 (Numpad)**: Move camera backward.
- **Q**: Rotate the camera around the target on the Y-axis (negative).
- **Z**: Rotate the camera around the target on the X-axis (positive).
- **D**: Rotate the camera around the target on the Y-axis (positive).
- **S**: Rotate the camera around the target on the X-axis (negative).
- **0 (Numpad Zero)**: Reset camera position.
- **N**: Advance the paused simulation to the next frame.
- **E**: Enable all disabled blower, and disable all enabled ones.
- **R**: Enable (if disabled)/Disable (if enabled) raycasts system.
- **L**: Fix (if unfixed)/Unfix (if fixed) rigid bodies.
- **Y**: Cycle the particle selection force to display (between [...] -> Pressure -> Viscosity -> Drag -> AllForces (except gravity) -> Total force on rigid body -> [...]).
- **I**: Reset the replaying to the first frame. This feature exists only in replay mode.
- **C**: Request all forces check to zero as per physics conservation of momentum law says for isolated systems that are in an equilibrium state.
- **J**: Force refresh watched script files. Or re read all of them.
- **X**: Save the current simulation state.
- **1 (Key)**: Decrease the physics delta time. Valid only if we are not in replay mode, or if CFL is disabled.
- **2 (Key)**: Increase the physics delta time. Valid only if we are not in replay mode, or if CFL is disabled.
- **3 (Key)**: Cycle the colour setting of the fluid particle data displayed from ... -> Velocity -> Pressure -> Density -> ... .
- **F1**: Show/Hide the kernel effect area of the selected particle. Does nothing if no particle has been selected.
- **F2**: Show/Hide the axis coordinate system and the gravity vector displayed on the UI HUD.
- **F3**: Set rendering to only display rigid bodies, no fluids.
- **F4**: Set selected particle forces always on top flag to true if false, false otherwise. But note that if it is true, you’ll always see the particle force if it is in front of the view point, but you’ll lose the depth information of the vector.
- **F5**: Set wireframe.
- **F6**: Set solid state with back face culling.
- **F7**: Set solid state without back face culling.
- **F8**: Set rendering to display all particles.
- **F9**: Set particle state without rendering walls.
- **F12**: Toggle the UI fields display (displays it if hidden, or hide it if displayed).
- **Ctrl+S**: Open the script file inside notepad++ (or notepad if notepad++ isn’t installed).
- **Ctrl+X**: Open the current xml config file inside notepad++ (or notepad if notepad++ isn’t installed).
- **Ctrl+L**: Open the Storm log viewer. This application will have its life shared with the Storm application.


## Mouse bindings
- **Left click on dynamic rigid body particle**:
    - Select the particle to display either its Pressure force, Viscosity force or the sum of those force (see “Cycle the particle selection force to display” key binding). This binding is only valid if the raycast is enabled (see “Enable/Disable raycasts system” key binding).
- **Middle click on particle**:
    - Set the camera target to the particle position. This binding is only valid if the raycast is enabled (see “Enable/Disable raycasts system” key binding).
- **Mouse wheel**:
    - Increase/Decrease the camera motion speed 
    - Increase/Decrease the clipping plane motion speed
    - Increase/Decrease the physics time step change speed. Valid only if we are not in replay mode, or if CFL is disabled.



# Scripting API

The scripting language used is lua. You’ll be able to send lua command to the engine from a plain text file that is watched once every “refreshTime” millisecond for any changes.
Just save your file after modifying it and the content will be sent. An alternative would be to use the Storm-ScriptSender tool.
There is also the initialization file that allows to further customize the initialization of the engine. This custom initialization is done at the end of the engine’s “normal” initialization.

The reason we decided to include a scripting API is because : 
- We have too many inputs and too few keys to control all features we have to add. And it starts to be really bothersome.
- We could control many instances of the application at the same time and use the script language as a way to synchronize animations.
- Better control over the parameters (we can use the exact value we want).
- It is the only way to control the application when it doesn’t run with a UI.


## Developer’s notes

The scripting wrapping was done to abstract modules from the scripting API we use. Like this, we could switch or add python, ... any scripting language we want with minimal effort. <br>
The entry point of the scripting API object/methods registration can be anything. Just use the macro “STORM_IS_SCRIPTABLE_ITEM” inside your class. If it is used inside a Singleton registered inside the SingletonAllocator, the call to the entry point will be automatic.<br>
Register all your methods, classes, ... inside ScriptImplementation.inl.h. An example of how to do it would be to look at how I did it with the SimulatorManager. Don’t mind the include because this file will be included inside source file only + all singleton will be included anyway.<br>
Finally, be aware that all scripting will be executed inside the same scripting thread that is a specific registered thread. Therefore, all method that the API calls should be responsible to dispatch the real execution to the expected threads to avoid any threading issues.


## Commands

Storm script commands are not part of any scripting language and are parsed separately from what is sent to the interpreter. Commands are also ignored by said interpreter.
They are like headers that purpose is to control how the script will be played. A script body can only have one or no header commands and is placed after the header.<br>
Header part of the script where commands are defined must begin and end by the delimiter string "**#####**".<br>
Then should follow the script body that will be sent to the interpreter. <br>
A command inside the header is declared on the same line : In another word, 2 different lines are 2 different commands.<br>
A command must start with a keyword defining what it is, followed by some parameters if needed. Both are separated by the character “**:**” .<br><br>
Here the list of available commands :
- **pid** : Parameters are the list of PIDs separated by a ’ ’ or a “,”. It specifies that the following script must be executed only by the processes referred by the listed PIDs.
- **enabled** : It specifies the following script is enabled if the parameter is either “true”, “on” or “1”. Disabled if parameter is either “false”, “off” or “0”. There could only be 1 of this key specified in the same command.


## Exposed methods

### Exposed Enumerations

#### - ColoredSetting
- **Velocity**: The colour field data to display should be the particles’ velocity norms.
- **Pressure**: The colour field data to display should be the particles’ pressure.
- **Density**: The colour field data to display should be the particles’ density.

#### - GraphicCutMode
- **Kernel**: To specify the graphic cut should have a depth of one kernel length.
- **Particle**: To specify the graphic cut should have a depth of one particle radius.


### Exposed Instance

#### - SimulatorManager (simulMgr)

- **void resetReplay()**: Reset the replay. This is the same method bound to the key inputs.
- **void saveSimulationState()**: Save the simulation state into a state file. This is the same method bound to the key inputs.
- **void advanceOneFrame()**: Advance the simulation to the next frame. Available only if the simulation is paused. This is the same method bound to the key inputs.
- **void advanceByFrame(int64_t frameCount)**: Advance the paused simulation by frameCount frames. The frameCount value must be positive !
- **void advanceToFrame(int64_t frameNumber)**: Advance the paused simulation to a specific frame. The frameNumber value must be positive !
- **void selectSpecificParticle(const unsigned int pSystemId, const std::size_t particleIndex)**: Select the particleIndex-th particle from the particle system refered by pSystemId.
- **void printRigidBodyMoment(const unsigned int id)**: Compute and print the total moment of the rigid body specified by id. It serves at debugging the rigid body rotation (how it spins). Note that this method is to be used only for dynamic rigid bodies.
- **void printRigidBodyGlobalDensity(const unsigned int id)**: Print the rigid body density evaluated from a predicted volume and its mass set in the config file. Disabled if the volume wasn’t computed.
- **void printFluidParticleData()**: Debug command to print to a human readable text giving all position, velocity and force values of all fluid particles. The data is printed inside the output (temp) directory inside “Debug” folder.
- **void logAverageDensity()**: Log the average density of the simulation.
- **void logVelocityData()**: Log min and max velocity of each particle system.
- **void logTotalVolume()**: Log the total volume taken by all particles contained inside the domain.
- **void writeRbEmptiness(const unsigned int id, const std::string &filePath)**: Computes the void distance between each particle of the rigidbody specified by id and the nearest fluid particle then write it to the csv file named filePath.
- **void writeCurrentFrameSystemForcesToCsv(const unsigned int id, const std::string &filePath)**: Write all forces (force, pressure, viscosity, drag) of particle system refered by its id to a csv file. Note that the system can be a fluid or a rigid body. "filePath" can accept macros.
- **void writeParticleNeighborhood(const unsigned int id, const std::size_t pIndex, const std::string &filePath)**: Write particle specified by id (particle system id) and pIndex (position in particle system) data from neighborhood. "filePath" can accept macros. If filepath extension is csv, then we'll devide the path into 2 files : the first suffixed with neighbor would contains neighbors and the other (suffixed with nonNeighbor) would contains all other particles from the same system. Otherwise, we'll write it into the same file.
- **void setGravityEnabled(bool enableGravityOnFluids)**: Enable/Disable gravity at runtime.
- **void setEnableThresholdDensity_DFSPH(bool enable)**: Enable/Disable the neighbour threshold on density solver. Enabled means that we’ll use a threshold. Disabled means that the density solving won’t use a neighbourhood threshold and solve the density whatever the neighbour count is. This method is only available if the solver is DFSPH.
- **void setNeighborThresholdDensity_DFSPH(size_t neighborCount)**: Set the neighbour threshold value on density solver. This method is only available if the solver is DFSPH.
- **void setUseRotationFix_DFSPH(size_t neighborCount)**: Enable rotation fix. This method is only available if the solver is DFSPH.
- **void selectRigidbodyToDisplayNormals(const unsigned int rbId)**: Select a rigidbody to display its normals.
- **void clearRigidbodyToDisplayNormals()**: Clear the rigidbody to display its normals selection.
- **void selectCustomForcesDisplay(std::string selectionCSL)**: Select forces, accumulate them and display the result inside the Custom appelation on the UI. Only works for the selected particle. The list of the force to choose should be set inside selectionCSL string (A comma separated list) that is not case sensitive. If the string is empty, the selection is cleared. Accepted values are :
  * "Pressure" : refers to the pressure force.
  * "Viscosity" : refers to the viscosity force.
  * "Drag" : refers to the drag force.
  * "Bernouilli" / "DynamicQ" / "DynamicPressure" : refer to the dynamic pressure force.
  * "NoStick" : refers to the no stick constraint penalty force.
  * "Coenda" : refers to the coenda penalty force.


#### - TimeManager (timeMgr)

- **bool changeSimulationPauseState()**: Pause/Unpause the simulation. This is the same method bound to the key inputs.
- **void resetPhysicsElapsedTime()**: Reset the elapsed time to 0.


#### - GraphicManager (graphicMgr)

- **void cycleColoredSetting()**: Cycle the particle colouring observed quantities. This is the same method bound to the key inputs.
- **void setColorSettingMinMaxValue(float minValue, float maxValue)**: Set the min and max values for the observed particles colour fields.
- **void setUseColorSetting(const ColoredSetting selectedColoredSetting)**: Set the setting to display.
- **void showCoordinateSystemAxis(const bool shouldShow)**: Display the axis coordinate system if true, hide it otherwise.
- **void setUIFieldEnabled(bool enable)**: Set if we should display the UI fields (enabled to true) or hide it (enabled to false).
- **void lockNearPlaneOnWatchedRb(unsigned int rbId)**: Set (or change) the rigid body specified by its id (rbId) to lock the near plane on.
- **void unlockNearPlaneOnWatchedRb()**: Clear the near-plane lock to a rigid body. The near plane will be freed for the user to move it manually.
- **void setVectMultiplicatorCoeff(const float newCoeff)**: Change the multplicator coefficient for the norm of the displayed vectors. Note that the multiplicator coefficient is only visual and has no impact on the actual norm of those vectors (only to be able to visualize better vectors that are too little to be displayed).
- **void makeCutAroundWatchedRb(const Storm::GraphicCutMode cutMode)**: Apply a cut around the center of the watched rb if any.
- **void makeCutAroundRigidbody(const unsigned int rbId, const Storm::GraphicCutMode cutMode)**: Apply a cut around the center of the specified rb. This method cannot be executed if we have a watched rb.
- **void makeCutAroundSelectedParticle(const Storm::GraphicCutMode cutMode)**: Apply a cut around the selected particle. This method cannot be executed if we have a watched rb.


#### - PhysicsManager (physicsMgr)

- **void setRigidBodyAngularDamping(const unsigned int rbId, const float angularVelocityDamping)**: Set the angular velocity damping value of the rigid body specified by its id. This method is only defined for dynamic rigid bodies.
- **void fixDynamicRigidBodyTranslation(const unsigned int rbId, const bool fixed)**: Set the specified rigid body’s translation fixed flag. This method is only defined for dynamic rigid bodies.
- **void setRigidBodiesFixed(const bool shouldFix)**: If shouldFixate is set to true, fix the rigid bodies in place. This is another way to fix/unfix without using inputs.
- **void reconnectPhysicsDebugger()**: Force the reconnection to the Physics Debugger which is the PhysX’s PVD (PhysX Visual Debugger).


#### - OSManager (osMgr)

- **void clearProcesses()**: Clear all processes (this closes all processes those lives are bound to the Storm application).


#### - WindowsManager (winMgr)

- **void restartApplication(const std::string_view &additionalArgs)**: Restart Storm application and applies additional parameters. Note that the application will be restarted to the current scene and will forward its current command line, therefore additional parameters should only contain parameters that aren’t already in those.



# References

TODO (Sorry, since the engine is a work in progress. This section will be updated last... But we promise to reference all used sources and cite their respective authors).

