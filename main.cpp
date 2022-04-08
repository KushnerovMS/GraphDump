#include <stdio.h>

#include "GraphDump.h"

int main ()
{
    Graph* graph = GraphOpen ();

    GraphNode node1 = {};
    node1.id = &node1;
    node1.rounded = 1;
    node1.shape = RECORD_SHAPE;
    node1.color = 0xff0f00;

    GraphNode node2 = {};
    node2.id = &node2;
    node2.rounded = 1;
    node2.shape = RECTANGLE_SHAPE;

    GraphEdge edge = {};

    GraphAddNode (graph, &node1, "node%d", 1);
    GraphAddNode (graph, &node2, "");

    GraphAddEdge (graph, &edge, node1.id, "", node2.id, "", 1, "");

    GraphDraw (graph, "img1.svg", "svg");


    graph = GraphOpen ();

    GraphAddNode (graph, &node1, "node%d", 1);
    GraphAddNode (graph, &node2, "");

    GraphAddEdge (graph, &edge, node1.id, "", node2.id, "", 1, "");

    GraphDraw (graph, "img2.svg", "svg");


    graph = GraphOpen ();



    GraphAddNode (graph, &node1, "node%d", 1);
    GraphAddNode (graph, &node2, "node2");

    GraphAddImage (graph, "img1.svg", "img1.svg");
    GraphAddEdge (graph, &edge, node1.id, "", "img1.svg", "", 1, "");

    GraphAddImage (graph, "img2.svg", "img2.svg");
    GraphAddEdge (graph, &edge, node2.id, "", "img2.svg", "", 1, "");

    GraphDraw (graph, "test.svg", "svg");


    return 0;
}
