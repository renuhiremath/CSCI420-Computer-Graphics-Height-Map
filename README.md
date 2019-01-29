# Computer-Graphics-Height-Map

Problem Statement : http://www-bcf.usc.edu/~jbarbic/cs420-s18/runuscedu/cs420-s18/assignments/assign1/assign1.html

Visualize a Height Map using OpenGL

- Use the OpenGL core profile, version 3.2 or higher, and shader-based OpenGL. The following are not allowed: compatibility profile, commands that were deprecated and/or removed in OpenGL version 3.2 or earlier, and the fixed-function pipeline. Exact specification is available here. If in doubt, please ask the instructor/TA. Submissions that do not follow these guidelines will receive zero points.
- Handle at least a 256x256 image for your height field at interactive frame rates (window size of 1280x720). Height field manipulations should run smoothly.
- Be able to render the height field as points, lines ("wireframe"), or solid triangles (with keys for the user to switch between the three). Usage of glPolygonMode (or similar) to achieve point or line rendering is not permitted. The points, lines and solid triangles must be modeled using GL_POINTS, GL_LINES, GL_TRIANGLES, or their "LOOP" or "STRIP" variants.
- Render as a perspective view, utilizing GL's depth buffer for hidden surface removal.
- Use input from the mouse to rotate the height field using OpenGLMatrix::Rotate.
- Use input from the mouse to move the height field using OpenGLMatrix::Translate.
- Use input from the mouse to change the dimensions of the height field using OpenGLMatrix::Scale.

Functionalities implemented:
- 1 - Points (White dots)
- 2 - Lines (White Lines whose colors depends on the height)
- 3 - Triangles (White Triangles whose colors depends on the height)

(Extra Credit)
- 4 - Triangle Strip (Red Triangles whose colors depends on the height)
- 5 - Wireframe (Red Triangles and White Lines whose colors depends on the height)

Keyboard Funtionality:
- t - Transformation
- r - Rotation
- s - Scale
