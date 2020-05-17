# Storm
SPH reimplementation for fluids simulation


# Setup
From the root folder containing this README. Modify the Build\Script\Internal\UserSettings.bat to match the path of each dependencies in your workstation.
Note that you should have downloaded all dependencies beforehand.


# Dependencies list
- Boost 1.72 compiled for Visual Studio 2019 (link: https://www.boost.org/users/history/version_1_72_0.html )


# Modules
- Storm : This is the executable that runs the SPH simulation.
- Storm-Config: This is the configuration module.
- Storm-Graphics: This is the module responsible to display and render the simulation. We use DirectX.
- Storm-Helper: This module contains all helpers that should be shared among all modules.
- Storm-Logger: This module is for logging.
- Storm-ModelBase: This module is the base for each modules. It ontains everything that could be used to bind the modules together without the need to reference each other.
- Storm-Physics: This module is responsible for initializing and managing Physics computations.


# Configuration

## Command line


## Config file


# Input bindings


