#include "mitsuba.h"

#include "core/logging.h"
#include "core/string_utilities.h"
#include "math/math_utilities.h"
#include "private/xml_utilities.h"

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xpath.h>

#include <cassert>
#include <memory>
#include <optional>

namespace cobalt::io::mitsuba {

namespace {

struct Schema {
    // core scene primitives
    const xml2::xmlString sensorType = "sensor";
    const xml2::xmlString shapeType = "shape";
    const xml2::xmlString emitterType = "emitter";
    const xml2::xmlString bsdfType = "bsdf";

    // common
    const xml2::xmlString typeName = "type";
    const xml2::xmlString fileName = "string(.//string[@name=\"filename\"]/@value)";
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
    const xml2::xmlString fovName = "number(.//float[@name=\"fov\"]/@value)";

    // emitter
    const xml2::xmlString areaTypeName = "area";
    const xml2::xmlString environmentMapName = "envmap";

    // blackbody
    const xml2::xmlString minWavelengthName = "number(.//float[@name=\"minwavelength\"]/@value)";
    const xml2::xmlString maxWavelengthName = "number(.//float[@name=\"maxwavelength\"]/@value)";
    const xml2::xmlString temperatureName = "number(.//float[@name=\"temperature\"]/@value)";

    // material
    const xml2::xmlString diffuseName = "diffuse";
    const xml2::xmlString dieletricName = "dielectric";
    const xml2::xmlString roughDielectricName = "roughdielectric";
    const xml2::xmlString thinDielectricName = "thindielectric";

    // spectrum
    const xml2::xmlString spectrumName = "spectrum";
    const xml2::xmlString rgbName = "rgb";
    const xml2::xmlString blackBodyName = "blackbody";    

    const xml2::xmlString textureName = "texture";

    const xml2::xmlString reflectanceName = ".//reflectance";
    const xml2::xmlString specularTransmittanceName = ".//specular_transmittance";
    const xml2::xmlString diffuseReflectanceName = ".//diffuse_reflectance";
    const xml2::xmlString specularReflectanceName = ".//specular_reflectance";

    const xml2::xmlString interiorIORName = ".//int_ior";
    const xml2::xmlString exteriorIORName = ".//ext_ior";
    const xml2::xmlString roughnessName = ".//float[@name=\"alpha\"]";
    const xml2::xmlString roughnessXName = ".//float[@name=\"alpha_u\"]";
    const xml2::xmlString roughnessYName = ".//float[@name=\"alpha_v\"]";

    // shape
    const xml2::xmlString sphereName = "sphere";

    // sphere
    const xml2::xmlString radiusName = "number(.//float[@name=\"radius\"]/@value)";
    const xml2::xmlString centerName = ".//point[@name=\"center\"]";

    // mesh
    const xml2::xmlString objTypeName = "obj";
};

std::optional<Spectrum> loadSpectrum(const xml2::xpath::Node &spectrumNode, const Schema &schema) {
    const xml2::xmlString typeString = spectrumNode.property(schema.typeName);
    if (typeString == schema.rgbName) {
        const xml2::xmlString valueString = spectrumNode.property(schema.valueName);
        const std::vector<float> rgb = core::split<float>(valueString.c_str(), ',');
        if (rgb.size() != 3) {
            CoLogError("spectrum count Mismatch in 'rgb': expected '3', got '%zu'", rgb.size());
            return std::nullopt;
        }

        return RGB {
            .r = rgb[0],
            .g = rgb[1],
            .b = rgb[2],
        };
    } else if (typeString == schema.blackBodyName) {
        // todo: I think xpath supports querying specifically for an attribute, which we could then directly cast?
        std::unique_ptr<float> minWavelength = spectrumNode.eval<float>(schema.minWavelengthName);
        std::unique_ptr<float> maxWavelength = spectrumNode.eval<float>(schema.maxWavelengthName);
        std::unique_ptr<float> temperature = spectrumNode.eval<float>(schema.temperatureName);

        if (!minWavelength || !maxWavelength || !temperature) {
            CoLogError("missing nodes for 'Blackbody'");
            return std::nullopt;
        }

        return BlackBody {
            .minWavelength = *minWavelength,
            .maxWavelength = *maxWavelength,
            .temperature = *temperature,
        };
    }

    CoLogError("Unsupported spectrum type '%s'", typeString.c_str());
    return std::nullopt;
}

std::optional<mat4f> loadTransform(const xml2::xpath::Node &transformNode, const Schema &schema) {
    // need to iterate IN ORDER to properly compose transforms
    xml2::xmlWeak<xmlNode> chidren = transformNode.children();
    xmlNodePtr iterator = chidren.get();

    mat4f transform(1.f);

    while (iterator != NULL) {
        xml2::xmlString valueProperty = xmlGetProp(iterator, schema.valueName.xml_str());
        xml2::xmlString_view transformType = iterator->name;
        const char *value = valueProperty.c_str();
        if (transformType == schema.translateExpression) {
            const std::vector<float> translationValues = core::split<float>(value, ' ');
            if (translationValues.size() != 3) {
                CoLogError("Mismatch: expected \"3\" elements in \"translation\", only found \"%zu\"", translationValues.size());
                return std::nullopt;
            }

            const vec3f translation(translationValues[0], translationValues[1], translationValues[2]);
            const mat4f translationMatrix = cobalt::utils::translationMatrix(translation);
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
                    angle = cobalt::utils::toRadians(std::stof(reinterpret_cast<char *>(angleString)));
                }
            }
            const mat4f rotationMatrix = cobalt::utils::rotationMatrix(axis, angle);
            transform = rotationMatrix * transform;
        } else if (transformType == schema.scaleExpression) {
            const std::vector<float> scaleValues = core::split<float>(value, ' ');
            if (scaleValues.size() != 3) {
                CoLogError("Mismatch: expected \"3\" elements in \"scale\", only found \"%zu\"", scaleValues.size());
                return std::nullopt;
            }

            const vec3f scale(scaleValues[0], scaleValues[1], scaleValues[1]);
            const mat4f scaleMatrix = cobalt::utils::scaleMatrix(scale);
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
                return std::nullopt;
            }
        }
        iterator = iterator->next;
    }
    return transform;
}

