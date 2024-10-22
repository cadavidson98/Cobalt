#ifndef CBLT_UTILS_XML_PARSER_H
#define CBLT_UTILS_XML_PARSER_H

#include <memory>
#include <optional>
#include <string>

namespace cblt::utils {

class CoXMLImpl;
class CoXMLElementImpl;

class CoXMLElement {
    public:
        // "leaf node"
        std::optional<int> intValue() const;
        std::optional<float> floatValue() const;
        std::optional<std::string> stringValue() const;
        // "object node"
        std::optional<CoXMLElement> elementValue(const std::string &elementName) const;

        CoXMLElement nextElement();

    private:
        CoXMLElement();
        CoXMLElement(std::shared_ptr<CoXMLElementImpl>);

        std::shared_ptr<CoXMLElementImpl> _impl;

        friend class CoXMLImpl;
        friend class CoXMLElementImpl;
};

class CoXML {

    public:
        ~CoXML();

        static std::shared_ptr<CoXML> create(std::string xmlDOM);

        bool hasElement(std::string attribute) const;
        CoXMLElement root() const;

    private:
        CoXML();

        std::shared_ptr<CoXMLImpl> _impl;
};

} // namespace cblt::utils

#endif // CBLT_UTILS_XML_PARSER_H
