#ifndef COBALT_RENDER_XML_UTILITIES_H
#define COBALT_RENDER_XML_UTILITIES_H

#include <libxml/xmlstring.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xpath.h>

#include <cassert>
#include <memory>
#include <optional>
#include <cstring>
#include <type_traits>

namespace xml2 {

struct xmlDeleter {
    void operator()(void *ptr) {
        xmlFree(ptr);
    }
};

struct xmlWeakDeleter {
    void operator()([[maybe_unused]] void *ptr) {
        // TODO: ref count? No Op?
    }
};

struct xmlDocDeleter {
    void operator()(xmlDoc *ptr) {
        xmlFreeDoc(ptr);
    }
};

struct xmlTextReaderDeleter {
    void operator()(xmlTextReader *ptr) {
        xmlFreeTextReader(ptr);
    }
};

struct xmlNodeDeleter {
    void operator()(xmlNodeSet *ptr) {
        xmlXPathFreeNodeSet(ptr);
    }
};

template<typename T, class Deleter = xmlDeleter>
struct xmlResource {
public:
    xmlResource(T *raw): _value{raw} {
    }

    ~xmlResource() {
        if (_value) {
            _deleter(_value);
        }
    }

    T &operator*() {
        return *_value;
    }

    T *operator->() {
        return _value;
    }

    void reset(T *ptr) {
        _value = ptr;
    }

    T *get() const {
        return _value;
    }

    explicit operator bool() const {
        return bool(_value);
    }

    xmlResource(xmlResource &other) = delete;
    xmlResource operator=(xmlResource other) = delete;
    xmlResource &operator=(xmlResource &other) = delete;

    xmlResource(xmlResource &&other) = default;
    xmlResource &operator=(xmlResource &&other) = default;

private:
    T *_value = nullptr;
    Deleter _deleter;
};


template<typename T>
using xmlWeak = xmlResource<T, xmlWeakDeleter>;

using TextReader = xmlResource<xmlTextReader, xmlTextReaderDeleter>;

// strings

class xmlString {
public:
    xmlString(const char *string): _string{xmlCharStrdup(string)} {
    }

    xmlString(xmlChar *string): _string{string} {
    }

    explicit operator bool() const {
        return bool(_string);
    }

    const char *c_str() const {
        return reinterpret_cast<const char *>(_string.get());
    }

    const xmlChar *xml_str() const {
        return _string.get();
    }

    friend bool operator==(const xmlString &lhs, const xmlString &rhs);
    friend bool operator!=(const xmlString &lhs, const xmlString &rhs);

    bool contains(const xmlString &substring) {
        return xmlStrstr(_string.get(), substring._string.get());
    }

private:
    xmlResource<xmlChar> _string;

    friend class xmlString_view;
};

class xmlString_view {
public:
    xmlString_view(const xmlChar *string): _stringView{string} {
    }

    xmlString_view(const xmlString &string): _stringView{string._string.get()} {
    }

    friend bool operator==(const xmlString_view &lhs, const xmlString_view &rhs);
    friend bool operator!=(const xmlString_view &lhs, const xmlString_view &rhs);

    bool contains(const xmlString_view substring) {
        return xmlStrstr(_stringView, substring._stringView);
    }

    const xmlChar *data() const {
        return _stringView;
    };

private:
    const xmlChar *_stringView;
};

inline bool operator==(const xmlString &lhs, const xmlString &rhs) {
    return xmlStrEqual(lhs._string.get(), rhs._string.get());
}

inline bool operator!=(const xmlString &lhs, const xmlString &rhs) {
    return !(lhs == rhs);
}

inline bool operator==(const xmlString_view &lhs, const xmlString_view &rhs) {
    return xmlStrEqual(lhs._stringView, rhs._stringView);
}

inline bool operator!=(const xmlString_view &lhs, const xmlString_view &rhs) {
    return !(lhs == rhs);
}

// xpath
namespace xpath {

class Node {
    public:
    Node(xmlNodePtr node, xmlXPathContextPtr context);
    Node(xmlNodeSetPtr node, xmlXPathContextPtr context);
        
