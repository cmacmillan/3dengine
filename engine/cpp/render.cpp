#include "render.h"
#include "engine.h"
#include "shader.h"
#include "texture.h"

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, SCamera3D * pCamera)
{
	Mat matWorldToClip = pCamera->MatWorldToClip();

	ID3D11DeviceContext1 * pD3ddevicecontext = g_game.m_pD3ddevicecontext;
	for (SDrawNode3D * pDrawnode3D : *parypDrawnode3DToRender)
	{
		if (pDrawnode3D->m_hMaterial == nullptr)
			continue;

		const SMaterial & material = *g_game.m_hMaterialShadowcaster;//*pDrawnode3D->m_hMaterial;
		const SShader & shader = *(material.m_hShader);
		ASSERT(pDrawnode3D->FIsDerivedFrom(TYPEK_DrawNode3D));
		ASSERT(shader.m_shaderk == SHADERK_3D);

		pD3ddevicecontext->RSSetState(shader.m_pD3drasterizerstate);
		pD3ddevicecontext->OMSetDepthStencilState(shader.m_pD3ddepthstencilstate, 0);

		const SMesh3D & mesh = *pDrawnode3D->m_hMesh;

		pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);

		pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
		pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

		ID3D11Buffer * aD3dbuffer[] = { g_game.m_cbufferDrawnode3D, g_game.m_cbufferGlobals };
		pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
		pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

		unsigned int cbVert = sizeof(SVertData3D);
		unsigned int s_cbMeshOffset = 0;

		pD3ddevicecontext->IASetVertexBuffers(0, 1, &g_game.m_cbufferVertex3D, &cbVert, &s_cbMeshOffset);	// BB don't constantly do this
		pD3ddevicecontext->IASetIndexBuffer(g_game.m_cbufferIndex, DXGI_FORMAT_R16_UINT, 0);					//  ...

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		pD3ddevicecontext->Map(g_game.m_cbufferDrawnode3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		SDrawNodeRenderConstants * pDrawnode3Drc = (SDrawNodeRenderConstants *) (mappedSubresource.pData);

		Mat matModel = pDrawnode3D->MatObjectToWorld();
		pDrawnode3Drc->FillOut(matModel, matWorldToClip);

		pD3ddevicecontext->Unmap(g_game.m_cbufferDrawnode3D, 0);

		BindMaterialTextures(&material, &shader);

		pD3ddevicecontext->DrawIndexed(mesh.m_cIndicies, mesh.m_iIndexdata, mesh.m_iVertdata);
	}

}

void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader)
{
	ASSERT(pMaterial->m_aryNamedtexture.size() == pShader->m_mpISlotStrName.size());

	ID3D11DeviceContext1 * pD3ddevicecontext  = g_game.m_pD3ddevicecontext;

	std::vector<ID3D11ShaderResourceView *> arypD3dsrview;
	std::vector<ID3D11SamplerState *> arypD3dsamplerstate;

	for (int i = 0; i < pShader->CNamedslot(); i++)
	{
		arypD3dsrview.push_back(nullptr);
		arypD3dsamplerstate.push_back(nullptr);
	}

	for (const SNamedTexture & namedtexture : pMaterial->m_aryNamedtexture)
	{
		for (const SNamedTextureSlot & namedslot : pShader->m_mpISlotStrName)
		{
			if (namedslot.m_strName == namedtexture.m_strName)
			{
				ASSERT(arypD3dsrview[namedslot.m_iSlot] == nullptr);
				ASSERT(arypD3dsamplerstate[namedslot.m_iSlot] == nullptr);
				arypD3dsrview[namedslot.m_iSlot] = namedtexture.m_hTexture->m_pD3dsrview;
				arypD3dsamplerstate[namedslot.m_iSlot] = namedtexture.m_hTexture->m_pD3dsamplerstate;
			}
		}
	}

	for (int i = 0; i < pShader->CNamedslot(); i++)
	{
		ASSERT(arypD3dsrview[i] != nullptr);
		ASSERT(arypD3dsamplerstate[i] != nullptr);
	}

	pD3ddevicecontext->PSSetShaderResources(0, arypD3dsrview.size(), arypD3dsrview.data());
	pD3ddevicecontext->PSSetSamplers(0, arypD3dsamplerstate.size(), arypD3dsamplerstate.data());
}

