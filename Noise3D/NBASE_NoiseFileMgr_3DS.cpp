/***********************************************************************

						Description: OBJ File Operation

************************************************************************/

#include "Noise3D.h"

/*******************************************************************

								DECLARATION

*********************************************************************/

const UINT c_chunkHeadByteLength = 6;

const std::string c_defaultTexNamePrefix = std::string("N3dSTexNAmE_");

const std::string c_defaultMatNamePrefix = std::string("N3dSMatNAmE_");

enum NOISE_3DS_CHUNKID
{
	//some of these chunks are not interested
	NOISE_3DS_CHUNKID_MAIN3D = 0x4D4D,
	NOISE_3DS_CHUNKID_3D_EDITOR_CHUNK = 0x3D3D,
	NOISE_3DS_CHUNKID_OBJECT_BLOCK = 0x4000,
	NOISE_3DS_CHUNKID_TRIANGULAR_MESH = 0x4100,
	NOISE_3DS_CHUNKID_VERTICES_LIST = 0x4110,
	NOISE_3DS_CHUNKID_FACES_DESCRIPTION = 0x4120,
	NOISE_3DS_CHUNKID_FACES_MATERIAL = 0x4130,
	NOISE_3DS_CHUNKID_MAPPING_COORDINATES_LIST = 0x4140,
	NOISE_3DS_CHUNKID_SMOOTHING_GROUP_LIST = 0x4150,
	NOISE_3DS_CHUNKID_LOCAL_COORDINATES_SYSTEM = 0x4160,
	NOISE_3DS_CHUNKID_LIGHT = 0x4600,
	NOISE_3DS_CHUNKID_SPOTLIGHT = 0x4610,
	NOISE_3DS_CHUNKID_CAMERA = 0x4700,
	NOISE_3DS_CHUNKID_MATERIAL_BLOCK = 0xAFFF,
	NOISE_3DS_CHUNKID_MATERIAL_NAME = 0xA000,
	NOISE_3DS_CHUNKID_AMBIENT_COLOR = 0xA010,
	NOISE_3DS_CHUNKID_DIFFUSE_COLOR = 0xA020,
	NOISE_3DS_CHUNKID_SPECULAR_COLOR = 0xA030,
	NOISE_3DS_CHUNKID_TEXTURE_MAP = 0xA200,
	NOISE_3DS_CHUNKID_BUMP_MAP = 0xA230,
	NOISE_3DS_CHUNKID_SPECULAR_MAP = 0xA204,
	NOISE_3DS_CHUNKID_REFLECTION_MAP = 0xA220,
	NOISE_3DS_CHUNKID_MAPPING_FILENAME = 0xA300,
	NOISE_3DS_CHUNKID_MAPPING_PARAMETERS = 0xA351,

	// Common chunks which can be found everywhere in the file
	NOISE_3DS_CHUNKID_VERSION = 0x0002,
	NOISE_3DS_CHUNKID_RGBF = 0x0010,	// float4 R; float4 G; float4 B
	NOISE_3DS_CHUNKID_RGBB = 0x0011,	// int1 R; int1 G; int B
	NOISE_3DS_CHUNKID_LINRGBF = 0x0013,	// float4 R; float4 G; float4 B
	NOISE_3DS_CHUNKID_LINRGBB = 0x0012,	// int1 R; int1 G; int B
	NOISE_3DS_CHUNKID_PERCENTW = 0x0030,	// int2   percentage
	NOISE_3DS_CHUNKID_PERCENTF = 0x0031,	// float4  percentage
};


#define BINARY_READ(var) \
fileIn.read((char*)&var,sizeof(var));


/***************************************************

				FUNCTION DEFINITION

***************************************************/

std::string DecorateMatName(std::string name);

void ReadChunk();

void	SkipCurrentChunk();

void	ReadStringFromFileA(std::string& outString);

