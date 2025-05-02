#ifndef CBLT_RENDER_NODE_H
#define CBLT_RENDER_NODE_H

#include <array>
#include <memory>

namespace cblt::render {

enum NodeType {
    Int,
    Float,
    Color,
    Vector,
};

class Node {
public:
    virtual ~Node() = default;

protected:
    static constexpr size_t kMaxChildren = 16;

    Node() = default;

    std::array<std::weak_ptr<Node>, kMaxChildren> _elements;
};

template<typename T>
class TypedNode : public Node {
public:
    TypedNode(T value): _value{value} {
    }

    virtual ~TypedNode() = default;

private:
    T _value;
};

} // namespace cblt::render

#endif // CBLT_RENDER_NODE_H
