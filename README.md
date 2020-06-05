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
- Because we use a lot of junctions and hardlinks to setup the project, we advise you to use an NTFS disk where everything are put together (or at least a disk format that supports it).



# Dependencies list
- **Boost 1.72** compiled for Visual Studio 2019 (link: https://www.boost.org/users/history/version_1_72_0.html ). Follow the instructions on their site.
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



# Modules
- **Storm**: This is the executable that runs the SPH simulation.
- **Storm-Config**: This is the configuration module.
- **Storm-Graphics**: This is the module responsible to display and render the simulation. We use DirectX.
- **Storm-Helper**: This module contains all helpers that should be shared among all modules.
- **Storm-Input**: This module is for managing inputs, bindings and their callback.
- **Storm-Loader**: This module's purpose is to load external object like meshes.
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
+ **$[StormResource]** will refer, in case StormRoot macro is valid, to where the Resource folder is.
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


### Scene Config

Scene configuration files contains all the data for running a simulation, therefore it is mandatory to specify one. If it was not set from the command line, Storm application will open an explorer windows to allow you to choose one.
Unlike the others config files, it can be named as you want. Here the wml tags you can set :

#### General
- **gravity (vector3, facultative)**: This is the gravity vector, in meter per second squared. Default value is { x=0.0, y=-9.81, z=0.0 }
- **particleRadius (float, falcultative)**: This is the particle radius in meter. It is also what is used for display but has also a physical value. Default value is 0.05.
- **kernelCoeff (float, falcultative)**: This is the kernel multiplicator coefficient (without unit). If this value is equal to 1.0, then the kernel length would be equal to the particle radius. Default value is 4.0.

#### Graphics
- **cameraPosition (vector3, facultative)**: This is the camera (viewpoint) initial position. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=-10.0 }.
- **cameraLookAt (vector3, facultative)**: This is the initial position of the target the camera look at. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=0.0 }.
- **zNear (float, facultative)**: This is the initial distance in meter from the camera which everything that is nearer than this distance won't be rendered. Default value is 0.01.
- **zFar (float, facultative)**: Same as zNear except that we skip displaying all objects farer than this distance value. Default value is 20.0.

#### RigidBodies
Inside this element should be put all rigidbodies. Each rigidbody should be specified under a tag named "RigidBody".

##### RigidBody
- **id (positive integer, mandatory)**: This is the unique id of the rigid body. It should be unique.
- **meshFile (string, mandatory, accept macro)**: This is mesh file path this rigid body is bound to.
- **isStatic (boolean, facultative)**: Specify this to tell the simulation that this object is fixed (won't move throughout the simulation). Default value is "true".
- **collisionType (string, facultative)**: Specify what is the collision shape should be handled. This is not case sensitive. Possible values are "None" (Default value), "Sphere", "Cube".
- **translation (vector3, facultative)**: The initial position in meters of the object. Default value is { x=0.0, y=0.0, z=0.0 }.
- **rotation (vector3, facultative)**: The initial rotation in degrees of the object (this is euler angle : roll, pich, yaw). Default value is { x=0.0, y=0.0, z=0.0 }.
- **scale (vector3, facultative)**: The initial scale of the object. Default value is { x=1.0, y=1.0, z=1.0 }.
- **staticFrictionCoeff (float, facultative)**: The static friction coefficient of the object, it should be larger than 0.0. PhysX needs it but physically speaking I don't know what to set. This is the minimum force norm threshold that makes our object move. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **dynamicFrictionCoeff (float, facultative)**: The dynamic friction coefficient of the object, it should be positive. PhysX needs it but physically speaking I don't know what to set. This is the the velocity reduction when a rigid body moves with a contact with another. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **restitutionCoeff (float, facultative)**: The restitution friction coefficient of the object (the bounciness of the object), it should be positive but close or above 1.0 may cause instabilities. PhysX needs it but physically speaking I don't know what to set. Closer it is to 0.0, less the object will bounce and more it will lose energy when being in contact with another rb. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.



# Input bindings


