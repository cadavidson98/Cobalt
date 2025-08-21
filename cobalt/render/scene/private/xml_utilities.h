#ifndef CBLT_RENDER_XML_UTILITIES_H
#define CBLT_RENDER_XML_UTILITIES_H

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xpath.h>

#include <cassert>

namespace xml2 {

struct xmlDeleter {
    void operator()(void *ptr) {
        xmlFree(ptr);
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

template<xmlXPathObjectType valueType>
bool xmlHoldsAlternative(xmlXPathObjectPtr xPathObject) {
    if (!xPathObject || valueType != xPathObject->type) {
        return false;
    }

    switch (xPathObject->type) {
    case XPATH_NODESET :
        return xPathObject->nodesetval && xPathObject->nodesetval->nodeNr && xPathObject->nodesetval->nodeTab;
    case XPATH_STRING : return xPathObject->stringval;
    case XPATH_BOOLEAN : [[fallthrough]];
    case XPATH_NUMBER : return true;
    default : break;
    }

    return false;
}

using TextReader = xmlResource<xmlTextReader, xmlTextReaderDeleter>;

} // namespace xml2

#endif // CBLT_RENDER_XML_UTILITIES_H
