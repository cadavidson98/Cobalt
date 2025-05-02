#include "principled_node.h"

namespace cblt::render {

std::shared_ptr<CoGraph> CoPrincipledNode::buildPrincipledGraph() {
    std::shared_ptr<CoGraph> principledSet = CoGraph::createWithTypedRoot<CoPrincipledNode>();

    principledSet->declareInput<NodeType::Color>("baseColor", CoColor(0, 0, 0));
    principledSet->declareInput<NodeType::Float>("metallic", 0.f);
    principledSet->declareInput<NodeType::Float>("subsurface", 0.f);
    principledSet->declareInput<NodeType::Float>("ior", 1.4f);
    principledSet->declareInput<NodeType::Float>("specular", 0.f);
    principledSet->declareInput<NodeType::Float>("specularTint", 0.f);
    principledSet->declareInput<NodeType::Float>("specularTransmission", 0.f);
    principledSet->declareInput<NodeType::Float>("roughness", 1.f);
    principledSet->declareInput<NodeType::Float>("anisotropic", 0.f);
    principledSet->declareInput<NodeType::Float>("sheen", 0.f);
    principledSet->declareInput<NodeType::Float>("sheenTint", 0.f);
    principledSet->declareInput<NodeType::Float>("clearcoat", 0.f);
    principledSet->declareInput<NodeType::Float>("clearcoatGloss", 0.f);

    return principledSet;
}

} // namespace cblt::render
