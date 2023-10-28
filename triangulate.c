#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "triangulate.h"
#include "list.h"
#include "dataStruct.h"

vec3f crossProduct(vec3f a, vec3f b){
    vec3f out;
    out.x = a.y*b.z - b.y*a.z;
    out.y = -a.x*b.z + b.x*a.z;
    out.z = a.x*b.y - b.x*a.y;

    return out;
}

void sortIndicies(){

}

void copyMat3f(float in[3][3], float out[3][3]){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            out[i][j] = in[i][j];
        }
    }
}

void switchMatRow(float in[3][3], int a, int b){
    float t[3];
    for (int i = 0; i < 3; i++){
        t[i] = in[i][a];
        in[i][a] = in[i][b];
        in[i][b] = t[i];
    }
}

void addMatRow(float in[3][3], int a, int b, float aScalingFactor){
    for (int i = 0; i < 3; i++){
        in[i][b] = in[i][a] * aScalingFactor + in[i][b];
    }
}

void solveSystem(float inMat[3][3], float outMat[3][3]){
    copyMat3f(inMat, outMat);
    /* printf("\nINPUT:"); */
    /* printf("\n%f %f %f\n", outMat[0][0], outMat[1][0], outMat[2][0]); */
    /* printf("%f %f %f\n", outMat[0][1], outMat[1][1], outMat[2][1]); */
    /* printf("%f %f %f\n", outMat[0][2], outMat[1][2], outMat[2][2]); */

    // Trying to get a non-zero value into the top left corner
    if (outMat[0][0] == 0){
        // In this case we need another row to go to the top
        if (outMat[0][1] == 0){
            if (outMat[0][2] == 0){
                printf("ERROR: Triangulation issue, didn't get a good basis\n");
                return;
            }
            switchMatRow(outMat, 0, 2);
        }else{
            switchMatRow(outMat, 0, 1);
        }
    }

    for(int i = 1; i < 3; i++){
        if (outMat[0][i] != 0){
            addMatRow(outMat, 0, i, -outMat[0][i]/outMat[0][0]);
        }
    }

    if (outMat[1][1] == 0){
        if (outMat[1][2] == 0){
            printf("ERROR: Triangulation issue, didn't get a good basis\n");
            return;
        }
        switchMatRow(outMat, 1, 2);
    }
    addMatRow(outMat, 1, 2, -outMat[1][2]/outMat[1][1]);
    addMatRow(outMat, 1, 0, -outMat[1][0]/outMat[1][1]);

    /* printf("OUTPUT:"); */
    /* printf("\n%f %f %f\n", outMat[0][0], outMat[1][0], outMat[2][0]); */
    /* printf("%f %f %f\n", outMat[0][1], outMat[1][1], outMat[2][1]); */
    /* printf("%f %f %f\n", outMat[0][2], outMat[1][2], outMat[2][2]); */
}

int sortCoords(const void* a, const void* b){
    float* v1 = (float*) a;
    float* v2 = (float*) b;

    if (v1[0] == v2[0]){
        if (v1[1] != v2[1]){
            return v1[1] < v2[1] ? 1 : -1;
        }
        return 0;
    }
    return v1[0] < v2[0] ? 1 : -1;
}

int arrayWrap(int i, int size){
    return ((i % size) + size) % size;
}


bool testIfTriValid(float p1[2], float p2[2], float p3[2]){
    vec3f a, b;
    a.x = p2[0] - p1[0];
    a.y = p2[1] - p1[1];
    a.z = 0;
    b.x = p3[0] - p2[0];
    b.y = p3[1] - p2[1];
    b.z = 0;

    vec3f c = crossProduct(a, b);

    return c.z > 0;
}

