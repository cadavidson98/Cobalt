#include "mitsuba_utilities.h"

#include "color.h"
#include "xml_utilities.h"

#include "core/logging.h"
#include "core/string_utilities.h"
#include "math/math_utilities.h"
#include "math/vec2.h"
#include "math/vec3.h"

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xpath.h>

#include <cassert>
#include <memory>
#include <type_traits>
#include <unordered_set>

namespace cblt::render::utils {

/// Mitsuba materials
class MitsubaDiffuse final : public MitsubaBSDF {
public:
    MitsubaDiffuse(CoSpectrum reflectance);
    virtual ~MitsubaDiffuse() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _reflectance;
};

struct MitsubaDielectric final : public MitsubaBSDF {
public:
    MitsubaDielectric(
        CoSpectrum specularReflectance,
        CoSpectrum specularTransmission,
        vec2f roughness,
        float interiorIOR,
        float exteriorIOR,
        bool isThin
    );
    virtual ~MitsubaDielectric();

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _specularReflectance;
    CoSpectrum _specularTransmission;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    float _interiorIOR;
    float _exteriorIOR;
    // use thin material approximation
    bool _isThin;
};

struct MitsubaConductor final : public MitsubaBSDF {
public:
    MitsubaConductor(CoSpectrum specularReflectance, vec2f roughness, float IOR);
    virtual ~MitsubaConductor() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _specularReflectance;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    // TODO: complex values for conductor ior
    float _IOR;
};

struct MitsubaPlastic final : public MitsubaBSDF {
public:
    MitsubaPlastic(
        CoSpectrum diffuseReflectance,
        CoSpectrum specularReflectance,
        vec2f roughness,
        float interiorIOR,
        float exteriorIOR
    );
    virtual ~MitsubaPlastic() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _diffuseReflectance;
    CoSpectrum _specularReflectance;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    float _interiorIOR;
    float _exteriorIOR;
};

MitsubaDiffuse::MitsubaDiffuse(CoSpectrum reflectance): _reflectance{reflectance} {
}

std::string MitsubaDiffuse::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaDiffuse::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _reflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = 0.f,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = 0.f,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}

MitsubaDielectric::MitsubaDielectric(
    CoSpectrum specularReflectance,
    CoSpectrum specularTransmission,
    vec2f roughness,
    float interiorIOR,
    float exteriorIOR,
    bool isThin
)
    : _specularReflectance{specularReflectance}, _specularTransmission{specularTransmission}, _roughness{roughness},
      _interiorIOR{interiorIOR}, _exteriorIOR{exteriorIOR}, _isThin{isThin} {
}

MitsubaDielectric::~MitsubaDielectric() {
}

std::string MitsubaDielectric::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaDielectric::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _specularReflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = 0.f,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = _roughness.x,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}

MitsubaConductor::MitsubaConductor(CoSpectrum specularReflectance, vec2f roughness, float IOR)
    : _specularReflectance{specularReflectance}, _roughness{roughness}, _IOR{IOR} {
}

std::string MitsubaConductor::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaConductor::properties() const {
    return MitsubaBSDF::Properties{};
}

MitsubaPlastic::MitsubaPlastic(
    CoSpectrum diffuseReflectance,
    CoSpectrum specularReflectance,
    vec2f roughness,
    float interiorIOR,
    float exteriorIOR
)
    : _diffuseReflectance{diffuseReflectance}, _specularReflectance{specularReflectance}, _roughness{roughness},
      _interiorIOR{interiorIOR}, _exteriorIOR{exteriorIOR} {
}

std::string MitsubaPlastic::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaPlastic::properties() const {
    return MitsubaBSDF::Properties{};
}

namespace {

struct MitsubaSchema {
    // core scene primitives
    const xmlString sensorType = "sensor";
    const xmlString meshType = "shape";
    const xmlString emitterType = "emitter";
    const xmlString bsdfType = "bsdf";

    // common
    const xmlString typeName = "type";
    const xmlString fileName = ".//string/@value";
    const xmlString valueName = "value";
    const xmlString floatName = "float";
    const xmlString stringName = "string";
    const xmlString idName = "id";

    // transform
    const xmlString transformExpression = ".//transform";
    const xmlString translateExpression = "translate";
    const xmlString rotateExpression = "rotate";
    const xmlString scaleExpression = "scale";
    const xmlString matrixName = "matrix";
    const xmlString xName = "x";
    const xmlString yName = "y";
    const xmlString zName = "z";
    const xmlString angleName = "angle";

