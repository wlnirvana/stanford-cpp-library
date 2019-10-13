# Kido customization

1. Skipped `FormPreviewFrame` while searching the main method so that the
   program can be launched properly in IntelliJ IDEA. Files changed:
    - `JavaTaskForce/src/acm/util/JTFTools.java`
    - `JavaBackEnd/eclipseproject/src/stanford/karel/Karel.java`
    - `JavaBackEnd/eclipseproject/src/stanford/karel/AbstractKarel.java`
2. Add `getFrameWidth` and `getFrameHeight` methods to `GRect.java` so that
   `G3DRect.java` compiles. Modifications are made according to the decompiled
   code. Files changed:
    - `JavaTaskForce/src/acm/graphics/GRect.java`
3. Removed the `final` keyword from the signatures of `paint` and `scale`
   methods of `GObject` so that the subclass `GRect` can override them. Files
   changed:
    - `JavaBackEnd/eclipseproject/src/acm/graphics/GObject.java`
4. Fixed `AbstractKarel` loading main class bug.

# How to compile updated JAR for kido

1. Compile updated JTF by `make` in `JavaTaskForce`
2. Update the `acm.jar` lib for the eclipse project
```
cd JavaTaskForce/classes; jar -uf ../../JavaBackEnd/eclipseproject/lib/acm.jar `find acm -name '*.class'`
```
3. Recompile the JavaBackEnd eclipse project in IntelliJ IDEA. Remember to set
   the wanted compiler level.
4. Make the updated JAR using the `obf/makejar.sh`
5. Copy the JAR to where needed


# stanford-cpp-library
The Stanford C++ libraries, created by Eric Roberts et al.
Currently maintained by Stanford CS Lecturer Marty Stepp.

Many thanks to Prof. Jeff Lutgen (jlutgen) of Whittier College,
who has submitted several useful pull requests and new features to the libraries.