//**********************************************
void		ParseMainChunk();//1

	void		Parse3DEditorChunk();//2

		void		ParseObjectBlock();//3

			void		ParseTriangularMeshChunk();//4

				void		ParseVerticesChunk();//5

				void		ParseFaceDescAndIndices();//5

					void		ParseFacesAndMatName();//6

				void		ParseTextureCoordinate();//5

		void	ParseMaterialBlock();//3
			
			void	ParseMaterialName();//4

			void	ParseAmbientColor();//4

			void	ParseDiffuseColor();//4

			void	ParseSpecularColor();//4

				void ReadAndParseColorChunk(NVECTOR3& outColor);//level 5 for each color chunk

			void	ParseDiffuseMap();//4

			void	ParseBumpMap();//4

			void	ParseSpecularMap();//4

				void	ReadAndParseMapChunk(std::string& outGeneratedTextureName);//level 5 for each map chunk

/***********************************************************************
Offset			Length			Description
0					2			Chunk	identifier
2					4			Chunk	length : chunk data + sub - chunks(6 + n + m)
6					n			Data
6 + n			m			Sub-chunks
*********************************************************************/

/***************************************************

Here are hierarchies of some chunks;
some chunks don't have data, only sub-chunks;
We just read interested chunks, then skip the rest.
Switch(chunkID) to decide whether to read.

MAIN CHUNK 0x4D4D
   3D EDITOR CHUNK 0x3D3D
      OBJECT BLOCK 0x4000
         TRIANGULAR MESH 0x4100
            VERTICES LIST 0x4110
            FACES DESCRIPTION 0x4120
               FACES MATERIAL 0x4130
            MAPPING COORDINATES LIST 0x4140
               SMOOTHING GROUP LIST 0x4150
            LOCAL COORDINATES SYSTEM 0x4160
         LIGHT 0x4600
            SPOTLIGHT 0x4610
         CAMERA 0x4700
      MATERIAL BLOCK 0xAFFF
         MATERIAL NAME 0xA000
         AMBIENT COLOR 0xA010
         DIFFUSE COLOR 0xA020
         SPECULAR COLOR 0xA030
         TEXTURE MAP 1 0xA200
         BUMP MAP 0xA230
         REFLECTION MAP 0xA220
         [SUB CHUNKS FOR EACH MAP]
            MAPPING FILENAME 0xA300
            MAPPING PARAMETERS 0xA351
      KEYFRAMER CHUNK 0xB000
         MESH INFORMATION BLOCK 0xB002
         SPOT LIGHT INFORMATION BLOCK 0xB007
         FRAMES (START AND END) 0xB008
            OBJECT NAME 0xB010
            OBJECT PIVOT POINT 0xB013
            POSITION TRACK 0xB020
            ROTATION TRACK 0xB021
            SCALE TRACK 0xB022
            HIERARCHY POSITION 0xB030
************************************************************/

/************************CHUNK DETAILS - FROM BLOG*****************************
http://anony3721.blog.163.com/blog/static/5119742011525103920153/

0x4D4D����chunk��ÿһ��3ds�ļ��������������ĳ���Ҳ�����ļ��ĳ��ȡ�������������chunk���༭�����͹ؼ�֡��
��chunk����
��chunk��0x3D3D��0xB000
���ȣ�ͷ����+��chunk����
���ݣ���

0x3D3D���༭����chunk���������У�������Ϣ���ƹ���Ϣ���������Ϣ�Ͳ�����Ϣ��
��chunk��0x4D4D
��chunk��0x4000��0xafff
���ȣ�ͷ����+��chunk����
���ݣ���

0x4000��������chunk�������������е�����
��chunk��0x3D3D
��chunk��0x4100
���ȣ�ͷ����+��chunk����+���ݳ���
���ݣ�
���ƣ��Կ��ֽڽ�β���ַ�����

0x4100��������Ϣ�������������ơ����㡢�桢��������ȡ�
��chunk��0x4000
��chunk��0x4110��0x4120��0x4140��0x4160
���ȣ�ͷ����+��chunk����
���ݣ���

0x4110��������Ϣ��
��chunk��0x4100
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
���������һ���֣�
�������꣨����������һ������x��y��z������*3*��������

0x4120������Ϣ��
��chunk��0x4100
��chunk��0x4130
���ȣ�ͷ����+��chunk����+���ݳ���
���ݣ�
�������һ���֣�
����������������һ������1��2��3������*3*�֣�

0x4130����������صĲ�����Ϣ��
��chunk��0x4120
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
���ƣ��Կ��ֽڽ�β���ַ�����
�������������ĸ�����һ���֣�
������������������������*�֣�

0x4140���������ꡣ
��chunk��0x4100
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
���������һ���֣�
���꣨����������һ������u��v������*2*��������

0x4160��ת������
��chunk��0x4100
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
x�������������������u��v��n��
y�������������������u��v��n��
z�������������������u��v��n��
Դ�����꣨����������x��y��z��

0xafff��������Ϣ��
��chunk��0x4D4D
��chunk��0xa000��0xa020��0xa200
���ȣ�ͷ����+��chunk����
���ݣ���

0xa000���������ơ�
��chunk��0xafff
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
���ƣ��Կ��ֽڽ�β���ַ�����


0xa020��Diffuse��
��chunk��0xafff
��chunk��0x0011��0x0012
���ȣ�ͷ����+��chunk����
���ݣ���


0xa200��������ͼ��
��chunk��0xafff
��chunk��0xa300
���ȣ�ͷ����+��chunk����
���ݣ���

0xa300����ͼ���ơ�
��chunk��0xa200
��chunk����
���ȣ�ͷ����+���ݳ���
���ݣ�
���ƣ��Կ��ֽڽ�β���ַ�����

*/


