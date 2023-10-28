typedef struct vec3fS {
    float x;
    float y;
    float z;
} vec3f;

typedef struct vec2fS {
    float x;
    float y;
} vec2f;

typedef struct vertexS {
    float* coords;
    long index;
} vertex;

long** triangulateFace(long* indicies, float** verticies, int numPoints, unsigned int* outTris);
