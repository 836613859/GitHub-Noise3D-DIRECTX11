
/***********************************************************************

                           h�� Texture manager

************************************************************************/
#pragma once

namespace Noise3D
{


	class /*_declspec(dllexport)*/ ITextureManager :
		public IFactory<ITexture>
	{
	public:
		ITexture*		CreatePureColorTexture(N_UID texName, UINT pixelWidth, UINT pixelHeight, NVECTOR4 color, bool keepCopyInMemory = false);

		ITexture*		CreateTextureFromFile(NFilePath filePath, N_UID texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight, bool keepCopyInMemory = false);

		ITexture*		CreateCubeMapFromFiles(NFilePath fileName[6], N_UID cubeTextureName, NOISE_CUBEMAP_SIZE faceSize);

		ITexture*		CreateCubeMapFromDDS(NFilePath dds_FileName, N_UID cubeTextureName, NOISE_CUBEMAP_SIZE faceSize);

		ITexture*		GetTexture(N_UID texName);

		UINT			GetTextureCount();

		bool			DeleteTexture(ITexture* pTex);

		bool			DeleteTexture(N_UID texName);

		void				DeleteAllTexture();

		bool			ValidateUID(N_UID texName);

		bool			ValidateUID(N_UID texName, NOISE_TEXTURE_TYPE texType);

	private:

		friend class IRenderer;

		friend IFactory<ITextureManager>;

		//���캯��
		ITextureManager();

		~ITextureManager();

		ITexture*		mFunction_CreateTextureFromFile_DirectlyLoadToGpu(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight);

		ITexture*		mFunction_CreateTextureFromFile_KeepACopyInMemory(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight);
	};
}