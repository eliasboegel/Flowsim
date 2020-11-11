# Flowsim
A small threaded 2D potential flow visual simulator.

This project is written in C++ using SDL2.

The color scale is not linear, i.e. use the results of the pressure value color scale with a grain of salt.

There are several compile-time settings, for different visualisation modes:
- Full screen pressure color scale output
- Stream line visualization
- Particle flow visualization

There is also a setting to control the number of CPU threads used to run the simulation and composition of the final flow field from all specified elementary flows. Ideally this should be set to the number of cores of your CPU.
