# RayTracer

## Description

### Instalation

1. Clone repository

    ```sh
        git clone https://github.com/DavidSolz/RayTracer.git
        cd RayTracer
    ```

2. Install required dependencies

3. Compile

4. Run application

### Requirements

    - GLFW 3.3.9 >=
    - GLEW 2.1.0 >=
    - OpenGL 1.2 >=
    - OpenCL 2.0 >=
    - CMake  3.29.4 >=
    - MinGW 11.0.0 >= ( only on Windows )
    - Operating system from Windows, MacOS or Linux family.

### Compilation

    On systems like MacOS and Linux just run internal script named `Build.sh`. It will find required depencencies and compile application into build catalogue.
    On systems like Windows there is need to use manual compilation due to lack of compability. To compile application provide specified command into terminal:

    ```sh
        cmake -G "MinGW Makefiles" -B build
        cmake --build build
    ```

### Run

    To run application you need to be in root catalogue and type into terminal :

    ```sh
        ./build/src/RayTracer_run.exe ( on Windows )
        or
        ./build/src/RayTracer_run ( on MacOS and Linux )
    ```

### Run parameters

    There is a dozen of possible parameters that modify application runtime logic.

    - H - shows a list of all possible parameters,
    - B - enables BVH acceleration,
    - V - enables vertical synchronization,
    - w  <width> - determines width of generated image,
    - h <height> - determines height of generated image,
    - L <filepath> - loads scene from specified description,
    - T <number of threads>- runs application on some number of threads
    - S - enables memory sharing between OpenCL and OpenGL ( works only with default GPU ),
    - O - enables automatic camera movement,
    - F <number of frames> - allows to specify number of frames that will be generated without visualization,

### Structure of scene description

    If there is need to specify custom scene, just describe it with a file that have .scn extension. Files of that type consatins some internal structure similar to JSON. It looks as below :

    ```json

    mtllib skybox2.mtl // <-- skybox material
    mtllib mat.mtl // <-- other materials used by objects

    !Best experience with resoultion 1000 x 1000 // <-- comment

    scene // <-- start of scene description
    {

        disk // <-- object type
        {
            position 500 4000 500 // <-- object attributes
            normal 0 -1 0 //
            radius 1000 //
            material Light // <-- material used by object
        }

        plane
        {
            position 500 100 500
            normal 0 1 0
            scale 5000 5000 5000
        }

    mesh mesh.obj // <-- mesh that will be placed on (0,0,0) coordinates

    } // <-- end of scene description

    ```

    There is list of default primitves and attributes that might be specified:

    - disk <position> <normal> <radius> <scale> <material>
    - sphere <positino> <radius> <scale> <material>
    - plane <position> <normal> <scale> <material>
    - cube <position> <scale> <material>
    - mesh <filename>

    All materials are comaptible with material template libray
