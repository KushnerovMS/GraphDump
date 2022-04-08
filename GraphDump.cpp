#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>

#include "lib/Logs.h"

#include "GraphDump.h"


Graph* GraphOpen (const char* graphName)
{
    Graph* graph = (Graph*) calloc (1, sizeof (Graph));

    if (graphName)
        graph -> name = strdup (graphName);
    else
    {
        graph -> name = (char*) calloc (L_tmpnam, sizeof (char));
        tmpnam (graph -> name);
    }

    graph -> file = fopen (graph -> name, "w");
    if (graph -> file == nullptr)
        return nullptr;

    fprintf (graph -> file, "digraph g {\n");

    return graph;
}

struct word
{
    const char* str;
    size_t n;
};

#define SEARCH_FOR_WORD(word, func)     \
    if (buff[i] == word.str[word.n])    \
    {                                   \
        if (word.n == 0)                \
            end = i;                    \
        word.n ++;                      \
                                        \
        if (word.str[word.n] == '\0')   \
            func                        \
    }                                   \
    else                                \
        word.n = 0;                         

bool _checkSuffix (const char* buff, const char* suff)
{
    struct word suffix = {suff, 0};
    size_t end = 0;
    for (size_t i = 0; buff[i] != '\0'; i ++)
    {
        SEARCH_FOR_WORD (suffix,
            {
                if (buff[i + 1] == '\0')
                    return 1;
            })
    }

    return 0;
}

bool _insertSvg (FILE* file, const char* svgName, double x, double y);

void _replaceImgWithSvg (const char* oldFileName, const char* newFileName)
{
    FILE* oldFile = fopen (oldFileName, "r");
    if (oldFile == nullptr)
    {
        Logs.error ("Can not open file \"%s\"", oldFileName);
        return;
    }
    FILE* newFile = fopen (newFileName, "w");
    if (newFile == nullptr)
    {
        Logs.error ("Can not open file %s", newFileName);
        return;
    }

    struct stat st = {};
    stat (oldFileName, &st);
    char* buff = (char*) calloc (st.st_size + 1, sizeof (char));
    if (buff == nullptr)
    {
        Logs.error ("Can not alloc buffer with size %u", st.st_size);
        exit (1);
    }
    fread (buff, st.st_size, sizeof (char), oldFile);
    fclose (oldFile);
    
    size_t begin = 0;
    size_t end = 0;

    struct word image = {"<image", 0};
    struct word ellipse = {"<ellipse", 0};

    size_t ellipseBegin = 0;
    
    for (size_t i = 0; i < st.st_size; i ++)
    {
        SEARCH_FOR_WORD (ellipse,
            {
                ellipseBegin = end;
            })
        SEARCH_FOR_WORD (image,
            {
                char imgName[100] = {};
                double x = 0;
                double y = 0;

                int n = 0;

                sscanf (buff + end, "<image xlink:href=\"%[^\"]\" width=\"%*[0-9]px\" height=\"%*[0-9]px\" preserveAspectRatio=\"xMinYMin meet\" x=\"%lf\" y=\"%lf\"/>%n",
                        imgName, &x, &y, &n);
                i = end + n;

                if (_checkSuffix (imgName, ".svg"))
                {
                    fwrite (buff + begin, ellipseBegin - begin, sizeof (char), newFile);

                    begin = i;

                    Logs.debug ("In file \"%s\" searched image \"%s\" (%le, %le)", oldFileName, imgName, x, y);

                    if (! _insertSvg (newFile, imgName, x, y))
                        fwrite (buff + ellipseBegin, i - ellipseBegin, sizeof (char), newFile);
                }
            })
    }

    fwrite (buff + begin, st.st_size - begin, sizeof (char), newFile);

    free (buff);

    fclose (newFile);
}

bool _insertSvg (FILE* file, const char* svgName, double x, double y)
{
    size_t begin = 0;
    size_t end = 0;

    FILE* svgFile = fopen (svgName, "r");
    if (svgFile == nullptr)
    {
        Logs.warn ("Can not open file \"%s\"", svgName);

        return 0;
    }
    else
    {
        struct stat st = {};
        stat (svgName, &st);
        char* buff = (char*) calloc (st.st_size, sizeof (char));
        if (buff == nullptr)
        {
            Logs.error ("Can not alloc buffer with size %u", st.st_size);

            fclose (svgFile);
            return 0;
        }

        fread (buff, st.st_size, sizeof (char), svgFile);
        fclose (svgFile);

        struct word svg = {"<svg", 0};
        for (size_t i = 0; i < st.st_size; i ++)
        {
            SEARCH_FOR_WORD (svg,
                {
                    begin = end;
                    break;
                })
        }
        struct word pt = {"pt", 0};
        for (size_t i = begin, ptCount = 0; i < st.st_size && ptCount < 2; i ++)
        {
            SEARCH_FOR_WORD (pt,
                {
                    fwrite (buff + begin, end - begin, sizeof (char), file);
                    begin = i + 1;
                })
        }
        begin ++;

        fprintf (file, "\" x=\"%le\" y=\"%le\"", x, y);

        fwrite (buff + begin, st.st_size - begin, sizeof (char), file);

        free (buff);

        return 1;
    }
}

#undef SEARCH_FOR_WORD