/*******************************************************************

								IMPLEMENTATION

*********************************************************************/


static uint64_t static_fileSize = 0;
static std::ifstream fileIn;
static uint64_t static_currentChunkFileEndPos = 0;

//used in generating unique Mat Name. One Mesh Loading for one unique Mesh Index
static UINT static_MeshIndex = 0;
//used in generating unique tex name
static UINT static_textureMapIndex = 0;


//--------Data From File--------
//if I use tree-like hierarchy parsing, then the REF of output buffers will be passed 
//through a lot of functions, which is troublesome. So static output Buffers are used 
//here ,rather than pass argument by ref , for convenience.
static std::string static_objectName;
static std::vector<NVECTOR3> static_verticesList;
static std::vector<NVECTOR2> static_texcoordList;
static std::vector<UINT> static_indicesList;
static std::vector<N_MeshSubsetInfo> static_subsetList;
static std::vector<N_Material> static_materialList;
static std::unordered_map<std::string,NFilePath>  static_TexName2FilePathPairList;//textures files that should be loaded








//-------------------------INTERFACE------------------------
BOOL NoiseFileManager::ImportFile_3DS(
	NFilePath pFilePath,
	std::string& outObjectName, 
	std::vector<NVECTOR3>& outVertexBuffer, 
	std::vector<NVECTOR2>& outTexCoordList, 
	std::vector<UINT>& outIndexBuffer, 
	std::vector<N_MeshSubsetInfo>& outSubsetList, 
	std::vector<N_Material>& outMaterialList,
	std::unordered_map<std::string, NFilePath>& out_TexName2FilePathPairList)
{
	static_objectName.clear();
	static_verticesList.clear();
	static_texcoordList.clear();
	static_indicesList.clear();
	static_subsetList.clear();
	static_materialList.clear();
	static_TexName2FilePathPairList.clear();

	fileIn.open(pFilePath, std::ios::binary);

	if (!fileIn.good())
	{
		DEBUG_MSG1("Noise File Manager : Import .3ds file failed!!");
		return FALSE;
	}

	fileIn.seekg(0, std::ios::end);
	static_currentChunkFileEndPos = 0;
	static_fileSize = fileIn.tellg();
	fileIn.seekg(0);

	//maybe linearly deal with chunks is also OK....
	//stack will be used when necessary because recursive solution
	//can sometimes be rewritten in non-recursive form using stack.
	while (!fileIn.eof() && fileIn.good())
	{
		ReadChunk();
	}
	fileIn.close();

	//Finish Parsing ,
	if (static_verticesList.size() == 0 || static_indicesList.size() == 0)
	{
		DEBUG_MSG1("Noise File Manager :Data Damaged!!!No Vertices or Indices loaded!");
		return FALSE;
	}

	//when material is not valid, a default subset should also be generated.
	//otherwise, the vertices wouldn't even be drawn ('Cos one subset for one draw call)
	if (static_subsetList.size() == 0)
	{
		N_MeshSubsetInfo tmpSubsetInfo;
		tmpSubsetInfo.matName = NOISE_MACRO_DEFAULT_MATERIAL_NAME;
		tmpSubsetInfo.primitiveCount = static_verticesList.size();
		tmpSubsetInfo.startPrimitiveID = 0;
		static_subsetList.push_back(tmpSubsetInfo);
	}

	//std::move transform lval into rval to avoid copying happens
	outObjectName = static_objectName;
	outIndexBuffer = std::move(static_indicesList);
	outVertexBuffer = std::move(static_verticesList);
	outTexCoordList = std::move(static_texcoordList);
	outSubsetList = std::move(static_subsetList);
	outMaterialList = std::move(static_materialList);
	out_TexName2FilePathPairList = std::move(static_TexName2FilePathPairList);

	//used to generate unique name
	++static_MeshIndex;

	return TRUE;
}



