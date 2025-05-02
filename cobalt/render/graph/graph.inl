#ifndef CBLT_RENDER_GRAPH_INL
#define CBLT_RENDER_GRAPH_INL

#include "graph.h"

#include "color.h"
#include "node.h"

#include "math/vec2.h"

#include <memory>
#include <type_traits>

namespace cblt::render {

namespace {

template <NodeType nodeType, typename T>
std::shared_ptr<Node> make_default(T value) {
    if constexpr (nodeType == NodeType::Float) {
        static_assert(std::is_convertible_v<T, float>);
        return std::shared_ptr<Node>(new TypedNode<float>(float(value)));
    } else if constexpr (nodeType == NodeType::Int) {
        static_assert(std::is_convertible_v<T, int>);
        return std::shared_ptr<Node>(new TypedNode<int>(int(value)));
    } else if constexpr (nodeType == NodeType::Vector) {
        static_assert(std::is_convertible_v<T, vec2f>);    
        return std::shared_ptr<Node>(new TypedNode<vec2f>(vec2f(value)));
    } else if constexpr (nodeType == NodeType::Color) {
        static_assert(std::is_convertible_v<T, CoColor>);    
        return std::shared_ptr<Node>(new TypedNode<CoColor>(CoColor(value)));
    } else {
        static_assert(false, "Unknown NodeType");
    }

    return nullptr;
}

}  // anonymous namespace

template<class NodeClass>
std::shared_ptr<CoGraph> CoGraph::createWithTypedRoot() {
    std::shared_ptr<Node> baseNode = std::shared_ptr<Node>(new NodeClass);

    return std::shared_ptr<CoGraph>(new CoGraph(baseNode));
}

CoGraph::CoGraph(std::shared_ptr<Node> base) : _base{base} {
}

template<NodeType nodeType, typename literal>
void CoGraph::declareInput(std::string_view name, literal &&fallback) {
    std::shared_ptr<Node> defaultNode = make_default<nodeType>(fallback);
    _inputNodes[std::string(name)] = defaultNode;
}

}  // namespace cblt::render

#endif  // CBLT_RENDER_GRAPH_INL