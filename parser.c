#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "list.h"
#include "parser.h"
#include "triangulate.h"

char beginsWith(char* full, char* beginning){
    for (int i = 0; beginning[i]; i++){
        if(beginning[i] != full[i]){
            return 0;
        }
        if (!full[i]){
            return 0;
        }
    }

    return 1;
}

// This parses the lines that start with v to get the x,y,z coordinates
float* getVertices(char* line){
    float* out = malloc(sizeof(float) * 3);
    int pos = -1;
    while (line[++pos] != ' ') {}
    while (line[++pos] == ' ') {}

    // Should be the at the position of the first number
    char* nextPos;
    nextPos = &line[pos];
    for (int i = 0; i < 3; i++){
        out[i] = strtof(nextPos, &nextPos);
    }

    return out;
}

typedef struct parsedFace {
    long* vertexIndicies;
    long* normalIndicies;
    long* textureIndicies;

    short numVerticies;
    short numNormals;
    short numTextures;
} ParsedFace;

// This parses the lines that start with f to get just the first value which is the vertex index
ParsedFace parseFace(char* line, int* outPos){
    ParsedFace out;
    out.numNormals = 0;
    out.numVerticies = 0;
    out.numTextures = 0;

    out.vertexIndicies = malloc(sizeof(long) * 3);
    out.normalIndicies = NULL;
    out.textureIndicies = NULL;
    *outPos = 0;
    int maxSize = 3;
    int pos = -1;
    while (line[++pos] != ' ') {}
    while (line[++pos] == ' ') {}

     // Should be the at the position of the first number
    char** nextPos = malloc(sizeof(char**));
    *nextPos = &line[pos];
    while(**nextPos != '\0'){
        if (*outPos >= maxSize){
            maxSize++;
            out.vertexIndicies = realloc(out.vertexIndicies, sizeof(long) * maxSize);
            if (out.normalIndicies) {
                out.normalIndicies = realloc(out.normalIndicies, sizeof(long) * maxSize);
            }
            if (out.textureIndicies) {
                out.textureIndicies = realloc(out.textureIndicies, sizeof(long) * maxSize);
            }
        }

        out.vertexIndicies[*outPos] = strtol(*nextPos, nextPos, 10) - 1;

        if (**nextPos == '/') {
            long** indexPosition = &out.textureIndicies;
            if (*(*nextPos + 1) == '/') {
                indexPosition = &out.normalIndicies;
                (*nextPos)++;
            }

            if (!*indexPosition) {
                *indexPosition = malloc(sizeof(long) * 3);
            }

            /* printf("Now at: %s\n", *nextPos); */
            /* printf("Starting at: %s\n", *nextPos + 1); */
            (*indexPosition)[*outPos] = strtol(*nextPos + 1, nextPos, 10) - 1;
            /* printf("Now at: %s\n", *nextPos); */
        }

        if (**nextPos == '/') {
            if (!out.normalIndicies) {
                out.normalIndicies = malloc(sizeof(long) * 3);
            }
            out.normalIndicies[*outPos] = strtol(*nextPos + 1, nextPos, 10) - 1;
        }
        /* printf("Now at: %s\n", *nextPos); */
        while (**nextPos != ' ' && **nextPos != '\0') {*nextPos = *nextPos + 1;}
        *outPos = *outPos + 1;
    }
    free(nextPos);

    return out;
}

