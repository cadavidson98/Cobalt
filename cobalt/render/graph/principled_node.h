#ifndef CBLT_RENDER_PRINCIPLED_NODE_H
#define CBLT_RENDER_PRINCIPLED_NODE_H

#include "graph.h"
#include "node.h"

namespace cblt::render {

class CoPrincipledNode : public Node {
public:
    CoPrincipledNode() = default;

    static std::shared_ptr<CoGraph> buildPrincipledGraph();

    virtual ~CoPrincipledNode() = default;

private:
};

} // namespace cblt::render

#endif // CBLT_RENDER_PRINCIPLED_NODE_H
