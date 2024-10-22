#include "xml_parser.h"

#include "logging.h"
#include "string_utils.h"

#define PUGIXML_NO_EXCEPTIONS
#include "pugixml.hpp"

#include <cstring>
#include <optional>
#include <string>

CBLT_DEFINE_LOG(CoLogXML);

namespace cblt::utils {

class CoXMLElementImpl {
    public:
        CoXMLElementImpl() = default;
        CoXMLElementImpl(pugi::xml_node element) {
            _xmlElement = element;
        }

        std::optional<int> intValue() const {
            const char *childValue = _xmlElement.value();
            if (std::strcmp(childValue, "") == 0) {
                return std::nullopt;
            }

            return int(std::atoi(childValue));
        }

        std::optional<float> floatValue() const {
            const char *childValue = _xmlElement.value();
            if (std::strcmp(childValue, "") == 0) {
                return std::nullopt;
            }

            return float(std::atof(childValue));
        }

        std::optional<std::string> stringValue() const {
            const char *childValue = _xmlElement.value();
            if (std::strcmp(childValue, "") == 0) {
                return std::nullopt;
            }

            return std::string(childValue);
        }

        // user defined types
        std::optional<CoXMLElement> elementValue(const std::string &elementName) const {
            pugi::xml_node childElement = _xmlElement.child(elementName.c_str());
            if (childElement.empty()) {
                return std::nullopt;
            }

            CoXMLElement element;
            element._impl->_xmlElement = childElement;

            return element;
        }

        CoXMLElement nextElement() {
            CoXMLElement element;
            element._impl->_xmlElement = _xmlElement.next_sibling();
            return element;
        }

    private:
        pugi::xml_node _xmlElement;
};

CoXMLElement::CoXMLElement()
    : _impl(new CoXMLElementImpl) {

      };

CoXMLElement::CoXMLElement(std::shared_ptr<CoXMLElementImpl> impl): _impl(impl) {
}

// "leaf node"
std::optional<int> CoXMLElement::intValue() const {
    return _impl->intValue();
}

std::optional<float> CoXMLElement::floatValue() const {
    return _impl->floatValue();
}

std::optional<std::string> CoXMLElement::stringValue() const {
    return _impl->stringValue();
}

std::optional<CoXMLElement> CoXMLElement::elementValue(const std::string &elementName) const {
    return _impl->elementValue(elementName);
}

CoXMLElement CoXMLElement::nextElement() {
    return _impl->nextElement();
}

class CoXMLImpl {
    public:
        CoXMLImpl();

        bool init(std::string xmlDOM);

        bool hasElement(std::string attributeName) const;
        CoXMLElement root() const;

    private:
        pugi::xml_document _xmlDOM;
}; // class CoXMLImpl

CoXMLImpl::CoXMLImpl() {
}

bool CoXMLImpl::init(std::string xmlDOM) {
    pugi::xml_parse_result result = _xmlDOM.load_string(xmlDOM.c_str());

    if (!result) {
        CoLogError(CoLogXML) << result.description() << std::endl;
        return false;
    }

    return true;
}

bool CoXMLImpl::hasElement(std::string attributeName) const {
    pugi::xml_node element = _xmlDOM.child(attributeName.c_str());
    return !element.empty();
}

CoXMLElement CoXMLImpl::root() const {
    std::shared_ptr<CoXMLElementImpl> rootElement(new CoXMLElementImpl(_xmlDOM.document_element()));
    return CoXMLElement(rootElement);
};

std::shared_ptr<CoXML> CoXML::create(std::string xmlDOM) {
    std::shared_ptr<CoXML> xml = std::shared_ptr<CoXML>(new CoXML);
    if (!xml->_impl->init(xmlDOM)) {
        return nullptr;
    }

    return xml;
}

CoXML::CoXML() {
    _impl = std::make_shared<CoXMLImpl>();
}

CoXML::~CoXML() {
}

bool CoXML::hasElement(std::string attributeName) const {
    return _impl->hasElement(attributeName);
}

CoXMLElement CoXML::root() const {
    return _impl->root();
}

} // namespace cblt::utils
