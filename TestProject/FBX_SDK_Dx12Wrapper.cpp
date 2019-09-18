#include "FBX_SDK_Dx12Wrapper.hpp"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <Shlwapi.h>
#include <algorithm>
#pragma comment(lib, "libfbxsdk.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace meigetsusoft {
	namespace DirectX {
		namespace Dx12 {
			namespace FBX {
				namespace Core {
					Manager::Manager() : InternalClass(FbxManager::Create()) {}

					Scene::Scene() : InternalClass(nullptr) {}

					Scene::Scene(const Manager& manager, const std::string& SceneName)
						: InternalClass(FbxScene::Create(manager.Get(), SceneName.c_str())) {}

					Importer::Importer() : InternalClass(nullptr) {}

					Importer::Importer(const Manager& manager, const std::string& ImporterName)
						: InternalClass(FbxImporter::Create(manager.Get(), ImporterName.c_str())) {}
				}

				bool FBXLoadManager::MakeScene(const std::string& SceneName) {
					if (this->FBXScenes.end() == this->FBXScenes.find(SceneName)) return false;
					this->FBXScenes[SceneName] = Core::Scene(this->FBXManager, SceneName);
					return true;
				}

				bool FBXLoadManager::DeleteScene(const std::string& SceneName) {
					const auto itr = this->FBXScenes.find(SceneName);
					if (itr != this->FBXScenes.end()) return false;
					this->FBXScenes.erase(itr);
					return true;
				}

				Core::Mesh FBXLoadManager::LoadFBXFile(const std::string& FilePath, const std::string& SceneName) {
					if (FALSE == PathFileExistsA(FilePath.c_str()))
						throw std::runtime_error(FilePath + " : File is not found.");
					FbxString FileName(FilePath.c_str());
					Core::Mesh mesh(nullptr);
					Core::Importer importer(this->FBXManager, "import-" + SceneName);
					importer->Initialize(FileName.Buffer(), -1, this->FBXManager->GetIOSettings());
					importer->Import(this->FBXScenes[SceneName].Get());
					for (int i = 0; i < this->FBXScenes[SceneName]->GetRootNode()->GetChildCount(); i++) {
						if (this->FBXScenes[SceneName]->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eMesh) continue;
						mesh = this->FBXScenes[SceneName]->GetRootNode()->GetChild(i)->GetMesh();
						break;
					}
					return mesh;
				}

				Mesh::Mesh(const Core::Mesh& mesh) {
					using GeoElement = FbxGeometryElement;
					using LayerElement = FbxLayerElement;
					const int polygonCount = mesh->GetPolygonCount();
					auto controlPoints = mesh->GetControlPoints();
					const int controlPointCount = mesh->GetControlPointsCount();
					int vertexID = 0;
					for (int polygon = 0; polygon < polygonCount; polygon++) {
						const int polyVertCount = mesh->GetPolygonSize(polygon);
						for (int polyVert = 0; polyVert < polyVertCount; polyVert++) {
							Vertex uniqueVert{};
							const int cpIndex = mesh->GetPolygonVertex(polygon, polyVert);
							uniqueVert.m_numControlPointIndex = cpIndex;
							uniqueVert.m_position = MSDirectX::XMFLOAT3(
								static_cast<float>(controlPoints[cpIndex].mData[0]),
								static_cast<float>(controlPoints[cpIndex].mData[1]),
								static_cast<float>(controlPoints[cpIndex].mData[2])
							);
							const int uvElementsCount = mesh->GetElementUVCount();
							for (int uvElement = 0; uvElement < uvElementsCount; uvElement++) {
								const Core::GeometryElementUV geometryElementUV = mesh->GetElementUV(uvElement);
								const auto mapMode = geometryElementUV->GetMappingMode();
								const auto refMode = geometryElementUV->GetReferenceMode();
								int directIndex = -1;
								if (GeoElement::eByControlPoint == mapMode) {
									if (GeoElement::eDirect == refMode) directIndex = cpIndex;
									else if (GeoElement::eIndexToDirect == refMode)
										directIndex = geometryElementUV->GetIndexArray().GetAt(cpIndex);
								}
								else if (
									GeoElement::eByPolygonVertex == mapMode
									&& (GeoElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
									)
									directIndex = mesh->GetTextureUVIndex(polygon, polyVert);
								if (directIndex == -1) continue;
								FbxVector2 uv = geometryElementUV->GetDirectArray().GetAt(directIndex);
								uniqueVert.m_UVs = MSDirectX::XMFLOAT2(
									static_cast<float>(uv.mData[0]),
									static_cast<float>(uv.mData[1])
								);
							}
							const int normalElementCount = mesh->GetElementNormalCount();
							for (int normalElement = 0; normalElement < normalElementCount; normalElement++) {
								const Core::GeometryElementNormal geometryElementNormal = mesh->GetElementNormal(normalElement);
								const LayerElement::EMappingMode mapMode = geometryElementNormal->GetMappingMode();
								const LayerElement::EReferenceMode refMode = geometryElementNormal->GetReferenceMode();
								int directIndex = -1;
								if (GeoElement::eByPolygonVertex == mapMode) {
									if (GeoElement::eDirect == refMode) directIndex = vertexID;
									else if (GeoElement::eIndexToDirect)
										directIndex = geometryElementNormal->GetIndexArray().GetAt(vertexID);
								}
								if (directIndex == -1) continue;
								FbxVector4 norm = geometryElementNormal->GetDirectArray().GetAt(directIndex);
								uniqueVert.m_normals = MSDirectX::XMFLOAT3(
									static_cast<float>(norm.mData[0]),
									static_cast<float>(norm.mData[1]),
									static_cast<float>(norm.mData[2])
								);
							}

							auto i = std::find(this->m_vVertices.begin(), this->m_vVertices.end(), uniqueVert);
							if (this->m_vVertices.end() == i) this->m_vVertices.emplace_back(uniqueVert);
							// TODO: change Indices element type from unsigned int to size_t
							this->m_vIndices.emplace_back(static_cast<unsigned int>(std::distance(this->m_vVertices.begin(), i)));
							vertexID++;
						}
					}
				}
			}
		}
	}
}
