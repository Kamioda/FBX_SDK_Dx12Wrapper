#pragma once
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>
#include <memory>
#include <map>
#include <type_traits>
#include <cstring>
namespace MSDirectX = ::DirectX;

namespace meigetsusoft {
	namespace DirectX {
		namespace Dx12 {
			namespace FBX {
				struct Vertex {
					MSDirectX::XMFLOAT3 m_position;
					MSDirectX::XMFLOAT3 m_normals;
					MSDirectX::XMFLOAT2 m_UVs;
					int m_numControlPointIndex;
				};
				static_assert(std::is_trivially_copyable_v<Vertex>);
				inline bool operator == (const Vertex& a, const Vertex& b) {
					return 0 == std::memcmp(static_cast<const void*>(&a), static_cast<const void*>(&b), sizeof(Vertex));
				}
				inline bool operator != (const Vertex& a, const Vertex& b) {
					return !(a == b);
				}
				namespace Core {
					template<class C> class FBXBase {
					private:
						C* instance;
					protected:
						using InternalClass = FBXBase<C>;
					public:
						FBXBase(C* Instance = nullptr) : instance(Instance) {}
						~FBXBase() { this->instance->Destroy(); }
						FBXBase(const FBXBase&) = delete;
						FBXBase(FBXBase&& f) : instance(f.instance) { f.instance = nullptr; }
						FBXBase& operator = (const FBXBase&) = delete;
						FBXBase& operator = (FBXBase&& f) {
							this->instance = f.instance;
							f.instance = nullptr;
							return *this;
						}
						C* operator -> () const noexcept { return this->instance; }
						C* Get() const noexcept { return this->instance; }
						C* Get() noexcept { return this->instance; }
					};
					class Manager : public FBXBase<FbxManager> {
					public:
						Manager();
					};

					class Scene : public FBXBase<FbxScene> {
					public:
						Scene(const Manager& manager, const std::string& SceneName);
					};

					class Importer : public FBXBase<FbxImporter> {
					public:
						Importer(const Manager& manager, const std::string& ImporterName);
					};

					class Mesh : public std::unique_ptr<FbxMesh> {
					public:
						Mesh(FbxMesh* mesh) : std::unique_ptr<FbxMesh>(mesh) {}
					};

					using GeometryElementUV = FBXBase<FbxGeometryElementUV>;
					using GeometryElementNormal = FBXBase<FbxGeometryElementNormal>;
				}

				class FBXLoadManager {
				private:
					Core::Manager FBXManager;
					std::map<std::string, Core::Scene> FBXScenes;
				public:
					FBXLoadManager() = default;
					bool MakeScene(const std::string& SceneName);
					bool DeleteScene(const std::string& SceneName);
					Core::Mesh LoadFBX(const std::string& FilePath, const std::string& SceneName);
				};

				class Mesh {
				private:
					std::vector<Vertex> m_vVertices;
					std::vector<unsigned int> m_vIndices;
				public:
					Mesh() = default;
					Mesh(const Core::Mesh& mesh);
					auto& GetVertices() { return this->m_vVertices; }
					auto& GetIndices() { return this->m_vIndices; }
					const auto& GetVertices() const noexcept { return this->m_vVertices; }
					const auto& GetIndices() const noexcept { return this->m_vIndices; }
				};

			}
		}
	}
}
