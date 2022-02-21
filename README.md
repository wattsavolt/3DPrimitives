# 3DPrimitives 

# Simple 3D renderer for rendering primitives. Works on Windows only.

# The renderer is a forward renderer with one pass and support for any number of point lights
and one spot light. It uses Phong shading model.

##Project
# The renderer is in its own static library.

# I created a windows application for testing 
This allows you to move and rotate the camera.

# It adds 50 meshes to the scene and 10 point lights and a spot light.

# W, A, S, D move camera and mouse rotates. ESC exits app. L turns off spotlight and O turns it on.

# The spotlight moves around with the camera and faces in the camera's forward direction.

# I didn't add support for textures so there is just one material per mesh added as a constant buffer.