Model loadModel(char* filename) {
    Model outModel;
    outModel.successful = true;
    outModel.numTris = 0;
    FILE *file = fopen(filename, "r");
    int maxSize = 40;
    unsigned long currSize = 0;
    unsigned int* out = malloc(sizeof(unsigned int) * maxSize);

    if(!file){
        printf("Failed to open file %s\n", filename);
        outModel.successful = false;
        return outModel;
    }

    // Defining a string that can temporarily hold a line that comes out of the file
    size_t len = 10;
    char** line = malloc(sizeof(char**));
    *line = malloc(sizeof(char) * len);

    // TODO Adjust the max size of this to be the average size of the models
    void* vertexList = createList(10);
    void* normalList = createList(10);

    long lineNum = 0;
    while(getline(line, &len, file) != -1){
        lineNum++;
        // Broken before this executes somehow
        if (beginsWith(*line, "v \0")){
            // Got a vertex, need to parse the 3 floats
            float* verts = getVertices(*line);

            // Storing the verticies in order
            listAppend(vertexList, verts);
        } else if (beginsWith(*line, "vn \0")) {
            // Got a normal need to parse the 3 floats
            float* normals = getVertices(*line);

            // Storing the normals in order
            listAppend(normalList, normals);
        } else if(beginsWith(*line, "f \0")){
            /* printf("Got new face\n"); */
            // Got a face
            // TODO Need to make the faces using the points that are stored in the vertexList
            int numVerticies;
            /* printf("Processing line %s\n", *line); */
            ParsedFace parsedFace = parseFace(*line, &numVerticies);
            /* printf("f %ld, %ld, %ld, %ld\n", parsedFace.vertexIndicies[0], parsedFace.vertexIndicies[1], parsedFace.vertexIndicies[2], parsedFace.vertexIndicies[3]); */

            /* printf("Number of verticies in face: %d\n", numVerticies); */
            unsigned int numTris;

            long** triangles;
            if (numVerticies == 3) {
                outModel.numTris += 1;
                if (maxSize <= currSize + 3){
                    maxSize = maxSize * 2;
                    out = realloc(out, sizeof(float) * maxSize);
                }

                out[currSize] = parsedFace.vertexIndicies[0];
                out[currSize + 1] = parsedFace.vertexIndicies[1];
                out[currSize + 2] = parsedFace.vertexIndicies[2];

                currSize = currSize + 3;
            } else {
                printf("Num verticies in face: %d\n", numVerticies);
                triangles = triangulateFace(parsedFace.vertexIndicies, (float**) getListElements(vertexList), numVerticies, &numTris);

                printf("Num tris: %d\n", numTris);
                outModel.numTris += numTris;

                for (int i = 0; i < numTris; i++){
                    //printf("I: %d\n", i);
                    if (maxSize <= currSize + 3){
                        maxSize = maxSize * 2;
                        out = realloc(out, sizeof(float) * maxSize);
                    }

                    // The triangulate face returns the relative verticies to what it was handed whcih is the faceIndicies
                    out[currSize] = parsedFace.vertexIndicies[triangles[i][0]];
                    out[currSize + 1] = parsedFace.vertexIndicies[triangles[i][1]];
                    out[currSize + 2] = parsedFace.vertexIndicies[triangles[i][2]];

                    /* printf("OUT: indicies: %d, %d, %d.\n", out[currSize], out[currSize + 1], out[currSize + 2]); */
                    currSize = currSize + 3;
                }
                free(parsedFace.vertexIndicies);
                if (parsedFace.normalIndicies) {
                    free(parsedFace.normalIndicies);
                }
                if (parsedFace.textureIndicies) {
                    free(parsedFace.textureIndicies);
                }
            }
        }
    }
    /* fclose(file); */

    unsigned int numVerts = getListSize(vertexList);
    float* outVerts = malloc(sizeof(float) * numVerts * 3);
    for (unsigned int i = 0; i < numVerts; i++){
        float* point = getListElement(vertexList, i);
        outVerts[i*3] = point[0];
        outVerts[i*3 + 1] = point[1];
        outVerts[i*3 + 2] = point[2];
    }

    unsigned int numNormals = getListSize(normalList);
    float* outNormals = malloc(sizeof(float) * numNormals * 3);
    for (unsigned int i = 0; i < numNormals; i++){
        float* point = getListElement(normalList, i);
        outNormals[i*3] = point[0];
        outNormals[i*3 + 1] = point[1];
        outNormals[i*3 + 2] = point[2];
    }

    free(*line);
    free(line);
    freeElements(vertexList);
    freeElements(normalList);

    out = realloc(out, sizeof(float) * currSize);
    outModel.indicies = out;
    outModel.verts = outVerts;
    outModel.numVerts = numVerts;

    outModel.normals = outNormals;
    outModel.numNormals = numNormals;

    /* printf("Finished parsing\n"); */
    /* printf("Num Tris: %ld\n", outModel.numTris); */

    return outModel;
}


/* int main(int argc, char** args){ */
/*     Model out = loadModel("./car.obj"); */
/*     // Model out = loadModel("./cube.obj"); */
/*     printf("Model created successfully: %s\n", out.successful ? "TRUE" : "FALSE"); */
/*     printf("Created %li float array\n", out.numTris); */
/* } */
