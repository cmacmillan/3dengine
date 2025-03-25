#include "render.h"
#include "engine.h"
#include "shader.h"
#include "texture.h"
#include "sun.h"

int g_cDraw3D = 0;

void Draw3DSingle(const SMaterial * pMaterial, const SMesh3D * pMesh, const Mat & matModel, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, SRgba rgba)
{
	ID3D11DeviceContext1 * pD3ddevicecontext = g_game.m_pD3ddevicecontext;
	const SShader * pShader = pMaterial->m_hShader.PT();

	if (pShader->m_data.m_shaderk == SHADERK_Error)
		return;

	Mat matModelInv = matModel.MatInverse();
	if (!FInFrustum(frustum, matModelInv, pMesh->m_posBoundingSphereLocal, pMesh->m_sRadiusBoundingSphereLocal))
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

	pDrawnode3Drc->FillOut(matModel, matModelInv, matWorldToClip, matWorldToCamera, rgba);

	pD3ddevicecontext->Unmap(g_game.m_cbufferDrawnode3D, 0);

	BindMaterialTextures(pMaterial, pShader);

	pD3ddevicecontext->DrawIndexed(pMesh->m_cIndicies, pMesh->m_iIndexdata, pMesh->m_iVertdata);

	UnbindTextures(pShader);

	g_cDraw3D++;
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

		Draw3DSingle(pMaterial, pDrawnode3D->m_hMesh.PT(), pDrawnode3D->MatObjectToWorld(), matWorldToClip, matWorldToCamera, frustum);
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

float SDistFromLineSeg(const Point & posPoint, const Point & posLineSeg0, const Point & posLineSeg1, const Point & posInward, const Vector & normalOrtho, Point * pPosClosest)
{
	// Compute normal to generate a sign

	Vector normalOutward = VecNormalize(VecCross(posLineSeg1 - posLineSeg0, normalOrtho));
	if (GDot(normalOutward, posInward - posLineSeg0) > 0.0f)
		normalOutward = -normalOutward;

	Vector dPosPoint = posPoint - posLineSeg0;
	float sFromLine = GDot(dPosPoint, normalOutward);
	float gSignOutward = GSign(sFromLine); // outward = positive, inward = negative

	Vector dPosLineSeg = posLineSeg1 - posLineSeg0;
	float sLineSeg = SLength(dPosLineSeg);
	Vector normalLineSeg = dPosLineSeg / sLineSeg;

	float sProjected = GDot(dPosPoint, normalLineSeg);

	if (sProjected > sLineSeg)
	{
		*pPosClosest = posLineSeg1;
		return SLength(posPoint - posLineSeg1) * gSignOutward;
	}
	else if (sProjected < 0.0f)
	{
		*pPosClosest = posLineSeg0;
		return SLength(posPoint - posLineSeg0) * gSignOutward;
	}
	else
	{
		*pPosClosest = sLineSeg * normalLineSeg;
		return sFromLine * gSignOutward;
	}

}

float SPointFromPlane(const Point & posPoint, const Point & posPlane0, const Point & posPlane1, const Point & posPlane2, const Point & posPlane3, const Point & posInward, Point * pPosClosest)
{
	// Making no assumptions about winding order, except that they don't form a bow-tie (e.g. we can walk around the perimeter by going 0->1->2->0)

	Vector normalOutward = VecNormalize(VecCross(posPlane1-posPlane0, posPlane2-posPlane0));
	Vector dPosInward = posInward - posPlane0;

	if (GDot(normalOutward, dPosInward) > 0.0f)
		normalOutward = -normalOutward;

	if (GDot(posPoint - posPlane0, normalOutward) < 0.0f)
	{
		// "Inside" the plane, early out
		return -1.0f;
	}

	Point posInPlane = VecProjectOnTangent(posPoint - posPlane0, normalOutward) + posPlane0;

	// Clip to be within polygon

	float sMax = -FLT_MAX;
	Point posClosest = g_posZero;

	Point aPos[4] = { posPlane0, posPlane1, posPlane2, posPlane3 };

	for (int iPos = 0; iPos < 4; iPos++)
	{
		Point posCur;
		float sCur = SDistFromLineSeg(posInPlane, aPos[iPos], aPos[(iPos + 1) % 4], aPos[(iPos + 2) % 4], normalOutward, &posCur);

		if (sCur > sMax);
		{
			sMax = sCur;
			posClosest = posCur;
		}
	}

	if (sMax < 0.0f)
	{
		// Inside the polygon in 2d

		*pPosClosest = posInPlane;

		return GDot(posPoint - posPlane0, normalOutward);
	}
	else
	{
		// Outside the polygon in 2d

		*pPosClosest = posClosest;

		return SLength(posClosest);
	}
}

bool FInFrustum(const SFrustum & frustumWorld, const Mat & matWorldToObject, Point posSphereLocal, float sRadiusLocal)
{
	SFrustum frustumLocal = FrustumTransform(frustumWorld, matWorldToObject);

	const Point posMin			= frustumLocal.m_posMin;
	const Point posMinYzMaxX	= frustumLocal.m_posMinYzMaxX;
	const Point posMinXzMaxY	= frustumLocal.m_posMinXzMaxY;
	const Point posMinXyMaxZ	= frustumLocal.m_posMinXyMaxZ;
	const Point posMinXMaxYz	= frustumLocal.m_posMinXMaxYz;
	const Point posMinYMaxXz	= frustumLocal.m_posMinXMaxYz;
	const Point posMinZMaxXy	= frustumLocal.m_posMinXMaxYz;
	const Point posMax			= frustumLocal.m_posMax;

	// Winding clockwise (if looking in from outside)

	// Find the smallest positive value

	Point posBest;
	float sMin = FLT_MAX;

	Point posCur;
	float sMinX = SPointFromPlane(posSphereLocal, posMin, posMinXzMaxY, posMinXMaxYz, posMinXyMaxZ, posMax, &posCur); // Min X Plane
	if (sMinX >= 0.0f && sMinX < sMin)
	{
		posBest = posCur;
		sMin = sMinX;
	}

	float sMaxX = SPointFromPlane(posSphereLocal, posMax, posMinZMaxXy, posMinYzMaxX, posMinYMaxXz, posMin, &posCur); // Max X Plane
	if (sMaxX >= 0.0f && sMaxX < sMin)
	{
		posBest = posCur;
		sMin = sMaxX;
	}

	float sMinY = SPointFromPlane(posSphereLocal, posMin, posMinXyMaxZ, posMinYMaxXz, posMinYzMaxX, posMax, &posCur); // Min Y Plane
	if (sMinY >= 0.0f && sMinY < sMin)
	{
		posBest = posCur;
		sMin = sMinY;
	}

	float sMaxY = SPointFromPlane(posSphereLocal, posMax, posMinXMaxYz, posMinXzMaxY, posMinZMaxXy, posMin, &posCur); // Max Y Plane
	if (sMaxY >= 0.0f && sMaxY < sMin)
	{
		posBest = posCur;
		sMin = sMaxY;
	}
	float sMinZ = SPointFromPlane(posSphereLocal, posMin, posMinYzMaxX, posMinZMaxXy, posMinXzMaxY, posMax, &posCur); // Min Z Plane
	if (sMinZ >= 0.0f && sMinZ < sMin)
	{
		posBest = posCur;
		sMin = sMinZ;
	}

	float sMaxZ = SPointFromPlane(posSphereLocal, posMax, posMinYMaxXz, posMinXyMaxZ, posMinXMaxYz, posMin, &posCur); // Max Z Plane
	if (sMaxZ >= 0.0f && sMaxZ < sMin)
	{
		posBest = posCur;
		sMin = sMaxZ;
	}

	if (sMin == FLT_MAX)
		return true;

	// TODO incorporate posSphereLocal!!!!

	return true;
	//return sMin < sRadiusLocal;
}

SFrustum FrustumTransform(const SFrustum & frustum, const Mat & mat)
{
	SFrustum frustumResult;
	frustumResult.m_posMin			= frustum.m_posMin * mat;
	frustumResult.m_posMinYzMaxX	= frustum.m_posMinYzMaxX * mat;
	frustumResult.m_posMinXzMaxY	= frustum.m_posMinXzMaxY * mat;
	frustumResult.m_posMinXyMaxZ	= frustum.m_posMinXyMaxZ * mat;
	frustumResult.m_posMax			= frustum.m_posMax * mat;
	return frustumResult;
}
