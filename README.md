# Storm
SPH reimplementation for fluids simulation


# Setup
From the root folder containing this README. Modify the Build\Script\Internal\UserSettings.bat to match the path of each dependencies in your workstation.
Note that : 
- you should have downloaded all dependencies beforehand and have done all setup that was specified (see next section).
- We rely on junctions. It means that all your dependencies folder and Storm project should be on compatible disks format (i.e. NTFS). But if junstion does not work, we advise to copy your dependencies under the Storm's root "Dependencies" folder after calling Setup (also change the way the batch file are executed or you will remove those dependencies each time).



# Prerequisite
- **Visual Studio Community 2019 v16.6.0 with C++20** (in fact the latest draft that was a preview of C++20). Maybe it works for a later Visual Studio but I have never tested it.
- **Python 2.7.6 or later** (needed to build PhysX, see "Dependencies list" section). To know what is the current version of your installed python (or if there is any), type python in a cmd console.
- **Visual Studio 2017 toolsets (v141)**
- **cmake_gui 3.15.0-rc1 or later**. (3.15.0-rc1 is what I used. I can't guarantee for a version below this one)...



# Dependencies list
- **Boost 1.72** compiled for Visual Studio 2019 (link: https://www.boost.org/users/history/version_1_72_0.html ). Follow the instructions on their site.
- **OIS v1.5** compiled for Visual Studio 2019 (link: https://github.com/wgois/OIS/tree/v1.5 ). Follow the instructions on their site.
	+ Generate the Visual studio file inside a folder named "bin" at OIS root folder (i.e if OIS is installed like this : C:/dep/OIS, then generate the vs project file into C:/dep/OIS/bin).
	+ generate with default settings
	+ build OIS into Debug and Release configurations.
- **PhysX v4.0.0** build for Visual Studio 2017 (link: https://github.com/NVIDIAGameWorks/PhysX/tree/4.0.0 ). (You can use Visual Studio 2019 but the toolset installed should be 2017 (v141), or not... see the first point)
	+ Follow instruction on their site. Note that it isn't said in their website, but the generated solution to build is under physx/compiler/[the setting you've chosen].
Note that I chose "vc15win64" settings. If you use another Visual Studio, be aware of Binary compatibility (Visual Studio 2015, 2017 and 2019 are binary compatible, but I don't know about my future and they aren't with the past). Be also aware that the result file won't be in the same folder than what was expected by this readme so adapt a little.
	+ Build the library in "release" and "debug".
	+ Go to "physx/bin/win.x86_64.vc141.mt" (this name could change if you have chosen another build setting) and copy "debug" and "release" folder inside a new folder "physx/lib"



# Modules
- **Storm**: This is the executable that runs the SPH simulation.
- **Storm-Config**: This is the configuration module.
- **Storm-Graphics**: This is the module responsible to display and render the simulation. We use DirectX.
- **Storm-Helper**: This module contains all helpers that should be shared among all modules.
- **Storm-Input**: This module is for managing inputs, bindings and their callback.
- **Storm-Logger**: This module is for logging.
- **Storm-Misc**: This module is intended to gather module manager helper that shouldn't be made into helpers since they are Storm project implementation related but cannot be put into any other modules. Example are the RandomManager (that should be deterministic), the TimeManager (that should be compliant with Storm simulator), the AsyncManager (that is intended to be executed in the main Simulation loop like Unreal's Async method), ... .
- **Storm-ModelBase**: This module is the base for each modules. It ontains everything that could be used to bind the modules together without the need to reference each other.
- **Storm-Physics**: This module is responsible for initializing and managing Physics computations.
- **Storm-Simulator**: This module is where the Simulation classes would be. It is responsible to handle SPH.
- **Storm-Windows**: This module is responsible for managing the Windows and everything related to it.


# Configuration

## Command line


## Config file

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
+ **$[StormIntermediate]** will refer, in case StormRoot macro is valid, to where the Output folder is.
+ **$[StormTmp]** will refer to the StormIntermediate if StormRoot macro is valid, or to OS defined temporary location.
+ **$[DateTime]** will refer to the current date when the Application is run (in filesystem compatible format : Weekday_Year_Month_Day_Hour_Minute_Second ).
+ **$[Date]**, like DateTime, will refer to the current date when the Application is run but without hours and lesser time division (in filesystem compatible format : Weekday_Year_Month_Day ).
	
	
Note that macros are applied to command line as well except for the path to the macro configuration were we will use only the built-in macros (it is kind of expected since we don't know about those macros unless we get to read the file specified by the path of the command line...). But you're safe to use the prebuilt macros.

### General config

General config (named Global.xml) is global configuration of the application. It is shared by all scenes.
Like the Macro config, you can either specify one to use with command line, or it will search for one inside the default config folder ("Config\Custom\General" or "Config\Custom\General\Original" if it doesn't find it). We advise you to create and make your changes to the one inside "Config\Custom\General".


Unless explicited, the following settings doesn't support Macros (see section Macro)

Here the architecture of the config file (each section are a tag in xml where the subsection should be put into)

#### Log (faculative)
- **logFolderPath (string, facultative, accept macro)** : The folder where to gather the log files. The default is the temporary path (StormTmp macro). If it is empty, default is considered.
- **logFileName (string, facultative, accept macro)** : The log file name of the current run. The default is empty. If it is empty, we won't log into a file (but the log will still be outputed to the console).
- **logLevel (string, facultative)** : The threshold level under which we ignore the log. Accepted values are in that inmportance order : Debug, DebugError, Comment, Warning, Error, Fatal.
Note that the maximum value you can set is Fatal, it means that no matter what level you set, we would still log "Fatal" and "Always" logs. The default is Debug.
- **override (boolean, facultative)** : If the log file specified should have its content overriden each time. If "false", the content will be appended instead. Default is "true".
- **removeOlderThanDays (integer, faculative)** : Specify the number of day Storm will keep the log file. Log files older than the current date minus this day count will be removed. Disable this feature by setting a number <= 0 (therefore, we will keep everything). Default is -1.
- **fpsWatching (boolean, facultative)** : Our time keeper can watch fps and log if the fps of a thread is too low compared to what was expected. Setting it to "false" prevent it to watch the fps, and therefore to log if fps is under 20% of its expected frame rate. Default is "false".


# Input bindings