// Need to make a thing to triangulate a monotone polygon
long** triangulateMonoPoly(long* indicies, float** verticies, int numPoints, unsigned int* outTris){
    // First step is to convert the 3D coordinates into 2D
    vec3f a, b, c;
    a.x = verticies[indicies[0]][0];
    a.y = verticies[indicies[0]][1];
    a.z = verticies[indicies[0]][2];

    b.x = verticies[indicies[1]][0];
    b.y = verticies[indicies[1]][1];
    b.z = verticies[indicies[1]][2];

    c.x = verticies[indicies[2]][0];
    c.y = verticies[indicies[2]][1];
    c.z = verticies[indicies[2]][2];

    float mat[3][3];
    mat[0][0] = a.x - b.x;
    mat[0][1] = a.y - b.y;
    mat[0][2] = a.z - b.z;
    mat[1][0] = a.x - c.x;
    mat[1][1] = a.y - c.y;
    mat[1][2] = a.z - c.z;

    // Array to contain the 2D coords of the face so we can triangulate it in 2D
    // Hopefully it is a safe assumption that the faces will be flat
    float** relativeCoords = malloc(sizeof(float*) * numPoints);

    for (int i = 0; i < numPoints; i++){
        long index = indicies[i];
        //printf("Index: %ld\n", index);
        //printf("Vertex: %d, Coords: %f, %f, %f\n", i, verticies[index][0], verticies[index][1], verticies[index][2]);

        mat[2][0] = a.x - verticies[index][0];
        mat[2][1] = a.y - verticies[index][1];
        mat[2][2] = a.z - verticies[index][2];

        // Going to need to store these in a better place
        float solvedMat[3][3];
        solveSystem(mat, solvedMat);

        if (fabsf(solvedMat[2][2]) > 0.001){
            printf("Unable to solve the system!\n");
            printf("OUTPUT:");
            printf("\n%f %f %f\n", solvedMat[0][0], solvedMat[1][0], solvedMat[2][0]);
            printf("%f %f %f\n", solvedMat[0][1], solvedMat[1][1], solvedMat[2][1]);
            printf("%f %f %f\n", solvedMat[0][2], solvedMat[1][2], solvedMat[2][2]);
        }

        // Now that I have the system solved I need to get the x, y coordinates out
        float x = solvedMat[2][0] / solvedMat[0][0];
        float y = solvedMat[2][1] / solvedMat[1][1];
        /* printf("New coords: %f, %f\n", x, y); */

        // Got the new coords out now we need to save them somewhere
        float* newCoords = malloc(sizeof(float) * 2);
        newCoords[0] = x;
        newCoords[1] = y;
        relativeCoords[i] = newCoords;
    }

    // We now have the coordinates in 2D in the array relativeCoords
    // First thing that we need to do is find the minimum x position
    float minX = relativeCoords[0][0];
    int startX = 0;
    for (int i = 0; i < numPoints; i++){
        if (relativeCoords[i][0] < minX){
            minX = relativeCoords[i][0];
            startX = i;
        }
    }

    // Now we need to find if we go right if that is up or down
    bool rightUp = true;
    float startY = relativeCoords[startX][1];
    int tmpInd = startX;
    while(true) {
        tmpInd++;
        if (startY > relativeCoords[tmpInd % numPoints][1]){
            rightUp = false;
            break;
        } else if (startY < relativeCoords[tmpInd % numPoints][1]){
            rightUp = true;
            break;
        }
    }

    int upDir = 1;
    if (!rightUp) {
        upDir = -1;
    }

    // Need to get our first 2 points
    long curTopInd = arrayWrap(startX + upDir,  numPoints);
    long curBotInd = startX;

    // Finding the start point which is what ever is bigger
    long curInd = curBotInd;
    if (relativeCoords[curTopInd][0] > relativeCoords[curBotInd][0]){
        curInd = curTopInd;
    }

    void* out = createList(5);

    bool chainIsTop = false;
    void* chain = createList(5);

    HashSet set = createHashSet();
    long* t = malloc(sizeof(long));
    *t = curTopInd;
    insertSetElement(&set, curTopInd, t);

    t = malloc(sizeof(long));
    *t = curBotInd;
    insertSetElement(&set, curBotInd, t);

    // Now we need to start a loop to go through all of the points
    while(true) {
        bool isTop = false;
        // Want to find the next point or the lower X
        if (relativeCoords[arrayWrap(curTopInd + upDir, numPoints)][0] < relativeCoords[arrayWrap(curBotInd - upDir, numPoints)][0]){
            curInd = curTopInd + upDir;
            isTop = true;
        } else {
            curInd = curBotInd - upDir;
            isTop = false;
        }
        // Trying to make sure that our index is in bounds
        curInd = arrayWrap(curInd, numPoints);

        t = malloc(sizeof(long));
        *t = curInd;
        if (!insertSetElement(&set, curInd, t)) {
            break;
        }

        // If we have a chain then we want to check to see if the new point can make a triangle with the last 2 points
        // We also need to keep doing this since we can make multiple triangles from just the chain
        if (getListSize(chain) > 1 && isTop == chainIsTop) {
            for (int i = getListSize(chain) - 1; i > 2; i--){
                long first = *(long*) getListElement(chain, i - 2);
                long second = *(long*) getListElement(chain, i - 1);
                if (testIfTriValid(relativeCoords[first], relativeCoords[second], relativeCoords[curInd])) {
                    printf("Triangle: %ld, %ld, %ld\n", curBotInd, curTopInd, curInd);
                    long* newTri = malloc(sizeof(long) * 3);
                    newTri[0] = curBotInd;
                    newTri[1] = curTopInd;
                    newTri[2] = curInd;

                    if (isTop) {
                        curTopInd = curInd;
                    } else {
                        curBotInd = curInd;
                    }

                    listAppend(out, newTri);
                    void* tmp = listRemoveLastElem(chain);
                    free(tmp);
                } else {
                    break;
                }
            }
        }

        // Now we need to find if our current three points
        // curInd, curTopInd, and curBotInd can make a triangle
        if(testIfTriValid(relativeCoords[curTopInd], relativeCoords[curBotInd], relativeCoords[curInd])) {
            // If there is a chain we need to do some more processing
            if (!getListSize(chain)){
                // Here we need to add a triangle to our output
                printf("Triangle: %ld, %ld, %ld\n", curBotInd, curTopInd, curInd);
                long* newTri = malloc(sizeof(long) * 3);
                newTri[0] = curBotInd;
                newTri[1] = curTopInd;
                newTri[2] = curInd;

                if (isTop) {
                    curTopInd = curInd;
                } else {
                    curBotInd = curInd;
                }

                listAppend(out, newTri);
            }

            // Setting the new current top or bottom position depending on
            // whether the current index is on the top or bottom 
            if (isTop) {
                curTopInd = curInd;
            } else {
                curBotInd = curInd;
            }
        } else if(isTop == chainIsTop) {
            long* curIndCopy = malloc(sizeof(long) * 1);
            *curIndCopy = curInd;
            listAppend(chain, curIndCopy);
        }

        if (isTop != chainIsTop) {
            // This is the case where the chain is on the opposite side of the current point
            // In this case we need to loop through the chain adding triangles since they will all be valid
            // We just go in order for this
            for (int i = 0; i < getListSize(chain); i++){
                long tmpInd = *(long*) getListElement(chain, i);
                printf("Triangle: %ld, %ld, %ld\n", curBotInd, curTopInd, tmpInd);
                long* newTri = malloc(sizeof(long) * 3);
                newTri[0] = curBotInd;
                newTri[1] = curTopInd;
                newTri[2] = tmpInd;

                listAppend(out, newTri);

                if (chainIsTop) {
                    curTopInd = tmpInd;
                } else {
                    curBotInd = tmpInd;
                }
            }

            freeElements(chain);
            chain = createList(5);
        }
    }

    freeElements(chain);

    for(int i = 0; i < numPoints; i++){
        free(relativeCoords[i]);
    }
    free(relativeCoords);

    // Need to get my output from my list
    long** outArr = (long**) listToArray(out, outTris);
    return outArr;
}