//----------------------------------------------------------------------------

//when encountering not interested chunk, use absolute end file pos of the chunk to skip
void SkipCurrentChunk()
{
	//sometimes we will encounter error / corrupted data , we must shutdown the reading 
	//of this chunk
	if (static_currentChunkFileEndPos < static_fileSize)
	{
		fileIn.seekg(static_currentChunkFileEndPos);
	}
	else
	{
		fileIn.seekg(0, std::ios::end);
	}
}

//Linearly read chunks
void ReadChunk()
{
	//because 3DS file stores data with a tree topology, so i do have considered 
	//recursive/tree-like reading...but there is some mysterious chunks which might vary
	//from time to time... ehh...linear reading will be clearer???

	const UINT c_chunkHeadByteLength = 6;

	//2 numbers at the head of every chunk
	uint16_t chunkID = 0;
	uint32_t chunkLength = 0;

	BINARY_READ(chunkID);
	BINARY_READ(chunkLength);

	//if chunk length corrupted , length data might be invalid
	if (chunkLength > static_fileSize)
	{
		static_currentChunkFileEndPos = static_fileSize - 1;
		fileIn.seekg(0, std::ios::end);
		DEBUG_MSG1("Load 3ds File : chunk is too long ! 3ds file might be damaged!!!");
		return;
	}

	uint64_t tmpFilePos = fileIn.tellg();
	static_currentChunkFileEndPos = tmpFilePos + chunkLength - c_chunkHeadByteLength;

	//in CURRENT chunk, there can be several sub-chunks to read
	switch (chunkID)
	{
	case NOISE_3DS_CHUNKID_MAIN3D:
		ParseMainChunk();//level1
		break;

	case NOISE_3DS_CHUNKID_3D_EDITOR_CHUNK:
		Parse3DEditorChunk();//level2
		break;

	case NOISE_3DS_CHUNKID_OBJECT_BLOCK:
		ParseObjectBlock();//level3
		break;

	case NOISE_3DS_CHUNKID_TRIANGULAR_MESH:
		ParseTriangularMeshChunk();//level4
		break;

	case NOISE_3DS_CHUNKID_VERTICES_LIST:
		ParseVerticesChunk();//level5
		break;

	case NOISE_3DS_CHUNKID_FACES_DESCRIPTION:
		ParseFaceDescAndIndices();//level5
		break;

	case NOISE_3DS_CHUNKID_FACES_MATERIAL:
		ParseFacesAndMatName();//level 6
		break;

	case NOISE_3DS_CHUNKID_MAPPING_COORDINATES_LIST:
		ParseTextureCoordinate();//level5
		break;

	case NOISE_3DS_CHUNKID_MATERIAL_BLOCK:
		ParseMaterialBlock();//level 3
		break;

	case NOISE_3DS_CHUNKID_MATERIAL_NAME:
		ParseMaterialName();//level 4
		break;

	case NOISE_3DS_CHUNKID_AMBIENT_COLOR:
		ParseAmbientColor();//level 4
		break;

	case NOISE_3DS_CHUNKID_DIFFUSE_COLOR:
		ParseDiffuseColor();//level 4
		break;

	case NOISE_3DS_CHUNKID_SPECULAR_COLOR:
		ParseSpecularColor();//level 4
		break;

	case NOISE_3DS_CHUNKID_TEXTURE_MAP:
		ParseDiffuseMap();//level 4
		break;

	case NOISE_3DS_CHUNKID_BUMP_MAP:
		ParseBumpMap();//level 4
		break;

	case NOISE_3DS_CHUNKID_SPECULAR_MAP:
		ParseSpecularMap();//level 4
		break;

		/*case NOISE_3DS_CHUNKID_REFLECTION_MAP:
		ParseReflectionMap();//4
		break;*/

	default:
		//not interested chunks , skip
		SkipCurrentChunk();
		break;
	}

}

