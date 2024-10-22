#include "utilities/xml_parser.h"

#include <cstring>
#include <gtest/gtest.h>
#include <iostream>

TEST(CobaltCoreUtilitiesTests, TestXMLParser) {
    static constexpr const char *kXMLString =
        "<note>"
        "<to>Tove</to>"
        "<from>Jani</from>"
        "<heading>Reminder</heading>"
        "<body>Don't forget me this weekend!</body>"
        "</note>";

    std::shared_ptr<cblt::utils::CoXML> xml = cblt::utils::CoXML::create(std::string(kXMLString));
    EXPECT_TRUE(xml != nullptr);
    EXPECT_TRUE(xml->hasElement("note"));
    cblt::utils::CoXMLElement noteElement = xml->root();
}

TEST(CobaltCoreUtilitiesTests, TestParsePrimitives) {

    {
        static constexpr const char *kXMLString = "<root><value>a string</value></root>";

        std::shared_ptr<cblt::utils::CoXML> xml = cblt::utils::CoXML::create(std::string(kXMLString));
        EXPECT_TRUE(xml != nullptr);
        EXPECT_TRUE(xml->hasElement("root"));
        cblt::utils::CoXMLElement noteElement = xml->root();
        std::optional<std::string> noteValue = noteElement.stringValue();
        EXPECT_TRUE(noteValue.has_value());
        EXPECT_TRUE(noteValue.value() == "a string");
    }
    {
        static constexpr const char *kXMLString = "<root><value>1</value></root>";

        std::shared_ptr<cblt::utils::CoXML> xml = cblt::utils::CoXML::create(std::string(kXMLString));
        EXPECT_TRUE(xml != nullptr);
        EXPECT_TRUE(xml->hasElement("root"));
        cblt::utils::CoXMLElement noteElement = xml->root();
        std::optional<int> noteValue = noteElement.intValue();
        EXPECT_TRUE(noteValue.has_value());
        EXPECT_TRUE(noteValue.value() == 1);
    }
    {
        static constexpr const char *kXMLString = "<root><value>2.5</value></root>";

        std::shared_ptr<cblt::utils::CoXML> xml = cblt::utils::CoXML::create(std::string(kXMLString));
        EXPECT_TRUE(xml != nullptr);
        EXPECT_TRUE(xml->hasElement("root"));
        cblt::utils::CoXMLElement noteElement = xml->root();
        std::optional<float> noteValue = noteElement.floatValue();
        EXPECT_TRUE(noteValue.has_value());
        EXPECT_TRUE(noteValue.value() == 2.5f);
    }
}
