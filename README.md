# Storm
SPH reimplementation for fluids simulation


# Setup
From the root folder containing this README. Modify the Build\Script\Internal\UserSettings.bat to match the path of each dependencies in your workstation.
Note that : 
- you should have downloaded all dependencies beforehand and have done all setup that was specified (see next section).
- We rely on junctions. It means that all your dependencies folder and Storm project should be on compatible disks format (i.e. NTFS). But if junstion does not work, we advise to copy your dependencies under the Storm's root "Dependencies" folder after calling Setup (also change the way the batch file are executed or you will remove those dependencies each time).


# Dependencies list
- Boost 1.72 compiled for Visual Studio 2019 (link: https://www.boost.org/users/history/version_1_72_0.html ). Follow the instructions on their site.
- OIS v1.5 compiled for Visual Studio 2019 (link: https://github.com/wgois/OIS/tree/v1.5 ). Follow the instructions on their site. I used cmake_gui 3.15.0-rc1.
	• Generate the Visual studio file inside a folder named "bin" at OIS root folder (i.e if OIS is installed like this : C:/dep/OIS, then generate the vs project file into C:/dep/OIS/bin).
	• generate with default settings 
	• build OIS into Debug and Release configurations.


# Modules
- Storm : This is the executable that runs the SPH simulation.
- Storm-Config: This is the configuration module.
- Storm-Graphics: This is the module responsible to display and render the simulation. We use DirectX.
- Storm-Helper: This module contains all helpers that should be shared among all modules.
- Storm-Input: This module is for managing inputs, bindings and their callback.
- Storm-Logger: This module is for logging.
- Storm-Misc: This module is intended to gather module manager helper that shouldn't be made into helpers since they are Storm project implementation related but cannot be put into any other modules. Example are the RandomManager (that should be deterministic), the TimeManager (that should be compliant with Storm simulator), the AsyncManager (that is intended to be executed in the main Simulation loop like Unreal's Async method), ... .
- Storm-ModelBase: This module is the base for each modules. It ontains everything that could be used to bind the modules together without the need to reference each other.
- Storm-Physics: This module is responsible for initializing and managing Physics computations.
- Storm-Simulator: This module is where the Simulation classes would be. It is responsible to handle SPH.
- Storm-Windows: This module is responsible for managing the Windows and everything related to it.


# Configuration

## Command line


## Config file


# Input bindings


