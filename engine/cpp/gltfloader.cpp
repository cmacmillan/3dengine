#include "gltfloader.h"

#include "mesh.h"
#include "engine.h"

#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "external/tiny_gltf.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

SMesh3D * PMeshLoad(const char * pChzPath)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string strErr;
	std::string strWarn;

	bool fSuccess = loader.LoadASCIIFromFile(&model, &strErr, &strWarn, StrPrintf("%s%s", ASSET_PATH, pChzPath).c_str());

	if (!strWarn.empty())
		g_game.PrintConsole(StrPrintf("%s\n",strWarn).c_str());

	if (!strErr.empty())
		g_game.PrintConsole(StrPrintf("%s\n",strErr).c_str());

	if (!fSuccess)
		return nullptr;

	SMesh3D * pMesh = new SMesh3D();

	ASSERT(model.meshes.size() == 1);
	tinygltf::Mesh * pTinymesh = &model.meshes[0];

	ASSERT(pTinymesh->primitives.size() == 1);
	tinygltf::Primitive * pPrim = &pTinymesh->primitives[0];
	ASSERT(pPrim->mode == TINYGLTF_MODE_TRIANGLES);

	{
		int iAccessor = pPrim->attributes["POSITION"];
		const tinygltf::Accessor * pAccessorVerts = &model.accessors[iAccessor];

		tinygltf::BufferView * pBufferviewVerts = &model.bufferViews[pAccessorVerts->bufferView];
		tinygltf::Buffer * pBufferVerts = &model.buffers[pBufferviewVerts->buffer];
		unsigned char * pBVert = pBufferVerts->data.data() + pBufferviewVerts->byteOffset + pAccessorVerts->byteOffset;
		ASSERT(pAccessorVerts->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
		ASSERT(pAccessorVerts->type == TINYGLTF_TYPE_VEC3);

		struct SVec3
		{
			float m_x, m_y, m_z;
		};

		SVec3 * pVec3 = reinterpret_cast<SVec3 *>(pBVert);
		for (int iVec3 = 0; iVec3 < pAccessorVerts->count; iVec3++)
		{
			SVec3 & vec3 = pVec3[iVec3];
			pMesh->m_aryVertdata.push_back({ Point(vec3.m_x,vec3.m_y, vec3.m_z),float2(0.0f, 0.0f) });
		}
	}

	{
		int iAccessor = pPrim->indices;
		const tinygltf::Accessor * pAccessorIndicies = &model.accessors[iAccessor];

		tinygltf::BufferView * pBufferviewIndicies = &model.bufferViews[pAccessorIndicies->bufferView];
		tinygltf::Buffer * pBufferIndicies = &model.buffers[pBufferviewIndicies->buffer];
		unsigned char * pBIndex = pBufferIndicies->data.data() + pBufferviewIndicies->byteOffset + pAccessorIndicies->byteOffset;
		ASSERT(pAccessorIndicies->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
		ASSERT(pAccessorIndicies->type == TINYGLTF_TYPE_SCALAR);

		unsigned short * pI = reinterpret_cast<unsigned short*>(pBIndex);
		for (int iI= 0; iI < pAccessorIndicies->count; iI++)
		{
			pMesh->m_aryIIndex.push_back(pI[iI]);
		}
	}

	return pMesh;
}
