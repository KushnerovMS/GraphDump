#ifndef GRAPH_DUMP_H
#define GRAPH_DUMP_H

#include <stdio.h>

//const char NODE_STYLE_STRUCTURE[] = "record";

const double LINE_COLOR_COEF = 0.8;

//const unsigned long GRAPH_BACKGROUND_COLOR = 0x909090ff;

enum NodeShape
{
    NONE_SHAPE      = 0,
    RECORD_SHAPE    = 1,
    RECTANGLE_SHAPE = 2
};

const char NODE_SHAPES[][100] = {"none",
                                "record",
                                "rectangle"};

typedef struct Graph_
{
    FILE* file;

    char* name;
} Graph;

typedef struct GraphNode_
{
    const void* id;

    NodeShape shape;
    bool rounded;
    unsigned color;
} GraphNode;

typedef GraphNode_ GraphCluster;

typedef struct GraphEdge_
{
    unsigned long color;
    int minLen;
    int penWidth;
    
} GraphEdge;

Graph*  GraphOpen (const char* graphName = nullptr);
void    GraphDraw (Graph* graph, const char* name, const char* format);


void    GraphAddNode (const Graph* graph, const GraphNode* node, const char* labelfrmt = nullptr, ...);
void    GraphAddImage(const Graph* graph, const void* id, const char* imageName = nullptr);

void    GraphAddEdge (const Graph* graph, const GraphEdge* edge,
                        const void* beginId, const char* beginPort,
                        const void* endId, const char* endPort,
                        bool streamLine = 0,
                        const char* labelfrmt = nullptr, ...);

void    GraphBeginCluster (const Graph* graph, const GraphCluster* cluster, const char* labelfrmt = nullptr, ...);
void    GraphEndCluster (const Graph* graph);

void    GraphStreamlineNodes (const Graph* graph, const void* id1, const void* id2);

#endif
