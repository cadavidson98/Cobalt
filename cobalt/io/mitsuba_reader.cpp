#include "mitsuba_reader.h"

#include "core/logging.h"
#include "core/string_utilities.h"
#include "math/math_utilities.h"
#include "private/xml_utilities.h"

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xpath.h>

#include "rgb2spec/rgb2spec.h"

#include <cassert>
#include <optional>

namespace cblt::io::mitsuba {

namespace {

struct Schema {
    // core scene primitives
    const xml2::xmlString sensorType = "sensor";
    const xml2::xmlString shapeType = "shape";
    const xml2::xmlString emitterType = "emitter";
    const xml2::xmlString bsdfType = "bsdf";

    // common
    const xml2::xmlString typeName = "type";
    const xml2::xmlString fileName = ".//string/@value";
    const xml2::xmlString valueName = "value";
    const xml2::xmlString floatName = "float";
    const xml2::xmlString stringName = "string";
    const xml2::xmlString idName = "id";

    // transform
    const xml2::xmlString transformExpression = ".//transform";
    const xml2::xmlString translateExpression = "translate";
    const xml2::xmlString rotateExpression = "rotate";
    const xml2::xmlString scaleExpression = "scale";
    const xml2::xmlString matrixName = "matrix";
    const xml2::xmlString xName = "x";
    const xml2::xmlString yName = "y";
    const xml2::xmlString zName = "z";
    const xml2::xmlString angleName = "angle";

    // camera
    const xml2::xmlString fovName = ".//float[@name=\"fov\"]";

    // emitter
    const xml2::xmlString environmentMapName = "envmap";

    // material
    const xml2::xmlString diffuseName = "diffuse";
    const xml2::xmlString dieletricName = "dielectric";
    const xml2::xmlString roughDielectricName = "roughdielectric";
    const xml2::xmlString thinDielectricName = "thindielectric";

    const xml2::xmlString spectrumName = "spectrum";
    const xml2::xmlString textureName = "texture";
    const xml2::xmlString rgbName = "rgb";

    const xml2::xmlString reflectanceName = ".//reflectance";
    const xml2::xmlString specularTransmittanceName = ".//specular_transmittance";
    const xml2::xmlString diffuseReflectanceName = ".//diffuse_reflectance";
    const xml2::xmlString specularReflectanceName = ".//specular_reflectance";

    const xml2::xmlString interiorIORName = ".//int_ior";
    const xml2::xmlString exteriorIORName = ".//ext_ior";
    const xml2::xmlString roughnessName = ".//float[@name=\"alpha\"]";
    const xml2::xmlString roughnessXName = ".//float[@name=\"alpha_u\"]";
    const xml2::xmlString roughnessYName = ".//float[@name=\"alpha_v\"]";

    // sphape
    const xml2::xmlString sphereName = "sphere";

    // sphere
    const xml2::xmlString radiusName = ".//float[@name=\"radius\"]";
    const xml2::xmlString centerName = ".//point[@name=\"center\"]";