void GraphDraw (Graph* graph, const char* name, const char* format)
{
    assert (graph);
    assert (name);
    assert (format);

    fprintf (graph -> file, "}");

    fclose (graph -> file);

    char command[1000] = {};

    if (strcmp (format, "svg") == 0)
    {
        char* tmpname = (char*) calloc (L_tmpnam, sizeof (char));
        tmpnam (tmpname);
        sprintf (command, "dot %s -T%s -o %s", graph -> name, format, tmpname);  
        system (command);

        _replaceImgWithSvg (tmpname, name);

        free (tmpname);
    }
    else
    {
        sprintf (command, "dot %s -T%s -o %s", graph -> name, format, name);  
        system (command);
    }

//    sprintf (command, "rm %s", graph -> name);  
//    system (command);

    free (graph -> name);
    free (graph);
}

unsigned long _getLineColor (unsigned color)
{
    if (color == 0)
        return 0xffffff;

    return ((unsigned)((color & (0xff << 16)) * LINE_COLOR_COEF) & (0xff << 16)) |
           ((unsigned)((color & (0xff <<  8)) * LINE_COLOR_COEF) & (0xff <<  8)) |
           ((unsigned)((color & (0xff <<  0)) * LINE_COLOR_COEF) & (0xff <<  0));
}

unsigned long _getFontColor (unsigned color)
{
    return (((color >> 16) & 0xff) + ((color >> 8) & 0xff) + ((color >> 0) & 0xff) >= 0x80 * 3)? 0x000000 : 0xffffff;
}

void GraphAddNode (const Graph* graph, const GraphNode* node, const char* labelfrmt, ...)
{
    assert (graph);
    assert (node);

    if (node -> id == nullptr)
        fprintf (graph -> file, "node [\n");
    else
        fprintf (graph -> file, "node%p[\n", node -> id);

    fprintf (graph -> file,
                "style = \"filled, %s\"\n"
                "shape = %s\n"
                "fillcolor = \"#%06x\"\n"
                "color = \"#%06x\"\n"
                "fontcolor = \"#%06x\"\n"
                "label = \"",
                (node -> rounded)? "rounded" : "",
                NODE_SHAPES[node -> shape],
                node -> color,
                _getLineColor (node -> color),
                _getFontColor (node -> color));

    if (labelfrmt)
    {
        va_list ap;
        va_start (ap, labelfrmt);
        vfprintf (graph -> file, labelfrmt, ap);
        va_end (ap);

        fprintf (graph -> file, "\"\n]\n");
    }
}
   
void GraphAddImage(const Graph* graph, const void* id, const char* imageName)
{
    assert (graph);

    fprintf (graph -> file,
                "node%p[\n"
                "label = \"\"\n",
                id);

    if (imageName)
        fprintf (graph -> file,
                "image = \"%s\"\n"
                "]\n",
                id, imageName);
    else
        fprintf (graph -> file,
                "image = \"%p.csv\"\n"
                "]\n",
                id, id);

}

void GraphAddEdge (const Graph* graph, const GraphEdge* edge,
                        const void* beginId, const char* beginPort,
                        const void* endId, const char* endPort,
                        bool streamLine,
                        const char* labelfrmt, ...)
{
    assert (graph);
    assert (edge);
    assert (beginPort);
    assert (endPort);

    if (streamLine)
        GraphStreamlineNodes (graph, beginId, endId);
    fprintf (graph -> file, "node%p%s -> node%p%s[\n"
        "minlen = %d\n"
        "color = \"#%06x\"\n"
        "penwidth = %d\n"
        "constraint = false\n"
        "label = \"",
        beginId, beginPort, endId, endPort,
        (edge -> minLen <= 0)? 1 : edge -> minLen,
        edge -> color,
        (edge -> penWidth <= 0)? 1 : edge -> penWidth);

    if (labelfrmt)
    {
        va_list ap;
        va_start (ap, labelfrmt);
        vfprintf (graph -> file, labelfrmt, ap);
        va_end (ap);

        fprintf (graph -> file, "\"]\n");
    }
}


void GraphBeginCluster (const Graph* graph, const GraphCluster* cluster, const char* labelfrmt, ...)
{
    assert (graph);
    assert (cluster);
    
    fprintf (graph -> file,
                "subgraph cluster%p {\n"
                    "style = \"filled, %s\"\n"
                    "fillcolor = \"#%06x\"\n"
                    "color = \"#%06x\"\n"
                    "fontcolor = \"#%06x\"\n"
                    "label = \"",
                cluster -> id,
                    (cluster -> rounded)? "rounded" : "",
                    cluster -> color,
                    _getLineColor (cluster -> color),
                    _getFontColor (cluster -> color));

    if (labelfrmt)
    {
        va_list ap;
        va_start (ap, labelfrmt);
        vfprintf (graph -> file, labelfrmt, ap);
        va_end (ap);
    }

    fprintf (graph -> file, "\"\n");
}

void GraphEndCluster (const Graph* graph)
{
    assert (graph);

    fprintf (graph -> file, "}\n");
}


void GraphStreamlineNodes (const Graph* graph, const void* id1, const void* id2)
{
    assert (graph);

    fprintf (graph -> file, "node%p -> node%p["
                            "constraint = true "
                            "style = invis]\n",
                            id1, id2);
}