//orderly read a string from file at current position
void ReadStringFromFileA(std::string & outString)
{
	do
	{
		char tmpChar = 0;
		BINARY_READ(tmpChar);

		if (tmpChar)
		{
			outString.push_back(tmpChar);
		}
		else
		{
			break;
		}
	} while (true);
}

void	ReadStringFromFileW(std::wstring& outString)
{
	do
	{
		char tmpChar = 0;
		BINARY_READ(tmpChar);

		if (tmpChar)
		{
			outString.push_back(tmpChar);
		}
		else
		{
			break;
		}
	} while (true);
}

//father: xxx color chunk
//child:none
void ReadAndParseColorChunk(NVECTOR3 & outColor)
{
	//please refer to project "Assimp" for more chunk information

	uint16_t chunkID = 0;
	uint32_t chunkLength = 0;
	BINARY_READ(chunkID);
	BINARY_READ(chunkLength);

	switch (chunkID)
	{
	case NOISE_3DS_CHUNKID_RGBF://float
	{
		BINARY_READ(outColor.x);
		BINARY_READ(outColor.y);
		BINARY_READ(outColor.z);
	}
	break;

	case NOISE_3DS_CHUNKID_RGBB://byte
	{
		uint8_t colorByte = 0;
		BINARY_READ(colorByte);
		outColor.x = float(colorByte) / 255.0f;
		BINARY_READ(colorByte);
		outColor.y = float(colorByte) / 255.0f;
		BINARY_READ(colorByte);
		outColor.z = float(colorByte) / 255.0f;
	}
	break;

	case NOISE_3DS_CHUNKID_PERCENTF://grey scale??
	{
		float colorPercentF = 0.0f;
		BINARY_READ(colorPercentF);
		outColor.x = outColor.y = outColor.z = colorPercentF;
	}
	break;

	case NOISE_3DS_CHUNKID_PERCENTW: //grey scale??
	{
		uint8_t colorByte = 0;
		BINARY_READ(colorByte);
		outColor.x = outColor.y = outColor.z = float(colorByte) / 255.0f;
	}
	break;

	default:
		//skip this color chunk
		fileIn.seekg(chunkLength - c_chunkHeadByteLength, std::ios::cur);
		return;
	}
}

//father: xxx map chunk
//child:none
void ReadAndParseMapChunk(std::string& outGeneratedTextureName)
{
	//please refer to project "Assimp" for more chunk information

	uint16_t chunkID = 0;
	uint32_t chunkLength = 0;
	BINARY_READ(chunkID);
	BINARY_READ(chunkLength);

	/*case Discreet3DS::CHUNK_PERCENTF:
	case Discreet3DS::CHUNK_PERCENTW:
	case Discreet3DS::CHUNK_MAT_MAP_USCALE:
	case Discreet3DS::CHUNK_MAT_MAP_VSCALE:
	case Discreet3DS::CHUNK_MAT_MAP_UOFFSET:
	case Discreet3DS::CHUNK_MAT_MAP_VOFFSET:
	case Discreet3DS::CHUNK_MAT_MAP_ANG:
	case Discreet3DS::CHUNK_MAT_MAP_TILING:*/

	switch (chunkID)
	{
	case NOISE_3DS_CHUNKID_MAPPING_FILENAME:
	{
		std::string texFilePath;
		//back() can retrieve the current processing material
		std::string texName=c_defaultTexNamePrefix + std::to_string(static_textureMapIndex);
		++static_textureMapIndex;

		//...Read File Path From File
		ReadStringFromFileA(texFilePath);

		//texName-filePath pair,so that texture creation could be done
		static_TexName2FilePathPairList.insert(std::make_pair(texName,texFilePath.c_str()));

		outGeneratedTextureName = std::move(texName);
	}
		break;

	default:
		//skip this color chunk
		fileIn.seekg(chunkLength - c_chunkHeadByteLength, std::ios::cur);
		break;
	}

}

