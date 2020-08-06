# Storm
SPH reimplementation for fluids simulation


# Setup
From the root folder containing this README. Modify the Build\Script\Internal\UserSettings.bat to match the path of each dependencies in your workstation.
Note that : 
- you should have downloaded all dependencies beforehand and have done all setup that was specified (see next section).
- We rely on junctions. It means that all your dependencies folder and Storm project should be on compatible disks format (i.e. NTFS). But if junctions don't work, we advise to copy your dependencies under the Storm's root "Dependencies" folder after calling Setup (also change the way the batch file are executed or you will remove those dependencies each time).



# Prerequisite
- **Visual Studio Community 2019 v16.6.3 with C++20** (in fact the latest draft that was a preview of C++20). Maybe it works for a later Visual Studio but I have never tested it.
- **Python 2.7.6 or later** (needed to build PhysX, see "Dependencies list" section). To know what is the current version of your installed python (or if there is any), type python in a cmd console.
- **Visual Studio 2017 toolsets (v141) and 2019 toolsets (v142)**
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
-**Catch2 v2.12.2**. Not mandatory, but it is used for unit testing. (link: https://github.com/catchorg/catch2/releases ). If you choose to not pull it, then unload All Automation project from your Storm.sln... If you want to run tests, do not forget to setup your Visual Studio for this. See section "Test Setup" for further details.


#Test Setup
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

#### Log (facultative)
- **logFolderPath (string, facultative, accept macro)** : The folder where to gather the log files. The default is the temporary path (StormTmp macro). If it is empty, default is considered.
- **logFileName (string, facultative, accept macro)** : The log file name of the current run. The default is empty. If it is empty, we won't log into a file (but the log will still be outputed to the console).
- **logLevel (string, facultative)** : The threshold level under which we ignore the log. Accepted values are in that inmportance order : Debug, DebugError, Comment, Warning, Error, Fatal.
Note that the maximum value you can set is Fatal, it means that no matter what level you set, we would still log "Fatal" and "Always" logs. The default is Debug.
- **override (boolean, facultative)** : If the log file specified should have its content overriden each time. If "false", the content will be appended instead. Default is "true".
- **removeOlderThanDays (integer, facultative)** : Specify the number of day Storm will keep the log file. Log files older than the current date minus this day count will be removed. Disable this feature by setting a number <= 0 (therefore, we will keep everything). Default is -1.
- **fpsWatching (boolean, facultative)** : Our time keeper can watch fps and log if the fps of a thread is too low compared to what was expected. Setting it to "false" prevent it to watch the fps, and therefore to log if fps is under 20% of its expected frame rate. Default is "false".
- **logGraphicDeviceMessage (boolean, facultative)** : If we should print the graphic device log message. Warning : there is a lot ! Default is "false" but enable it only if it is necessary.


#### Graphics (facultative)
- **screenWidth (unsigned integer, facultative)** : Set the expected windows width at startup. Note that it could be something else near this value... Default is 1200.
- **screenHeight (unsigned integer, facultative)** : Set the expected windows height at startup. Note that it could be something else near this value... Default is 800.
- **fontSize (unsigned integer, facultative)** : the font size of any written information displayed in the HUD... Default is 16.


### Scene Config

Scene configuration files contains all the data for running a simulation, therefore it is mandatory to specify one. If it was not set from the command line, Storm application will open an explorer windows to allow you to choose one.
Unlike the others config files, it can be named as you want. Here the wml tags you can set :


#### General
- **startPaused (boolean, facultative)**: If true, the simulation will be paused when the application start. Default is false.
- **gravity (vector3, facultative)**: This is the gravity vector, in meter per second squared. Default value is { x=0.0, y=-9.81, z=0.0 }
- **particleRadius (float, falcultative)**: This is the particle radius in meter. It is also what is used for display but has also a physical value. Default value is 0.05.
- **kernelCoeff (float, falcultative)**: This is the kernel multiplicator coefficient (without unit). If this value is equal to 1.0, then the kernel length would be equal to the particle radius. Default value is 4.0.
- **CFLCoeff (positive float, falcultative)**: This is the coefficient lambda for the CFL (Courant-Friedrich-Levy conditions). It is used only if CFL is enabled. Default value is 0.4.
- **MaxCFLTime (positive float, falcultative)**: This is the max time the CFL (Courant-Friedrich-Levy conditions) could take in seconds. It is used only if CFL is enabled. Default value is 0.5 seconds (500ms).
- **kernel (string, falcultative)**: This is the kernel we would use. It isn't case sensitive and the accepted values are for now "CubicSpline" (default).
- **physicsTime (float, falcultative)**: This is the iteration loop physics time in seconds. If this value is less or equal to 0, then we would adapt it automatically using CFL condition. Default value is -1.
- **fps (float, falcultative)**: This is the expected frame rate (it has nothing to do with the physics time, this is the refresh rate of many loop inside the engine to not consume too much cpu). If this value is less or equal to 0, then we would set it to the default value which is 60 FPS.
- **simulation (string, mandatory)**: This is the simulation mode we will run. It isn't case sensitive. Accepted values (for now) are "PCISPH" and "WCSPH".
- **maxPredictIteration (positive integer, facultative)**: This is the max iteration we're allowed to make inside one simulation loop when doing prediction iteration. It is to avoid infinite loop. It should be an integer strictly greater than 0. Default value is 150.
- **maxDensityError (positive float, facultative)**: This is the max density error under which we would continue the prediction iteration (or until maxPredictIteration hit). It should be less or equal than 0. Default value is 0.01.


#### Graphics
- **cameraPosition (vector3, facultative)**: This is the camera (viewpoint) initial position. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=-10.0 }.
- **cameraLookAt (vector3, facultative)**: This is the initial position of the target the camera look at. Each coordinate are meters. Default value is { x=0.0, y=0.0, z=0.0 }.
- **zNear (float, facultative)**: This is the initial distance in meter from the camera which everything that is nearer than this distance won't be rendered. Default value is 0.01.
- **zFar (float, facultative)**: Same as zNear except that we skip displaying all objects farer than this distance value. Default value is 20.0.
- **minColorValue (float, facultative)**: Set the minimum value for the watched value to display the coldest color. If the watched value is the velocity, this is to be expressed as a squared norm. Default value is 0.01
- **maxColorValue (float, facultative)**: Set the maximum value for the watched value to display the hotest color. If the watched value is the velocity, this is to be expressed as a squared norm. Default value is 100
- **grid (vector3, facultative)**: Set the grid dimension. X coord will be the grid width, Z its depth and Y will be the height where the grid will be drawn. Note that X and Z will be ceiled. Default value is { x=10.0, y=0.0, z=10.0 }
- **particleDisplay (boolean, facultative)**: Specify if Solids should be displayed as particle on start. If not, they will be displayed as meshes. "false" by default.


