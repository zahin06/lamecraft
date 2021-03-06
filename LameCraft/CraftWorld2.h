#ifndef CraftWorld_H
#define CraftWorld_H

#include <string>

#include <Aurora/Math/Frustum.h>

#include "Blocks.h"
#include "SimpleMeshChunk2.h"

typedef struct
{
	int distance;
	int chunk;
}TransparentOrder;

class CraftWorld
{
public:

	enum BlockSettings
	{
		 OpLighSource = 0x10,
		 OpSideRender = 0x20,
		 OpActAsLadder = 0x40,
		 OpLightTraveled = 0x80
	};

	CraftWorld();
	~CraftWorld();

	block_t& GetBlock (const int x, const int y, const int z) ;
	block_t& GetBlockLight (const int x, const int y, const int z);
	block_t& GetBlockSettings (const int x, const int y, const int z);
	block_t BlockAtPoint(Vector3 pos);
	
	//map creation
	void initWorld(int worldSize,int chunkSize);
	void initWorldBlocksLight();
	void setTextureSize(int texture,int chunk);


	void GetSpecialBlockVerts(int i,BaseBlock *blockType);
	void GetNormalBlockVerts(int i,BaseBlock *blockType);
	void buildblocksVerts();

	//chunks handling
	void createChunks(const int StartX, const int StartY, const int StartZ);
	void createTransparentChunks(const int StartX, const int StartY, const int StartZ);
	void rebuildChunk(int id);
	void rebuildTransparentChunk(int id);
	void rebuildNearestChunks(int id,Vector3 pos);
	void createWorldChunkss();
	void SetAllChunksToUpdate();

	void GetSpecialBlock(int x,int y, int z,int &iVertex,SimpleMeshChunk* MeshChunk,block_t Block,bool transparent);
	void GetNormalBlock(int x,int y, int z,int &iVertex,SimpleMeshChunk* MeshChunk,block_t Block,bool transparent);

	//load/save options
	void SaveCompressedWorld(std::string filename);
	void LoadWorld(const char *filename);
	void LoadCompressedWorld(std::string filename);

	//lightening
	void InitLightValues();
	void SetWolrdTime(float time);
	void LightTravel(int x,int y,int z,int steps,int lightLevel);

	//
	void buildMap();
	void destroyMap();

	//rendering
	void drawWorld(Frustum &camFrustum,bool camUpdate);
	void drawCubes(int i);

	//pickin,collision etc
	int groundHeight (const int x, const int z);
	int getChunkId(Vector3 pos);
	int BlockSoundAtPos(Vector3 pos);

	bool SolidAtPointForPlayer(Vector3 pos);
	bool SolidAtPoint(Vector3 pos);
	bool PlayerInWater(Vector3 pos);

	bool BlockTransparentOrSpecial(const int x, const int y, const int z);
	bool BlockTransparentOrLightSource(const int x, const int y, const int z);
	bool BlockTransparent(const int x, const int y, const int z);
	bool BlockSpecial(const int x, const int y, const int z);
	bool BlockEditable(const int x, const int y, const int z);
	
	bool isSolid(const int x, const int y, const int z);

	bool LightSourceBlock(int id);
	bool CanPutBlockHere(const int x, const int y, const int z,int blockID);
	void SetLigtSourcePosition(const int x, const int y, const int z,int blockID);
	void CheckForTorchSupport(const int x, const int y, const int z,int blockID);
	void RemoveLigtSourceAtPosition(const int x, const int y, const int z,int blockID);
	void AddChunkToUpdate(const int x, const int y, const int z);
	void RebuildChunksLight(Vector3 pos,int currentChunk,int blockID);
	void UpdateLightAreaIn(Vector3 pos);
	void UpdateWorldLightForChunk(int chunkID);

	int GetDrawntTrianglesCount() { return drawnTriangles; }

	Vector3 GetPlayerPos();
	void UpdatePlayerZoneBB(Vector3 playerPosition);
	void UpdateWorldTime(float dt);
	int GetBlockTypesCount();



	char worldName[50];
	int createdChunksCount;
	Vector3	fogColor;
	float worldDayTime;
	float worldTime;

	bool freezeDayTime;
	float sunTime;
	int worldVersion;

private:

	int WORLD_SIZE;
	int CHUNK_SIZE;

	int texutreSize;
	int textureChunk;
	float percent;

	std::vector<SimpleMeshChunk*> mChunks;
	std::vector<SimpleMeshChunk*> mTransparentChunks;

	block_t* m_Blocks;
	block_t* m_BlockLight;
	block_t* m_BlockSettings;

	//player zone aabb
	BoundingBox playerZoneBB;
	Vector3 playerZoneSize;
	Vector3 playerPos;
	int chunksCreatedInFrameCount;
	int transparentchunksCreatedInFrameCount;
	float updateChunkTimer;
	bool updateChunksSwitch;

	//world time

	float worldHour;//1 game hour = 50 real seconds = 24 game hours = 20 real minutes

	//information
	int drawnTriangles;

	//transparent order rendering
	TransparentOrder transOrder[512];
	int transOrderCont;
	float lightShadowFactor;
	Vector3 lightFactor;

	//lightining
	Vector3 lightColor;
	Vector3	ambientColor;
	float lightAngle;
	float factor1;
	float factor2;
	float factor3;
};


#endif
