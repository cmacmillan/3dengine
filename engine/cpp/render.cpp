#include "render.h"
#include "engine.h"
#include "shader.h"
#include "texture.h"
#include "sun.h"

int g_cDraw3D = 0;

void Draw3DSingle(const SMaterial * pMaterial, const SMesh3D * pMesh, const Mat & matModel, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, float gMaxScale, SRgba rgba)
{
	ID3D11DeviceContext1 * pD3ddevicecontext = g_game.m_pD3ddevicecontext;
	const SShader * pShader = pMaterial->m_hShader.PT();

	if (pShader->m_data.m_shaderk == SHADERK_Error)
		return;

	Point posBoundingSphereWorld = pMesh->m_posBoundingSphereLocal * matModel;
	float sRadiusBoundingSphere = pMesh->m_sRadiusBoundingSphereLocal * gMaxScale;
	if (!FInFrustum(frustum, posBoundingSphereWorld, sRadiusBoundingSphere))
		return;

	pD3ddevicecontext->RSSetState(pShader->m_data.m_pD3drasterizerstate);
	pD3ddevicecontext->OMSetDepthStencilState(pShader->m_data.m_pD3ddepthstencilstate, 0);

	pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pD3ddevicecontext->IASetInputLayout(pShader->m_data.m_pD3dinputlayout);

	pD3ddevicecontext->VSSetShader(pShader->m_data.m_pD3dvertexshader, nullptr, 0);
	pD3ddevicecontext->PSSetShader(pShader->m_data.m_pD3dfragshader, nullptr, 0);

	ID3D11Buffer * aD3dbuffer[] = { g_game.m_cbufferGlobals, g_game.m_cbufferDrawnode3D};
	pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
	pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

	unsigned int cbVert = sizeof(SVertData3D);
	unsigned int s_cbMeshOffset = 0;

	pD3ddevicecontext->IASetVertexBuffers(0, 1, &g_game.m_cbufferVertex3D, &cbVert, &s_cbMeshOffset);	// BB don't constantly do this
	pD3ddevicecontext->IASetIndexBuffer(g_game.m_cbufferIndex, DXGI_FORMAT_R16_UINT, 0);					//  ...

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	pD3ddevicecontext->Map(g_game.m_cbufferDrawnode3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	SDrawNodeRenderConstants * pDrawnode3Drc = (SDrawNodeRenderConstants *) (mappedSubresource.pData);

	pDrawnode3Drc->FillOut(matModel, matModel.MatInverse(), matWorldToClip, matWorldToCamera, rgba);

	pD3ddevicecontext->Unmap(g_game.m_cbufferDrawnode3D, 0);

	BindMaterialTextures(pMaterial, pShader);

	pD3ddevicecontext->DrawIndexed(pMesh->m_cIndicies, pMesh->m_iIndexdata, pMesh->m_iVertdata);

	UnbindTextures(pShader);

	if (!g_game.m_fDebugDrawing) // Don't count debug draws
	{
		g_cDraw3D++;
	}
}

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, bool fDrawAsShadowcaster)
{
	ID3D11DeviceContext1 * pD3ddevicecontext = g_game.m_pD3ddevicecontext;
	for (SDrawNode3D * pDrawnode3D : *parypDrawnode3DToRender)
	{
		if (pDrawnode3D->m_hMaterial == nullptr)
			continue;

		const SMaterial * pMaterial = pDrawnode3D->m_hMaterial.PT();
		const SShader * pShader = pMaterial->m_hShader.PT();

		if (fDrawAsShadowcaster)
		{
			if (!pShader->m_data.m_fShadowcast)
				continue;

			// Swap material out for shadowcaster material

			pMaterial = g_game.m_hMaterialShadowcaster.PT();
			pShader = pMaterial->m_hShader.PT();
		}

		ASSERT(pDrawnode3D->FIsDerivedFrom(TYPEK_DrawNode3D));

		Draw3DSingle(pMaterial, pDrawnode3D->m_hMesh.PT(), pDrawnode3D->MatObjectToWorld(), matWorldToClip, matWorldToCamera, frustum, pDrawnode3D->GScaleMaxCache());

		DoNothing(); // Here so pDrawnode3D doens't go out of scope
	}
}

