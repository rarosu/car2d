A simple top-down car physics simulation made for an assignment for the course FY1403 taken during the first semester 2011 at Blekinge Institute of Technology. The car can be controlled with WASD or the arrow keys and R and F can be used to gear up and down. Scroll wheel can be used to zoom in and out. See config.yaml for more controls.

All the physics can be seen in the method update_physics() in code/car2d_main/car.cpp. Attributes for the car can be seen and changed in assets/cars/test_car.yaml. The car is a point-mass without suspension in regards to forces applied to it and the traction model is simplified, taking into account that the tires have a maximum amount of traction before losing grip. The model could be improved by:

* integrating wheel velocity and calculating a slip ratio to use for forward traction.
* using combined linear functions to estimate the cornering forces and the forward traction force.
* possibly using the Pacejka model for traction.
* having different friction constants for different surfaces.

The application in its current state is finished, though I might return to improve it at some point. N.B. GLI seems to have issues with 32-bit builds, make sure the program is compiled in 64-bit to be able to run it.

Libraries used:
    SDL2: https://www.libsdl.org/index.php
    GL3W: https://github.com/skaslev/gl3w
    GLM: https://github.com/g-truc/glm
    GLI: https://github.com/g-truc/gli
    yaml-cpp: https://github.com/jbeder/yaml-cpp
    yaml-cpp no-boost branch by blair1618: https://github.com/jbeder/yaml-cpp/pull/309
    freetype-gl: https://github.com/rougier/freetype-gl
    FreeType: http://www.freetype.org
    
Art credits:
    stktex_generic_earth_a.dds 
    stk_generic_grassb.dds
        by samuncle released under CC-BY-SA 3.0. Modified to .dds format.
    SourceCodePro-Regular.ttf by Adobe Systems Incorporated released under SIL Open Font License (see font directory).