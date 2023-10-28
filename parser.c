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

// This parses the lines that start with f to get just the first value which is the vertex index
long* getFaceIndicies(char* line, int* outPos){
    long* out = malloc(sizeof(long) * 3);
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
            out = realloc(out, sizeof(long) * maxSize);
        }

        out[*outPos] = strtol(*nextPos, nextPos, 10) - 1;
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

    long lineNum = 0;
    while(getline(line, &len, file) != -1){
        lineNum++;
        // Broken before this executes somehow
        if (beginsWith(*line, "v \0")){
            // Got a vertex, need to parse the 3 floats
            float* verts = getVertices(*line);

            // Storing the verticies in order
            listAppend(vertexList, verts);
        }else if(beginsWith(*line, "f \0")){
            /* printf("Got new face\n"); */
            // Got a face
            // TODO Need to make the faces using the points that are stored in the vertexList
            int numVerticies;
            /* printf("Processing line %s\n", *line); */
            long* faceIndicies = getFaceIndicies(*line, &numVerticies);
            /* printf("f %ld, %ld, %ld, %ld\n", faceIndicies[0], faceIndicies[1], faceIndicies[2], faceIndicies[3]); */

            /* printf("Number of verticies in face: %d\n", numVerticies); */
            unsigned int numTris;

            long** triangles;
            if (numVerticies == 3) {
                outModel.numTris += 1;
                if (maxSize <= currSize + 3){
                    maxSize = maxSize * 2;
                    out = realloc(out, sizeof(float) * maxSize);
                }

                out[currSize] = faceIndicies[0];
                out[currSize + 1] = faceIndicies[1];
                out[currSize + 2] = faceIndicies[2];

                currSize = currSize + 3;
            } else {
                printf("Num verticies in face: %d\n", numVerticies);
                triangles = triangulateFace(faceIndicies, (float**) getListElements(vertexList), numVerticies, &numTris);

                outModel.numTris += numTris;

                for (int i = 0; i < numTris; i++){
                    //printf("I: %d\n", i);
                    if (maxSize <= currSize + 3){
                        maxSize = maxSize * 2;
                        out = realloc(out, sizeof(float) * maxSize);
                    }

                    // The triangulate face returns the relative verticies to what it was handed whcih is the faceIndicies
                    out[currSize] = faceIndicies[triangles[i][0]];
                    out[currSize + 1] = faceIndicies[triangles[i][1]];
                    out[currSize + 2] = faceIndicies[triangles[i][2]];

                    /* printf("OUT: indicies: %d, %d, %d.\n", out[currSize], out[currSize + 1], out[currSize + 2]); */
                    currSize = currSize + 3;
                }
                free(faceIndicies);
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

    free(*line);
    free(line);
    freeElements(vertexList);

    out = realloc(out, sizeof(float) * currSize);
    outModel.indicies = out;
    outModel.verts = outVerts;
    outModel.numVerts = numVerts;
    return outModel;
}


/* int main(int argc, char** args){ */
/*     Model out = loadModel("./car.obj"); */
/*     // Model out = loadModel("./cube.obj"); */
/*     printf("Model created successfully: %s\n", out.successful ? "TRUE" : "FALSE"); */
/*     printf("Created %li float array\n", out.numTris); */
/* } */
