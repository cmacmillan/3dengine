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

// NOTE I think the thing we want to cache is the models. Read it into the data structure once, then we can query it whenever to spawn something in.
//  Large data like meshes should be cached as well, but for all the minutea we can probably get away with doing it this way
//  Names can even just be pointers into the model

SMesh3D * PMeshLoad(tinygltf::Model * pModel, tinygltf::Mesh * pTinymesh);

bool FTryLoadModel(const char * pChzPath, tinygltf::Model * pModel)
{
	tinygltf::TinyGLTF loader;
	std::string strErr;
	std::string strWarn;

	bool fSuccess = loader.LoadASCIIFromFile(pModel, &strErr, &strWarn, StrPrintf("%s%s", ASSET_PATH, pChzPath).c_str());

	if (!strWarn.empty())
		g_game.PrintConsole(StrPrintf("%s\n",strWarn).c_str());

	if (!strErr.empty())
		g_game.PrintConsole(StrPrintf("%s\n",strErr).c_str());

	return fSuccess;
}

// TODO create on add child or something like that to update transform heirarchy

void SpawnNode(tinygltf::Model * pModel, int iNode, SNode * pNodeParent)
{
	tinygltf::Node * pNode = &pModel->nodes[iNode];

	SNode3D * pNode3d = nullptr;

	if (pNode->mesh != -1)
	{
		SDrawNode3D * pDrawnode = new SDrawNode3D(pNodeParent->HNode(), pNode->name);
		pDrawnode->m_hMesh = (PMeshLoad(pModel, &pModel->meshes[pNode->mesh]))->HMesh();
		pDrawnode->m_hMaterial = g_game.m_hMaterialDefault3d;
		pNode3d = pDrawnode;
	}
	else
	{
		SNode3D * pNode3d = new SNode3D(pNodeParent->HNode(), pNode->name);
	}

	// TODO each of these call UpdateSelfAndChildTransformCache, create some combined version

	if (pNode->translation.size() > 0)
		pNode3d->SetPosLocal(Point(pNode->translation[0], pNode->translation[1], pNode->translation[2]));

	if (pNode->rotation.size() > 0)
		pNode3d->SetQuatLocal(Quat(pNode->rotation[3], pNode->rotation[0], pNode->rotation[1], pNode->rotation[2]));

	if (pNode->scale.size() > 0)
		pNode3d->SetVecScaleLocal(Vector(pNode->scale[0], pNode->scale[1], pNode->scale[2]));

	for (int iNodeChild : pNode->children)
	{
		SpawnNode(pModel, iNodeChild, pNode3d);
	}
}

void SpawnScene(const char * pChzPath)
{
	tinygltf::Model model;
	VERIFY(FTryLoadModel(pChzPath, &model));

	ASSERT(model.scenes.size() == 1);

	tinygltf::Scene * pScene = &model.scenes[0];

	SNode * pNodeRoot = g_game.m_hNodeRoot.PT();
	for (int iNode = 0; iNode < pScene->nodes.size(); iNode++)
	{
		SpawnNode(&model, pScene->nodes[iNode], pNodeRoot);
	}

	return;
}

SMesh3D * PMeshLoad(tinygltf::Model * pModel, tinygltf::Mesh * pTinymesh)
{
	SMesh3D * pMesh = new SMesh3D();

	// NOTE if you were to apply 2 materials to a model, you'd probably end up with 2 primitives
	
	ASSERT(pTinymesh->primitives.size() == 1); // Currently only support 1 primitive per mesh
	tinygltf::Primitive * pPrim = &pTinymesh->primitives[0];
	ASSERT(pPrim->mode == TINYGLTF_MODE_TRIANGLES);

	{
		int iAccessorVerts = pPrim->attributes["POSITION"];
		int iAccessorNormals = pPrim->attributes["NORMAL"];
		int iAccessorUvs = pPrim->attributes["TEXCOORD_0"];

		const tinygltf::Accessor * pAccessorVerts = &pModel->accessors[iAccessorVerts];
		const tinygltf::Accessor * pAccessorNormals = &pModel->accessors[iAccessorNormals];
		const tinygltf::Accessor * pAccessorUvs = &pModel->accessors[iAccessorUvs];

		tinygltf::BufferView * pBufferviewVerts = &pModel->bufferViews[pAccessorVerts->bufferView];
		tinygltf::BufferView * pBufferviewNormals = &pModel->bufferViews[pAccessorNormals->bufferView];
		tinygltf::BufferView * pBufferviewUvs = &pModel->bufferViews[pAccessorUvs->bufferView];

		tinygltf::Buffer * pBufferVerts = &pModel->buffers[pBufferviewVerts->buffer];
		tinygltf::Buffer * pBufferNormals = &pModel->buffers[pBufferviewNormals->buffer];
		tinygltf::Buffer * pBufferUvs = &pModel->buffers[pBufferviewUvs->buffer];

		unsigned char * pBVert = pBufferVerts->data.data() + pBufferviewVerts->byteOffset + pAccessorVerts->byteOffset;
		unsigned char * pBNormal = pBufferNormals->data.data() + pBufferviewNormals->byteOffset + pAccessorNormals->byteOffset;
		unsigned char * pBUv = pBufferUvs->data.data() + pBufferviewUvs->byteOffset + pAccessorUvs->byteOffset;

		ASSERT(pAccessorVerts->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
		ASSERT(pAccessorNormals->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
		ASSERT(pAccessorUvs->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

		ASSERT(pAccessorVerts->type == TINYGLTF_TYPE_VEC3);
		ASSERT(pAccessorNormals->type == TINYGLTF_TYPE_VEC3);
		ASSERT(pAccessorUvs->type == TINYGLTF_TYPE_VEC2);

		struct SVec3
		{
			float m_x, m_y, m_z;
		};

		SVec3 * pVec3Vert = reinterpret_cast<SVec3 *>(pBVert);
		SVec3 * pVec3Normal = reinterpret_cast<SVec3 *>(pBNormal);
		float2 * pVecUv = reinterpret_cast<float2 *>(pBUv);
		for (int iVec3 = 0; iVec3 < pAccessorVerts->count; iVec3++)
		{
			SVec3 & vec3Vert = pVec3Vert[iVec3];
			SVec3 & vec3Normal = pVec3Normal[iVec3];
			float2 & vecUv = pVecUv[iVec3];
			ASSERT(FIsNear(1.0f, Vector(vec3Normal.m_x,vec3Normal.m_y, vec3Normal.m_z).SLength()));

			pMesh->m_aryVertdata.push_back({ Point(vec3Vert.m_x,vec3Vert.m_y, vec3Vert.m_z), Vector(vec3Normal.m_x,vec3Normal.m_y, vec3Normal.m_z), vecUv});
		}
	}

	{
		int iAccessor = pPrim->indices;
		const tinygltf::Accessor * pAccessorIndicies = &pModel->accessors[iAccessor];

		tinygltf::BufferView * pBufferviewIndicies = &pModel->bufferViews[pAccessorIndicies->bufferView];
		tinygltf::Buffer * pBufferIndicies = &pModel->buffers[pBufferviewIndicies->buffer];
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

SMesh3D * PMeshLoadSingle(const char * pChzPath)
{
	tinygltf::Model model;
	if (!FTryLoadModel(pChzPath, &model))
		return nullptr;

	ASSERT(model.meshes.size() == 1);
	tinygltf::Mesh * pTinymesh = &model.meshes[0];

	return PMeshLoad(&model, pTinymesh);
}