//*****************************************************************

//*************************************************
//father : none
//child : 3D Editor Chunk
void ParseMainChunk()
{
	//no data in this chunk ,only need to go into sub-chunks
};

//*************************************************
//father : Main3D
//child :	3D EDITOR CHUNK 0x3D3D
//			MATERIAL BLOCK 0xAFFF
void Parse3DEditorChunk()
{
	//no data in this chunk ,only need to go into sub-chunks
};

//*************************************************
//father : 3D Editor Chunk
//child : TriangularMesh,  (Light , Camera)
void ParseObjectBlock()
{
	//data: object name
	ReadStringFromFileA(static_objectName);
};

//*************************************************
//father : Object Block
//child : VerticesChunk, FaceDescription(Indices) , TextureCoordinate
void ParseTriangularMeshChunk()
{
	//no data in this chunk ,only need to go into sub-chunks
}

//*************************************************
//father::Triangular Mesh Chunk
//child: none
void ParseVerticesChunk()
{
	//data: Vertices List:
	uint16_t verticesCount = 0;//unsigned short
	BINARY_READ(verticesCount);

	static_verticesList.reserve(verticesCount);
	for (UINT i = 0;i < verticesCount;i++)
	{
		NVECTOR3 tmpVertex(0, 0, 0);
		BINARY_READ(tmpVertex.x);
		BINARY_READ(tmpVertex.y);
		BINARY_READ(tmpVertex.z);
		static_verticesList.push_back(tmpVertex);
	}

}

//*************************************************
//father::Triangular Mesh Chunk
//child: FACES MATERIAL 0x4130
void ParseFaceDescAndIndices()
{
	//data:faces count + n * (3*indices+faceDesc)
	uint16_t faceCount = 0;
	BINARY_READ(faceCount);

	static_indicesList.reserve(faceCount);

	//I want to combine various object into one 
	uint16_t indexOffset = uint16_t(static_indicesList.size());
	for (UINT i = 0;i < faceCount;i++)
	{
		//1 face for 3 indices
		uint16_t tmpIndex = 0;

		for (UINT i = 0;i < 3;i++)
		{
			BINARY_READ(tmpIndex);
			static_indicesList.push_back(indexOffset + tmpIndex);
		}

		uint16_t faceFlag = 0;
		//face option,side visibility , etc.
		BINARY_READ(faceFlag);
	}

}

//*************************************************
//father::FACES DESCRIPTION 0x4120
//child: none
void ParseFacesAndMatName()
{
	//i guess one indices(faces) block can
	//possess several this kind of FaceMaterial Block ...

	//data : 
	//1. material name (end with '/0')
	//2. faces count that linked to current material (ushort) 
	//3. FACE indices

	std::string matName("");

	std::vector<N_MeshSubsetInfo> currMatSubsetList;//faces can be 
	std::vector<uint16_t> static_indicesList;

	//material name
	ReadStringFromFileA(matName);

	//faces that link to this material
	uint16_t faceCount = 0;
	BINARY_READ(faceCount);

	//read FACE indices block
	static_indicesList.reserve(faceCount);
	for (UINT i = 0;i < faceCount;i++)
	{
		uint16_t tmpFaceIndex = 0;
		BINARY_READ(tmpFaceIndex);
		static_indicesList.push_back(tmpFaceIndex);
	}

	if (static_indicesList.size() == 0)
	{
		DEBUG_MSG1( "face Material Info : no faces attached to this material!!");
		return;
	}

	//generate subsets list (for current Material)
	N_MeshSubsetInfo newSubset;
	newSubset.matName = DecorateMatName(matName);
	newSubset.primitiveCount = 0;
	newSubset.startPrimitiveID = static_indicesList.at(0);
	currMatSubsetList.push_back(newSubset);
	for (UINT i = 0;i < static_indicesList.size();i++)
	{
		UINT startTriangleID = currMatSubsetList.back().startPrimitiveID;
		UINT endTriangleID = startTriangleID + currMatSubsetList.back().primitiveCount - 1;
		//region growing (extend subset region)
		//if next index is adjacent to current region ,then grow
		if (endTriangleID + 1 == static_indicesList[i])
		{
			currMatSubsetList.back().primitiveCount += 1;
		}
		else
		{
			//this index isn't adjacent to the growing region
			//so create a new region
			newSubset.matName = matName;
			newSubset.primitiveCount = 1;
			newSubset.startPrimitiveID = static_indicesList.at(i);
			currMatSubsetList.push_back(newSubset);
		}
	}

	//add to global subset list
	for (auto subset : currMatSubsetList)
	{
		static_subsetList.push_back(subset);
	}

}