std::optional<Spectrum> loadBSDF(const xml2::xpath::Node &bsdfNode, const Schema &schema) {
    const xml2::xmlString type = bsdfNode.property(schema.typeName);
    if (type == "diffuse") {
        std::unique_ptr<xml2::xpath::Node> spectrum = bsdfNode.eval<xml2::xpath::Node>(schema.rgbName);
        if (!spectrum) {
            return std::nullopt;
        }

        xml2::xmlString valueString = spectrum->property(schema.valueName);
        const std::vector<float> rgb = core::split<float>(valueString.c_str(), ' ');
        if (rgb.size() != 3) {
            CoLogError("Mismatch: expected \"3\" elements in \"rgb\", only found \"%zu\"", rgb.size());
            return std::nullopt;
        }

        return RGB {
            .r = rgb[0],
            .g = rgb[1],
            .b = rgb[2],
        };
    }

    return std::nullopt;
}

std::optional<Emitter> loadEmitter(const xml2::xpath::Node &emitterNode, const Schema &schema) {
    xml2::xmlString emitterType = emitterNode.property(schema.typeName);
    if (!emitterType) {
        CoLogError("Emitter node missing attribute 'type'");
        return std::nullopt;
    }

    if (emitterType == schema.environmentMapName) {
        std::unique_ptr<xml2::xmlString> fileName = emitterNode.eval<xml2::xmlString>(schema.fileName);
        if (!fileName) {
            CoLogError("Missing Filename string");
            return std::nullopt;
        }

        return Emitter {
            .radiance = {},
            .emissionMap = {
                .fileName = fileName->c_str(),
                .fileExtension = cobalt::core::fileExtension(fileName->c_str()),
            },
        };
    } else if (emitterType == schema.areaTypeName) {
        std::unique_ptr<xml2::xpath::Node> spectrumNode = emitterNode.eval<xml2::xpath::Node>(schema.spectrumName);
        if (!spectrumNode) {
            CoLogError("Missing 'rgb' attribute");
            return std::nullopt;
        }

        std::optional<Spectrum> spectrum = loadSpectrum(*spectrumNode, schema);
        if (!spectrum) {
            return std::nullopt;
        }

        return Emitter {
            .radiance = *spectrum,
        }; 
    }

    CoLogError("Unsupported");
    return std::nullopt;
}

std::optional<Camera> loadCamera(const xml2::xpath::Node &cameraNode, const Schema &schema) {
    std::unique_ptr<xml2::xpath::Node> transform = cameraNode.eval<xml2::xpath::Node>(schema.transformExpression);
    if (!transform) {
        CoLogError("Missing transform for Camera");
        return std::nullopt;
    }

    std::optional<mat4f> cameraTransform = loadTransform(*transform, schema);
    if (!cameraTransform) {
        return std::nullopt;
    }

    std::unique_ptr<float> fov = cameraNode.eval<float>(schema.fovName); 
    if (!fov) {
        CoLogError("Missing Field of View for camera");
        return std::nullopt;
    }

    const float cameraFov = cobalt::utils::toRadians(*fov);
    return Camera{
        .fov = cameraFov,
        .transform = *cameraTransform,
    };
}

