# LibRetro Core Implemented in Java

Tested on Linux.  Include paths for Windows will need to be added to Makefile.

Java must be installed and these env variables must be set in order to compile *and* in order to run.

Set paths

    export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
    export LD_LIBRARY_PATH=$JAVA_HOME/lib/server

Compile C:

    make

Compile Java:

    javac LibRetro.java
    jar cf Test.jar LibRetro.class

Run:

    RetroArch-Linux-x86_64.AppImage -v -L java_libretro.so Test.jar