#### Fluid
This element is all setting appartaining to a fluid. Here the tag you can set inside :
- **id (positive integer, mandatory)**: This is the unique id of the fluid. It should be unique (Note that it should not be equal to any rigid body id too).
- **fluidBlock (tag, at least one)**: This is the fluid generator settings. There should be at least one.
	+ **firstPoint (vector3, facultative)**: This is one of the corner of the box where fluid particle should be generated. It cannot have the same value than secondPoint, default value is { x=0.0, y=0.0, z=0.0 }.
	+ **secondPoint (vector3, facultative)**: This is the opposite corner from firstPoint where fluid particle should be generated. It cannot have the same value than firstPoint, default value is { x=0.0, y=0.0, z=0.0 }.
- **density (positive float, falcultative)**: This is the rest density of the fluid in kg.m^-3. Default is 1.2754 kg.m^-3 which is the density of Dry air at 0 °C and normal ATM pressure.
- **pressureK1 (positive zero-able float, falcultative)**: This is the pressure stiffness constant coefficient used when initializing the pressure using State equation. In formulas, it is often found as k1. Default is 50.
- **pressureK2 (positive zero-able float, falcultative)**: This is the pressure exponent constant coefficient used when initializing the pressure using State equation. In formulas, it is often found as k2. Default is 7.
- **viscosity (positive float, falcultative)**: This is the dynamic viscosity of the fluid in N.s/m² (or Pa.s). Default is 0.00001715 N.s/m² which is the dynamic viscosity of Dry air at 0 °C and normal ATM pressure.
- **soundSpeed (positive float, falcultative)**: This is the speed of sound inside the given fluid in m/s. Default is 331.4 m/s which is the speed of sound of Dry air at 0 °C) and normal ATM pressure (340 is for 15 °C).


