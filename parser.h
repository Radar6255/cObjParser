typedef struct ModelS{
    float* verts;
    float* normals;
    unsigned int* indicies;

    long numTris;
    long numNormals;
    long numVerts;
    int successful;
} Model;

Model loadModel(char* filename);
