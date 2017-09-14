
/***********************************************************************

								h��Model Loader (mainly from file)

************************************************************************/

#pragma once

namespace Noise3D
{
	class IMesh;
	class IAtmosphere;

	class /*_declspec(dllexport)*/ IModelLoader
	{
	public:

		bool		LoadPlane(IMesh* pTargetMesh, float fWidth, float fDepth, UINT iRowCount = 5, UINT iColumnCount = 5);

		bool		LoadBox(IMesh* pTargetMesh, float fWidth, float fHeight, float fDepth, UINT iDepthStep = 3, UINT iWidthStep = 3, UINT iHeightStep = 3);

		bool		LoadSphere(IMesh* pTargetMesh, float fRadius, UINT iColumnCount = 20, UINT iRingCount = 20);

		bool		LoadCylinder(IMesh* pTargetMesh,float fRadius, float fHeight, UINT iColumnCount = 40, UINT iRingCount = 8);

		bool		LoadCustomizedModel(IMesh* pTargetMesh, const std::vector<N_DefaultVertex>& vertexList, const std::vector<UINT>& indicesList);

		bool		LoadFile_STL(IMesh* pTargetMesh, NFilePath pFilePath);

		bool		LoadFile_OBJ(IMesh* pTargetMesh, NFilePath pFilePath);

		//bool		LoadFile_3DS(NFilePath pFilePath, std::vector<IMesh*>& outMeshPtrList, std::vector<N_UID>& outMeshNameList);

		bool		LoadFile_FBX(NFilePath pFilePath, std::vector<IMesh*>& outMeshPtrList, std::vector<N_UID>& outMeshNameList);

		bool		LoadSkyDome(IAtmosphere* pAtmo, float fRadiusXZ, float fHeight);

		bool		LoadSkyBox(IAtmosphere* pAtmo, float fWidth, float fHeight, float fDepth);

	private:

		friend class IFactory<IModelLoader>;

		IModelLoader();

		~IModelLoader();

		//internal mesh loading helper
		IFileIO mFileIO;
		IGeometryMeshGenerator mMeshGenerator;

	};

};