    // camera
    const xmlString fovName = ".//float[@name=\"fov\"]";

    // emitter
    const xmlString environmentMapName = "envmap";

    // material
    const xmlString diffuseName = "diffuse";
    const xmlString dieletricName = "dielectric";
    const xmlString roughDielectricName = "roughdielectric";
    const xmlString thinDielectricName = "thindielectric";

    const xmlString spectrumName = "spectrum";
    const xmlString textureName = "texture";
    const xmlString rgbName = "rgb";

    const xmlString reflectanceName = ".//reflectance";
    const xmlString specularTransmittanceName = ".//specular_transmittance";
    const xmlString diffuseReflectanceName = ".//diffuse_reflectance";
    const xmlString specularReflectanceName = ".//specular_reflectance";

    const xmlString interiorIORName = ".//int_ior";
    const xmlString exteriorIORName = ".//ext_ior";
    const xmlString roughnessName = ".//float[@name=\"alpha\"]";
    const xmlString roughnessXName = ".//float[@name=\"alpha_u\"]";
    const xmlString roughnessYName = ".//float[@name=\"alpha_v\"]";

    // mesh
    const xmlString objTypeName = "obj";
};

std::optional<float> loadFloat(xmlNodePtr node, const MitsubaSchema &schema) {
    if (node->name != schema.floatName) {
        return std::nullopt;
    }

    const xmlString valueProperty = xmlGetProp(node, schema.valueName.xml_str());
    return float(std::stof(valueProperty.c_str()));
};

mat4f loadTransform(xmlNodePtr transformNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    // need to iterate IN ORDER to properly compose transforms
    xmlNodePtr transformChildren = transformNode->children;
    xmlNodePtr iterator = transformChildren;

    mat4f transform(1.f);

    while (iterator != NULL) {
        xmlString valueProperty = xmlGetProp(iterator, schema.valueName.xml_str());
        xmlString_view transformType = iterator->name;
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
                xmlString_view rotationAxis = rotateIterator->name;
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

std::optional<MitsubaCamera>
loadCamera(xmlNodePtr cameraNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    xmlResource<xmlXPathObject> transform(xmlXPathNodeEval(cameraNode, schema.transformExpression.xml_str(), context));
    if (!transform || !xmlHoldsAlternative<XPATH_NODESET>(transform.get())) {
        CoLogError("Missing transform for Camera");
        return std::nullopt;
    }

    const mat4f cameraTransform = loadTransform(*transform->nodesetval->nodeTab, context, schema);

    xmlResource<xmlXPathObject> fovPath(xmlXPathNodeEval(cameraNode, schema.fovName.xml_str(), context));
    if (!fovPath || !xmlHoldsAlternative<XPATH_NODESET>(fovPath.get())) {
        CoLogError("Missing Field of View for camera");
        return std::nullopt;
    }

    std::optional<float> fovValue = loadFloat(*fovPath->nodesetval->nodeTab, schema);
    if (!fovValue) {
        return std::nullopt;
    }

    const float cameraFov = cblt::utils::toRadians(*fovValue);
    return MitsubaCamera{
        .fov = cameraFov,
        .transform = cameraTransform,
    };
}

std::shared_ptr<MitsubaBSDF>
loadMaterial(xmlNodePtr materialNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    xmlString materialType = xmlGetProp(materialNode, schema.typeName.xml_str());
    if (!materialType) {
        CoLogError("unsupported material type");
        return nullptr;
    }

    auto getSpectrum = [&schema,
                        context](xmlNodePtr node, const xmlChar *reflectanceName) -> std::optional<CoSpectrum> {
        // TODO: need the xmlResource wrapper?
        xmlResource<xmlXPathObject> reflectanceObject(xmlXPathNodeEval(node, reflectanceName, context));
        if (!xmlHoldsAlternative<XPATH_NODESET>(reflectanceObject.get())) {
            return std::nullopt;
        }

        xmlNodePtr reflectanceNode = reflectanceObject->nodesetval->nodeTab[0];
        const xmlString_view reflectanceSource = reflectanceNode->name;
        if (reflectanceSource == schema.rgbName) {
            xmlString valueProperty = xmlGetProp(reflectanceNode, schema.valueName.xml_str());
            const std::vector<float> rgbValues = cblt::core::split<float>(valueProperty.c_str(), ' ');
            return CoSpectrum(CoColor(rgbValues[0], rgbValues[1], rgbValues[2], 1.f));
        }

        return std::nullopt;
    };

    auto getIOR = [&schema, context](xmlNodePtr node, const xmlChar *name) -> std::optional<float> {
        xmlResource<xmlXPathObject> iorObject = xmlXPathNodeEval(node, schema.exteriorIORName.xml_str(), context);
        if (!xmlHoldsAlternative<XPATH_NODESET>(iorObject.get())) {
            return std::nullopt;
        }

        xmlNodePtr iorNode = iorObject->nodesetval->nodeTab[0];
        std::optional<float> ior = loadFloat(node, schema);
        if (!ior) {
            CoLogError("Only \'float\' type supported for IOR");
            return std::nullopt;
        }

        return ior;
    };

    auto getRoughness = [&schema, context](xmlNodePtr node) -> vec2f {
        xmlResource<xmlXPathObject> roughnessObject = xmlXPathNodeEval(node, schema.roughnessName.xml_str(), context);
        xmlResource<xmlXPathObject> roughnessXObject = xmlXPathNodeEval(node, schema.roughnessXName.xml_str(), context);
        xmlResource<xmlXPathObject> roughnessYObject = xmlXPathNodeEval(node, schema.roughnessYName.xml_str(), context);
        static constexpr float kDefaultRoughness = 0.1f;

        if (xmlHoldsAlternative<XPATH_NODESET>(roughnessObject.get())) {
            const float roughness =
                loadFloat(roughnessObject->nodesetval->nodeTab[0], schema).value_or(kDefaultRoughness);
            return vec2f(roughness, roughness);
        } else if (xmlHoldsAlternative<XPATH_NODESET>(roughnessXObject.get()) &&
                   xmlHoldsAlternative<XPATH_NODESET>(roughnessYObject.get())) {
            const float roughnessX =
                loadFloat(roughnessXObject->nodesetval->nodeTab[0], schema).value_or(kDefaultRoughness);
            const float roughnessY =
                loadFloat(roughnessYObject->nodesetval->nodeTab[0], schema).value_or(kDefaultRoughness);

            return vec2f(roughnessX, roughnessY);
        }

        return vec2f(kDefaultRoughness, kDefaultRoughness);
    };

    if (materialType == schema.diffuseName) {
        std::optional<CoSpectrum> diffuseReflectance = getSpectrum(materialNode, schema.reflectanceName.xml_str());
        if (!diffuseReflectance) {
            CoLogError("Missing reflectance for Diffuse BSDF");
            return nullptr;
        }
        std::shared_ptr<MitsubaBSDF> diffuse = std::make_shared<MitsubaDiffuse>(*diffuseReflectance);
    } else if (materialType.contains(schema.dieletricName)) {
        // defaults
        static constexpr float kAirIOR = 1.00027f;
        static constexpr float kBk7IOR = 1.5046f;
        const CoSpectrum whiteSpectrum(CoColor(1.f, 1.f, 1.f, 1.f));

        const CoSpectrum reflectance =
            getSpectrum(materialNode, schema.specularReflectanceName.xml_str()).value_or(whiteSpectrum);
        const CoSpectrum transmittance =
            getSpectrum(materialNode, schema.specularTransmittanceName.xml_str()).value_or(whiteSpectrum);

        const float interiorIOR = getIOR(materialNode, schema.interiorIORName.xml_str()).value_or(kBk7IOR);
        const float exteriorIOR = getIOR(materialNode, schema.exteriorIORName.xml_str()).value_or(kAirIOR);

        const bool isRough = materialType == schema.roughDielectricName;
        const bool isThin = materialType == schema.thinDielectricName;
        vec2f roughness(0.f, 0.f);
        if (isRough) {
            roughness = getRoughness(materialNode);
        }

        return std::shared_ptr<MitsubaBSDF>(
            new MitsubaDielectric(reflectance, transmittance, roughness, interiorIOR, exteriorIOR, isThin)
        );
    } else {
        CoLogError("unsupported material type");
        return nullptr;
    }

    return nullptr;
}

std::optional<MitsubaMesh> loadMesh(xmlNodePtr meshNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    xmlString meshType = xmlGetProp(meshNode, schema.typeName.xml_str());
    if (!meshType || meshType != schema.objTypeName) {
        CoLogError("Unsupported Mesh type '%s'", meshType.c_str());
        return std::nullopt;
    }

    xmlResource<xmlXPathObject> fileNode(xmlXPathNodeEval(meshNode, schema.fileName.xml_str(), context));
    xmlChar *fileNameString(xmlXPathCastToString(fileNode.get()));
    if (!fileNameString) {
        CoLogError("Missing Filename for mesh");
        return std::nullopt;
    }

    xmlResource<xmlXPathObject> transform(xmlXPathNodeEval(meshNode, schema.transformExpression.xml_str(), context));
    mat4f meshTransform;
    if (xmlHoldsAlternative<XPATH_NODESET>(transform.get())) {
        xmlNodeSetPtr transformNode = transform->nodesetval;
        meshTransform = loadTransform(transformNode->nodeTab[0], context, schema);
    }

    xmlResource<xmlXPathObject> bsdf(xmlXPathNodeEval(meshNode, schema.bsdfType.xml_str(), context));
    std::shared_ptr<MitsubaBSDF> material = nullptr;
    if (bsdf) {
        material = loadMaterial(bsdf->nodesetval->nodeTab[0], context, schema);
    }

    std::string meshFileName = reinterpret_cast<char *>(fileNameString);
    return MitsubaMesh{
        .fileName = meshFileName,
        .fileExtension = meshType.c_str(),
        .transform = meshTransform,
        .material = material,
    };
}

} // anonymous namespace

/// Mitsuba entrypoint

bool readMitsuba(const std::string_view fileName, std::shared_ptr<MitsubaDelegate> delegate) {
    MitsubaSchema schema;
    xmlResource<xmlTextReader, xmlTextReaderDeleter> mitsubaReader = xmlNewTextReaderFilename(fileName.data());

    if (!mitsubaReader) {
        CoLogError("XML Parsing error occured while reading %s", fileName.data());
        return false;
    }

    int returnValue = 0;
    do {
        returnValue = xmlTextReaderRead(mitsubaReader.get());
        const int rawType = xmlTextReaderNodeType(mitsubaReader.get());
        if (rawType == -1) {
            break;
        }

        const xmlReaderTypes type = static_cast<xmlReaderTypes>(rawType);
        if (type == XML_READER_TYPE_ELEMENT) {
            xmlString name = xmlTextReaderName(mitsubaReader.get());
            if (name == schema.sensorType) {
                xmlNodePtr cameraNode = xmlTextReaderExpand(mitsubaReader.get());

                xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(cameraNode->doc);

                const std::optional<MitsubaCamera> camera = loadCamera(cameraNode, xpathContext.get(), schema);
                if (camera) {
                    delegate->readSensor(*camera);
                }

                xmlTextReaderNext(mitsubaReader.get());
            } else if (name == schema.bsdfType) {
                xmlNodePtr bsdfNode = xmlTextReaderExpand(mitsubaReader.get());

                xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(bsdfNode->doc);

                std::shared_ptr<MitsubaBSDF> bsdf = loadMaterial(bsdfNode, xpathContext.get(), schema);
                if (bsdf) {
                    delegate->readBsdf(bsdf);
                }

                xmlTextReaderNext(mitsubaReader.get());
            } else if (name == schema.emitterType) {
                xmlNodePtr emitterNode = xmlTextReaderExpand(mitsubaReader.get());

                xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(emitterNode->doc);

                xmlString emitterType = xmlGetProp(emitterNode, schema.typeName.xml_str());
                if (emitterType && emitterType == schema.environmentMapName) {
                    xmlXPathObjectPtr fileNameNode =
                        xmlXPathNodeEval(emitterNode, schema.fileName.xml_str(), xpathContext.get());
                    xmlString fileName = xmlXPathCastToString(fileNameNode);
                    if (!fileName) {
                        CoLogError("Missing Filename string");
                        continue;
                    }

                    delegate->readEmitter({
                        .emissionMap = {
                                        .fileName = fileName.c_str(),
                                        .fileExtension = cblt::core::fileExtension(fileName.c_str()),
                                        },
                    });
                }

                xmlTextReaderNext(mitsubaReader.get());

            } else if (name == schema.meshType) {
                xmlNodePtr meshNode = xmlTextReaderExpand(mitsubaReader.get());

                xmlResource<xmlXPathContext> xpathContext = xmlXPathNewContext(meshNode->doc);

                const std::optional<MitsubaMesh> mesh = loadMesh(meshNode, xpathContext.get(), schema);
                if (mesh) {
                    delegate->readMesh(*mesh);
                }

                xmlTextReaderNext(mitsubaReader.get());
            }
        }
    } while (returnValue == 1);

    return true;
}

} // namespace cblt::render::utils
