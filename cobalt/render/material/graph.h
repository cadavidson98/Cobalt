#ifndef CBLT_RENDER_GRAPH_H
#define CBLT_RENDER_GRAPH_H

namespace cblt::render {

/*
    class TextureNode : public Node {
    public:
        TextureNode() {
        }

        InputSocket &coordinate() {
            return _coordinate;
        }

        InputSocket &face() {
            return _face;
        }

        OutputSocket &color() {
            return _color;
        }

    private:
        InputSocket<vec2f> _coordinate;
        InputSocket<uint> _face;
        OutputSocket<CoColor> _color;
    }

    struct PrincipledNode : public Node {
        PrincipledNode() :
            _baseColor{NodeType::Color, TypedNode<CoColor>(CoColor(0, 0, 0))},
            _metallic{NodeType::Metallic},
            _ {
        }

        InputSocket &baseColor() {
            return getInput("baseColor");
        }
    }

    Graph *graph = Graph::emptyGraph();
    Cache *cache = Cache::create(graph);

    PrincipledNode *principledNode = new PrincipledNode();
    graph->addNode(principledNode);

    you might need and edge list in each socket, fool!
    spoiler alert: you do :)

    NodeSocket<CoColor> &baseColor = principledNode->baseColor();

    if (std::has_value<MitsubaTexture>(baseColor)) {
        TextureNode *textureNode = cache->findOrInsertTexture(std::get<MitsubaTexture>(baseColor));
        NodeSocket &textureColor = textureNode->color();
        graph->link(baseColor, textureColor);
    } else {
        ValueNode *valueNode = new ValueNode<NodeType::Color, Color>(std::get<Color>(baseColor));
        graph->addNode(valueNode);
        OutputSocket &constantColor = valueNode->value();
        graph->link(baseColor, inputColor);
    }

    // is thread safe because it will use thread local memory to set node inputs
    // before traversing the graph to compute values (ex: texture sampling uv & face);
    CoSpectrum bsdf = graph->execute(inputNodes);
*/

} // namespace cblt::render

#endif // CBLT_RENDER_GRAPH_H