void UnbindTextures(const SShader * pShader)
{
	// NOTE only necessary for render textures

	ID3D11DeviceContext1 * pD3ddevicecontext  = g_game.m_pD3ddevicecontext;

	// BB Very jank that we're doing an allocation here. Perfect use case for a stack array

	std::vector<void *> arypV;

	int cNamedslot = pShader->CNamedslot();
	for (int i = 0; i < cNamedslot; i++)
	{
		arypV.push_back(nullptr);
	}

	pD3ddevicecontext->PSSetShaderResources(0, cNamedslot, reinterpret_cast<ID3D11ShaderResourceView **>(arypV.data()));
	pD3ddevicecontext->PSSetSamplers(0, cNamedslot, reinterpret_cast<ID3D11SamplerState **>(arypV.data()));
}

void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader)
{
	ASSERT(pMaterial->m_aryNamedtexture.size() == pShader->m_data.m_mpISlotStrName.size());

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
		for (const SNamedTextureSlot & namedslot : pShader->m_data.m_mpISlotStrName)
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

void BindGlobalsForCamera(SCamera3D * pCamera, SCamera3D * pCameraShadow)
{
	ID3D11DeviceContext1 * pD3ddevicecontext = g_game.m_pD3ddevicecontext;

	D3D11_MAPPED_SUBRESOURCE mappedSubresourceGlobals;
	pD3ddevicecontext->Map(g_game.m_cbufferGlobals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceGlobals);
	ShaderGlobals * pShaderglobals = (ShaderGlobals *) (mappedSubresourceGlobals.pData);
	pShaderglobals->m_t = g_game.m_syst;
	pShaderglobals->m_dT = g_game.m_dT;
	pShaderglobals->m_vecWinSize = g_game.VecWinSize();
	pShaderglobals->m_matCameraToWorld = pCamera->MatObjectToWorld();
	pShaderglobals->m_matWorldToCamera = pCamera->MatObjectToWorld().MatInverse();
	pShaderglobals->m_matClipToWorld = pCamera->MatClipToWorld();
	pShaderglobals->m_matWorldToClip = pCamera->MatWorldToClip();
	pShaderglobals->m_matWorldToShadowClip = (pCameraShadow) ? pCameraShadow->MatWorldToClip() : g_matIdentity;

	// Lights are z = towards the light in blender

	SSun * pSun = g_game.m_hSun.PT();
	pShaderglobals->m_normalSunDir = (pSun) ? pSun->VecZWorld().VecNormalized() : g_vecZAxis;

	pShaderglobals->m_xClipNear = pCamera->m_xNearClip;
	pShaderglobals->m_xClipFar = pCamera->m_xFarClip;
	pShaderglobals->m_radHFov = pCamera->m_radFovHorizontal; // NOTE will be junk when rendering orthographically
	pD3ddevicecontext->Unmap(g_game.m_cbufferGlobals, 0);
}

bool FInFrustum(const SFrustum & frustum, Point posSphere, float sRadius)
{
	return false;
	// BB this is very slow. We should be earlying out when possible, also we can precompute any of the values we need from the frustum, since those don't change
	//  Will probably need a seperate function from posclosestinfrustum that incorporates those ideas

	Point posClosest = PosClosestInFrustum(posSphere, frustum);
	float sSqrDist = SLengthSqr(posSphere - posClosest);
	return sSqrDist < sRadius * sRadius;
}

Point PosClosestInFrustum(const Point & posPoint, const SFrustum & frustum)
{
	struct SPlane
	{
		const Point * m_apPos[4];
		const Point * m_pPosInside;
	};

	SPlane aPlane[6] = 
	{
		{ &frustum.m_posMin, &frustum.m_posMinXzMaxY, &frustum.m_posMinXMaxYz, &frustum.m_posMinXyMaxZ, &frustum.m_posMax },
		{ &frustum.m_posMax, &frustum.m_posMinZMaxXy, &frustum.m_posMinYzMaxX, &frustum.m_posMinYMaxXz, &frustum.m_posMin },
		{ &frustum.m_posMin, &frustum.m_posMinXyMaxZ, &frustum.m_posMinYMaxXz, &frustum.m_posMinYzMaxX, &frustum.m_posMax },
		{ &frustum.m_posMax, &frustum.m_posMinXMaxYz, &frustum.m_posMinXzMaxY, &frustum.m_posMinZMaxXy, &frustum.m_posMin },
		{ &frustum.m_posMin, &frustum.m_posMinYzMaxX, &frustum.m_posMinZMaxXy, &frustum.m_posMinXzMaxY, &frustum.m_posMax },
		{ &frustum.m_posMax, &frustum.m_posMinYMaxXz, &frustum.m_posMinXyMaxZ, &frustum.m_posMinXMaxYz, &frustum.m_posMin }
	};

	bool fOutside = false;
	float sBest = FLT_MAX;
	Point posBest = g_posZero;
	for (int iPlane = 0; iPlane < DIM(aPlane); iPlane++)
	{
		const SPlane & plane = aPlane[iPlane];
		const Point & pos0 = *plane.m_apPos[0];
		const Point & pos1 = *plane.m_apPos[1];
		const Point & pos2 = *plane.m_apPos[2];
		const Point & pos3 = *plane.m_apPos[3];
		const Point & posInward = *plane.m_pPosInside;

		Point posResult = PosClosestInQuadToPoint(posPoint, pos0, pos1, pos2, pos3);

		Vector normalOutward = VecNormalize(VecCross(pos1 - pos0, pos2 - pos0));
		if (GDot(normalOutward, posInward - pos0) > 0.0f)
			normalOutward = -normalOutward;

		float gDot = GDot(normalOutward, posPoint-pos0);
		if (gDot > 0.0f)
		{
			fOutside = true;
			float s = SLength(posResult - posPoint);
			if (s < sBest)
			{
				sBest = s;
				posBest = posResult;
			}
		}
	}

	if (fOutside)
	{
		return posBest;
	}

	return posPoint;
}

SFrustum FrustumTransform(const SFrustum & frustum, const Mat & mat)
{
	SFrustum frustumResult;
	frustumResult.m_posMin			= frustum.m_posMin * mat;
	frustumResult.m_posMinYzMaxX	= frustum.m_posMinYzMaxX * mat;
	frustumResult.m_posMinXzMaxY	= frustum.m_posMinXzMaxY * mat;
	frustumResult.m_posMinXyMaxZ = frustum.m_posMinXyMaxZ * mat;
	frustumResult.m_posMinXMaxYz = frustum.m_posMinXMaxYz * mat;
	frustumResult.m_posMinYMaxXz = frustum.m_posMinYMaxXz * mat;
	frustumResult.m_posMinZMaxXy = frustum.m_posMinZMaxXy * mat;
	frustumResult.m_posMax = frustum.m_posMax * mat;
	return frustumResult;
}

void SFrustum::DebugDraw(SRgba rgba)
{
	g_game.DebugDrawLine(m_posMin, m_posMinYzMaxX, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMin, m_posMinXzMaxY, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMin, m_posMinXyMaxZ, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMax, m_posMinXMaxYz, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMax, m_posMinYMaxXz, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMax, m_posMinZMaxXy, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinYzMaxX, m_posMinZMaxXy, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinYzMaxX, m_posMinYMaxXz, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinXzMaxY, m_posMinXMaxYz, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinXzMaxY, m_posMinZMaxXy, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinXyMaxZ, m_posMinXMaxYz, 0.0f, rgba);
	g_game.DebugDrawLine(m_posMinXyMaxZ, m_posMinYMaxXz, 0.0f, rgba);
}
