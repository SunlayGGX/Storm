# Storm
SPH reimplementation for fluids simulation


# Setup
From the root folder containing this README. Execute once Build\Script\Setup.bat, then modify the newly created Build\Script\UserSettings.bat to match the path of each dependencies in your workstation.
Reexecute Build\Script\Setup.bat once all paths inside Build\Script\UserSettings.bat are ok.
Note that : 
- you should have downloaded all dependencies beforehand and have done all setup that was specified (see next section).
- We rely on junctions. It means that all your dependencies folder and Storm project should be on compatible disks format (i.e. NTFS). But if junctions don't work, we advise to copy your dependencies under the Storm's root "Dependencies" folder after calling Setup (also change the way the batch file are executed or you will remove those dependencies each time).



# Prerequisite
- **Visual Studio Community 2019 v16.8.1 with C++20** (in fact the latest draft that was a preview of C++20). Maybe it works for a later Visual Studio but I have never tested it.
- **Python 2.7.6 or later** (needed to build PhysX, see "Dependencies list" section). To know what is the current version of your installed python (or if there is any), type python in a cmd console.
- **Visual Studio 2017 toolsets (v141) and 2019 toolsets (v142)**
- **cmake_gui 3.15.0-rc1 or later**. (3.15.0-rc1 is what I used. I can't guarantee for a version below this one)...
- Because we use a lot of junctions and hardlinks to setup the project, we advise you to use an NTFS disk where everything are put together (or at least a disk format that supports it).
- **.NET Framework 4.7.2** if you want to compile and use the LogViewer tool.


# Dependencies list
- **Boost 1.75** compiled for Visual Studio 2019 (link: https://sourceforge.net/projects/boost/files/boost-binaries/1.75.0/ ). Choose the right and execute the downloaded executable (for me, since I'm using VS 2019 with a x64 architecture, I downloaded "boost_1_75_0-msvc-14.2-64.exe").
	+ Choose to extract boost into a new folder named boost_1_75_0__2019 inside your dependencies folder (if not, you'll have to change the path to boost dependencies inside UserSettings.bat).
    + Rename the folder "lib[xx]-msvc-[yy.y]" to "lib" with xx your platform architecture (for me it was 64 because I'm working on a x64 architecture) and yy.y your toolset version (For me, it was 14.2).
    + Create a new folder inside boost_1_75_0__2019 named "include".
    + Move the folder "boost" ([YourDependenciesFolder]/boost_1_75_0__2019/boost) into the new include folder ([YourDependenciesFolder]/boost_1_75_0__2019/include). You should have your boost folder at path ([YourDependenciesFolder]/boost_1_75_0__2019/include/boost).
- **Eigen 3.3.7** (link: https://gitlab.com/libeigen/eigen/-/tree/3.3.7).
- **OIS v1.5** compiled for Visual Studio 2019 (link: https://github.com/wgois/OIS/tree/v1.5 ). Follow the instructions on their site.
	+ Generate the Visual studio file inside a folder named "bin" at OIS root folder (i.e if OIS is installed like this : C:/dep/OIS, then generate the vs project file into C:/dep/OIS/bin).
	+ generate with default settings
	+ build OIS into Debug and Release configurations.
- **Assimp v5.0.1** (link: https://github.com/assimp/assimp/releases/tag/v5.0.1 ). Take the .zip file (and the pdf if you want some documentation on how to use Assimp)...
	+ Use CMake. Keep all settings except for the "LIBRARY_SUFFIX" and "ASSIMP_LIBRARY_SUFFIX" that should be set to an empty field value (don't forgot to validate).
	You should also name the folder where to build the binary "bin" directly under the root (the folder containing the README).
	+ Build the generated sln both in debug and release configurations.
- **PhysX v4.0.0** build for Visual Studio 2017 (link: https://github.com/NVIDIAGameWorks/PhysX/tree/4.0.0 ). (You can use Visual Studio 2019 but the toolset installed should be 2017 (v141), or not... see the first point)
	+ Follow instruction on their site. Note that it isn't said in their website, but the generated solution to build is under physx/compiler/[the setting you've chosen].
Note that I chose "vc15win64" settings. If you use another Visual Studio, be aware of Binary compatibility (Visual Studio 2015, 2017 and 2019 are binary compatible, but I don't know about my future and they aren't with the past). Be also aware that the result file won't be in the same folder than what was expected by this readme so adapt a little.
	+ Build the library in "release" and "debug".
	+ Go to "physx/(Your output directory)/win.x86_64.vc141.mt" (this name could change if you have chosen another build setting) and copy "debug" and "release" folder inside a new folder "physx/lib"
- **Lua v5.4.0 release 2 (24/08/2020)** (link: http://luabinaries.sourceforge.net/). Take the static binaries for Windows 64 platform unless you want to compile them from scratch. Normally, it's name is lua-5.4.0_Win64_vc16_lib. I took the vc16 library since I'm using Visual Studio 2019, and Win64 because I'm working with a x64 workstation and project.
	+ If needed, ensure that the .lib file is inside a folder named "bin" beside "include" folder. Move it inside if it isn't.
- **Sol2 v3.2.2** (link: https://github.com/ThePhD/sol2/tree/v3.2.2). Just download the zip and extract it.
- **Catch2 v2.12.2**. Not mandatory, but it is used for unit testing. (link: https://github.com/catchorg/catch2/releases ). If you choose to not pull it, then unload All Automation project from your Storm.sln... If you want to run tests, do not forget to setup your Visual Studio for this. See section "Test Setup" for further details.


# Test Setup
I'm using catch2 as our main unit test library. But to be able to use it, you should :
- Install Catch2 adapter as a Visual studio extension (go to Extensions -> Manage Extensions -> Online, Then download and Install "Test Adapter for Catch2").
- Configure your run settings. Go to Test -> Configure Run Settings -> Select Solution Wide runsettings File. Navigate from Storm root directory to Source\Automation\StormAutomation-Base and select catch2.runsettings... 
- Select your processor architecture : Go to Test -> Processor Architecture for AnyCPU Projects and select "x64". Though I don't think this step is really useful...
- Maybe you'll have to disable Boost test adapter. Go to Extensions -> Manage Extensions -> Installed and search for "Test Adapter for Boost.Test". If it is enabled, you can disable it.


# Modules
- **Storm**: This is the executable that runs the SPH simulation.
- **Storm-Config**: This is the configuration module.
- **Storm-Graphics**: This is the module responsible to display and render the simulation. We use DirectX.
- **Storm-Helper**: This module contains all helpers that should be shared among all modules.
- **Storm-Input**: This module is for managing inputs, bindings and their callback.
- **Storm-Loader**: This module's purpose is to load external object like meshes.
- **Storm-Logger**: This module is for logging.
- **Storm-Misc**: This module is intended to gather module manager helper that shouldn't be made into helpers since they are Storm project implementation related but cannot be put into any other modules. Example are the RandomManager (that should be deterministic), the TimeManager (that should be compliant with Storm simulator), the ThreadManager (that is intended to be executed in the main Simulation loop like Unreal's Async method), ... .
- **Storm-Network**: This module is intended to be the crossroad with the outside. It contains everything that could be used to manage network and web.
- **Storm-ModelBase**: This module is the base for each modules. It contains everything that could be used to bind the modules together without the need to reference each other.
- **Storm-Physics**: This module is responsible for initializing and managing Physics computations.
- **Storm-Profiler**: This module is to allow getting some profiling data. This does not intend to replace the Visual Studio buit-in tool or any other external library, but just a way to register times, speed, ... and display it inside the Storm application UI or logging it.
- **Storm-Script**: This module is to abstract scripting used inside the engine. Under the hood, it is lua, but it can change and this module is here to hide it.
- **Storm-Serializer**: This module is to handle serialization process. Serialization for cooking the simulation into real time replay for example.
- **Storm-Space**: This module is where we implement the space visualisation of the domain. Be it Voxels, Grids, octree, ... This is where we compute and store element for a fast neighborhood search.
- **Storm-Simulator**: This module is where the Simulation classes would be. It is responsible to handle SPH.
- **Storm-Windows**: This module is responsible for managing the Windows and everything related to it.


# Tools
Some Tools were developped to ease our life. Those tools are :
- **Storm-LogViewer**: This is a little UI tool made with C# and WPF in an afternoon (sorry for the dirty code inside) those purpose is to make the log prettier to see, to read and to sort (to find the log we want easily and efficiently from the hundreds of logs hard to see on a command line windows).
- **Storm-Restarter**: This tool isn't made to be manipulated directly and is used to restart Storm application. You need to build it if you want to restart Storm from the menu.


# Launcher
Inside "Launcher" folder present into the Storm root folder, you'll find some utility scripts (batch) to easy start applications from built executable.
To use them, you need to have built all Storm projects once, and have all dependencies setup.
- **Storm**: This folder contains script to start the simulator (Storm.exe).
   - **Storm_Release.bat**: Start Release version of the simulator with default settings.  
   - **Storm_Debug.bat**: Start Debug version of the simulator with default settings. 
- **Storm-LogViewer**: This folder contains script to start the log viewer (Storm-LogViewer.exe).
   - **Storm-LogViewer.bat**: Start Storm-LogViewer.exe with default setting.
   - **Storm-LogViewer_NoInitRead.bat**: Start Storm-LogViewer.exe without initial read... The Storm-LogViewer will begin reading the last wrote log file from the moment where it was launched.


# Configuration

## Command line
Here are the command lines allowed values for the different executables. Command line keys are case unsensitive.

### Storm.exe
This is the simulation application. Command lines are exposed like this : --key=value or --key.
- **help (no value, facultative)**: Displays the help. The simulation won't be run and the other command line argument won't have any effect.
- **scene (string, facultative, accept macro)**: This is the scene config file path to use. If there is none, then we will ask the user to choose one at the start of the application with the default installed file explorer.
- **macroConfig (string, facultative, accept built-in only macros)**: This is the macro config file path to use. If there is none, then we will select the one inside the default Config folder (the one inside Custom/General takes precedence over the one inside Custom/General/Original).
- **generalConfig (string, facultative, accept macro)**: This is the general config file path to use. If there is none, then we will select the one inside the default Config folder (the one inside Custom/General takes precedence over the one inside Custom/General/Original).
- **tempPath (string, facultative, accept macro)**: This is the temporary path to use (to a folder). If there is none, then we will select the default temporary path folder.
- **mode (string, facultative)**: Specify the record/replay mode of the simulator. Accepted (case unsensitive) values are "Record" or "Replay". If the simulator is in record mode, then the simulation played will be recorded for a future replay. Default is unset, which means the application will just simulate without doing anything.
- **recordFile (string, facultative)**: Specify the path the recording will be. Record file path should remain unset if record mode is left unset, otherwise it should reference a valid record file in case we're in Replay mode (Note that this setting can be left Unset if there is a path inside the scene config loaded).
- **regenPCache (no value, facultative)**: Specify this flag and we will regenerate the rigid body particle cache data.
- **noUI (no value, facultative)**: Specify we don't want to visualize the simulation (save and speed up CPU ressource to focus only on what is important). This should be used with Record mode. Default is unset (which displays the simulation inside a UI).
- **threadPriority (string, facultative)**: Specify the priority of the simulation thread was should be taken. If it is unset, default OS priority will be applied. Accepted values (case unsensitive) are 'Below', 'Normal' or 'High'.
- **stateFile (string, facultative, accept macro)**: Specify the simulation state file to load from. If unset, we won't load a state file. Default is unset. Note that it doesn't make sense to start with a state file when we're replaying, therefore this setting isn't available if the mode is set to "Replay".
- **noPhysicsTimeLoad (no value, facultative)**: Specify we don't want to load the physics time recorded in the state file. Default is false (we want to load it). Note that the setting should only be used if we specified a state file to load.
- **noVelocityLoad (no value, facultative)**: Specify we don't want to load the velocities part of the simulation state file. Default is false (we want to load them). Note that the setting should only be used if we specified a state file to load.
- **noForceLoad (no value, facultative)**: Specify we don't want to load the forces part of the simulation state file. Default is false (we want to load them). Note that the setting should only be used if we specified a state file to load.


### Storm-LogViewer.exe
This is the application to see logs in a more friendly manner. Command lines are exposed like this : key=value. The accepted command line arguments are :
- **MacroConfigFilePath (string, facultative, accept built-in only macros)**: This is the macro config file path to use. If there is none, then we will select the one inside the default Config folder (the one inside Custom/General takes precedence over the one inside Custom/General/Original).
- **LogFilePath (string, facultative, accept macro)**: This is the log file to display. If there is none, then we will select the one latest inside the default Log folder (located inside the default temporary folder). By not setting it, we also allow the LogViewer to parse the next log when the day change (we will always select the latest file at runtime, at the moment we check for the file modification)...
- **NoInitialRead (no value, facultative)**: When a Simulator application is already running, then we'll just read and display the log after the moment since the log viewer started... Otherwise, we would display all logs.
- **ReadLast (no value, facultative)**: Read the last log file found. This flag is ignored if we specified a log file to read with LogFilePath...

### Storm-Packager.exe
This application is to package easily a running version of Storm application and Co. into a zip. It can also change and build a branch easily.
This is not a professional application though, so it does not intend to replace huge integration tool we could find on the market. But it has the advantage to be custom made for Storm application (it is like a build script, but made with c++ that I'm so much more familiar with)...
When you start it, wait for it to finish. Thanks... The allowed commands are :
- **build (string, facultative)**: When specifying it with the branch name to build (i.e --build=develop), the packager will build the specified branch before packaging it (in the example, it will build checkout develo, then build Storm.sln, package, and finally it will revert to the branch we were before checking out develop).


## Config file

### Generality
When reading this section, all xml tags are described like this :
- **tagName (type, modality:[mandatory/faculative/semi-faculative], ...)** : description<br>

♦ <ins>tagName</ins> is the name of the xml tag.<br>
♦ <ins>type</ins> is the value type of the xml value.<br>
♦ <ins>modality</ins> is the importance the tag is.
- If it is "mandatory" and the tag isn't set, the application will abort.
- If it is "faculative", the default value will be chosen if the value isn't set.
- If it is "semi-facultative", then the modality will depend on another setting : it could become mandatory or should remain unset.<br>
<br>

Except some exceptions described below, you should define the xml value for tagName like this : <br>
> \<tagName>value<\tagName>


<ins>Exceptions</ins>
- if the type is "vector3", then the xml should be defined like this : \<tagName x="xValue" y="yValue" z="zValue" \\>
- if the type is "RGBAcolor", then the xml should be defined like this : \<tagName r="rValue" g="gValue" b="bValue" a="aValue" \\>. Besides, all r, g, b, a values are to be float values between 0.0 and 1.0 included.
- if the type is "article", "misc", then see the section "Internal".


### Macro Configs

Macro are runtime substituated text defined by the user. In some text, we try to find a key and substituate in place with a value. The key would be a text under $[...] (i.e a macro with a key named "toto" and a value "titi", if inside a text we see $[toto] then we will replace it by titi).
It is useful to slimmer down a path or some texts.
Macro configuration are stored inside an xml file named Macro.xml located inside "Config\Custom\General".

It is shared by all Scenes and if there is no Macro.xml defined by the user, we will use the one inside the folder "Config\Custom\General\Original" that is a copy of the commited "Config\Template\General\Macro.xml". Note that we advise you to not make modification to this one and make your own Macro.xml at "Config\Custom\General" instead.
Or you can specify the path to the Macro.xml from the command line (see the specific section)

A Macro is defined by a tag "macro" inside "macros" and has 2 attributes :
+ "key" (string, mandatory) : a text by which we identify a macro. Do not add the macro identifier $[].
+ "value" (string, mandatory) : a text to substituate the key.
	
Besides, you can reference a macro into another macro, in any kind of order you want (define a macro after a macro that will use it). But beware, we solve the macro iteratively so do not make circular dependencies of macros or Storm.exe will exit after complaining.

There are some pre-built-in macros that aren't defined inside the macro file and can be used anywhere (even in command line) :
+ **$[StormExe]** will refer to the Storm executable.
+ **$[StormFolderExe]** will refer to folder that contains Storm executable that is running.
+ **$[StormRoot]** will refer, in case the executable location was never man-made changed, to the Storm root folder.
+ **$[StormConfig]** will refer, in case StormRoot macro is valid, to where Config files are set.
+ **$[StormResource]** will refer, in case StormRoot macro is valid, to where the Resource folder is.
+ **$[StormIntermediate]** will refer, in case StormRoot macro is valid, to where the Output folder is.
+ **$[StormRecord]** will refer, in case StormRoot macro is valid, to where the Record folder is.
+ **$[StormScripts]** will refer, in case StormRoot macro is valid, to where the Scripts folder is (where scripts will be put by default).
+ **$[StormStates]** will refer, in case StormRoot macro is valid, to where the States folder is (where all states will be registered in a folder representing the scene name).
+ **$[StormTmp]** will refer to the StormIntermediate if StormRoot macro is valid, or to OS defined temporary location.
+ **$[DateTime]** will refer to the current date when the Application is run (in filesystem compatible format : Weekday_Year_Month_Day_Hour_Minute_Second ).
+ **$[Date]**, like DateTime, will refer to the current date when the Application is run but without hours and lesser time division (in filesystem compatible format : Weekday_Year_Month_Day ).
+ **$[PID]** will refer to the process unique ID (PID).
+ **$[SceneName]** will refer to the chosen scene name. This is an exception to the pre-build-in macros which can be used anywhere. This macro can only be used after selecting a scene, therefore can only be used at some point like when we read the general config and the scene config. Therefore, be cautious when using this pre built macro.
+ **$[SceneStateFolder]** will refer to the default state folder path. Since this macro is made from SceneName macro, it is also an exception to the pre-build-in macros which can be used anywhere. This macro can only be used after selecting a scene, therefore can only be used at some point like when we read the general config and the scene config. Therefore, be cautious when using this pre built macro.


Note that macros are applied to command line as well except for the path to the macro configuration were we will use only the built-in macros (it is kind of expected since we don't know about those macros unless we get to read the file specified by the path of the command line...). But you're safe to use the prebuilt macros.


### General config

General config (named Global.xml) is global configuration of the application. It is shared by all scenes.
Like the Macro config, you can either specify one to use with command line, or it will search for one inside the default config folder ("Config\Custom\General" or "Config\Custom\General\Original" if it doesn't find it). We advise you to create and make your changes to the one inside "Config\Custom\General".


Unless explicited, the following settings doesn't support Macros (see section Macro)

Here the architecture of the config file (each section are a tag in xml where the subsection should be put into)


#### Application

- **displayBranch (boolean, facultative)**: If true, the branch the application was built upon will be displayed as part as its title when starting the application. It is useful for when we want to compare runs built from many branch and we want to know which is which. Default is false (because it is not really graceful if I want the application to be a proper application in the future).


#### Debug (facultative)

##### - Log (facultative)
- **logFolderPath (string, facultative, accept macro)** : The folder where to gather the log files. The default is the temporary path (StormTmp macro). If it is empty, default is considered.
- **logFileName (string, facultative, accept macro)** : The log file name of the current run. The default is empty. If it is empty, we won't log into a file (but the log will still be outputed to the console). Be aware to give unique name using the $[PID] macro or unexpected logging behavior could occurs because the file could be written by many processes at the same time (if you decides to start multiple Storm processes).
- **logLevel (string, facultative)** : The threshold level under which we ignore the log. Accepted values are in that inmportance order : Debug, DebugWarning, DebugError, Comment, Warning, Error, Fatal.
Note that the maximum value you can set is Fatal, it means that no matter what level you set, we would still log "Fatal" and "Always" logs. The default is Debug.
- **override (boolean, facultative)** : If the log file specified should have its content overriden each time. If "false", the content will be appended instead. Default is "true".
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


#### Web (facultative)
- **browser (string, facultative)**: Specify the browser to use when opening an url. This is not case sensitive. Accepted values are "Chrome", "Firefox", "Edge", "InternetExplorer" or you can leave the field empty. Default is no browser. Note that if there is no browser set, then opening an url won't work and an error will be issued instead.
- **incognito (boolean, facultative)**: Specify we want to open a new browser window in incognito mode when we'll browse through an url. Default is false.


#### Graphics (facultative)
- **screenWidth (unsigned integer, facultative)** : Set the expected windows width at startup. Note that it could be something else near this value... Default is 1200.
- **screenHeight (unsigned integer, facultative)** : Set the expected windows height at startup. Note that it could be something else near this value... Default is 800.
- **screenX (integer, facultative)** : Set the expected windows x position at startup. If it is unset (default), then a default value will be chosen from the OS...
- **screenY (integer, facultative)** : Set the expected windows y position at startup. If it is unset (default), then a default value will be chosen from the OS...
- **fontSize (unsigned integer, facultative)** : the font size of any written information displayed in the HUD... Default is 16.
- **nearFarPlaneFixed (boolean, faculative)**: Specify if the normal behavior of the near and far plane should be to NOT translate along the camera moves. Default is true.
- **selectedParticleAlwaysOnTop (boolean, faculative)**: Specify if the selected particle should be displayed on top of all particles (on the near plane). Default is false.
- **selectedParticleForceAlwaysOnTop (boolean, faculative)**: Specify if the selected particle force should be displayed on top of all particles (on the near plane). Default is true.


#### Simulation (faculative)
- **allowNoFluid (boolean, facultative)**: If true, we will allow the scene config file to not have any fluid (useful for testing rigid body features without minding particles while developping). Default is false.



### Scene Config

Scene configuration files contains all the data for running a simulation, therefore it is mandatory to specify one. If it was not set from the command line, Storm application will open an explorer windows to allow you to choose one.
Unlike the others config files, it can be named as you want. Here the xml tags you can set :


#### General
- **startPaused (boolean, facultative)**: If true, the simulation will be paused when the application start. Default is false.
- **gravity (vector3, facultative)**: This is the gravity vector, in meter per second squared. Default value is { x=0.0, y=-9.81, z=0.0 }
- **particleRadius (float, falcultative)**: This is the particle radius in meter. It is also what is used for display but has also a physical value. Default value is 0.05.
- **kernelCoeff (positive float, falcultative)**: This is the kernel multiplicator coefficient (without unit). If this value is equal to 1.0, then the kernel length would be equal to the particle radius. Default value is 4.0.
- **maxKernelIncrementCoeff (positive float, falcultative)**: This is a runtime end increase value to the kernel coefficient (without unit). It is made to increase the kernel coefficient at runtime with the elapsed physics time. Default value is 0.0 (no increase).
- **kernelIncrementCoeffEndTime (positive float, falcultative)**: This is the end time in seconds when we stop increasing the kernel coefficient. After this time, the kernel length would be at its final value. Default value is the feature is disabled.
- **CFLCoeff (positive float, falcultative)**: This is the coefficient lambda for the CFL (Courant-Friedrich-Levy conditions). It is used only if CFL is enabled. Default value is 0.4.
- **MaxCFLTime (positive float, falcultative)**: This is the max time the CFL (Courant-Friedrich-Levy conditions) could take in seconds. It is used only if CFL is enabled. Default value is 0.5 seconds (500ms).
- **CFLIteration (positive integer, falcultative)**: This is the max time the CFL (Courant-Friedrich-Levy conditions) Should be run within one iteration. Default value is 2.
- **kernel (string, falcultative)**: This is the kernel we would use. It isn't case sensitive and the accepted values are for now "CubicSpline" (default) and "SplishSplashCubicSpline".
- **physicsTime (float, falcultative)**: This is the iteration loop physics time in seconds. If this value is less or equal to 0, then we would adapt it automatically using CFL condition. Default value is -1.
- **fps (float, falcultative)**: This is the expected frame rate (it has nothing to do with the physics time, this is the refresh rate of many loop inside the engine to not consume too much cpu). If this value is less or equal to 0, then we would set it to the default value which is 60 FPS.
- **simulationNoWait (boolean, falcultative)**: Set this flag to true to run the simulation as fast as possible (disabling the framerate binding on the simulation thread, therefore removing the synchronisation wait that bind it to a specific framerate). Default is false
- **simulation (string, mandatory)**: This is the simulation mode we will run. It isn't case sensitive. Accepted values (for now) are "DFSPH", "IISPH", "PCISPH" and "WCSPH".
- **minPredictIteration (positive integer, facultative)**: This is the minimum iteration we need to make inside one simulation loop when doing prediction iteration. The more iteration loop before returning a result there is, the stabler ans slower the simulation would be. Default value is 2 (0 is ignored, we should at least make one iteration).
- **maxPredictIteration (positive integer, facultative)**: This is the max iteration we're allowed to make inside one simulation loop when doing prediction iteration. It is to avoid infinite loop. It should be an integer strictly greater than 0 and should be greater or equal than minimum prediction iteration. Default value is 150.
- **maxDensityError (positive float, facultative)**: This is the max density error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.
- **neighborCheckStep (positive integer, facultative)**: This is a char between 1 and 255. This specify that we will recompute the neighborhood every neighborCheckStep step. Default is 1 (we recompute each step of the simulation).
- **collidingFluidRemoval (boolean, facultative)**: If true, the fluid particle colliding with already existing rigid body particle will be removed at initialization time. This does not reproduce the behavior of SplishSplash. Default is true.
- **startFixRigidBodies (boolean, facultative)**: If true, dynamic rigidbodies will be fixated in place at simulation start. See input keys to unfix them. Default is false.
- **endPhysicsTime (float, facultative)**: This is the end time (physics time) in seconds the simulation should stop. After this time, the Simulator will exit... The value should be greater than zero. Default is unset (the simulator will continue indefinitely).


#### Graphics
- **cameraPosition (vector3, facultative)**: This is the camera (viewpoint) initial position. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=-10.0 }.
- **cameraLookAt (vector3, facultative)**: This is the initial position of the target the camera look at. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=0.0 }.
- **zNear (float, facultative)**: This is the initial distance in meter from the camera which everything that is nearer than this distance won't be rendered. Default value is 0.01.
- **zFar (float, facultative)**: Same as zNear except that we skip displaying all objects farer than this distance value. Default value is 20.0.
- **minVelocityColorValue (float, facultative)**: Set the minimum velocity value for the watched value to display the coldest color. This is to be expressed as a squared norm. Default value is 0.01.
- **maxVelocityColorValue (float, facultative)**: Set the maximum velocity value for the watched value to display the hotest color. This is to be expressed as a squared norm. Default value is 100.
- **minPressureColorValue (float, facultative)**: Set the minimum pressure value for the watched value to display the coldest color. Default value is 0.
- **maxPressureColorValue (float, facultative)**: Set the maximum pressure value for the watched value to display the hotest color. Default value is 10000.
- **minDensityColorValue (float, facultative)**: Set the minimum density value for the watched value to display the coldest color. Default value is 0.
- **maxDensityColorValue (float, facultative)**: Set the maximum density value for the watched value to display the hotest color. Default value is 2000.
- **blowerAlpha (float, facultative)**: Set the blower visualization alpha channel. Must be between 0 and 1 included. Default value is 0.25.
- **grid (vector3, facultative)**: Set the grid dimension. X coord will be the grid width, Z its depth and Y will be the height where the grid will be drawn. Note that X and Z will be ceiled. Default value is { x=10.0, y=0.0, z=10.0 }
- **particleDisplay (boolean, facultative)**: Specify if Solids should be displayed as particle on start. If not, they will be displayed as meshes. "false" by default.
- **constraintThickness (float, facultative)**: Specify the thickness of the line when visualizing the constraint. It should be a positive non-zero value. Default is "General.particleRadius / 3.0".
- **constraintColor (RGBAcolor, facultative)**: Specify the color of the line when visualizing the constraint. Default is { r=1.0, g=0.1, b=0.1, a=0.8 }.
- **forceThickness (float, facultative)**: Specify the thickness of the line when visualizing the selected particle force. It should be a positive non-zero value. Default is "General.particleRadius / 3.0".
- **forceColor (RGBAcolor, facultative)**: Specify the color of the line when visualizing the selected particle force. Default is { r=0.0, g=1.0, b=1.0, a=0.8 }.


#### Record
- **recordFps (float, semi-facultative)**: This is the record fps. It becomes mandatory if the Simulator is started in Record mode.
- **recordFile (string, facultative, accept macros)**: Specify the path the recording will be. This path will be used in case it wasn't set from the command line.
- **replayRealTime (boolean, facultative)**: Fix the replay to be the nearest possible from a real time replay. It means that the simulation speed will try to be as near as possible to 1.0. Default is true.


#### Script
- **enabled (bool, facultative)**: Setting it to false prevent the engine to watch the scripting file. Default is true. Note that the scripting API will still be functional, it is that the engine won't be able to answer to outside commands.
- **Init (tag)**: The is the section to control the initialization script call. For further details, please refer to "Scripting API" section.
	+ **initScriptFile (string, facultative, accept macros)**: The path to the script init file containing the initialization script. Default is empty, we won't use any script to initialize the engine. Note that the path should either remain empty, be a valid path to a plain text file, or shouldn't exist (we'll create one for you).
- **Runtime (tag)**: The is the section to control the script used as a way to communicate with the engine at runtime. For further details, please refer to "Scripting API" section.
	+ **watchedScriptFile (string, facultative, accept macros)**: The path to the watched script file. Default is equivalent to "$[StormScripts]/RuntimeScript.txt". This path should either remain empty, point to a valid text file, or not exist (we'll create one for you).
	+ **refreshTime (unsigned int, facultative)**: The refresh time in milliseconds of the watched script. Default is 100 ms. Cannot be equal to 0. Besides, note that even if it is below the framerate time of the engine, it won't be watched faster (in another word, the real value is the maximum between this "value" and "1000 / the expected engine framerate")


#### Fluid
This element is all setting appartaining to a fluid. Here the tag you can set inside :
- **id (positive integer, mandatory)**: This is the unique id of the fluid. It should be unique (Note that it should not be equal to any rigid body id and blower id too).
- **fluidBlock (tag, at least one)**: This is the fluid generator settings. There should be at least one.
	+ **firstPoint (vector3, facultative)**: This is one of the corner of the box where fluid particle should be generated. It cannot have the same value than secondPoint, default value is { x=0.0, y=0.0, z=0.0 }.
	+ **secondPoint (vector3, facultative)**: This is the opposite corner from firstPoint where fluid particle should be generated. It cannot have the same value than firstPoint, default value is { x=0.0, y=0.0, z=0.0 }.
	+ **denseMode (string, facultative)**: This is the load mode impacting how we will generate and set the particle positions inside the block. It isn't case sensitive. Accepted values are : "Normal" (default) or "SplishSplash" (to use SplishSplash algorithm to generate the particles).
- **density (positive float, falcultative)**: This is the rest density of the fluid in kg.m^-3. Default is 1.2754 kg.m^-3 which is the density of Dry air at 0 °C and normal ATM pressure.
- **pressureK1 (positive zero-able float, falcultative)**: This is the pressure stiffness constant coefficient used when initializing the pressure using State equation. In formulas, it is often found as k1. Default is 50000.
- **pressureK2 (positive zero-able float, falcultative)**: This is the pressure exponent constant coefficient used when initializing the pressure using State equation. In formulas, it is often found as k2. Default is 7.
- **relaxationCoeff (positive zero-able float, falcultative)**: This is the relaxation coefficient (alias omega) used inside some Simulation methods when computing prediction pressures. It should be between 0.0 and 1.0 included. Default is 0.5.
- **initRelaxationCoeff (positive zero-able float, falcultative)**: This is the pressure initial relaxation coefficient used inside some Simulation methods (like IISPH) when initializing pressure fields. It should be greater or equal to 0.0. Default is 0.5.
- **viscosity (positive float, falcultative)**: This is the dynamic viscosity of the fluid in N.s/m² (or Pa.s). Default is 0.00001715 N.s/m² which is the dynamic viscosity of Dry air at 0 °C and normal ATM pressure.
- **soundSpeed (positive float, falcultative)**: This is the speed of sound inside the given fluid in m/s. Default is 331.4 m/s which is the speed of sound of Dry air at 0 °C) and normal ATM pressure (340 is for 15 °C).
- **enableGravity (boolean, falcultative)**: Enable the gravity for the associated fluid particle system. Default is true.


#### RigidBodies
Inside this element should be put all rigidbodies. Each rigidbody should be specified under a tag named "RigidBody".

##### RigidBody
- **id (positive integer, mandatory)**: This is the unique id of the rigid body.  It should be unique (Note that it should not be equal to any fluid id or blower id).
- **meshFile (string, mandatory, accept macro)**: This is mesh file path this rigid body is bound to.
- **isStatic (boolean, facultative)**: Specify this to tell the simulation that this object is fixed (won't move throughout the simulation). Default value is "true".
- **wall (boolean, facultative)**: Specify that this object is the wall (considered to be a wall). By defainition, a wall is static so if the value is true, the object will be considered static no matter the value set to isStatic. Default value is "false".
- **collisionType (string, facultative)**: Specify what is the collision shape should be handled. This is not case sensitive. Possible values are "None" (Default value), "Sphere", "Cube" and "Custom".
- **translation (vector3, facultative)**: The initial position in meters of the object. Default value is { x=0.0, y=0.0, z=0.0 }.
- **rotation (vector3, facultative)**: The initial rotation in degrees of the object (this is euler angle : roll, pich, yaw). Default value is { x=0.0, y=0.0, z=0.0 }.
- **scale (vector3, facultative)**: The initial scale of the object. Default value is { x=1.0, y=1.0, z=1.0 }.
- **staticFrictionCoeff (float, facultative)**: The static friction coefficient of the object, it should be larger than 0.0. PhysX needs it but physically speaking I don't know what to set. This is the minimum force norm threshold that makes our object move. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **dynamicFrictionCoeff (float, facultative)**: The dynamic friction coefficient of the object, it should be positive. PhysX needs it but physically speaking I don't know what to set. This is the the velocity reduction when a rigid body moves with a contact with another. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **restitutionCoeff (float, facultative)**: The restitution friction coefficient of the object (the bounciness of the object), it should be positive but close or above 1.0 may cause instabilities. PhysX needs it but physically speaking I don't know what to set. Closer it is to 0.0, less the object will bounce and more it will lose energy when being in contact with another rb. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **angularDamping (float, facultative)**: PhysX angular velocity damping of 0.05 is a lie, therefore we need to expose a tweakable setting to do it ourself. The value shouldn't be greater than 1.0. 0.0 means no damping, 1.0 means full damping (the object won't rotate anymore). If the value is negative (then the velocity will be increased instead, but beware because there is a high chance the simulation could become unstable). Besides, this setting is only for dynamic rigidbodies. Default is 0.05 (as PhysX sets).
- **fixTranslation (boolean, facultative)**: Specify if the rigid body is allowed to translate. If true, then the rigid body will only be able to rotate in place. Default is false.
- **mass (positive float, mandatory)**: The mass of the rigid body in kg. This has to be strictly positive (> 0.0). 
- **viscosity (positive float, facultative)**: The viscosity of the rigid body in Pa.s (???). This has to be strictly positive (> 0.0).
- **layerCount (positive integer, facultative)**: The boundary particles layer to generate along the rigid body surface. Should be a non zero positive integer. Default is 1.
- **layeringGeneration (string, facultative)**: The technique used to generate boundary particles layer on the rigid body surface. It isn't case sensitive and the accepted values are "Scaling" (default) and "Dissociated".
  * "Scaling" technique generates the layer as if the layer was generated by scaling the rigid body.
  * "Dissociated" technique generates the layer by extruding each triangle along its normal.
- **pInsideRemovalTechnique (string, facultative)**: Specify the technique used to detect and remove fluid particle inside the rigidbody. It isn't case sensitive and the accepted values are "None" (default) and "Normals". Note that it is illegal to set something else than "None" for wall rigidbodies.
  * "None" means no technique will be used, therefore we won't check nor remove inside particle.
  * "Normals" means we will use rigidbody normals to detect a particle inside the rigidbodies... We suppose the rigid bodies has normals those directions point outside and that the rigidbody has no holes.


#### Constraints

##### Constraint
- **rbId1 (positive integer, mandatory)**: This is the id of the first rigid body the contraints is attached to. It is a mandatory setting and the rigid body id must exist.
- **rbId2 (positive integer, mandatory)**: This is the id of the second rigid body the contraints is attached to. It is a mandatory setting and the rigid body id must exist.
- **length (positive float, facultative)**: This is the max length (in meter) of the contraints separating both rigid bodies (+ the initial distance between those rigidbodies). It should be a positive value and default is 0.0.
- **rb1LinkOffset (vector3, facultative)**: This is the translation offset of the link endpoint on the rigid body 1 relative from its center. All coordinate are expressed in meters. Default is { x=0.0, y=0.0, z=0.0 } (the link end point is located at the center of the rigid body 1).
- **rb2LinkOffset (vector3, facultative)**: This is the translation offset of the link endpoint on the rigid body 2 relative from its center. All coordinate are expressed in meters. Default is { x=0.0, y=0.0, z=0.0 } (the link end point is located at the center of the rigid body 2).
- **noRotation (boolean, facultative)**: If true, the Constraint won't rotate along the constraint normal axis (rotation along the constraint axis will still be allowed). Default is "true".
- **visualize (boolean, facultative)**: If true, the Constraint will be displayed. Default is "true".


#### Blowers

##### Blower
- **id (positive integer, mandatory)**: This is the blower id. This setting is mandatory. Note that this will also be the id of the blower rigid body that will be generated along your blower so it should also be unique with rigidbodies id and fluids id.
- **type (string, mandatory)**: This is the blower type. This setting is non case sensitive and is mandatory. It defines what your underlying blower would be. For now, the accepted values are "Cube", "Sphere", "Cylinder", "Cone", "RepulsionSphere", "PulseExplosion" and "Explosion".
- **startTime (positive float, facultative)**: This defines the time in simulation second time your blower start working. Before this value, your blower will remain disabled. This value + fadeInTime shouldn't be greater than the endTime - fadeOutTime. Default value is 0.0 (the blower start right away).
- **endTime (positive float, facultative)**: This defines the time in simulation second time your blower stop completely. After this value, your blower will remain disabled. This value minus fadeOutTime shouldn't be lesser than the start time + fadeInTime. If -1.0 is specified, the endTime will be ignored and the blower will continue indefinitely. Default value is -1.0.
- **fadeInTime (positive float, facultative)**: This defines the time in simulation second time the blower will take to attain its peak force. 0.0 means that it will be instant. The increase rate is linear between startTime and startTime + fadeInTime.
- **fadeOutTime (positive float, facultative)**: This defines the time in simulation second time the blower will take to stop from its peak force to a null force. 0.0 means that it will be instant. The decrease rate is linear between endTime - fadeOutTime and endTime.
- **radius (positive float, semi-mandatory)**: This defines the radius in meters of the blower. This setting is mandatory for Sphere type blowers and should be a positive non-zero floating point number. Any uses for another blower type than sphere derived blowers aren't allowed.
- **upRadius (positive float, semi-mandatory)**: This defines the radius in meters of the Cone blower up disk. This setting is mandatory for Cone type blowers and should be a positive floating point number. Any uses for another blower type than Cone derived blowers aren't allowed.
- **downRadius (positive float, semi-mandatory)**: This defines the radius in meters of the Cone blower down disk. This setting is mandatory for Cone type blowers and should be a positive floating point number. Any uses for another blower type than Cone derived blowers aren't allowed.
- **height (positive float, semi-mandatory)**: This defines the height in meters of the blower. This setting is mandatory for Cylinder type blowers and should be a positive non-zero floating point number. Any uses for another blower type than cylinder derived blowers aren't allowed.
- **dimension (vector3, semi-mandatory)**: This defines the dimension of the blower. This setting is mandatory for Cube type blowers and all coordinates should be positive non-zero values. This is the scaling of a 1m x 1m x 1m cube... Any uses for another blower type than cubes derived blowers aren't allowed.
- **position (vector3, facultative)**: This defines the position (all coordinates are meters) of the origin center of the blower. Default is { x=0.0, y=0.0, z=0.0 }...
- **force (vector3, facultative)**: This defines the force applied by the blower to each particle in range (inside its effect area defined by the blower's dimension). Default is { x=0.0, y=0.0, z=0.0 }...


### Internal

Internal Config is a versioned config that mustn't change much. It contains data that I didn't want to hard code.<br><br>
Since this configuration file shouldn't be exposed a lot (but I'll document it nevertheless), we wouldn't be permissive in case an error happens and immediately exit the application.<br>
The location of the Internal config named "InternalConfig.xml" is inside under the folder "$[StormConfig]/Internal".
So now, you are warned to not modify this config file unless it is truly necessary.

#### References
References contains a list of all references to articles, papers, books, websites, ... I used :
- **article (tag, facultative)**: Specify the reference is an article/papers/book. The exposed attributes are :
	+ **authors (string, mandatory)**: Specify the authors of the reference. If there is more than one authors, they should all be separated by a comma ','.
	+ **name (string, mandatory)**: Specify the name of the reference. Note that the '\n' will be interpreted as a line break.
	+ **date (string, mandatory)**
	+ **serialNumber (string, mandatory)**: ISBN, SN, DOI, ... It can be anything that allows to reference this article uniquely.
	+ **url (string, facultative)**: An URL to where we can retrieve the article easily. (Though we don't specify if it will be free of charge)
	+ **bibTex (string, facultative)**: The relative path from BibTex folder (folder besides the InternalConfig.xml) to a text file containing the BibTex reference of the article. Note that we don't validate the BibTex string is valid.
- **misc (tag, facultative)**: Specify a miscellaneous reference. It can be anything like a tutorial, a website, another engine source code, ...
	+ **authors (string, mandatory)** Specify the authors of the reference. If there is more than one authors, they should all be separated by a comma ','.
	+ **name (string, mandatory)**: Specify the name of the reference. Note that the '\n' will be interpreted as a line break.
	+ **date (string, facultative)**
	+ **serialNumber (string, facultative)**: Anything that allows to reference this reference uniquely.
	+ **url (string, facultative)**: An URL to where we can retrieve the reference easily. (Though we don't specify if it will be free of charge)
	+ **bibTex (string, facultative)**: The relative path from BibTex folder (folder besides the InternalConfig.xml) to a text file containing the BibTex reference of the article. Note that we don't validate the BibTex string is valid.


# Input bindings

## Key bindings

Note : If the term in the parenthesis is "Numpad", then the keybinding is the value inside the Numpad only. If it is "Key", then the value is not the one present on the Numpad.

- **Escape key (ESC)**: Leave the application.
- **Space bar**: Pause application if it is running. Unpause if it is paused.
- **+ (Numpad)**: Move near clipping plane forward.
- **- (Numpad)**: Move near clipping plane backward.
- **/ (Numpad)**: Move far clipping plane backward.
- **\* (Numpad)**: Move far clipping plane forward.
- **Up Arrow**: Move camera up.
- **Down Arrow**: Move camera down.
- **Left Arrow**: Move camera left.
- **Right Arrow**: Move camera right.
- **8 (Numpad)**: Move camera forward.
- **2 (Numpad)**: Move camera backward.
- **Q**: Rotate the camera around the target on Y axis (negative).
- **Z**: Rotate the camera around the target on X axis (positive).
- **D**: Rotate the camera around the target on Y axis (positive).
- **S**: Rotate the camera around the target on X axis (negative).
- **0 (Numpad Zero)**: Reset camera position.
- **N**: Advance the paused simulation to the next frame.
- **E**: Enable all disabled blower, and disable all enabled ones.
- **R**: Enable (if disabled)/Disable (if enabled) raycasts system.
- **L**: Fix (if unfixed)/Unfix (if fixed) rigid bodies.
- **Y**: Cycle the particle selection force to display (between [...] -> Pressure -> Viscosity -> AllForces (except gravity) -> Total force on rigid body -> [...]).
- **I**: Reset the replaying to the first frame. This feature exists only in replay mode.
- **C**: Request all forces check to zero as per physics conservation of momentum law says for isolated systems that is in an equilibrium state.
- **J**: Force refresh watched script files. Or re read all of them.
- **X**: Save the current simulation state.
- **1 (Key)**: Decrease the physics delta time. Valid only if we are not in replay mode, or if CFL is disabled.
- **2 (Key)**: Increase the physics delta time. Valid only if we are not in replay mode, or if CFL is disabled.
- **F1**: Debug command to print to a human readable text giving all position, velocity and force values of all fluid particles. The data is printed inside the output (temp) directory inside "Debug" folder.
- **F2**: Show/Hide the axis coordinate system displayed on the UI.
- **F3**: Cycle the color setting of the fluid particle data displayed from ... -> Velocity -> Pressure -> Density -> ... .
- **F4**: Set selected particle forces always on top flag to true if false, false otherwise. But note that if it is true, you'll always see the particle force if it is in front of the view point, but you'll lose the depth information of the vector.
- **F5**: Set wireframe.
- **F6**: Set solid state with back face culling.
- **F7**: Set solid state without back face culling.
- **F8**: Set rendering to display all particles.
- **F9**: Set particle state without rendering walls.
- **F12**: Toggle the UI fields display (displays it if hidden, or hide it if displayed).
- **Ctrl+S**: Open the script file inside notepad++ (or notepad if notepad++ isn't installed).
- **Ctrl+X**: Open the current xml config file inside notepad++ (or notepad if notepad++ isn't installed).
- **Ctrl+L**: Open the Storm log viewer. This application will have its life shared with the Storm application.


## Mouse bindings
- **Left click on dynamic rigid body particle**:
    - Select the particle to display either its Pressure force, Viscosity force or the sum of those force (see "Cycle the particle selection force to display" key binding). This binding is only valid if the raycast is enabled (see "Enable/Disable raycasts system" key binding).
- **Middle click on particle**:
    - Set the camera target to the particle position. This binding is only valid if the raycast is enabled (see "Enable/Disable raycasts system" key binding).
- **Mouse wheel**:
    - Increase/Decrease the camera motion speed 
    - Increase/Decrease the clipping plane motion speed
    - Increase/Decrease the physics time step change speed. Valid only if we are not in replay mode, or if CFL is disabled.


# Scripting API

Scripting is done with lua. You'll be able to send lua command to the engine from a plain text file that is watched once every "refreshTime" millisecond for any changes.
Just save your file after modifying it and the content will be sent.
There is also the initialization file that allows to customize further the initialization of the engine. This custom initialization is done after the engine ended its normal initialization.

The reasons I decided to include a scripting API is because : 
- we have too much inputs and too little keys to control all features I want to add. And it is starting to be a real bother to remember all keys.
- we could control many instance of the application at the same time (just make them watch the same scripting file and they'll execute the command seemingly at the same time).
- Better control over the parameter (where key input uses pre-built-in default value or separate manipulation to control how the action will respond, we can just pass users parameters to the script to execute).
- Script is the only way to control the application when it doesn't run in UI mode.


## Developpers notes

The scripting wrapping was done to abstract modules from the scripting API we use. Like this, we could switch or add python, ... any scripting language we want with minimal effort. <br>
The entry point of the scripting API object/methods registration can be anything. Just use the macro "STORM_IS_SCRIPTABLE_ITEM" inside your class. If it is used inside a Singleton registered inside the SingletonAllocator, the call to the entry point will be automatic.<br>
Register all your methods, classes, ... inside ScriptImplementation.inl.h. An example of how to do it would be to look at how I did it with the SimulatorManager. Don't mind the include because this file will be included inside source file only + all singleton will be included anyway.<br>
Finally, be aware that all scripting will be executed inside the same Scripting thread that is a specific registered thread. Therefore, all method that the API calls should be responsible to dispatch the real execution to the expected threads to avoid any threading issues.


## Commands

Storm script commands are not part of any scripting language and are parsed separately from what is sent to the interpreter. Commands are also ignored by said interpreter.
They are like headers those purpose is to control how the script will be played. A script body can only have one or no header commands and is placed after the header.<br>
Header part of the script where commands are defined must begin and end by the delimitor string "**#####**".<br>
Then should follow the script body that will be sent to the interpreter. <br>
A command inside the header is declared on the same line : In another word, 2 different lines are 2 different commands.<br>
A command must start with a keyword defining what it is, folowwed by some parameters if needed. Both are separated by the character "**:**" .<br><br>
Here the list of available commands :
- **pid** : Parameters are the list of PIDs separated by a ' ' or a ','. It specifies that the following script must be executed only by the processes referred by the listed PIDs.
- **enabled** : It specifies the following script is enabled if the parameter is either "true", "on" or "1". Disabled if parameter is either "false", "off" or "0". There could only be 1 of this key specified in the same command.


## Exposed methods

#### SimulatorManager (simulMgr)

- **void resetReplay()**: Reset the replay. This is the same method bound to the key inputs.
- **void saveSimulationState()**: Save the simulation state into a state file. This is the same method bound to the key inputs.
- **void advanceOneFrame()**: Advance the simulation to the next frame. Available only if the simulation is paused. This is the same method bound to the key inputs.
- **void advanceByFrame(int64_t frameCount)**: Advance the paused simulation by frameCount frames. The frameCount value must be positive !
- **void advanceToFrame(int64_t frameNumber)**: Advance the paused simulation to a specific frame. The frameNumber value must be positive !


#### TimeManager (timeMgr)

- **bool changeSimulationPauseState()**: Pause/Unpause the simulation. This is the same method bound to the key inputs.


#### GraphicManager (graphicMgr)

- **void cycleColoredSetting()**: Cycle the particle coloring observed quantities. This is the same method bound to the key inputs.
- **void setColorSettingMinMaxValue(float minValue, float maxValue)**: Set the min and max values for the observed particle colors fields.
- **void showCoordinateSystemAxis(const bool shouldShow)**: Display the axis coordinate system if true, hide it otherwise.
- **void setUIFieldEnabled(bool enable)**: Set if we should display the UI fields (enabled to true) or hide it (enabled to false).


#### PhysicsManager (physicsMgr)

- **void setRigidBodyAngularDamping(const unsigned int rbId, const float angularVelocityDamping)**: Set the angular velodity damping value of the rigid body specified by its id. This method is only defined for dynamic rigid bodies.
- **void fixDynamicRigidBodyTranslation(const unsigned int rbId, const bool fixed)**: Set the specified rigid body's translation fixed flag. This method is only defined for dynamic rigid bodies.


#### OSManager (osMgr)

- **void clearProcesses()**: Clear all processes (this closes all processes those life will be shared with Storm application).


#### WindowsManager (winMgr)

- **void restartApplication(const std::string_view &additionalArgs)**: Restart Storm application and applies additional parameters. Note that the application will be restarted with to the current scene and will forward its current command line, therefore additional parameters should only contains parameters that aren't already in those.