#### RigidBodies
Inside this element should be put all rigidbodies. Each rigidbody should be specified under a tag named "RigidBody".

##### RigidBody
- **id (positive integer, mandatory)**: This is the unique id of the rigid body.  It should be unique (Note that it should not be equal to any fluid id too).
- **meshFile (string, mandatory, accept macro)**: This is mesh file path this rigid body is bound to.
- **isStatic (boolean, facultative)**: Specify this to tell the simulation that this object is fixed (won't move throughout the simulation). Default value is "true".
- **wall (boolean, facultative)**: Specify that this object is the wall (considered to be a wall). By defainition, a wall is static so if the value is true, the object will be considered static no matter the value set to isStatic. Default value is "false".
- **collisionType (string, facultative)**: Specify what is the collision shape should be handled. This is not case sensitive. Possible values are "None" (Default value), "Sphere", "Cube".
- **translation (vector3, facultative)**: The initial position in meters of the object. Default value is { x=0.0, y=0.0, z=0.0 }.
- **rotation (vector3, facultative)**: The initial rotation in degrees of the object (this is euler angle : roll, pich, yaw). Default value is { x=0.0, y=0.0, z=0.0 }.
- **scale (vector3, facultative)**: The initial scale of the object. Default value is { x=1.0, y=1.0, z=1.0 }.
- **staticFrictionCoeff (float, facultative)**: The static friction coefficient of the object, it should be larger than 0.0. PhysX needs it but physically speaking I don't know what to set. This is the minimum force norm threshold that makes our object move. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **dynamicFrictionCoeff (float, facultative)**: The dynamic friction coefficient of the object, it should be positive. PhysX needs it but physically speaking I don't know what to set. This is the the velocity reduction when a rigid body moves with a contact with another. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **restitutionCoeff (float, facultative)**: The restitution friction coefficient of the object (the bounciness of the object), it should be positive but close or above 1.0 may cause instabilities. PhysX needs it but physically speaking I don't know what to set. Closer it is to 0.0, less the object will bounce and more it will lose energy when being in contact with another rb. See http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html.
- **mass (positive float, mandatory)**: The mass of the rigid body in kg. This has to be strictly positive (> 0.0). 
- **viscosity (positive float, facultative)**: The viscosity of the rigid body in Pa.s (???). This has to be strictly positive (> 0.0). 



# Input bindings
- **Escape key (ESC)**: Leave the application.
- **Space bar**: Pause application if it is running. Unpause if it is paused.
- **+ (Numpad)**: Move near clipping plane forward.
- **- (Numpad)**: Move near clipping plane backward.
- **/ (Numpad)**: Move far clipping plane backward.
- **Star (Numpad)**: Move far clipping plane forward.
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
- **Mouse wheel up**: Increase camera and clipping plane motion speed.
- **Mouse wheel down**: Decrease camera and clipping plane motion speed.
- **B**: Set solid state with back face culling.
- **N**: Set solid state without back face culling.
- **V**: Set wireframe.
- **C**: Set rendering to display all particles.
- **X**: Set particle state without rendering walls.
- **F1**: Debug command to print to a human readable text giving all position, velocity and force values of all fluid particles. The data is printed inside the output (temp) directory inside "Debug" folder.