std::optional<Sphere> loadSphere(const xml2::xpath::Node &sphereNode, const Schema &schema) {
    static constexpr vec3f kDefaultCenter = {0.f, 0.f, 0.f};
    static constexpr float kDefaultRadius = 1.f;

    std::unique_ptr<float> radius = sphereNode.eval<float>(schema.radiusName);
    std::unique_ptr<xml2::xpath::Node> center = sphereNode.eval<xml2::xpath::Node>(schema.centerName);

    const float sphereRadius = radius ? *radius : kDefaultRadius;
    vec3f sphereCenter = kDefaultCenter;

    if (center) {
        const xml2::xmlString valueProperty = center->property(schema.valueName);
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

std::optional<Mesh> loadMesh(const xml2::xpath::Node &meshNode, const Schema &schema) {
    xml2::xmlString meshType = meshNode.property(schema.typeName);
    if (!meshType || meshType != schema.objTypeName) {
        CoLogError("Unsupported Mesh type '%s'", meshType.c_str());
        return std::nullopt;
    }

    std::unique_ptr<xml2::xmlString> fileName = meshNode.eval<xml2::xmlString>(schema.fileName);
    if (!fileName) {
        CoLogError("Missing Filename for mesh");
        return std::nullopt;
    }

    return Mesh{
        .fileName = fileName->c_str(),
        .fileExtension = meshType.c_str(),
    };
}

template<typename ShapeType, typename LoadShapeFunctor>
std::optional<Shape<ShapeType>> loadTypedShape(
    const xml2::xpath::Node &shapeNode,
    const Schema &schema,
    LoadShapeFunctor functor
) {
    mat4f shapeToWorld(1.f);
    Spectrum spectrum;

    std::unique_ptr<xml2::xpath::Node> transformNode = shapeNode.eval<xml2::xpath::Node>(schema.transformExpression);

    if (transformNode) {
        std::optional<mat4f> shapeTransform = loadTransform(*transformNode, schema);
        if (!shapeTransform) {
            return std::nullopt;
        }

        shapeToWorld = *shapeTransform;
    }

    std::unique_ptr<xml2::xpath::Node> bsdfNode = shapeNode.eval<xml2::xpath::Node>(schema.bsdfType);

    if (bsdfNode) {
        std::optional<Spectrum> shapeSpectrum = loadBSDF(*bsdfNode, schema);
        if (!shapeSpectrum) {
            return std::nullopt;
        }

        spectrum = *shapeSpectrum;
    }

    std::unique_ptr<xml2::xpath::Node> emitterNode = shapeNode.eval<xml2::xpath::Node>(schema.emitterType);
    if (emitterNode) {
        std::optional<Emitter> emitterSpectrum = loadEmitter(*emitterNode, schema);
        if (!emitterNode) {
            return std::nullopt;
        }

        spectrum = emitterSpectrum->radiance;
    }

    std::optional<ShapeType> shape = functor(shapeNode, schema);
    if (!shape) {
        return std::nullopt;
    }

    return Shape<ShapeType>{
        .shape = std::move(*shape),
        .spectrum = spectrum,
        .transform = shapeToWorld,
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

                xml2::xpath::Node node(cameraNode, xpathContext.get());

                const std::optional<Camera> camera = loadCamera(node, schema);

                if (!camera || !delegate->readSensor(*camera)) {
                    return false;
                }
            } else if (name == schema.emitterType) {
                xmlNodePtr emitterNode = xmlTextReaderExpand(mitsubaReader.get());

                xml2::xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(emitterNode->doc);

                xml2::xpath::Node node(emitterNode, xpathContext.get());

                const std::optional<Emitter> emitter = loadEmitter(node, schema);

                if (!emitter || !delegate->readEmitter(*emitter)) {
                    return false;
                }
            } else if (name == schema.shapeType) {
                xmlNodePtr shapeNode = xmlTextReaderExpand(mitsubaReader.get());

                xml2::xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(shapeNode->doc);
                const xml2::xmlString typeProperty = xmlGetProp(shapeNode, schema.typeName.xml_str());

                xml2::xpath::Node node(shapeNode, xpathContext.get());

                if (typeProperty == schema.objTypeName) {
                    const std::optional<Shape<Mesh>> mesh = loadTypedShape<Mesh>(node, schema, loadMesh);

                    if (!mesh || !delegate->readMesh(*mesh)) {
                        return false;
                    }
                } else if (typeProperty == schema.sphereName) {
                    const std::optional<Shape<Sphere>> sphere = loadTypedShape<Sphere>(node, schema, loadSphere);

                    if (!sphere || !delegate->readSphere(*sphere)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace cobalt::io::mitsuba
