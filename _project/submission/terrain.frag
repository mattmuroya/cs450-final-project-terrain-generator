#version 330 compatibility

// Interpolated lighting vectors from vertex shader
in vec3        gNormalVector;  // Normal vector
in vec3        gLightVector;   // Vector from fragment to light
in vec3        gEyeVector;     // Vector from fragment to eye

// Lighting parameters
uniform float  uKa;            // Ambient lighting coefficient
uniform float  uKd;            // Diffuse lighting coefficient
uniform float  uKs;            // Specular lighting coefficient
uniform float  uSh;

// Color parameters
in      float   gHeight;

uniform int    uMultiColor;
uniform vec4   uBaseColor;
uniform vec4   uColor0;
uniform vec4   uColor1;
uniform vec4   uColor2;
uniform vec4   uColor3;

uniform int uNormalMap;

void main() {
    //--------------------------------------------------------------------------
    // Normalize lighting vectors (interpolated from vertex shader values)
    //--------------------------------------------------------------------------

    vec3  normalVector = normalize(gNormalVector);
    vec3  lightVector  = normalize(gLightVector);
    vec3  eyeVector    = normalize(gEyeVector);

    //--------------------------------------------------------------------------
    // Calculate lighting components (Phong reflection model)
    //--------------------------------------------------------------------------

    // ----- Ambient -----

    // Ambient light is light that "bounces" around the scene, lighting objects
    // from all directions, making a surface visible even if not directly facing
    // a light source.
    float ambient = uKa * 1.0f;

    // ----- Diffuse -----
    
    // Diffuse lighting (Lambertian reflectance) uses the dot product between a
    // fragment's normalized light and normal vectors to vary light intensity
    // higher or lower as the surface faces toward or away from the light.

    // When a surface faces the light directly (i.e. the angle between the light
    // and normal vectors is 0), the dot product simplifies to cos(0) = 1.0,
    // resulting in 100% diffuse lighting intensity.
    
    // As the surface turns to become perpendicular to the light (i.e. the angle
    // between the light and normal vectors approaches pi/2), the dot product
    // approaches cos(pi/2) = 0.0, resulting in 0% diffuse lighting intensity.
    float diffuse = dot(normalVector, lightVector);

    // Past pi/2, there is no more change in lighting intensity since the
    // minimum intensity is 0%, so we clamp the minimum value to 0.0. Finally,
    // multiply by the diffuse lighting coefficient.
    diffuse = uKd * max(diffuse, 0.0);

    // ----- Specular -----

    // Specular lighting is a heuristic (non-physics based) computation that
    // approximates specular highlights on shiny/glossy surfaces.

    // As with diffuse lighting, we use the dot product between two normalized
    // vectors (in this case, the eye vector and the reflection of light off the
    // surface) to determine the lighting intensity.
    vec3  reflectionVector = normalize(reflect(-lightVector, normalVector));
    float specular         = dot(eyeVector, reflectionVector);

    // As with diffuse lighting, clamp the min value to 0.0.
    specular = max(specular, 0.0);

    // Raise to shininess factor and multiply by specular lighting coefficient.
    specular = uKs * pow(specular, uSh);

    //--------------------------------------------------------------------------
    // Set final fragment color
    //--------------------------------------------------------------------------

    // Multiply base color by combined lighting component intensities.
    if (uNormalMap == 1)
    {
        gl_FragColor = vec4(normalVector, 1.0);
    }
    else
    {
        vec4 color = uBaseColor;

        if (uMultiColor == 1)
        {
            if (gHeight < -8) color = uColor0;
            else if (gHeight < 2) color = uColor1;
            else if (gHeight < 9) color = uColor2;
            else color = uColor3;
        }
        gl_FragColor = color * (ambient + diffuse + specular);
    }
}
