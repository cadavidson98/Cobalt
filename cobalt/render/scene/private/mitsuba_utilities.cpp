#include "mitsuba_utilities.h"

#include "camera.h"
#include "obj_utilities.h"
#include "texture.h"

#include "core/logging.h"
#include "core/string_utilities.h"
#include "geometry/mesh.h"
#include "math/math_utilities.h"
#include "math/vec3.h"

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xpath.h>

#include <memory>

namespace cblt::render::utils {

namespace {

template<typename T>
struct xmlResource {
public:
    xmlResource(T *raw): value{raw} {
    }

    ~xmlResource() {
        if (value) {
            xmlFree(value);
        }
    }

    T &operator*() {
        return *value;
    }

    T *operator->() {
        return value;
    }

    T *get() const {
        return value;
    }

    operator bool() const {
        return bool(value);
    }

private:
    T *value;
};

struct MitsubaSchema {

    // core scene primitives
    xmlChar *sensorExpression = xmlCharStrdup("//sensor");
    xmlChar *meshExpression = xmlCharStrdup("//shape");
    xmlChar *emitterExpression = xmlCharStrdup("//emitter");
    xmlChar *bsdfExpression = xmlCharStrdup(".//bsdf");

    // common
    xmlChar *typeName = xmlCharStrdup("type");
    xmlChar *fileName = xmlCharStrdup(".//string/@value");
    xmlChar *valueName = xmlCharStrdup("value");

    // transform
    xmlChar *transformExpression = xmlCharStrdup(".//transform");
    xmlChar *translateExpression = xmlCharStrdup("translate");
    xmlChar *rotateExpression = xmlCharStrdup("rotate");
    xmlChar *scaleExpression = xmlCharStrdup("scale");
    xmlChar *matrixName = xmlCharStrdup("matrix");
    xmlChar *xName = xmlCharStrdup("x");
    xmlChar *yName = xmlCharStrdup("y");
    xmlChar *zName = xmlCharStrdup("z");
    xmlChar *angleName = xmlCharStrdup("angle");

    // camera
    xmlChar *fovName = xmlCharStrdup(".//float[@name=\"fov\"]/@value");

    // emitter
    xmlChar *environmentMapName = xmlCharStrdup("envmap");

    // material
    xmlChar *diffuseName = xmlCharStrdup("diffuse");
    xmlChar *dieletricName = xmlCharStrdup("dielectric");
    xmlChar *roughDielectricName = xmlCharStrdup("roughdielectric");
    xmlChar *reflectanceName = xmlCharStrdup(".//*/[@name='reflectance']");
    xmlChar *rgbName = xmlCharStrdup("rgb");
    xmlChar *spectrumName = xmlCharStrdup("spectrum");
    xmlChar *textureName = xmlCharStrdup("texture");

    // mesh
    xmlChar *objTypeName = xmlCharStrdup("obj");

