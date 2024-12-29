#include "mitsuba_builder.h"

#include "string_utils.h"
#include "texture.h"
#include "xml_parser.h"

#include <algorithm>

namespace cblt::render {

CoMitsubaBuilder::CoMitsubaBuilder(const CoSceneBuilder::CreateInfo &createInfo) {
    _mitsubaXML = createInfo.parser;
}

bool CoMitsubaBuilder::buildCameras() {
    utils::CoXMLElement sceneRoot = _mitsubaXML->root();
    std::optional<utils::CoXMLElement> cameraNode = sceneRoot.elementValue("sensor");
    if (!cameraNode) {
        return false;
    }

    std::optional<utils::CoXMLElement> fovNode = cameraNode->elementValue("fov");
    std::optional<utils::CoXMLElement> fovAxisNode = cameraNode->elementValue("fov_axis");
    std::optional<utils::CoXMLElement> nearNode = cameraNode->elementValue("near_clip");
    std::optional<utils::CoXMLElement> farNode = cameraNode->elementValue("far_clip");
    std::optional<utils::CoXMLElement> transformNode = cameraNode->elementValue("to_world");

    if (!fovNode || !fovAxisNode || !nearNode || !farNode || !transformNode) {
        return false;
    }

    float fov = fovNode->floatValue().value_or(1.f);
    float nearPlane = nearNode->floatValue().value_or(1.f);
    float farPlane = farNode->floatValue().value_or(1.f);
    simd::mat4f transform = _buildTransform(transformNode.value());

    return true;
}

bool CoMitsubaBuilder::buildLights() {
    utils::CoXMLElement sceneRoot = _mitsubaXML->root();
    std::optional<utils::CoXMLElement> lightsNode = sceneRoot.elementValue("emitters");

    if (!lightsNode) {
        return false;
    }

    for (const utils::CoXMLElement &lightNode : lightsNode->children()) {
        std::string lightType = lightNode.stringValue("type");
        if (lightType == "envmap") {
            // load environment map
            std::string filePath = lightNode.stringValue("filename");
            std::string fileExtension = utils::extension(filePath);
            std::shared_ptr<CoTexture> environmentMap = CoTexture::Create({
                .fileName = filePath,
                .fileExtension = fileExtension,
            });
            // todo: push this back somewhere?
            if (!environmentMap) {
                return false;
            }
        }
    }
    return true;
}

simd::mat4f CoMitsubaBuilder::_buildTransform(const utils::CoXMLElement &transformElement) const {
    std::optional<utils::CoXMLElement> matrixNode = transformElement.elementValue("matrix");
    if (matrixNode) {
        // load from matrix directly; this is a column major array of floats
        static const std::string kIdentityString = "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
        std::vector<std::string> stringValues = utils::split(matrixNode->stringValue().value_or(kIdentityString), ' ');
        std::vector<float> floatValues(stringValues.size());
        std::transform(
            stringValues.begin(),
            stringValues.end(),
            floatValues.begin(),
            floatValues.end(),
            [](const std::string &string) {
                return std::stof(string);
            }
        );

        assert(floatValues.size() == 16);

        return simd::mat4f(floatValues.data());
    }
    // need
    // to
    // load
    // rotation,
    // scale,
    // and
    // translation
    // separately
    auto xRotationNode = transformElement.elementValue("x");
    auto yRotationNode = transformElement.elementValue("y");
    auto zRotationNode = transformElement.elementValue("z");
    auto translationNode = transformElement.elementValue("translate");
    auto scaleNode = transformElement.elementValue("scale");
}

} // namespace
  // cblt::render