/* // Need to make a thing to triangulate a monotone polygon */
/* int** triangulateMonoPoly(long* indicies, float** verticies, int numPoints){ */
/*     // First step is to convert the 3D coordinates into 2D */
/*     vec3f a, b, c; */
/*     a.x = verticies[indicies[0]][0]; */
/*     a.y = verticies[indicies[0]][1]; */
/*     a.z = verticies[indicies[0]][2]; */
/*  */
/*     b.x = verticies[indicies[1]][0]; */
/*     b.y = verticies[indicies[1]][1]; */
/*     b.z = verticies[indicies[1]][2]; */
/*  */
/*     c.x = verticies[indicies[2]][0]; */
/*     c.y = verticies[indicies[2]][1]; */
/*     c.z = verticies[indicies[2]][2]; */
/*  */
/*     float mat[3][3]; */
/*     mat[0][0] = a.x - b.x; */
/*     mat[0][1] = a.y - b.y; */
/*     mat[0][2] = a.z - b.z; */
/*     mat[1][0] = a.x - c.x; */
/*     mat[1][1] = a.y - c.y; */
/*     mat[1][2] = a.z - c.z; */
/*  */
/*     // Array to contain the 2D coords of the face so we can triangulate it in 2D */
/*     // Hopefully it is a safe assumption that the faces will be flat */
/*     float** relativeCoords = malloc(sizeof(float*) * numPoints); */
/*  */
/*     for (int i = 0; i < numPoints; i++){ */
/*         long index = indicies[i]; */
/*         //printf("Index: %ld\n", index); */
/*         //printf("Vertex: %d, Coords: %f, %f, %f\n", i, verticies[index][0], verticies[index][1], verticies[index][2]); */
/*  */
/*         mat[2][0] = a.x - verticies[index][0]; */
/*         mat[2][1] = a.y - verticies[index][1]; */
/*         mat[2][2] = a.z - verticies[index][2]; */
/*  */
/*         // Going to need to store these in a better place */
/*         float solvedMat[3][3]; */
/*         solveSystem(mat, solvedMat); */
/*  */
/*         if (abs(solvedMat[2][2]) > 0.001){ */
/*             printf("Unable to solve the system!\n"); */
/*             printf("OUTPUT:"); */
/*             printf("\n%f %f %f\n", solvedMat[0][0], solvedMat[1][0], solvedMat[2][0]); */
/*             printf("%f %f %f\n", solvedMat[0][1], solvedMat[1][1], solvedMat[2][1]); */
/*             printf("%f %f %f\n", solvedMat[0][2], solvedMat[1][2], solvedMat[2][2]); */
/*         } */
/*  */
/*         // Now that I have the system solved I need to get the x, y coordinates out */
/*         float x = solvedMat[2][0] / solvedMat[0][0]; */
/*         float y = solvedMat[2][1] / solvedMat[1][1]; */
        /* printf("New coords: %f, %f\n", x, y); */
