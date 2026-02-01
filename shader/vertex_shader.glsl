#version 330 core


layout(location = 0) in float h; 


out vec3 vColor;


uniform mat4 mvp;
uniform float scaleXZ;
uniform float scaleY;
uniform int terrainWidth;
uniform int terrainHeight;

uniform int viewMode; // 0 = 2D, 1 = 3D
// na 3D
uniform float minLat;
uniform float maxLat;
uniform float minLon;
uniform float maxLon;
uniform float earthRadius;

const float PI = 3.14159265359;

void main() {
    int i = gl_VertexID / terrainWidth;
    int j = gl_VertexID % terrainWidth;

    vec3 pos;

    if (viewMode == 0) {
        // 2D
        float x = j * scaleXZ - (terrainWidth * scaleXZ * 0.5);
        float z = i * scaleXZ - (terrainHeight * scaleXZ * 0.5);
        float y = h * scaleY;
        pos = vec3(x, y, z);
    }
    else {
        // 3D KULA
        float lat = mix(minLat, maxLat, float(i) / float(terrainHeight - 1));
        float lon = mix(minLon, maxLon, float(j) / float(terrainWidth - 1));

        lat = radians(lat);
        lon = radians(lon);

        float radius = earthRadius + h * scaleY;

        pos.x = radius * cos(lat) * cos(lon);
        pos.y = radius * sin(lat);
        pos.z = radius * cos(lat) * sin(lon);
    }

    gl_Position = mvp * vec4(pos, 1.0);

    float hf = h;
    if (hf < 0.0) vColor = vec3(0,0,1);
    else if (hf < 500.0) vColor = vec3(0,hf/500.0,0);
    else if (hf < 1000.0) vColor = vec3(hf/500.0-1,1,0);
    else if (hf < 2000.0) vColor = vec3(1,2.0-hf/1000.0,0);
    else vColor = vec3(1,hf/2000.0-1,hf/2000.0-1);
}