    ~MitsubaSchema() {
        xmlFree(sensorExpression);
        xmlFree(meshExpression);
        xmlFree(emitterExpression);
        xmlFree(bsdfExpression);

        xmlFree(typeName);
        xmlFree(fileName);
        xmlFree(valueName);

        xmlFree(transformExpression);
        xmlFree(translateExpression);
        xmlFree(rotateExpression);
        xmlFree(scaleExpression);
        xmlFree(matrixName);
        xmlFree(xName);
        xmlFree(yName);
        xmlFree(zName);
        xmlFree(angleName);

        xmlFree(fovName);

        xmlFree(environmentMapName);

        xmlFree(diffuseName);
        xmlFree(dieletricName);
        xmlFree(roughDielectricName);
        xmlFree(reflectanceName);
        xmlFree(rgbName);
        xmlFree(spectrumName);
        xmlFree(textureName);

        xmlFree(objTypeName);
    }
};

mat4f loadTransform(xmlNodePtr transformNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    // need to iterate IN ORDER to properly compose transforms
    xmlNodePtr transformChildren = transformNode->children;
    xmlNodePtr iterator = transformChildren;

    mat4f transform(1.f);

    while (iterator != NULL) {
        xmlResource<xmlChar> valueProperty(xmlGetProp(iterator, schema.valueName));
        const char *value = reinterpret_cast<char *>(valueProperty.get());
        if (xmlStrEqual(iterator->name, schema.translateExpression)) {
            const std::vector<float> translationValues = core::split<float>(value, ' ');
            const vec3f translation(translationValues[0], translationValues[1], translationValues[2]);
            const mat4f translationMatrix = cblt::utils::translationMatrix(translation);
            transform = translationMatrix * transform;
        } else if (xmlStrEqual(iterator->name, schema.rotateExpression)) {
            // rotations can be expressed using an arbitrary vector ("value")
            // or as an "x", "y", or "z" attribute
            vec3f axis(0.f, 0.f, 0.f);
            float angle = 0.f;
            xmlAttrPtr rotateIterator = NULL;
            for (rotateIterator = iterator->properties; rotateIterator; rotateIterator = rotateIterator->next) {
                if (xmlStrEqual(rotateIterator->name, schema.xName)) {
                    xmlChar *xString = rotateIterator->children->content;
                    axis.x = std::stof(reinterpret_cast<char *>(xString));
                } else if (xmlStrEqual(rotateIterator->name, schema.yName)) {
                    xmlChar *yString = rotateIterator->children->content;
                    axis.y = std::stof(reinterpret_cast<char *>(yString));
                } else if (xmlStrEqual(rotateIterator->name, schema.zName)) {
                    xmlChar *zString = rotateIterator->children->content;
                    axis.z = std::stof(reinterpret_cast<char *>(zString));
                } else if (xmlStrEqual(rotateIterator->name, schema.valueName)) {
                    const std::vector<float> rotationValues = core::split<float>(value, ' ');
                    assert(rotationValues.size() >= 3);
                    axis = vec3f(rotationValues[0], rotationValues[1], rotationValues[2]);
                } else if (xmlStrEqual(rotateIterator->name, schema.angleName)) {
                    xmlChar *angleString = rotateIterator->children->content;
                    angle = cblt::utils::toRadians(std::stof(reinterpret_cast<char *>(angleString)));
                }
            }
            const mat4f rotationMatrix = cblt::utils::rotationMatrix(axis, angle);
            transform = rotationMatrix * transform;
        } else if (xmlStrEqual(iterator->name, schema.scaleExpression)) {
            const std::vector<float> scaleValues = core::split<float>(value, ' ');
            const vec3f scale(scaleValues[0], scaleValues[1], scaleValues[1]);
            const mat4f scaleMatrix = cblt::utils::scaleMatrix(scale);
            transform = scaleMatrix * transform;
        } else if (xmlStrEqual(iterator->name, schema.matrixName)) {
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

std::shared_ptr<CoCamera> loadCamera(xmlNodePtr cameraNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    xmlResource<xmlXPathObject> transform(xmlXPathNodeEval(cameraNode, schema.transformExpression, context));
    xmlNodeSetPtr transformNode = transform->nodesetval;

    mat4f cameraTransform;
    if (transformNode->nodeNr > 0) {
        cameraTransform = loadTransform(transformNode->nodeTab[0], context, schema);
    }

    xmlResource<xmlXPathObject> fovPath(xmlXPathNodeEval(cameraNode, schema.fovName, context));
    if (fovPath->nodesetval == NULL) {
        CoLogError("Missing Field of View for camera");
        return nullptr;
    }
    const float fov = cblt::utils::toRadians(xmlXPathCastNodeToNumber(*fovPath->nodesetval->nodeTab));

    const CoCamera::CreateFromProjectionInfo createInfo{
        .hFov = fov,
        .vFov = fov,
        .cameraToWorld = cameraTransform,
    };

    return std::shared_ptr<CoCamera>(new CoCamera(createInfo));
}

std::optional<MitsubaMaterial>
loadMaterial(xmlNodePtr materialNode, xmlXPathContextPtr context, const MitsubaSchema &schema) {
    xmlResource<xmlChar> materialType(xmlGetProp(materialNode, schema.typeName));
    if (!materialType) {
        CoLogError("unsupported material type");
        return std::nullopt;
    }

    if (xmlStrEqual(materialType.get(), schema.diffuseName)) {
        xmlResource<xmlXPathObject> reflectanceObject(xmlXPathNodeEval(materialNode, schema.reflectanceName, context));
        if (!reflectanceObject || !reflectanceObject->nodesetval) {
            CoLogError("Missing reflectance for Diffuse BSDF");
            return std::nullopt;
        }

        const xmlNodePtr reflectanceNode = reflectanceObject->nodesetval->nodeTab[0];
        const xmlChar *reflectanceSource = reflectanceNode->name;
        if (xmlStrEqual(reflectanceSource, schema.rgbName)) {
            xmlResource<xmlChar> valueProperty(xmlGetProp(reflectanceNode, schema.valueName));
            const char *valueString = reinterpret_cast<const char *>(valueProperty.get());
            const std::vector<float> rgbValues = cblt::core::split<float>(valueString, ' ');
            return MitsubaMaterial{
                .baseColor = vec4f(rgbValues[0], rgbValues[1], rgbValues[2], 1.f),
            };
        } else if (xmlStrEqual(reflectanceSource, schema.textureName)) {
            xmlResource<xmlXPathObject> fileNameProperty(xmlXPathNodeEval(reflectanceNode, schema.fileName, context));
            xmlResource<xmlChar> fileName = xmlXPathCastToString(fileNameProperty.get());
            return MitsubaMaterial{
                .baseColor = MitsubaTexture{
                                            .fileName = std::string(reinterpret_cast<char *>(fileName.get())),
                                            },
            };
        }

    } else if (xmlStrEqual(materialType.get(), schema.dieletricName)) {

    } else if (xmlStrEqual(materialType.get(), schema.roughDielectricName)) {

    } else {
        CoLogError("unsupported material type");
        return std::nullopt;
    }

    return std::nullopt;
}

std::shared_ptr<geom::CoMesh> loadMesh(
    const std::string &parentDirectory,
    xmlNodePtr meshNode,
    xmlXPathContextPtr context,
    const MitsubaSchema &schema
) {
    xmlResource<xmlChar> meshType = xmlGetProp(meshNode, schema.typeName);
    if (!meshType || !xmlStrEqual(meshType.get(), schema.objTypeName)) {
        CoLogError("Unsupported Mesh type '%s'", reinterpret_cast<const char *>(meshType.get()));
        return nullptr;
    }

    xmlResource<xmlXPathObject> fileNode(xmlXPathNodeEval(meshNode, schema.fileName, context));
    xmlChar *fileNameString(xmlXPathCastToString(fileNode.get()));
    if (!fileNameString) {
        CoLogError("Missing Filename for mesh");
        return nullptr;
    }

    xmlResource<xmlXPathObject> transform(xmlXPathNodeEval(meshNode, schema.transformExpression, context));
    xmlNodeSetPtr transformNode = transform->nodesetval;
    mat4f meshTransform;
    if (transformNode && transformNode->nodeNr > 0) {
        meshTransform = loadTransform(transformNode->nodeTab[0], context, schema);
    }

    std::string meshFileName = reinterpret_cast<char *>(fileNameString);
    return readObjFile(parentDirectory + meshFileName);
}

} // anonymous namespace

std::optional<MitsubaScene>
readMitsuba(const std::string &fileName, const std::string &parentDirectory, core::CoCallback &callback) {
    xmlInitParser();
    callback.pump("Parsing scene file");
    xmlDocPtr mitsubaDOM = xmlParseFile(fileName.c_str());
    if (!mitsubaDOM) {
        CoLogError("XML Parsing error occured while reading %s", fileName.c_str());
        return std::nullopt;
    }

    MitsubaSchema schema;

    xmlXPathContextPtr xpathContext = xmlXPathNewContext(mitsubaDOM);
    if (!xpathContext) {
        CoLogError("XML Parsing error occured while reading %s", fileName.c_str());
        return std::nullopt;
    }

    xmlResource<xmlXPathObject> sensorObject(xmlXPathEvalExpression(schema.sensorExpression, xpathContext));
    xmlResource<xmlXPathObject> meshesObject(xmlXPathEvalExpression(schema.meshExpression, xpathContext));
    xmlResource<xmlXPathObject> emittersObject(xmlXPathEvalExpression(schema.emitterExpression, xpathContext));

    if (!sensorObject || !sensorObject->nodesetval || !meshesObject || !meshesObject->nodesetval || !emittersObject ||
        !emittersObject->nodesetval) {
        CoLogError("Failed to find scene objects in XML");
        return std::nullopt;
    }

    xmlNodeSetPtr sensorNodes = sensorObject->nodesetval;
    if (sensorNodes->nodeNr <= 0) {
        CoLogError("Failed to find camera node in XML");
        return std::nullopt;
    }

    callback.pump("loading camera", 5);
    std::shared_ptr<CoCamera> camera = loadCamera(sensorNodes->nodeTab[0], xpathContext, schema);
    if (!camera) {
        return std::nullopt;
    }

    callback.pump("loading lights", 5);
    std::shared_ptr<CoTexture> environmentMap = nullptr;
    xmlNodeSetPtr emitterNodes = emittersObject->nodesetval;
    const size_t numEmitters(emitterNodes->nodeNr);
    for (size_t idx = 0; idx < numEmitters; ++idx) {
        // TODO: load ALL lights, but for now we only will scrape for the environment map
        xmlNodePtr emitterNode = emitterNodes->nodeTab[idx];
        xmlChar *emitterType = xmlGetProp(emitterNode, schema.typeName);
        if (emitterType && xmlStrEqual(emitterType, schema.environmentMapName)) {
            xmlXPathObjectPtr fileNameNode = xmlXPathNodeEval(emitterNode, schema.fileName, xpathContext);
            xmlChar *fileNameString(xmlXPathCastToString(fileNameNode));
            if (!fileNameString) {
                CoLogError("Missing Filename string");
                continue;
            }
            char *fileName = reinterpret_cast<char *>(fileNameString);
            environmentMap = CoTexture::create({
                .fileName = parentDirectory + fileName,
                .fileExtension = cblt::core::fileExtension(std::string_view(fileName)),
            });

            if (environmentMap) {
                break;
            }
        }
    }

    callback.pump("loading meshes", 20);
    xmlNodeSetPtr meshNodes = meshesObject->nodesetval;

    const size_t numMeshes(meshNodes->nodeNr);
    std::vector<std::shared_ptr<geom::CoMesh>> meshes(numMeshes);

    for (size_t idx = 0; idx < numMeshes; ++idx) {
        xmlNodePtr meshNode = meshNodes->nodeTab[idx];
        std::shared_ptr<geom::CoMesh> mesh = loadMesh(parentDirectory, meshNode, xpathContext, schema);
        if (!mesh) {
            return std::nullopt;
        }
        meshes[idx] = std::move(mesh);
    }

    xmlFreeDoc(mitsubaDOM);
    xmlCleanupParser();

    return MitsubaScene{
        .camera = std::move(camera),
        .meshes = std::move(meshes),
        .environmentMap = std::move(environmentMap),
    };
}

} // namespace cblt::render::utils
