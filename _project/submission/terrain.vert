#version 330 compatibility

// Uniforms

uniform float uNX;
uniform float uNZ;
uniform float uDepthSquares;
uniform float uTime;

uniform float uOffsetX;
uniform float uOffsetZ;

// Transformation matrices

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

// Vertex attributes in object (model) space from vertex buffer
in vec3       aVertex;         // Vertex position coordinates

// Lighting vectors in view space to be sent to geometry and fragment shader
out vec3      vPosition;
out vec3      vLightVector;    // Vector from vertex to light in view space
out vec3      vEyeVector;      // Vector from vertex to eye in view space

// Position for color calculations
out float     vHeight;

// Light and eye positions
const vec3    LIGHT_WC = vec3( 0., 100., -20. ); //vec3( 0., 15., 15. );
const vec3    EYE_EC   = vec3( 0.,  0., 0. );


float hash(vec2 p)
{
    // Generate pseudo-random value between 0 and 1
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

vec2 randomGradient(vec2 gridPoint)
{
    // Convert hash value to angle (in radians) along the unit circle
    float angle = hash(gridPoint) * 6.28318530718;

    // Return the x and y components of the resultant angle
    return vec2(cos(angle), sin(angle));
}

float perlin(vec2 point)
{
    // Integer part of the coordinates (the "cell" that contains this point)
    // Fractional part of the coordinates (how far into the cell the coordinates are)
    vec2 gridPoint = floor(point);  
    vec2 offset = point - gridPoint; 

    // Get gradients (2D vector) for each corner of the "cell"
    vec2 g00 = randomGradient(gridPoint + vec2(0.0, 0.0));
    vec2 g10 = randomGradient(gridPoint + vec2(1.0, 0.0));
    vec2 g01 = randomGradient(gridPoint + vec2(0.0, 1.0));
    vec2 g11 = randomGradient(gridPoint + vec2(1.0, 1.0));

    // Calculate dot product for coordinate and each corner (distance to each gradient)
    float d00 = dot(g00, offset - vec2(0.0, 0.0));
    float d10 = dot(g10, offset - vec2(1.0, 0.0));
    float d01 = dot(g01, offset - vec2(0.0, 1.0));
    float d11 = dot(g11, offset - vec2(1.0, 1.0));

    // Interpolate between the values (linear interpolation)
    float u = offset.x;  // X interpolation factor
    float v = offset.y;  // Y interpolation factor

    float nx0 = mix(d00, d10, u);
    float nx1 = mix(d01, d11, u);

    // Returns a value between -1 and +1
    return mix(nx0, nx1, v);  // Final interpolation between the two results
}

float perlinMultiOctave(vec2 point, int octaves, float persistence)
{
    float total = 0.0;
    float frequency = 1.0;  // Base frequency (larger values for more zoomed-in noise)
    float amplitude = 1.0;  // Base amplitude (larger values for more influence)

    for (int i = 0; i < octaves; i++) {
        total += perlin(point * frequency) * amplitude;  // Apply Perlin noise with frequency and amplitude
        frequency *= 2;  // Double the frequency for next octave (zoom in)
        amplitude *= persistence;  // Decrease the amplitude (less influence as octaves increase)
    }

    return total;
}

float getHeight(float x, float z, float scale, int octaves, float persistence)
{
    float height = perlinMultiOctave(vec2(x,z) * scale, octaves, persistence);
    //return max(30 * height, -9);
    return 30 * height;
}

void main() {
    //--------------------------------------------------------------------------
    // Get vertex coordinate data for calculations
    //--------------------------------------------------------------------------

    // Convert vertex to vec4 for compatibility with matrix
    vec4 vertexMC = vec4(aVertex, 1.f);

    // Get noise coords and determine y-height
    vertexMC.y = getHeight(vertexMC.x+uOffsetX, vertexMC.z+uOffsetZ, 0.01f, 6, 0.6f);

    //--------------------------------------------------------------------------
    // Set `out` variables (lighting vectors) to geometry/fragment shader
    //--------------------------------------------------------------------------

    // Model coordinate height for color calculations
    vHeight = vertexMC.y;

    // Transform model coordinates -> world coordinates -> eye coordinates
    vec4 vertexWC = uModelMatrix * vertexMC;
    vec4 vertexEC = uViewMatrix * vertexWC;

    // Set final vertex position
    gl_Position = uProjectionMatrix * vertexEC;

    // Position data to geometry shader
    vPosition = vec3(vertexEC);

    // Apply view matrix to light position and get vector from vertex to light
    vec4 lightEC = uViewMatrix * vec4(LIGHT_WC, 1.f);
    vLightVector = normalize( lightEC.xyz - vertexEC.xyz );

    // Calculate vector from vertex to eye
    vEyeVector = normalize( EYE_EC - vertexEC.xyz );
}
