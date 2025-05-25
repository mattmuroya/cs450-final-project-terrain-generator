#version 330 core

// Input/output primitive parameters
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Variables from vertex shader (array of discrete vertex values)
in vec3   vPosition[];
in vec3   vLightVector[];
in vec3   vEyeVector[];
in float  vHeight[];

// Lighting variables to fragment shader
out vec3  gNormalVector;
out vec3  gLightVector;
out vec3  gEyeVector;

out float  gHeight;

void main()
{
    //--------------------------------------------------------------------------
    // Recalculate surface normal
    //--------------------------------------------------------------------------

    // The surface normal of a triangle is the cross product of any two of its
    // edge vectors. The geometry shader allows us to access data (passed from
    // the vertex shader) for each vertex of a polygon so we can calculate a
    // single surface normal for the entire triangle for a flat shading effect.

    vec3 p0 = vPosition[0];
    vec3 p1 = vPosition[1];
    vec3 p2 = vPosition[2];

    // Define two edge vectors
    vec3 U = p1 - p0;
    vec3 V = p2 - p0;

    // Set normalized cross product as surface normal (applies to all vertices)
    gNormalVector = normalize(cross(U, V));

    //--------------------------------------------------------------------------
    // Emit vertices
    //--------------------------------------------------------------------------

    float avgHeight = (vHeight[0] + vHeight[1] + vHeight[2]) / 3.f;

    // Set independent attributes for each vertex separately
    for (int i = 0; i < 3; i++) {
        gLightVector = vLightVector[i];
        gEyeVector   = vEyeVector[i];
        gHeight      = avgHeight;
        gl_Position  = gl_in[i].gl_Position;
        EmitVertex();
    }

    // Technically, the vertex shader doesn't output individual triangles; here,
    // we output a triangle_strip, limited to three vertices as defined in the
    // initial `layout` statement.
    EndPrimitive();
}
