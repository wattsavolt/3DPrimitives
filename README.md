# 3DPrimitives 

Simple 3D renderer for rendering primitives. Works on Windows only.

The renderer is a forward renderer with one pass and support for any number of point lights
and one spot light. It uses the Phong shading model.

## Project Structure

The project consists of a sample application and a renderer. 

The renderer is in its own static library.

The sample application allows you to move and rotate the camera.

It adds 50 meshes to the scene and 10 point lights and a spot light.

The spotlight moves around with the camera and faces in the camera's forward direction.

The code can be compiled with CMAKE.

## Controls

1. W, A, S, D buttons move the camera.

2. The mouse rotates the camera. 

3. ESC exits the application. 

4. L turns off spotlight and O turns it on.