/*  */
/*         // Got the new coords out now we need to save them somewhere */
/*         float* newCoords = malloc(sizeof(float) * 2); */
/*         newCoords[0] = x; */
/*         newCoords[1] = y; */
/*         relativeCoords[i] = newCoords; */
/*     } */
/*     // We now have the coordinates in 2D in the array relativeCoords */
/*  */
/*     // Need to sort the points */
    /* qsort(relativeCoords, numPoints, sizeof(float*), sortCoords); */
/*  */
    /* for (int i = 0; i < numPoints; i++){ */
    /*     printf("I: %d | Coords: %f, %f\n", i, relativeCoords[i][0], relativeCoords[i][1]); */
    /* } */
/*  */
/*     float minX = relativeCoords[0][0]; */
/*     int leftMostXIndex = 0; */
/*     // First step is to find the left most point */
/*     for (int i = 0; i < numPoints; i++){ */
/*         if (relativeCoords[i][0] < minX){ */
/*             leftMostXIndex = i; */
/*             minX = relativeCoords[i][0]; */
/*         } */
/*     } */
/*  */
/*     // Now we need to find which direction is up essentially */
/*     bool upPos = false; */
/*     int i = leftMostXIndex; */
/*     while(true){ */
/*         i++; */
/*         float* tmp = relativeCoords[arrayWrap(i, numPoints)]; */
/*         if (tmp[0] != minX){ */
/*             if (tmp[1] > relativeCoords[leftMostXIndex][1]){ */
/*                 upPos = true; */
/*             } */
/*             break; */
/*         } */
/*     } */
/*  */
/*     // Now we know which way to get to the upper and lower chain */
/*     // Next we need to store the upper and lower first two points */
/*     int curBottomInd = leftMostXIndex; */
/*     int curTopInd = leftMostXIndex; */
/*     int minUpperXInd = leftMostXIndex; */
/*     int minLowerXInd = leftMostXIndex; */
/*  */
/*     float nextPosX = relativeCoords[arrayWrap(leftMostXIndex + 1, numPoints)][0]; */
/*     float nextNegX = relativeCoords[arrayWrap(leftMostXIndex - 1, numPoints)][0]; */
/*     int currentPointIndex = 0; */
/*  */
/*     // Trying to find the next point that we would run into */
/*     if (nextNegX < nextPosX){ */
/*         currentPointIndex = leftMostXIndex - 1; */
/*         if (upPos){ */
/*             curBottomInd = currentPointIndex; */
/*         } else{ */
/*             curTopInd = currentPointIndex; */
/*         } */
/*     } else{ */
/*         currentPointIndex = leftMostXIndex + 1; */
/*         if (upPos){ */
/*             curTopInd = currentPointIndex; */
/*         } else{ */
/*             curBottomInd = currentPointIndex; */
/*         } */
/*     } */
/*  */
/*     // Finding the direction that we are looping around the points */
/*     int nextUp = -1; */
/*     if (upPos){ */
/*         nextUp = 1; */
/*     } */
/*  */
/*     bool chainUp = false; */
/*     void* chain = createList(5); */
/*     void* out = createList(5); */
/*  */
/*     // TODO Getting stuck in this loop */
/*     // Next we need to start going through successive points and trying to add triangles */
/*     while(true){ */
/*         // First need to find the next point that we are going to use */
/*         float* nextUpperPos = relativeCoords[arrayWrap(curTopInd + nextUp, numPoints)]; */
/*         float* nextLowerPos = relativeCoords[arrayWrap(curBottomInd - nextUp, numPoints)]; */
/*         bool curUpper = false; */
/*  */
/*         if (nextUpperPos[0] < nextLowerPos[0]){ */
/*             // This means that we are going to be advancing the upper chain next */
/*             currentPointIndex = curTopInd + nextUp; */
/*             curUpper = true; */
/*         }else{ */
/*             // This means that we are going to be advancing the lower chain next */
/*             currentPointIndex = curTopInd - nextUp; */
/*         } */
/*         float* currentPos = relativeCoords[arrayWrap(currentPointIndex, numPoints)]; */
/*  */
/*         // Not sure what is the first thing to check here */
/*         if (chainUp == curUpper && getListSize(chain) > 0){ */
/*             // Here we need to see if we can make a triangle or two with the chain and this vertex, if not then just add it to the chain */
/*             // Need to do a cross product between the vector of the current point and last point added to the chain */
/*             // The other vector is the current point and the point before last added to the chain */
/*             while(true){ */
/*                 // Kind of want to make an array of sorts that has the last used upper and lower points first and then the rest of the chain after */
/*  */
/*                 vertex* n = (vertex*) getListElement(chain, getListSize(chain) - 1); */
/*                 vertex tempVert; */
/*                 float tmpCoords[2]; */
/*                 vertex* m; */
/*  */
/*                 if (getListSize(chain) < 2){ */
/*                     m = &tempVert; */
/*                     tempVert.coords = &tmpCoords; */
/*                     tmpCoords[0] = currentPos[0]; */
/*                     tmpCoords[1] = currentPos[1]; */
/*  */
/*                     tempVert.index = currentPointIndex; */
/*  */
/*                     // Need to figure out how we are storing the vertex */
/*                     // Here we need to get the coords for the last vertex on the chain that was added that we are currently on */
/*                     // Wait what am I doing here... I'm trying to get something to work for sure */
/*                     // If there are less than 2 points in the chain that means that we need to add to the chain unless it is on the opposite side */
/*                     listAppend(chain, m); */
/*                 }else { */
/*                     m = (vertex*) getListElement(chain, getListSize(chain) - 2); */
/*                 } */
/*  */
/*                 vec3f a, b; */
/*                 a.x = currentPos[0] - n->coords[0]; */
/*                 a.y = currentPos[1] - n->coords[1]; */
/*                 a.z = 0; */
/*                 b.x = n->coords[0] - m->coords[0]; */
/*                 b.y = n->coords[1] - m->coords[1]; */
/*                 b.z = 0; */
/*  */
/*                 // Going to assume if this is positive that it means we can make the triangle */
/*                 if(crossProduct(a, b).z > 0){ */
/*                     // In this case we want to add the triangle... */
/*                     long* newTri = malloc(sizeof(long) * 3); */
/*                     newTri[0] = currentPointIndex; */
/*                     newTri[1] = n->index; */
/*                     newTri[2] = m->index; */
/*  */
/*                     listAppend(out, newTri); */
/*                     free(listRemoveLastElem(chain)); */
/*                     continue; */
/*                 } */
/*                 break; */
/*             } */
/*         } else{ */
/*             // Here we need to use up the chain if there is any, at the very least we need to check to see if we can make a new triangle with the curTop and curBottom indicies */
/*         } */
/*  */
/*         if(curUpper){ */
/*             curTopInd = currentPointIndex; */
/*         }else{ */
/*             curBottomInd = currentPointIndex; */
/*         } */
/*     } */
/*  */
/*     freeList(chain); */
/*     freeElements(chain); */
/*  */
/*     for(int i = 0; i < numPoints; i++){ */
/*         free(relativeCoords[i]); */
/*     } */
/*     free(relativeCoords); */
/* } */

long** triangulateFace(long* indicies, float** verticies, int numPoints, unsigned int* outTris){
    // TODO Should be splitting the polygon into monotone polygons first
    return triangulateMonoPoly(indicies, verticies, numPoints, outTris);
}