    xmlWeak<xmlNode> children() const;
    xmlString property(xmlString_view propertyName) const;
        
    template<typename T>
    std::unique_ptr<T> eval(xmlString_view xpath) const;
        
    private:

    xmlResource<xmlNodeSet, xmlNodeDeleter> _node;
    xmlWeak<xmlXPathContext> _context;
};

template <typename T>
struct xpath_traits {
    static constexpr xmlXPathObjectType value = XPATH_UNDEFINED;
};

template <>
struct xpath_traits<float> {
    static constexpr xmlXPathObjectType value = XPATH_NUMBER;
};

template <>
struct xpath_traits<bool> {
    static constexpr xmlXPathObjectType value = XPATH_BOOLEAN;
};

template <>
struct xpath_traits<xmlString> {
    static constexpr xmlXPathObjectType value = XPATH_STRING;
};

template <>
struct xpath_traits<Node> {
    static constexpr xmlXPathObjectType value = XPATH_NODESET;
};

inline Node::Node(xmlNodePtr node, xmlXPathContextPtr context) : _node{nullptr}, _context{context} {
    _node.reset(xmlXPathNodeSetCreate(node));
}

inline Node::Node(xmlNodeSetPtr node, xmlXPathContextPtr context)
    : _node{node}, _context{context} {
}

inline xmlWeak<xmlNode> Node::children() const {
    return xmlWeak<xmlNode>(_node.get()->nodeTab[0]->children);
}

inline xmlString Node::property(xmlString_view propertyName) const {
    return xmlGetProp(_node.get()->nodeTab[0], propertyName.data());
}

template<typename T>
std::unique_ptr<T> Node::eval(xmlString_view xpath) const {
    // note: this WON'T free the nodeset (if present)
    xmlResource<xmlXPathObject> object = xmlXPathNodeEval(_node.get()->nodeTab[0], xpath.data(), _context.get());
    if (!object) {
        return nullptr;
    }

    static constexpr const xmlXPathObjectType kRequiredValue = xpath_traits<T>::value;
    const xmlXPathObjectType heldValue = object->type;
    if (heldValue != kRequiredValue) {
        return nullptr;
    }

    if constexpr (kRequiredValue == XPATH_NODESET) {
        static_assert(std::is_same_v<T, Node>);
        if (!object->nodesetval->nodeNr) {
            return nullptr;
        }

        return std::make_unique<Node>(object->nodesetval, _context.get());
    } else if constexpr (kRequiredValue == XPATH_STRING) {
        static_assert(std::is_same_v<T, xmlString>);
        return std::make_unique<xmlString>(object->stringval);
    } else if constexpr (kRequiredValue == XPATH_NUMBER) {
        static_assert(std::is_arithmetic_v<T>);
        return std::make_unique<T>(object->floatval);
    } else if constexpr (kRequiredValue == XPATH_BOOLEAN) {
        static_assert(std::is_same_v<T, bool>);
        return std::make_unique<T>(object->boolval);
    } else {
        return nullptr;
    }
}

template<xmlXPathObjectType valueType>
bool xmlHoldsAlternative(xmlXPathObjectPtr xPathObject) {
    if (!xPathObject || valueType != xPathObject->type) {
        return false;
    }

    switch (xPathObject->type) {
    case XPATH_NODESET :
        return xPathObject->nodesetval && xPathObject->nodesetval->nodeNr && xPathObject->nodesetval->nodeTab;
    case XPATH_STRING :
        return xPathObject->stringval;
    case XPATH_BOOLEAN :
        return true;
    case XPATH_NUMBER :
        return true;
    case XPATH_USERS :
        [[fallthrough]];
    case XPATH_XSLT_TREE :
        [[fallthrough]];
    case XPATH_UNDEFINED :
        [[fallthrough]];
    default :
        break;
    }

    return false;
}

}  // namespace xpath

} // namespace xml2

#endif // COBALT_RENDER_XML_UTILITIES_H