//*************************************************
//father::Triangular Mesh
//child: smoothing group
void ParseTextureCoordinate()
{
	//data: 
	//1.texcoord count (2byte)
	//2.  n * (u + v) ,2n float
	uint16_t texcoordCount = 0;
	BINARY_READ(texcoordCount);

	for (UINT i = 0;i < texcoordCount;i++)
	{
		NVECTOR2 tmpTexCoord(0, 0);
		BINARY_READ(tmpTexCoord.x);
		BINARY_READ(tmpTexCoord.y);
		static_texcoordList.push_back(tmpTexCoord);
	}

}

//*************************************************
//father:3D Editor Chunk 
//child: MATERIAL NAME 0xA000
//         AMBIENT COLOR 0xA010
//			DIFFUSE COLOR 0xA020
//			SPECULAR COLOR 0xA030
//			TEXTURE MAP 1 0xA200
//			BUMP MAP 0xA230
//			REFLECTION MAP 0xA220
void ParseMaterialBlock()
{
	//Data:None

	//but this identifier signify creation a new material
	N_Material tmpMat;
	static_materialList.push_back(tmpMat);
}

//*************************************************
//father:Material block
//child:  none
void ParseMaterialName()
{
	//data: material name
	std::string orginalMatName;
	ReadStringFromFileA(orginalMatName);
	static_materialList.back().mMatName = DecorateMatName(orginalMatName);
}

//*************************************************
//father:Material Block
//child	: sub color chunk
void ParseAmbientColor()
{
	//data:sub chunks

	uint64_t filePos = 0;
	do
	{
		ReadAndParseColorChunk(static_materialList.back().baseMaterial.mBaseAmbientColor);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}

//father:Material Block
//child	: sub color chunk
void ParseDiffuseColor()
{
	uint64_t filePos = 0;
	do
	{
		ReadAndParseColorChunk(static_materialList.back().baseMaterial.mBaseDiffuseColor);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}

//father:Material Block
//child	: sub color chunk
void ParseSpecularColor()
{
	uint64_t filePos = 0;
	do
	{
		ReadAndParseColorChunk(static_materialList.back().baseMaterial.mBaseSpecularColor);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}

//father:Material Block
//child	: Mapping File Path  (Map option) 
void ParseDiffuseMap()
{
	//data:sub chunks
	uint64_t filePos = 0;
	do
	{
		std::string texName;
		//mapName-mapFilePath pair will be added in this function
		ReadAndParseMapChunk(static_materialList.back().diffuseMapName);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}

//father:Material Block
//child	: Mapping File Path  (Map option) 
void ParseBumpMap()
{
	//data:sub chunks
	uint64_t filePos = 0;
	do
	{
		//mapName-mapFilePath pair will be added in this function
		ReadAndParseMapChunk(static_materialList.back().normalMapName);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}

//father:Material Block
//child	: Mapping File Path  (Map option) 
void ParseSpecularMap()
{
	//data:sub chunks
	uint64_t filePos = 0;
	do
	{
		//mapName-mapFilePath pair will be added in this function
		ReadAndParseMapChunk(static_materialList.back().specularMapName);
		filePos = fileIn.tellg();
	} while (filePos<static_currentChunkFileEndPos);
}


std::string DecorateMatName(std::string name) { return c_defaultMatNamePrefix + name + std::to_string(static_MeshIndex); };
