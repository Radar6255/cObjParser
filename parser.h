typedef struct ModelS{
    float* verts;
    unsigned int* indicies;

    long numTris;
    long numVerts;
    int successful;
} Model;

Model loadModel(char* filename);