    // mesh
    const xml2::xmlString objTypeName = "obj";
};

std::optional<float> loadFloat(xmlNodePtr node, const Schema &schema) {
    if (node->name != schema.floatName) {
        return std::nullopt;
    }

    const xml2::xmlString valueProperty = xmlGetProp(node, schema.valueName.xml_str());
    return float(std::stof(valueProperty.c_str()));
};

mat4f loadTransform(xmlNodePtr transformNode, const Schema &schema) {
    // need to iterate IN ORDER to properly compose transforms
    xmlNodePtr transformChildren = transformNode->children;
    xmlNodePtr iterator = transformChildren;

    mat4f transform(1.f);

    while (iterator != NULL) {
        xml2::xmlString valueProperty = xmlGetProp(iterator, schema.valueName.xml_str());
        xml2::xmlString_view transformType = iterator->name;
        const char *value = valueProperty.c_str();
        if (transformType == schema.translateExpression) {
            const std::vector<float> translationValues = core::split<float>(value, ' ');
            const vec3f translation(translationValues[0], translationValues[1], translationValues[2]);
            const mat4f translationMatrix = cblt::utils::translationMatrix(translation);
            transform = translationMatrix * transform;
        } else if (transformType == schema.rotateExpression) {
            // rotations can be expressed using an arbitrary vector ("value")
            // or as an "x", "y", or "z" attribute
            vec3f axis(0.f, 0.f, 0.f);
            float angle = 0.f;
            xmlAttrPtr rotateIterator = NULL;
            for (rotateIterator = iterator->properties; rotateIterator; rotateIterator = rotateIterator->next) {
                xml2::xmlString_view rotationAxis = rotateIterator->name;
                if (rotationAxis == schema.xName) {
                    xmlChar *xString = rotateIterator->children->content;
                    axis.x = std::stof(reinterpret_cast<char *>(xString));
                } else if (rotationAxis == schema.yName) {
                    xmlChar *yString = rotateIterator->children->content;
                    axis.y = std::stof(reinterpret_cast<char *>(yString));
                } else if (rotationAxis == schema.zName) {
                    xmlChar *zString = rotateIterator->children->content;
                    axis.z = std::stof(reinterpret_cast<char *>(zString));
                } else if (rotationAxis == schema.valueName) {
                    const std::vector<float> rotationValues = core::split<float>(value, ' ');
                    assert(rotationValues.size() >= 3);
                    axis = vec3f(rotationValues[0], rotationValues[1], rotationValues[2]);
                } else if (rotationAxis == schema.angleName) {
                    xmlChar *angleString = rotateIterator->children->content;
                    angle = cblt::utils::toRadians(std::stof(reinterpret_cast<char *>(angleString)));
                }
            }
            const mat4f rotationMatrix = cblt::utils::rotationMatrix(axis, angle);
            transform = rotationMatrix * transform;
        } else if (transformType == schema.scaleExpression) {
            const std::vector<float> scaleValues = core::split<float>(value, ' ');
            const vec3f scale(scaleValues[0], scaleValues[1], scaleValues[1]);
            const mat4f scaleMatrix = cblt::utils::scaleMatrix(scale);
            transform = scaleMatrix * transform;
        } else if (transformType == schema.matrixName) {
            const std::vector<float> matrixValues = core::split<float>(value, ' ');
            if (matrixValues.size() == 16) {
                // 4x4
                const mat4f matrix{
                    vec4f{matrixValues[0], matrixValues[4],  matrixValues[8], matrixValues[12]},
                    vec4f{matrixValues[1], matrixValues[5],  matrixValues[9], matrixValues[13]},
                    vec4f{matrixValues[2], matrixValues[6], matrixValues[10], matrixValues[14]},
                    vec4f{matrixValues[3], matrixValues[7], matrixValues[11], matrixValues[15]},
                };
                transform = matrix * transform;
            } else if (matrixValues.size() == 9) {
                // 3x3
                const mat4f matrix{
                    vec4f{matrixValues[0], matrixValues[3], matrixValues[6], 0.f},
                    vec4f{matrixValues[1], matrixValues[4], matrixValues[7], 0.f},
                    vec4f{matrixValues[2], matrixValues[5], matrixValues[8], 0.f},
                    vec4f{            0.f,             0.f,             0.f, 1.f},
                };
                transform = matrix * transform;
            } else {
                // error
                CoLogError("Invalid arguments for 'matrix' transform");
            }
        }
        iterator = iterator->next;
    }
    return transform;
}

Spectrum loadBSDF(xmlNodePtr bsdfNode, xmlXPathContextPtr context, const Schema &schema) {
    const xml2::xmlString type = xmlGetProp(bsdfNode, schema.typeName.xml_str());
    if (type == "diffuse") {
        xml2::xmlResource<xmlXPathObject> spectrum = xmlXPathNodeEval(bsdfNode, schema.rgbName.xml_str(), context);

        if (xml2::xmlHoldsAlternative<XPATH_NODESET>(spectrum.get())) {
            xml2::xmlString valueString = xmlGetProp(*spectrum->nodesetval->nodeTab, schema.valueName.xml_str());
            const std::vector<float> rgb = core::split<float>(valueString.c_str(), ' ');
            assert(rgb.size() == 3);
            static RGB2Spec *sRGBModel = nullptr;
            if (!sRGBModel) {
                sRGBModel = rgb2spec_load(RGB2SPEC_COLOR_SRGB);
                assert(sRGBModel);
            }

            Spectrum spectrum;

            rgb2spec_fetch(sRGBModel, const_cast<float *>(rgb.data()), spectrum.coefficients.data());

            return spectrum;
        }
    }

    return Spectrum{};
}

std::optional<Camera> loadCamera(xmlNodePtr cameraNode, xmlXPathContextPtr context, const Schema &schema) {
    xml2::xmlResource<xmlXPathObject> transform(
        xmlXPathNodeEval(cameraNode, schema.transformExpression.xml_str(), context)
    );
    if (!transform || !xml2::xmlHoldsAlternative<XPATH_NODESET>(transform.get())) {
        CoLogError("Missing transform for Camera");
        return std::nullopt;
    }

    const mat4f cameraTransform = loadTransform(*transform->nodesetval->nodeTab, schema);

    xml2::xmlResource<xmlXPathObject> fovPath(xmlXPathNodeEval(cameraNode, schema.fovName.xml_str(), context));
    if (!fovPath || !xml2::xmlHoldsAlternative<XPATH_NODESET>(fovPath.get())) {
        CoLogError("Missing Field of View for camera");
        return std::nullopt;
    }

    std::optional<float> fovValue = loadFloat(*fovPath->nodesetval->nodeTab, schema);
    if (!fovValue) {
        return std::nullopt;
    }

    const float cameraFov = cblt::utils::toRadians(*fovValue);
    return Camera{
        .fov = cameraFov,
        .transform = cameraTransform,
    };
}

std::optional<Sphere> loadSphere(xmlNodePtr sphereNode, xmlXPathContextPtr context, const Schema &schema) {
    xml2::xmlResource<xmlXPathObject> radius = xmlXPathNodeEval(sphereNode, schema.radiusName.xml_str(), context);
    xml2::xmlResource<xmlXPathObject> center = xmlXPathNodeEval(sphereNode, schema.centerName.xml_str(), context);

    static constexpr float kDefaultRadius = 1.f;
    static constexpr vec3f kDefaultCenter = {0.f, 0.f, 0.f};

    float sphereRadius = kDefaultRadius;
    vec3f sphereCenter = kDefaultCenter;

    if (radius && xml2::xmlHoldsAlternative<XPATH_NODESET>(radius.get())) {
        sphereRadius = loadFloat(*radius->nodesetval->nodeTab, schema).value_or(kDefaultRadius);
    }

    if (center && xml2::xmlHoldsAlternative<XPATH_NODESET>(center.get())) {
        const xml2::xmlString valueProperty = xmlGetProp(*center->nodesetval->nodeTab, schema.valueName.xml_str());
        const std::vector<float> centerValues = core::split<float>(valueProperty.c_str(), ' ');
        if (centerValues.size() != 3) {
            CoLogError("Invalid value '%s' for attribute 'point'", valueProperty.c_str());
            return std::nullopt;
        }

        sphereCenter = {
            .x = centerValues[0],
            .y = centerValues[1],
            .z = centerValues[2],
        };
    }

    return Sphere{
        .center = sphereCenter,
        .radius = sphereRadius,
    };
}

std::optional<Mesh> loadMesh(xmlNodePtr meshNode, xmlXPathContextPtr context, const Schema &schema) {
    xml2::xmlString meshType = xmlGetProp(meshNode, schema.typeName.xml_str());
    if (!meshType || meshType != schema.objTypeName) {
        CoLogError("Unsupported Mesh type '%s'", meshType.c_str());
        return std::nullopt;
    }

    xml2::xmlResource<xmlXPathObject> fileNode(xmlXPathNodeEval(meshNode, schema.fileName.xml_str(), context));
    xmlChar *fileNameString(xmlXPathCastToString(fileNode.get()));
    if (!fileNameString) {
        CoLogError("Missing Filename for mesh");
        return std::nullopt;
    }

    std::string meshFileName = reinterpret_cast<char *>(fileNameString);
    return Mesh{
        .fileName = meshFileName,
        .fileExtension = meshType.c_str(),
    };
}

template<typename ShapeType, typename LoadShapeFunctor>
std::optional<Shape<ShapeType>> loadTypedShape(
    xmlNodePtr shapeNode,
    xmlXPathContextPtr context,
    const Schema &schema,
    LoadShapeFunctor functor
) {
    mat4f shapeToWorld(1.f);
    Spectrum spectrum;

    xml2::xmlResource<xmlXPathObject> transform =
        xmlXPathNodeEval(shapeNode, schema.transformExpression.xml_str(), context);

    if (xml2::xmlHoldsAlternative<XPATH_NODESET>(transform.get())) {
        xmlNodeSetPtr transformNode = transform->nodesetval;
        shapeToWorld = loadTransform(*transformNode->nodeTab, schema);
    }

    xml2::xmlResource<xmlXPathObject> bsdf = xmlXPathNodeEval(shapeNode, schema.bsdfType.xml_str(), context);
    if (xml2::xmlHoldsAlternative<XPATH_NODESET>(bsdf.get())) {
        xmlNodeSetPtr bsdfNode = bsdf->nodesetval;
        spectrum = loadBSDF(*bsdfNode->nodeTab, context, schema);
    }

    std::optional<ShapeType> shape = functor(shapeNode, context, schema);
    if (!shape) {
        return std::nullopt;
    }

    return Shape<ShapeType>{
        .shape = std::move(*shape),
        .transform = shapeToWorld,
        .spectrum = spectrum,
    };
}

} // anonymous namespace

/// entrypoint

bool read(const std::string_view fileName, std::shared_ptr<FileReaderDelegate> delegate) {
    Schema schema;
    xml2::TextReader mitsubaReader = xmlNewTextReaderFilename(fileName.data());

    if (!mitsubaReader) {
        CoLogError("XML Parsing error occured while reading %s", fileName.data());
        return false;
    }

    while (xmlTextReaderRead(mitsubaReader.get()) > 0) {
        const int rawType = xmlTextReaderNodeType(mitsubaReader.get());
        if (rawType == -1) {
            break;
        }

        const xmlReaderTypes type = static_cast<xmlReaderTypes>(rawType);
        if (type == XML_READER_TYPE_ELEMENT) {
            xml2::xmlString name = xmlTextReaderName(mitsubaReader.get());
            if (name == schema.sensorType) {
                xmlNodePtr cameraNode = xmlTextReaderExpand(mitsubaReader.get());

                xml2::xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(cameraNode->doc);

                const std::optional<Camera> camera = loadCamera(cameraNode, xpathContext.get(), schema);
                if (!camera || !delegate->readSensor(*camera)) {
                    return false;
                }
            } else if (name == schema.emitterType) {
                xmlNodePtr emitterNode = xmlTextReaderExpand(mitsubaReader.get());

                xml2::xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(emitterNode->doc);

                xml2::xmlString emitterType = xmlGetProp(emitterNode, schema.typeName.xml_str());
                if (emitterType && emitterType == schema.environmentMapName) {
                    xmlXPathObjectPtr fileNameNode =
                        xmlXPathNodeEval(emitterNode, schema.fileName.xml_str(), xpathContext.get());
                    xml2::xmlString fileName = xmlXPathCastToString(fileNameNode);
                    if (!fileName) {
                        CoLogError("Missing Filename string");
                        return false;
                    }

                    const Emitter emitter = {
                        .emissionMap = {
                                        .fileName = fileName.c_str(),
                                        .fileExtension = cblt::core::fileExtension(fileName.c_str()),
                                        },
                    };

                    if (!delegate->readEmitter(emitter)) {
                        return false;
                    }
                }
            } else if (name == schema.shapeType) {
                xmlNodePtr shapeNode = xmlTextReaderExpand(mitsubaReader.get());

                xml2::xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(shapeNode->doc);
                const xml2::xmlString typeProperty = xmlGetProp(shapeNode, schema.typeName.xml_str());
                if (typeProperty == schema.objTypeName) {
                    const std::optional<Shape<Mesh>> mesh =
                        loadTypedShape<Mesh>(shapeNode, xpathContext.get(), schema, loadMesh);

                    if (!mesh || !delegate->readMesh(*mesh)) {
                        return false;
                    }
                } else if (typeProperty == schema.sphereName) {
                    const std::optional<Shape<Sphere>> sphere =
                        loadTypedShape<Sphere>(shapeNode, xpathContext.get(), schema, loadSphere);

                    if (!sphere || !delegate->readSphere(*sphere)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace cblt::io::mitsuba
