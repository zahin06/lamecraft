#include "CraftWorld2.h"
#include <Aurora/Graphics/RenderManager.h>
#include <Aurora/Utils/Logger.h>

#include <string.h>
#include <zlib.h>  

CraftWorld::CraftWorld()
{
	WORLD_SIZE = 0;
	CHUNK_SIZE = 0;
	//
	playerZoneSize = Vector3(48,32,48);
	updateChunksSwitch = false;
	worldTime = 0.0f;
	updateChunkTimer = 0.0f;
	chunksCreatedInFrameCount = 0;
	transparentchunksCreatedInFrameCount = 0;

	transOrderCont = 0;
	lightShadowFactor = 0.4f;
	lightFactor = Vector3(1.0f,1.0f,1.0f);
	worldTime = 0.0f;

	worldDayTime = 12.0f;
	worldHour = 0.0f;

	freezeDayTime = false;
	worldVersion = 3;
}

CraftWorld::~CraftWorld()
{
	//delete arrays
	delete [] m_Blocks;
	delete [] m_BlockLight;
	delete [] m_BlockSettings;

	//delete chunks
	for(unsigned int i = 0;i < mChunks.size();i++)
	{
		delete mChunks[i];
	}
	mChunks.clear();

	//water chunks
	for(unsigned int i = 0;i < mTransparentChunks.size();i++)
	{
		delete mTransparentChunks[i];
	}
	mTransparentChunks.clear();

}

void CraftWorld::SaveCompressedWorld(std::string  filename)
{
	//first save version and name on normal file
	FILE * pFile;
	pFile = fopen(filename.c_str(),"wb");

	if(pFile == NULL)
		return;

	//version
	fwrite(&worldVersion,sizeof(int),1,pFile);

	//name
	fwrite(worldName,sizeof(char),50,pFile);

	//close file
	fclose(pFile);

	//now save compressed map info
	filename = filename + "c";

	//open file
	gzFile saveFile;
	saveFile = gzopen(filename.c_str(),"wb");
	if(saveFile == 0)
		return;

	//player pos
	gzwrite(saveFile, &playerPos.x,sizeof(float));
	gzwrite(saveFile, &playerPos.y,sizeof(float));
	gzwrite(saveFile, &playerPos.z,sizeof(float));
		
	//size
	gzwrite(saveFile, &WORLD_SIZE,sizeof(int));

	//chunksize
	gzwrite(saveFile, &CHUNK_SIZE,sizeof(int));

	//cubes
	gzwrite(saveFile, m_Blocks,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

	//light
	gzwrite(saveFile, m_BlockLight,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

	//settings
	gzwrite(saveFile, m_BlockSettings,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

	//world time
	gzwrite(saveFile, &worldDayTime,sizeof(float));

	//freeze world time
	gzwrite(saveFile, &freezeDayTime,sizeof(bool));

	//sun time
	gzwrite(saveFile, &sunTime,sizeof(float));
		
	//close file
	gzclose(saveFile);
	saveFile=0;
}

void CraftWorld::LoadWorld(const char *filename)
{
	FILE * pFile;
	pFile = fopen(filename,"rb");

	if(pFile == NULL)
		return;

	//version
	fread(&worldVersion,sizeof(int),1,pFile);

	//name
	fread(worldName,sizeof(char),50,pFile);

	//size
	fread(&WORLD_SIZE,sizeof(int),1,pFile);

	//chunksize
	fread(&CHUNK_SIZE,sizeof(int),1,pFile);

	//cubes
	m_Blocks = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	fread(m_Blocks,sizeof(unsigned char),(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE),pFile);

	//light
	m_BlockLight = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	fread(m_BlockLight,sizeof(unsigned char),(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE),pFile);

	m_BlockSettings = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	memset(m_BlockSettings, 0, sizeof(block_t) * WORLD_SIZE * WORLD_SIZE * WORLD_SIZE);

	//close file
	fclose(pFile);
}

void CraftWorld::LoadCompressedWorld(std::string  filename)
{
	//load normal save info
	FILE * pFile;
	pFile = fopen(filename.c_str(),"rb");

	if(pFile == NULL)
		return;

	//version
	fread(&worldVersion,sizeof(int),1,pFile);

	//name
	fread(worldName,sizeof(char),50,pFile);
	//close file
	fclose(pFile);

	//load compressed map data
	filename = filename + "c";

	//open file
	gzFile saveFile;
	saveFile = gzopen(filename.c_str(),"rb");
	if(saveFile == 0)
		return;

	//player pos
	gzread(saveFile, &playerPos.x,sizeof(float));
	gzread(saveFile, &playerPos.y,sizeof(float));
	gzread(saveFile, &playerPos.z,sizeof(float));

	//size
	gzread(saveFile, &WORLD_SIZE,sizeof(int));

	//chunksize
	gzread(saveFile, &CHUNK_SIZE,sizeof(int));

	//cubes
	m_Blocks = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	gzread(saveFile, m_Blocks,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

	//light
	m_BlockLight = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	gzread(saveFile, m_BlockLight,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

	//settings
	m_BlockSettings = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	if(worldVersion >= 3)
	{
		//settings
		gzread(saveFile, m_BlockSettings,sizeof(unsigned char)*(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE));

		//world time
		gzread(saveFile, &worldDayTime,sizeof(float));

		//freeze world time?
		gzread(saveFile, &freezeDayTime,sizeof(bool));

		//sun time
		gzread(saveFile, &sunTime,sizeof(float));

	}else
	{
		memset(m_BlockSettings, 0, sizeof(block_t) * WORLD_SIZE * WORLD_SIZE * WORLD_SIZE);
	}

	//close file
	gzclose(saveFile);
	saveFile=0;
}

void CraftWorld::initWorld(int worldSize,int chunkSize)
{
	//inicjuj mape
	WORLD_SIZE = worldSize;
	CHUNK_SIZE = chunkSize;

	m_Blocks = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	memset(m_Blocks, 0, sizeof(block_t) * WORLD_SIZE * WORLD_SIZE * WORLD_SIZE);

	m_BlockLight = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	memset(m_BlockLight, 255, sizeof(block_t) * WORLD_SIZE * WORLD_SIZE * WORLD_SIZE);

	m_BlockSettings = new block_t[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
	memset(m_BlockSettings, 0, sizeof(block_t) * WORLD_SIZE * WORLD_SIZE * WORLD_SIZE);
}

void CraftWorld::initWorldBlocksLight()
{
	int x, y, z;
	block_t Light;
	block_t DeltaLight = 16;

	for (z = 0; z < WORLD_SIZE; ++z)
	{
		for (x = 0; x < WORLD_SIZE; ++x)
		{
			Light = 255;

			for (y = WORLD_SIZE - 1; y >= 0; --y)
			{
				GetBlockLight(x, y, z) = Light;

				if (GetBlock(x, y, z) != 0)
				{
					if (Light >= DeltaLight)
						Light -= DeltaLight;
				}
			}
		}
	}
}

void CraftWorld::InitLightValues()
{
	lightAngle = 10;
	lightColor = Vector3(1, 1, 1);
	fogColor = lightColor * 0.8f;
	ambientColor = lightColor / 3.0f;

	//light face factors
	if (lightAngle >= 0 &&lightAngle <= 180)
		factor1 = sinf(lightAngle * 3.1415926f / 180.0f);
	else
		factor1 = 0.0f;

	if(factor1 < 0.1f)
		factor1 = 0.1f;

	if (lightAngle >= 315 || lightAngle <= 45)
		factor2 = fabs(cosf(lightAngle * 3.1415926f / 180.0f));
	else
		factor2 = fabs(sinf(lightAngle * 3.1415926f / 180.0f));

	if (factor2 < 0.1f)
		factor2 = 0.1f;

	if (lightAngle >= 135 && lightAngle <= 225)
		factor3 = fabs(cosf(lightAngle * 3.1415926f / 180.0f));
	else
		factor3 = fabs(sinf(lightAngle * 3.1415926f / 180.0f));

	if (factor3 < 0.1f)
		factor3 = 0.1f;
}

void CraftWorld::SetWolrdTime(float time)
{
	//6am = SunRise
	//Light is symmetric about noon
	//4am-8am = dawn
	//4am color = (0.1, 0.1, 0.1)
	//6am color = (1, 0.6, 0.04)
	//8am color = (1, 1, 1)

	worldDayTime = time;//std::fmod(time, 24);

	lightAngle = worldDayTime / 24.0f * 360.0f - 90.0f;
	if (lightAngle < 0)lightAngle += 360.0f;

	if (worldDayTime <= 4 || worldDayTime >= 22)
	{
		lightColor = Vector3(0.1f, 0.1f, 0.1f);
	}
	else if (worldDayTime >= 8 && worldDayTime <= 16)
	{
		lightColor = Vector3(1, 1, 1);
	}
	else if (worldDayTime >= 4 && worldDayTime <= 6)
	{
		lightColor.x = (worldDayTime - 4.0f)/2.0f * 0.9f + 0.1f;
		lightColor.y = (worldDayTime - 4.0f)/2.0f * 0.5f + 0.1f;
		lightColor.z = (worldDayTime - 4.0f)/2.0f * -0.06f + 0.1f;
	}
	else if (worldDayTime >= 6 && worldDayTime <= 8)
	{
		lightColor.x = 1.0f;
		lightColor.y = (worldDayTime - 6.0f)/2.0f * 0.4f + 0.6f;
		lightColor.z = (worldDayTime - 6.0f)/2.0f * 0.96f + 0.04f;
	}
	else if (worldDayTime >= 16 && worldDayTime <= 18)
	{
		lightColor.x = 1.0f;
		lightColor.y = (18.0f - worldDayTime)/2.0f * 0.4f + 0.6f;
		lightColor.z = (18.0f - worldDayTime)/2.0f * 0.96f + 0.04f;
	}
	else if (worldDayTime >= 18 && worldDayTime <= 22)
	{
		lightColor.x = (20.0f - worldDayTime)/2.0f * 0.9f + 0.3f;
		lightColor.y = (20.0f - worldDayTime)/2.0f * 0.5f + 0.3f;
		lightColor.z = (20.0f - worldDayTime)/2.0f * -0.06f + 0.3f;
	}
	else	//Shouldn't get here
	{
		lightColor = Vector3(1, 1, 1);
	}

	//ambientColor = lightColor / 3.0f;
	ambientColor = Vector3(0.05f,0.05f,0.05f);

	fogColor = lightColor * 0.80f;

	//light face factors
	if (lightAngle >= 0 &&lightAngle <= 180)
		factor1 = sinf(lightAngle * 3.1415926f / 180.0f);
	else
		factor1 = 0.0f;

	if(factor1 < 0.1f)
		factor1 = 0.1f;

	if (lightAngle >= 315 || lightAngle <= 45)
		factor2 = fabs(cosf(lightAngle * 3.1415926f / 180.0f));
	else
		factor2 = fabs(sinf(lightAngle * 3.1415926f / 180.0f));

	if (factor2 < 0.1f)
		factor2 = 0.1f;

	if (lightAngle >= 135 && lightAngle <= 225)
		factor3 = fabs(cosf(lightAngle * 3.1415926f / 180.0f));
	else
		factor3 = fabs(sinf(lightAngle * 3.1415926f / 180.0f));

	if (factor3 < 0.1f)
		factor3 = 0.1f;
}

void CraftWorld::buildMap()
{
	createWorldChunkss();
}

void CraftWorld::destroyMap()
{
	for(unsigned int i = 0;i < mChunks.size();i++)
	{
		mChunks[i]->reset();
	}

	for(unsigned int i = 0;i < mTransparentChunks.size();i++)
	{
		mTransparentChunks[i]->reset();
	}
}

void CraftWorld::setTextureSize(int texture,int chunk)
{
	texutreSize = texture;
	textureChunk = chunk;

	percent = (float)textureChunk / (float)texutreSize;
}

block_t& CraftWorld::GetBlock (const int x, const int y, const int z) 
{
	return m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE];
}

block_t& CraftWorld::GetBlockLight (const int x, const int y, const int z)
{
	return m_BlockLight[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE];
}

block_t& CraftWorld::GetBlockSettings(const int x, const int y, const int z)
{
	return m_BlockSettings[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE];
}

void CraftWorld::CheckForTorchSupport(const int x, const int y, const int z,int blockID)
{
	for(int zz = z-1;zz < z+2;zz++)
	{
		for(int xx = x-1;xx < x+2;xx++)
		{
			for(int yy = y-1;yy < y+2;yy++)
			{
				if(GetBlock(xx,yy,zz) == Torch::getID())
				{
					if(!CanPutBlockHere(xx,yy,zz,Torch::getID()))
					{
						GetBlock(xx,yy,zz) = 0;
						RemoveLigtSourceAtPosition(xx,yy,zz,Torch::getID());
					}
				}
			}
		}
	}
}

void CraftWorld::RemoveLigtSourceAtPosition(const int x, const int y, const int z,int blockID)
{
	if(blockID == JackOLantern::getID()|| blockID == Torch::getID())
	{
		int current = 0;

		for(int zz = z-7;zz < z+7;zz++)
		{
			for(int xx = x-7;xx < x+7;xx++)
			{
				for(int yy = y-7;yy < y+7;yy++)
				{
					if (xx >= 0 || yy >= 0 || zz >= 0  || xx < WORLD_SIZE || yy < WORLD_SIZE || zz < WORLD_SIZE)
					{
						if((GetBlockSettings(xx,yy,zz) & OpLighSource) != 0)
						{
							current = GetBlockSettings(xx,yy,zz) & 0xF;
							GetBlockSettings(xx,yy,zz) ^= current & 0xF;
							GetBlockSettings(xx,yy,zz) ^= 0 & 0xF;
							//turn off light here
							GetBlockSettings(xx,yy,zz) ^= OpLighSource;
						}
					}
				}
			}
		}

		BoundingBox lBox = BoundingBox(Vector3(x - 7,y - 7,z - 7),Vector3(x + 7,y + 7,z + 7));
		for(unsigned int i = 0; i < mChunks.size();i++)
		{
			if(lBox.intersect(mChunks[i]->bBox))
				UpdateWorldLightForChunk(i);
		}

	}
}

bool CraftWorld::CanPutBlockHere(const int x, const int y, const int z,int blockID)
{
	if(blockID == Torch::getID())
	{
		bool canPlaseTorch = false;

		if(GetBlock(x,y-1,z) != 0 && GetBlock(x,y-1,z) != Torch::getID())
		{
			canPlaseTorch = true;
		}else if(GetBlock(x-1,y,z) != 0 && GetBlock(x-1,y,z) != Torch::getID())
		{
			canPlaseTorch = true;
		}else if(GetBlock(x+1,y,z) != 0 && GetBlock(x+1,y,z) != Torch::getID())
		{
			canPlaseTorch = true;
		}else if(GetBlock(x,y,z-1) != 0 && GetBlock(x,y,z-1) != Torch::getID())
		{
			canPlaseTorch = true;
		}else if(GetBlock(x,y,z+1) != 0 && GetBlock(x,y,z+1) != Torch::getID())
		{
			canPlaseTorch = true;
		}

		return canPlaseTorch;
	}

	return true;
}

void CraftWorld::LightTravel(int x,int y,int z,int steps,int lightLevel)
{
	if (x >= 0 && y >= 0 && z >= 0  && x < WORLD_SIZE && y < WORLD_SIZE && z < WORLD_SIZE && steps > 0 && lightLevel >= 0)
	{
		//if((GetBlockSettings(x,y,z) & OpLightTraveled) == 0)//not traveled
		//{
			//mark traveled
			//GetBlockSettings(x,y,z) ^= OpLightTraveled;

			//if it's transparent block or light source...
			if(BlockTransparent(x,y,z) == true || GetBlock(x,y,z) == Torch::getID())
			{
				bool canGofuther = false;
				//check if it's lightened
				if((GetBlockSettings(x,y,z) & OpLighSource) != 0)//lightened
				{
					//get current value
					int current = GetBlockSettings(x,y,z) & 0xF;
					//set new only if it's brighter
					if(current < lightLevel)
					{
						//clear this value
						GetBlockSettings(x,y,z) ^= current & 0xF;
						//set new value
						GetBlockSettings(x,y,z) ^= lightLevel & 0xF;

						canGofuther = true;
					}
				}else
				{
					int current = GetBlockSettings(x,y,z) & 0xF;
					//clear this value
					GetBlockSettings(x,y,z) ^= current & 0xF;
					//set new value
					GetBlockSettings(x,y,z) ^= lightLevel & 0xF;
					//mark as lightened
					GetBlockSettings(x,y,z) ^= OpLighSource;

					canGofuther = true;
				}


				//go futher
				if(canGofuther)
				{
					LightTravel(x+1,y,z,steps-1,lightLevel-2);
					LightTravel(x-1,y,z,steps-1,lightLevel-2);
					LightTravel(x,y+1,z,steps-1,lightLevel-2);
					LightTravel(x,y-1,z,steps-1,lightLevel-2);
					LightTravel(x,y,z+1,steps-1,lightLevel-2);
					LightTravel(x,y,z-1,steps-1,lightLevel-2);
				}
			}
		//}
	}
}

void CraftWorld::SetLigtSourcePosition(const int x, const int y, const int z,int blockID)
{
	//put correct light based on lightsource type
	if(blockID == JackOLantern::getID() || blockID == Torch::getID())
	{
		LightTravel(x,y,z,7,15);
	}
}

bool CraftWorld::BlockTransparentOrSpecial(const int x, const int y, const int z)
{
	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return true;

	if(blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->transparent || blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->blockSpecial)
		return true;

	return false;
}

bool CraftWorld::BlockTransparentOrLightSource(const int x, const int y, const int z)
{
	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return true;

	return (blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->transparent || blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->lightSource);
}

bool CraftWorld::BlockTransparent(const int x, const int y, const int z)
{
	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return true;

	return blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->transparent;
}

bool CraftWorld::BlockSpecial(const int x, const int y, const int z)
{
	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return true;

	return blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->blockSpecial;
}

bool CraftWorld::BlockEditable(const int x, const int y, const int z)
{
	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return false;

	return blockTypes[m_Blocks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE]]->editable;
}

bool CraftWorld::LightSourceBlock(int id)
{
	return blockTypes[id]->lightSource;
}

void CraftWorld::GetSpecialBlockVerts(int i,BaseBlock* blockType)
{
	std::vector<Vector3*> mPosition;
	std::vector<Vector3*> mColour;
	std::vector<Vector2*> mtextures;
	std::vector<Vector3*> mTriangle;

	int iVertex = 0;

	float x = -0.5f;
	float y = -0.5f;
	float z = -0.5f;

	if(i == Torch::getID())
	{
		float down = 1.0f - percent * (blockType->textureRow + 1);
		float up = down + percent - ((percent / (float)textureChunk) * 6);

		float left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
		float right = left + ((percent / (float)textureChunk) * 2);

		//x-1
		mPosition.push_back(new Vector3(x + 0.45f,y			,z + 0.45f + 0.1f	));mtextures.push_back(new Vector2(right	, down));	mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f,y + 0.6f	,z + 0.45f + 0.1f	));mtextures.push_back(new Vector2(right	, up));	mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f,y + 0.6f	,z + 0.45f			));mtextures.push_back(new Vector2(left	, up));	mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f,y			,z + 0.45f			));mtextures.push_back(new Vector2(left	, down));	mColour.push_back(new Vector3(1,1,1));

		mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
		mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

		iVertex += 4;

		//x+1
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y		,z + 0.45f));mtextures.push_back(new Vector2(right, down));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y+ 0.6f,z + 0.45f));mtextures.push_back(new Vector2(right, up));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y+ 0.6f,z + 0.45f + 0.1f));mtextures.push_back(new Vector2(left, up));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y		,z + 0.45f + 0.1f));mtextures.push_back(new Vector2(left, down));mColour.push_back(new Vector3(1,1,1));

		mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
		mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

		iVertex += 4;

		//z-1
		mPosition.push_back(new Vector3(x + 0.45f		, y + 0.6f	, z + 0.45f));mtextures.push_back(new Vector2(right, up));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f, y + 0.6f	, z + 0.45f));mtextures.push_back(new Vector2(left, up));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f, y			, z + 0.45f));mtextures.push_back(new Vector2(left, down));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f		, y			, z + 0.45f));mtextures.push_back(new Vector2(right, down));mColour.push_back(new Vector3(1,1,1));

		mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
		mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

		iVertex += 4;

		//z+1
		mPosition.push_back(new Vector3(x + 0.45f		, y			, z + 0.45f + 0.1f));mtextures.push_back(new Vector2(left, down));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f, y			, z + 0.45f + 0.1f));mtextures.push_back(new Vector2(right, down));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f + 0.1f, y + 0.6f	, z + 0.45f + 0.1f));mtextures.push_back(new Vector2(right, up));mColour.push_back(new Vector3(1,1,1));
		mPosition.push_back(new Vector3(x + 0.45f		, y + 0.6f	, z + 0.45f + 0.1f));mtextures.push_back(new Vector2(left, up));mColour.push_back(new Vector3(1,1,1));

		mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
		mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

		iVertex += 4;

		//y+1
		{
			down = 1.0f - percent * (blockType->textureRow + 1) + ((percent / (float)textureChunk) * 8);
			up = down + ((percent / (float)textureChunk) * 2);

			left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
			right = left + ((percent / (float)textureChunk) * 2);

			mPosition.push_back(new Vector3(x + 0.45f		,y + 0.6f,z + 0.45f + 0.1f));mtextures.push_back(new Vector2(left, up));mColour.push_back(new Vector3(1,1,1));
			mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f + 0.1f));mtextures.push_back(new Vector2(right, up));mColour.push_back(new Vector3(1,1,1));
			mPosition.push_back(new Vector3(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f));mtextures.push_back(new Vector2(right, down));mColour.push_back(new Vector3(1,1,1));
			mPosition.push_back(new Vector3(x + 0.45f		,y + 0.6f,z + 0.45f));mtextures.push_back(new Vector2(left, down));mColour.push_back(new Vector3(1,1,1));

			mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
			mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

			iVertex += 4;
		}

		blockTypes[i]->vertices = (CraftPSPVertex*)memalign(16,( mTriangle.size() * 3) * sizeof(CraftPSPVertex));
		//build verts
		//vertices
		int vert = 0;
		unsigned int size =  mTriangle.size();
		for(unsigned int j = 0;j < size;j++)
		{
			blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->x]->x;
			blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->x]->y;
			blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->x]->x,mColour[mTriangle[j]->x]->y,mColour[mTriangle[j]->x]->z,1.0f);
			blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->x]->x;
			blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->x]->y;
			blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->x]->z;
			vert++;

			blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->y]->x;
			blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->y]->y;
			blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->y]->x,mColour[mTriangle[j]->y]->y,mColour[mTriangle[j]->y]->z,1.0f);
			blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->y]->x;
			blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->y]->y;
			blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->y]->z;
			vert++;

			blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->z]->x;
			blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->z]->y;
			blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->z]->x,mColour[mTriangle[j]->z]->y,mColour[mTriangle[j]->z]->z,1.0f);
			blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->z]->x;
			blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->z]->y;
			blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->z]->z;
			vert++;
		}

		//clear the cache or there will be some errors
		sceKernelDcacheWritebackInvalidateRange(blockTypes[i]->vertices,( mTriangle.size() * 3) * sizeof(CraftPSPVertex));
		//sceKernelDcacheWritebackInvalidateAll();

		for(unsigned int aa = 0;aa < mPosition.size();aa++)
		{
			delete mPosition[aa];
			delete mtextures[aa];
			delete mColour[aa];
		}
		
		mPosition.clear();
		mtextures.clear();
		mColour.clear();

		for(unsigned int aa = 0;aa < mTriangle.size();aa++)
			delete mTriangle[aa];
		
		mTriangle.clear();
	}
}

void CraftWorld::GetNormalBlockVerts(int i,BaseBlock *blockType)
{
	std::vector<Vector3*> mPosition;
	std::vector<Vector3*> mColour;
	std::vector<Vector2*> mtextures;
	std::vector<Vector3*> mTriangle;

	int iVertex = 0;

	float x = -0.5f;
	float y = -0.5f;
	float z = -0.5f;

	float down = 1.0f - percent * (blockType->textureRow + 1);
	float up = down + percent;

	float left = percent * blockType->upPlane;
	float right = left + percent;

	iVertex = 0;

	//light
	float BlockLight  = 1.0f;  //For the two x faces
	float BlockLight1 = BlockLight * 0.9f;		//For the two z faces
	float BlockLight2 = BlockLight * 0.8f;		//For the two y faces

	//x
	left = percent * blockType->sidePlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x, y, z+1));	mtextures.push_back(new Vector2(right, down)); mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x, y+1, z+1));	mtextures.push_back(new Vector2(right, up)); mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x, y+1, z));		mtextures.push_back(new Vector2(left, up)); mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x, y,   z));		mtextures.push_back(new Vector2(left, down)); mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;

	//x
	left = percent * blockType->sidePlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x+1, y,   z));	mtextures.push_back(new Vector2(right, down));mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x+1, y+1, z));	mtextures.push_back(new Vector2(right, up));mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x+1, y+1, z+1));	mtextures.push_back(new Vector2(left, up));mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));
	mPosition.push_back(new Vector3(x+1, y,   z+1));	mtextures.push_back(new Vector2(left, down));mColour.push_back(new Vector3(BlockLight,BlockLight,BlockLight));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;

	//up
	left = percent * blockType->downPlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x,   y, z));		mtextures.push_back(new Vector2(left, up)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x+1, y, z));		mtextures.push_back(new Vector2(right, up)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x+1, y, z+1));	mtextures.push_back(new Vector2(right, down)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x,   y, z+1));	mtextures.push_back(new Vector2(left, down)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;

	//down
	left = percent * blockType->upPlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x,   y+1, z+1));	mtextures.push_back(new Vector2(left, up)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x+1, y+1, z+1));	mtextures.push_back(new Vector2(right, up)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x+1, y+1, z));		mtextures.push_back(new Vector2(right, down)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));
	mPosition.push_back(new Vector3(x,   y+1, z));		mtextures.push_back(new Vector2(left, down)); mColour.push_back(new Vector3(BlockLight2,BlockLight2,BlockLight2));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;

	left = percent * blockType->sidePlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x,   y+1, z));		mtextures.push_back(new Vector2(right, up)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x+1, y+1, z));		mtextures.push_back(new Vector2(left, up)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x+1, y,   z));		mtextures.push_back(new Vector2(left, down)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x,   y,   z));		mtextures.push_back(new Vector2(right, down)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;

	left = percent * blockType->sidePlane;
	right = left + percent;

	mPosition.push_back(new Vector3(x,   y,   z+1));		mtextures.push_back(new Vector2(left, down)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x+1, y,   z+1));		mtextures.push_back(new Vector2(right, down)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x+1, y+1, z+1));		mtextures.push_back(new Vector2(right, up)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));
	mPosition.push_back(new Vector3(x,   y+1, z+1));		mtextures.push_back(new Vector2(left, up)); mColour.push_back(new Vector3(BlockLight1,BlockLight1,BlockLight1));

	mTriangle.push_back(new Vector3(iVertex, iVertex+1, iVertex+2));
	mTriangle.push_back(new Vector3(iVertex+2, iVertex+3, iVertex));

	iVertex += 4;


	blockTypes[i]->vertices = (CraftPSPVertex*)memalign(16,( mTriangle.size() * 3) * sizeof(CraftPSPVertex));
	//build verts
	//vertices
	int vert = 0;
	unsigned int size =  mTriangle.size();
	for(unsigned int j = 0;j < size;j++)
	{
		blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->x]->x;
		blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->x]->y;
		blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->x]->x,mColour[mTriangle[j]->x]->y,mColour[mTriangle[j]->x]->z,1.0f);
		blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->x]->x;
		blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->x]->y;
		blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->x]->z;
		vert++;

		blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->y]->x;
		blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->y]->y;
		blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->y]->x,mColour[mTriangle[j]->y]->y,mColour[mTriangle[j]->y]->z,1.0f);
		blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->y]->x;
		blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->y]->y;
		blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->y]->z;
		vert++;

		blockTypes[i]->vertices[vert].u = mtextures[mTriangle[j]->z]->x;
		blockTypes[i]->vertices[vert].v = mtextures[mTriangle[j]->z]->y;
		blockTypes[i]->vertices[vert].color = GU_COLOR( mColour[mTriangle[j]->z]->x,mColour[mTriangle[j]->z]->y,mColour[mTriangle[j]->z]->z,1.0f);
		blockTypes[i]->vertices[vert].x = mPosition[mTriangle[j]->z]->x;
		blockTypes[i]->vertices[vert].y = mPosition[mTriangle[j]->z]->y;
		blockTypes[i]->vertices[vert].z = mPosition[mTriangle[j]->z]->z;
		vert++;
	}

	//clear the cache or there will be some errors
	sceKernelDcacheWritebackInvalidateRange(blockTypes[i]->vertices,( mTriangle.size() * 3) * sizeof(CraftPSPVertex));
	//sceKernelDcacheWritebackInvalidateAll();

	for(unsigned int aa = 0;aa < mPosition.size();aa++)
	{
		delete mPosition[aa];
		delete mtextures[aa];
		delete mColour[aa];
	}
	
	mPosition.clear();
	mtextures.clear();
	mColour.clear();

	for(unsigned int aa = 0;aa < mTriangle.size();aa++)
		delete mTriangle[aa];
		
	mTriangle.clear();
}

void CraftWorld::buildblocksVerts()
{
	//create vertices for each block type
	for(unsigned int i = 1;i < blockTypes.size();i++)
	{
		BaseBlock *blockType = blockTypes[i];

		if(blockType->blockSpecial)
			GetSpecialBlockVerts(i,blockType);
		else
			GetNormalBlockVerts(i,blockType);
	}
}

void CraftWorld::createChunks(const int StartX, const int StartY, const int StartZ)
{
	SimpleMeshChunk* MeshChunk = new SimpleMeshChunk();

	int iVertex = 0;
	block_t Block;

	/* Only create visible faces of each chunk */
	MeshChunk->chunkStartZ = StartZ;
	MeshChunk->chunkStartY = StartY;
	MeshChunk->chunkStartX = StartX;

	MeshChunk->bBox = BoundingBox(Vector3(StartX,StartY,StartZ),Vector3(StartX + CHUNK_SIZE,StartY + CHUNK_SIZE,StartZ + CHUNK_SIZE));

	//if we are in player zone on the start then build map
	if(playerZoneBB.intersect(MeshChunk->bBox))
	{
		createdChunksCount++;
		//light
		for (int z = StartZ; z < CHUNK_SIZE + StartZ; ++z)
		{
			for (int y = StartY; y < CHUNK_SIZE + StartY; ++y)
			{
				for (int x = StartX; x < CHUNK_SIZE + StartX; ++x)
				{
					Block = GetBlock(x,y,z);
					if(BlockTransparent(x,y,z) == true)continue;//if block is transparent don't continue

					if(!BlockSpecial(x,y,z))
						GetNormalBlock(x,y,z,iVertex,MeshChunk,Block,false);
					else
						GetSpecialBlock(x,y,z,iVertex,MeshChunk,Block,false);
				}
			}
		}

		MeshChunk->end();
	}else
	{
		MeshChunk->end();
		MeshChunk->created = false;
	}

	MeshChunk->id = mChunks.size();
	mChunks.push_back(MeshChunk);
}


void CraftWorld::createTransparentChunks(const int StartX, const int StartY, const int StartZ)
{
	SimpleMeshChunk* MeshChunk = new SimpleMeshChunk();

	int iVertex = 0;
	block_t Block;

	MeshChunk->chunkStartZ = StartZ;
	MeshChunk->chunkStartY = StartY;
	MeshChunk->chunkStartX = StartX;

	MeshChunk->bBox = BoundingBox(Vector3(StartX,StartY,StartZ),Vector3(StartX + CHUNK_SIZE,StartY + CHUNK_SIZE,StartZ + CHUNK_SIZE));

	//if we are in player zone on the start then build map
	if(playerZoneBB.intersect(MeshChunk->bBox))
	{
		for (int z = StartZ; z < CHUNK_SIZE + StartZ; ++z)
		{
			for (int y = StartY; y < CHUNK_SIZE + StartY; ++y)
			{
				for (int x = StartX; x < CHUNK_SIZE + StartX; ++x)
				{
					Block = GetBlock(x,y,z);
					if (BlockTransparent(x,y,z) == false || Block == 0) continue;

					if(!BlockSpecial(x,y,z))
						GetNormalBlock(x,y,z,iVertex,MeshChunk,Block,true);
					else
						GetSpecialBlock(x,y,z,iVertex,MeshChunk,Block,true);
				}
			}
		}
		MeshChunk->end();
	}else
	{
		MeshChunk->end();
		MeshChunk->created = false;
	}

	//MeshChunk->end();
	MeshChunk->id = mChunks.size();
	mTransparentChunks.push_back(MeshChunk);
}

void CraftWorld::rebuildChunk(int id)
{
	SimpleMeshChunk* MeshChunk = mChunks[id];

	int iVertex = 0;
	block_t Block;

	int StartZ = MeshChunk->chunkStartZ;
	int StartY = MeshChunk->chunkStartY;
	int StartX = MeshChunk->chunkStartX;

	MeshChunk->reset();

	for (int z = StartZ; z < CHUNK_SIZE + StartZ; ++z)
	{
		for (int y = StartY; y < CHUNK_SIZE + StartY; ++y)
		{
			for (int x = StartX; x < CHUNK_SIZE + StartX; ++x)
			{
				Block = GetBlock(x,y,z);
				if(BlockTransparent(x,y,z) == true)continue;//if block is transparent don't continue

				if(!BlockSpecial(x,y,z))
					GetNormalBlock(x,y,z,iVertex,MeshChunk,Block,false);
				else
					GetSpecialBlock(x,y,z,iVertex,MeshChunk,Block,false);
			}
		}
	}

	MeshChunk->end();
}

void CraftWorld::rebuildTransparentChunk(int id)
{
	SimpleMeshChunk* MeshChunk = mTransparentChunks[id];

	int iVertex = 0;
	block_t Block;

	int StartZ = MeshChunk->chunkStartZ;
	int StartY = MeshChunk->chunkStartY;
	int StartX = MeshChunk->chunkStartX;

	MeshChunk->reset();

	for (int z = StartZ; z < CHUNK_SIZE + StartZ; ++z)
	{
		for (int y = StartY; y < CHUNK_SIZE + StartY; ++y)
		{
			for (int x = StartX; x < CHUNK_SIZE + StartX; ++x)
			{
				Block = GetBlock(x,y,z);
				if (BlockTransparent(x,y,z) == false || Block == 0) continue;

				if(!BlockSpecial(x,y,z))
					GetNormalBlock(x,y,z,iVertex,MeshChunk,Block,true);
				else
					GetSpecialBlock(x,y,z,iVertex,MeshChunk,Block,true);
			}
		}
	}

	MeshChunk->end();
}

void CraftWorld::GetSpecialBlock(int x,int y, int z,int &iVertex,SimpleMeshChunk* MeshChunk,block_t Block,bool transparent)
{
	if(Block == Torch::getID())
	{
		//texture stuff
		BaseBlock *blockType = blockTypes[Block];
		float down = 1.0f - percent * (blockType->textureRow + 1);
		float up = down + percent - ((percent / (float)textureChunk) * 6);

		float left = percent * blockType->upPlane;
		float right = left + percent;

		//standing torch
		block_t Block1 = GetBlock(x,y-1,z);

		if(Block1 != 0 && Block1 != Torch::getID())
		{
			left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
			right = left + ((percent / (float)textureChunk) * 2);

			//x-1
			MeshChunk->info(x + 0.45f,y			,z + 0.45f + 0.1f	,right	, down	,1,1,1);
			MeshChunk->info(x + 0.45f,y + 0.6f	,z + 0.45f + 0.1f	,right	, up	,1,1,1);
			MeshChunk->info(x + 0.45f,y + 0.6f	,z + 0.45f			,left	, up	,1,1,1);
			MeshChunk->info(x + 0.45f,y			,z + 0.45f			,left	, down	,1,1,1);

			MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
			MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

			iVertex += 4;

			//x+1
			MeshChunk->info(x + 0.45f + 0.1f,y		,z + 0.45f,right, down,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f,y+ 0.6f,z + 0.45f,right, up,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f,y+ 0.6f,z + 0.45f + 0.1f,left, up,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f,y		,z + 0.45f + 0.1f,left, down,1,1,1);

			MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
			MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

			iVertex += 4;

			//z-1
			MeshChunk->info(x + 0.45f		, y + 0.6f	, z + 0.45f,right, up,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f, y + 0.6f	, z + 0.45f,left, up,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f, y			, z + 0.45f,left, down,1,1,1);
			MeshChunk->info(x + 0.45f		, y			, z + 0.45f,right, down,1,1,1);

			MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
			MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

			iVertex += 4;

			//z+1
			MeshChunk->info(x + 0.45f		, y			, z + 0.45f + 0.1f,left, down,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f, y			, z + 0.45f + 0.1f,right, down,1,1,1);
			MeshChunk->info(x + 0.45f + 0.1f, y + 0.6f	, z + 0.45f + 0.1f,right, up,1,1,1);
			MeshChunk->info(x + 0.45f		, y + 0.6f	, z + 0.45f + 0.1f,left, up,1,1,1);

			MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
			MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

			iVertex += 4;

			//y+1
			{
				down = 1.0f - percent * (blockType->textureRow + 1) + ((percent / (float)textureChunk) * 8);
				up = down + ((percent / (float)textureChunk) * 2);

				left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
				right = left + ((percent / (float)textureChunk) * 2);

				MeshChunk->info(x + 0.45f		,y + 0.6f,z + 0.45f + 0.1f,left, up,1,1,1);
				MeshChunk->info(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f + 0.1f,right, up,1,1,1);
				MeshChunk->info(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f,right, down,1,1,1);
				MeshChunk->info(x + 0.45f		,y + 0.6f,z + 0.45f,left, down,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;
			}
		}else
		{
			Vector3 posLeft,posRight;
			Vector3 posUpLeft,posUpRight;
			Vector3 posUpLeft2,posUpRight2;
			bool canPlaseTorch = false;

			if(GetBlock(x-1,y,z) != 0 && GetBlock(x-1,y,z) != Torch::getID())
			{
				posLeft = Vector3(x,y,z+0.55f);
				posRight = Vector3(x,y,z+0.45f);
				posUpLeft = Vector3(x+0.45,y,z+0.55f);
				posUpRight = Vector3(x+0.45,y,z+0.45f);
				posUpLeft2 = Vector3(x+0.55,y,z+0.55f);
				posUpRight2 = Vector3(x+0.55,y,z+0.45f);

				canPlaseTorch = true;
			}else if(GetBlock(x+1,y,z) != 0 && GetBlock(x+1,y,z) != Torch::getID())
			{
				posRight = Vector3(x+1,y,z+0.55f);
				posLeft = Vector3(x+1,y,z+0.45f);
				posUpRight2 = Vector3(x+0.45,y,z+0.55f);
				posUpLeft2 = Vector3(x+0.45,y,z+0.45f);
				posUpRight = Vector3(x+0.55,y,z+0.55f);
				posUpLeft = Vector3(x+0.55,y,z+0.45f);

				canPlaseTorch = true;
			}else if(GetBlock(x,y,z-1) != 0 && GetBlock(x,y,z-1) != Torch::getID())
			{
				posLeft = Vector3(x+0.45f,y,z);
				posRight = Vector3(x+0.55f,y,z);
				posUpLeft = Vector3(x+0.45f,y,z+0.45f);
				posUpRight = Vector3(x+0.55f,y,z+0.45f);
				posUpLeft2 = Vector3(x+0.45f,y,z+0.55f);
				posUpRight2 = Vector3(x+0.55,y,z+0.55f);

				canPlaseTorch = true;
			}else if(GetBlock(x,y,z+1) != 0 && GetBlock(x,y,z+1) != Torch::getID())
			{
				posRight = Vector3(x+0.45f,y,z+1);
				posLeft = Vector3(x+0.55f,y,z+1);
				posUpRight2 = Vector3(x+0.45f,y,z+0.45f);
				posUpLeft2 = Vector3(x+0.55f,y,z+0.45f);
				posUpRight = Vector3(x+0.45f,y,z+0.55f);
				posUpLeft = Vector3(x+0.55,y,z+0.55f);

				canPlaseTorch = true;
			}

			if(canPlaseTorch)
			{
				left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
				right = left + ((percent / (float)textureChunk) * 2);

				//x-1
				MeshChunk->info( posLeft.x, y+0.1f, posLeft.z, right, down, 1, 1, 1);
				MeshChunk->info( posUpLeft.x,y + 0.6f ,posUpLeft.z	,right	, up	,1,1,1);
				MeshChunk->info( posUpRight.x,y + 0.6f,posUpRight.z,left	, up	,1,1,1);
				MeshChunk->info( posRight.x, y+0.1f, posRight.z,left	, down	,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;

				//x+1
				MeshChunk->info(posRight.x,y		,posRight.z,right, down,1,1,1);
				MeshChunk->info(posUpRight2.x,y + 0.6f,posUpRight2.z,right, up,1,1,1);
				MeshChunk->info(posUpLeft2.x,y + 0.6f,posUpLeft2.z,left, up,1,1,1);
				MeshChunk->info(posLeft.x, y, posLeft.z,left, down,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;

				//z-1
				MeshChunk->info(posUpRight.x,y + 0.6f,posUpRight.z,right, up,1,1,1);
				MeshChunk->info(posUpRight2.x,y + 0.6f,posUpRight2.z,left, up,1,1,1);
				MeshChunk->info(posRight.x,y,posRight.z,left, down,1,1,1);
				MeshChunk->info(posRight.x,y+0.1f,posRight.z,right, down,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;

				//z+1
				MeshChunk->info(posLeft.x, y+0.1f, posLeft.z,left, down,1,1,1);
				MeshChunk->info(posLeft.x, y, posLeft.z,right, down,1,1,1);
				MeshChunk->info(posUpLeft2.x, y+0.6, posUpLeft2.z,right, up,1,1,1);
				MeshChunk->info(posUpLeft.x, y+0.6, posUpLeft.z,left, up,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;

				//y+1
				down = 1.0f - percent * (blockType->textureRow + 1) + ((percent / (float)textureChunk) * 8);
				up = down + ((percent / (float)textureChunk) * 2);

				left = percent * blockType->sidePlane + ((percent / (float)textureChunk) * 7);
				right = left + ((percent / (float)textureChunk) * 2);

				MeshChunk->info(x + 0.45f		,y + 0.6f,z + 0.45f + 0.1f,left, up,1,1,1);
				MeshChunk->info(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f + 0.1f,right, up,1,1,1);
				MeshChunk->info(x + 0.45f + 0.1f,y + 0.6f,z + 0.45f,right, down,1,1,1);
				MeshChunk->info(x + 0.45f		,y + 0.6f,z + 0.45f,left, down,1,1,1);

				MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
				MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

				iVertex += 4;
			}
		}

	}
}

void CraftWorld::GetNormalBlock(int x,int y, int z,int &iVertex,SimpleMeshChunk* MeshChunk,block_t Block,bool transparent)
{
	/* Only create visible faces of each chunk */
	bool DefaultBlock = false;
	bool transparentBlock;

	block_t Block1 = 0;

	Vector3 light1,light2,light3,light4;
	float BaseLight = 1.0f;

	Vector3 BlockColory1;    //Top face
	Vector3 BlockColory2;    //Bottom face
	Vector3 BlockColorx1;    //Sunset face
	Vector3 BlockColorx2;    //Sunrise face
	Vector3 BlockColorz1;     //Front/back faces
	Vector3 BlockColorz2;     //Front/back faces

	//texture stuff
	BaseBlock *blockType = blockTypes[Block];

	float down = 1.0f - percent * (blockType->textureRow + 1);
	float up = down + percent;

	float left = percent * blockType->upPlane;
	float right = left + percent;

	bool canCreate = false;

	//lightened
	//if time is between 21-4 use settings table for lightening
	//if time is between 5-20 use normal light table but compare it with settings table
	//	if setting is brighter then use it
	if(worldDayTime >= 21 || worldDayTime <= 4)//night
	{
		if((GetBlockSettings(x,y,z) & OpLighSource) != 0)//block is lightened
		{
			BaseLight  = (float)(GetBlockSettings(x, y, z) & 0xF)/16.0f; // 255.0f;  //For the two x faces

			BlockColorx1 = BlockColorx2 = Vector3(BaseLight,BaseLight,BaseLight);
			BlockColorz1  = BlockColorz2 = Vector3(BaseLight,BaseLight,BaseLight) * 0.9f;
			BlockColory1 = BlockColory2 = Vector3(BaseLight,BaseLight,BaseLight) * 0.8f;
		}else//normal light
		{
			//light
			BaseLight  = GetBlockLight(x, y, z) / 255.0f;  //For the two x faces
			//float BlockLight1 = BlockLight * 0.9f;		//For the two z faces
			//float BlockLight2 = BlockLight * 0.8f;		//For the two y faces

			BlockColory1 = lightColor * (factor1 * BaseLight) + ambientColor;
			BlockColory1.saturate();
			BlockColory2 = lightColor * (factor1 / 2.0f * BaseLight) + ambientColor;
			BlockColory2.saturate();
			BlockColorz1  = lightColor * (factor1 * 0.70f * BaseLight) + ambientColor;
			BlockColorz1.saturate();
			BlockColorz1 *= 0.80f;
			BlockColorz2 = BlockColorz1;

			BlockColorx1 = lightColor * (factor2 * 0.80f * BaseLight) + ambientColor;
			BlockColorx1.saturate();
			BlockColorx1 *= 0.95f;

			BlockColorx2 = lightColor * (factor3 * 0.80f * BaseLight) + ambientColor;
			BlockColorx2.saturate();
			BlockColorx2 *= 0.95f;
		}
	}else//day
	{
		if((GetBlockSettings(x,y,z) & OpLighSource) != 0)//block is lightened
		{
			//int normal = GetBlockLight(x, y, z);
			//int lightened = (GetBlockSettings(x, y, z) & 0xF) * 16;

			float normal = lightColor.x * (factor1 * (GetBlockLight(x, y, z) / 255.0f)) + ambientColor.x;
			float lightened = (GetBlockSettings(x, y, z) & 0xF) / 15.0f;

			if(lightened > normal)
			{
				BaseLight = lightened; // 255.0f;
				BlockColorx1 = BlockColorx2 = Vector3(BaseLight,BaseLight,BaseLight);
				BlockColorz2 = BlockColorz1  = Vector3(BaseLight,BaseLight,BaseLight) * 0.9f;
				BlockColory1 = BlockColory2 = Vector3(BaseLight,BaseLight,BaseLight) * 0.8f;
			}else
			{
				BaseLight  = GetBlockLight(x, y, z) / 255.0f;

				BlockColory1 = lightColor * (factor1 * BaseLight) + ambientColor;
				BlockColory1.saturate();
				BlockColory2 = lightColor * (factor1 / 2.0f * BaseLight) + ambientColor;
				BlockColory2.saturate();
				BlockColorz1  = lightColor * (factor1 * 0.70f * BaseLight) + ambientColor;
				BlockColorz1.saturate();
				BlockColorz1 *= 0.80f;
				BlockColorz2 = BlockColorz1;

				BlockColorx1 = lightColor * (factor2 * 0.80f * BaseLight) + ambientColor;
				BlockColorx1.saturate();
				BlockColorx1 *= 0.95f;

				BlockColorx2 = lightColor * (factor3 * 0.80f * BaseLight) + ambientColor;
				BlockColorx2.saturate();
				BlockColorx2 *= 0.95f;
			}

		}else
		{
			BaseLight  = GetBlockLight(x, y, z) / 255.0f;

			BlockColory1 = lightColor * (factor1 * BaseLight) + ambientColor;
			BlockColory1.saturate();
			BlockColory2 = lightColor * (factor1 / 2.0f * BaseLight) + ambientColor;
			BlockColory2.saturate();
			BlockColorz1  = lightColor * (factor1 * 0.70f * BaseLight) + ambientColor;
			BlockColorz1.saturate();
			BlockColorz1 *= 0.80f;
			BlockColorz2 = BlockColorz1;

			BlockColorx1 = lightColor * (factor2 * 0.80f * BaseLight) + ambientColor;
			BlockColorx1.saturate();
			BlockColorx1 *= 0.95f;

			BlockColorx2 = lightColor * (factor3 * 0.80f * BaseLight) + ambientColor;
			BlockColorx2.saturate();
			BlockColorx2 *= 0.95f;
		}
	}

	//faces
	//x-1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (x > 0)
		{
			transparentBlock = BlockTransparent(x-1,y,z);
			Block1 = GetBlock(x-1,y,z);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (x > 0)transparentBlock = BlockTransparentOrSpecial(x-1,y,z);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate)
	{
		left = percent * blockType->sidePlane;
		right = left + percent;

		if((GetBlockSettings(x-1,y,z) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x-1, y, z) & 0xF) / 15.0f;
			if(lightened > BlockColorx1.x)
				BlockColorx1.x = BlockColorx1.y = BlockColorx1.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColorx1;
		lightFactor = BlockColorx1 * lightShadowFactor;

		//simple shadows
		//up
		if(!BlockTransparentOrLightSource(x-1,y+1,z) || !BlockTransparentOrLightSource(x-1,y,z+1) || !BlockTransparentOrLightSource(x-1,y+1,z+1))
		{
			light2-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x-1,y+1,z) || !BlockTransparentOrLightSource(x-1,y,z-1) || !BlockTransparentOrLightSource(x-1,y+1,z-1))
		{
			light4-=lightFactor;
		}

		//down
		if(!BlockTransparentOrLightSource(x-1,y-1,z) || !BlockTransparentOrLightSource(x-1,y,z+1) || !BlockTransparentOrLightSource(x-1,y-1,z+1))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x-1,y-1,z) || !BlockTransparentOrLightSource(x-1,y,z-1) || !BlockTransparentOrLightSource(x-1,y-1,z-1))
		{
			light3-=lightFactor;
		}

		MeshChunk->position(x, y,   z+1);	MeshChunk->textureCoord(right, down); MeshChunk->colour(light1.x,light1.y,light1.z);
		MeshChunk->position(x, y+1, z+1);	MeshChunk->textureCoord(right, up); MeshChunk->colour(light2.x,light2.y,light2.z);
		MeshChunk->position(x, y+1, z);		MeshChunk->textureCoord(left, up); MeshChunk->colour(light4.x,light4.y,light4.z);
		MeshChunk->position(x, y,   z);		MeshChunk->textureCoord(left, down); MeshChunk->colour(light3.x,light3.y,light3.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}


	//x+1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (x < WORLD_SIZE - 1)
		{
			transparentBlock = BlockTransparent(x+1,y,z);
			Block1 = GetBlock(x+1,y,z);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (x < WORLD_SIZE - 1)transparentBlock = BlockTransparentOrSpecial(x+1,y,z);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate)
	{
		left = percent * blockType->sidePlane;
		right = left + percent;

		if((GetBlockSettings(x+1,y,z) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x+1, y, z) & 0xF) / 15.0f;
			if(lightened > BlockColorx2.x)
				BlockColorx2.x = BlockColorx2.y = BlockColorx2.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColorx2;
		lightFactor = BlockColorx2 * lightShadowFactor;

		//simple shadows
		//up
		if(!BlockTransparentOrLightSource(x+1,y+1,z) || !BlockTransparentOrLightSource(x+1,y,z+1) || !BlockTransparentOrLightSource(x+1,y+1,z+1))
		{
			light2-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x+1,y+1,z) || !BlockTransparentOrLightSource(x+1,y,z-1) || !BlockTransparentOrLightSource(x+1,y+1,z-1))
		{
			light4-=lightFactor;
		}

		//down
		if(!BlockTransparentOrLightSource(x+1,y-1,z) || !BlockTransparentOrLightSource(x+1,y,z+1) || !BlockTransparentOrLightSource(x+1,y-1,z+1))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x+1,y-1,z) || !BlockTransparentOrLightSource(x+1,y,z-1) || !BlockTransparentOrLightSource(x+1,y-1,z-1))
		{
			light3-=lightFactor;
		}

		MeshChunk->position(x+1, y,   z);	MeshChunk->textureCoord(right, down);MeshChunk->colour(light3.x,light3.y,light3.z);
		MeshChunk->position(x+1, y+1, z);	MeshChunk->textureCoord(right, up);MeshChunk->colour(light4.x,light4.y,light4.z);
		MeshChunk->position(x+1, y+1, z+1);	MeshChunk->textureCoord(left, up);MeshChunk->colour(light2.x,light2.y,light2.z);
		MeshChunk->position(x+1, y,   z+1);	MeshChunk->textureCoord(left, down);MeshChunk->colour(light1.x,light1.y,light1.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}

	//y-1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (y > 0)
		{
			transparentBlock = BlockTransparent(x,y-1,z);
			Block1 = GetBlock(x,y-1,z);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (y > 0)transparentBlock = BlockTransparentOrSpecial(x,y-1,z);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate)
	{
		//up
		left = percent * blockType->downPlane;
		right = left + percent;

		if((GetBlockSettings(x,y-1,z) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x, y-1, z) & 0xF) / 15.0f;
			if(lightened > BlockColory2.x)
				BlockColory2.x = BlockColory2.y = BlockColory2.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColory2;
		lightFactor = BlockColory2 * lightShadowFactor;

		//simple shadows
		if(!BlockTransparentOrLightSource(x-1,y-1,z) || !BlockTransparentOrLightSource(x-1,y-1,z-1) || !BlockTransparentOrLightSource(x,y-1,z-1))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y-1,z-1) || !BlockTransparentOrLightSource(x+1,y-1,z-1) || !BlockTransparentOrLightSource(x+1,y-1,z))
		{
			light2-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x+1,y-1,z) || !BlockTransparentOrLightSource(x+1,y-1,z+1) || !BlockTransparentOrLightSource(x,y-1,z+1))
		{
			light3-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y-1,z+1) || !BlockTransparentOrLightSource(x-1,y-1,z+1) || !BlockTransparentOrLightSource(x-1,y-1,z))
		{
			light4-=lightFactor;
		}

		MeshChunk->position(x,   y, z);		MeshChunk->textureCoord(left, up); MeshChunk->colour(light1.x,light1.y,light1.z);
		MeshChunk->position(x+1, y, z);		MeshChunk->textureCoord(right, up); MeshChunk->colour(light2.x,light2.y,light2.z);
		MeshChunk->position(x+1, y, z+1);	MeshChunk->textureCoord(right, down); MeshChunk->colour(light3.x,light3.y,light3.z);
		MeshChunk->position(x,   y, z+1);	MeshChunk->textureCoord(left, down); MeshChunk->colour(light4.x,light4.y,light4.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}


	//top face
	//y+1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (y < WORLD_SIZE - 1)
		{
			transparentBlock = BlockTransparent(x,y+1,z);
			Block1 = GetBlock(x,y+1,z);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (y < WORLD_SIZE - 1)transparentBlock = BlockTransparentOrSpecial(x,y+1,z);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate)
	{
		if((GetBlockSettings(x,y+1,z) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x, y+1, z) & 0xF) / 15.0f;
			if(lightened > BlockColory1.x)
				BlockColory1.x = BlockColory1.y = BlockColory1.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColory1;
		lightFactor = BlockColory1 * lightShadowFactor;

		//simple shadows
		if(!BlockTransparentOrLightSource(x,y+1,z+1) || !BlockTransparentOrLightSource(x-1,y+1,z+1) || !BlockTransparentOrLightSource(x-1,y+1,z))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y+1,z+1) || !BlockTransparentOrLightSource(x+1,y+1,z+1) || !BlockTransparentOrLightSource(x+1,y+1,z))
		{
			light2-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x+1,y+1,z) || !BlockTransparentOrLightSource(x+1,y+1,z-1) || !BlockTransparentOrLightSource(x,y+1,z-1))
		{
			light3-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y+1,z-1) || !BlockTransparentOrLightSource(x-1,y+1,z-1) || !BlockTransparentOrLightSource(x-1,y+1,z))
		{
			light4-=lightFactor;
		}

		//down
		left = percent * blockType->upPlane;
		right = left + percent;

		MeshChunk->position(x,   y+1, z+1);		MeshChunk->textureCoord(left, up); MeshChunk->colour(light1.x,light1.y,light1.z);
		MeshChunk->position(x+1, y+1, z+1);		MeshChunk->textureCoord(right, up); MeshChunk->colour(light2.x,light2.y,light2.z);
		MeshChunk->position(x+1, y+1, z);		MeshChunk->textureCoord(right, down); MeshChunk->colour(light3.x,light3.y,light3.z);
		MeshChunk->position(x,   y+1, z);		MeshChunk->textureCoord(left, down); MeshChunk->colour(light4.x,light4.y,light4.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}

	//z-1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (z > 0)
		{
			transparentBlock = BlockTransparent(x,y,z-1);
			Block1 = GetBlock(x,y,z-1);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (z > 0)transparentBlock = BlockTransparentOrSpecial(x,y,z-1);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate)
	{
		left = percent * blockType->sidePlane;
		right = left + percent;

		if((GetBlockSettings(x,y,z-1) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x, y, z-1) & 0xF) / 15.0f;
			if(lightened > BlockColorz1.x)
				BlockColorz1.x = BlockColorz1.y = BlockColorz1.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColorz1;
		lightFactor = BlockColorz1 * lightShadowFactor;

		//simple shadows
		//up
		if(!BlockTransparentOrLightSource(x,y+1,z-1) || !BlockTransparentOrLightSource(x-1,y,z-1) || !BlockTransparentOrLightSource(x-1,y+1,z-1))
		{
			light2-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y+1,z-1) || !BlockTransparentOrLightSource(x+1,y,z-1) || !BlockTransparentOrLightSource(x+1,y+1,z-1))
		{
			light4-=lightFactor;
		}

		//down
		if(!BlockTransparentOrLightSource(x,y-1,z-1) || !BlockTransparentOrLightSource(x-1,y,z-1) || !BlockTransparentOrLightSource(x-1,y-1,z-1))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y-1,z-1) || !BlockTransparentOrLightSource(x+1,y,z-1) || !BlockTransparentOrLightSource(x+1,y-1,z-1))
		{
			light3-=lightFactor;
		}

		MeshChunk->position(x,   y+1, z);		MeshChunk->textureCoord(right, up); MeshChunk->colour(light2.x,light2.y,light2.z);
		MeshChunk->position(x+1, y+1, z);		MeshChunk->textureCoord(left, up); MeshChunk->colour(light4.x,light4.y,light4.z);
		MeshChunk->position(x+1, y,   z);		MeshChunk->textureCoord(left, down); MeshChunk->colour(light3.x,light3.y,light3.z);
		MeshChunk->position(x,   y,   z);		MeshChunk->textureCoord(right, down); MeshChunk->colour(light1.x,light1.y,light1.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}


	//z+1
	transparentBlock = DefaultBlock;
	canCreate = false;
	if(transparent)
	{
		Block1 = 1;
		if (z < WORLD_SIZE - 1)
		{
			transparentBlock = BlockTransparent(x,y,z+1);
			Block1 = GetBlock(x,y,z+1);
		}

		if (transparentBlock == false || Block1 == 0)
			canCreate = true;
	}else
	{
		if (z < WORLD_SIZE - 1)transparentBlock = BlockTransparentOrSpecial(x,y,z+1);
		if (transparentBlock == true)
			canCreate = true;
	}

	if (canCreate == true)
	{
		left = percent * blockType->sidePlane;
		right = left + percent;

		if((GetBlockSettings(x,y,z+1) & OpLighSource) != 0)//block is lightened
		{
			float lightened = (GetBlockSettings(x, y, z+1) & 0xF) / 15.0f;
			if(lightened > BlockColorz2.x)
				BlockColorz2.x = BlockColorz2.y = BlockColorz2.z = lightened;
		}

		light1 = light2 = light3 = light4 = BlockColorz2;
		lightFactor = BlockColorz2 * lightShadowFactor;

		//simple shadows
		//up
		if(!BlockTransparentOrLightSource(x,y+1,z+1) || !BlockTransparentOrLightSource(x-1,y,z+1) || !BlockTransparentOrLightSource(x-1,y+1,z+1))
		{
			light2-=lightFactor;
		}
		if(!BlockTransparentOrLightSource(x,y+1,z+1) || !BlockTransparentOrLightSource(x+1,y,z+1) || !BlockTransparentOrLightSource(x+1,y+1,z+1))
		{
			light4-=lightFactor;
		}

		//down
		if(!BlockTransparentOrLightSource(x,y-1,z+1) || !BlockTransparentOrLightSource(x-1,y,z+1) || !BlockTransparentOrLightSource(x-1,y-1,z+1))
		{
			light1-=lightFactor;
		}

		if(!BlockTransparentOrLightSource(x,y-1,z+1) || !BlockTransparentOrLightSource(x+1,y,z+1) || !BlockTransparentOrLightSource(x+1,y-1,z+1))
		{
			light3-=lightFactor;
		}

		MeshChunk->position(x,   y,   z+1);		MeshChunk->textureCoord(left, down); MeshChunk->colour(light1.x,light1.y,light1.z);
		MeshChunk->position(x+1, y,   z+1);		MeshChunk->textureCoord(right, down); MeshChunk->colour(light3.x,light3.y,light3.z);
		MeshChunk->position(x+1, y+1, z+1);		MeshChunk->textureCoord(right, up); MeshChunk->colour(light4.x,light4.y,light4.z);
		MeshChunk->position(x,   y+1, z+1);		MeshChunk->textureCoord(left, up); MeshChunk->colour(light2.x,light2.y,light2.z);

		MeshChunk->triangle(iVertex, iVertex+1, iVertex+2);
		MeshChunk->triangle(iVertex+2, iVertex+3, iVertex);

		iVertex += 4;
	}
}

void CraftWorld::UpdateWorldLightForChunk(int chunkID)
{
	int StartZ = mChunks[chunkID]->chunkStartZ;
	int StartY = mChunks[chunkID]->chunkStartY;
	int StartX = mChunks[chunkID]->chunkStartX;

	for (int z = StartZ; z < CHUNK_SIZE + StartZ; ++z)
	{
		for (int y = StartY; y < CHUNK_SIZE + StartY; ++y)
		{
			for (int x = StartX; x < CHUNK_SIZE + StartX; ++x)
			{
				block_t Block = GetBlock(x,y,z);
				if(BlockTransparent(x,y,z) == true)continue;//if block is transparent don't continue

				//light source?
				if(LightSourceBlock(Block))
					SetLigtSourcePosition(x,y,z,Block);
			}
		}
	}
}

void CraftWorld::UpdateLightAreaIn(Vector3 pos)
{
	int biggestLight = 8;

	int startx = pos.x - biggestLight > 0 ? pos.x - biggestLight : 0;
	int starty = pos.y - biggestLight > 0 ? pos.y - biggestLight : 0;
	int startz = pos.z - biggestLight > 0 ? pos.z - biggestLight : 0;

	int endx = pos.x + biggestLight < WORLD_SIZE ? pos.x + biggestLight : WORLD_SIZE;
	int endy = pos.y + biggestLight < WORLD_SIZE ? pos.y + biggestLight : WORLD_SIZE;
	int endz = pos.z + biggestLight < WORLD_SIZE ? pos.z + biggestLight : WORLD_SIZE;

	for (int z = startz; z < endz; ++z)
	{
		for (int y = starty; y < endy; ++y)
		{
			for (int x = startx; x < endx; ++x)
			{
				block_t Block = GetBlock(x,y,z);
				if(LightSourceBlock(Block))
				{
					RemoveLigtSourceAtPosition(x,y,z,Block);
					SetLigtSourcePosition(x,y,z,Block);
					AddChunkToUpdate(x,y,z);
				}
			}
		}
	}
}

void CraftWorld::AddChunkToUpdate(const int x, const int y, const int z)
{
	int chunkTarget = getChunkId(Vector3(x,y,z));
	if(chunkTarget != -1)
		mChunks[chunkTarget]->needUpdate = true;
}

void CraftWorld::RebuildChunksLight(Vector3 pos,int currentChunk,int blockID)
{
	if(blockID == JackOLantern::getID() || blockID == Torch::getID())
	{
		BoundingBox lBox = BoundingBox(Vector3(pos.x - 7,pos.y - 7,pos.z - 7),Vector3(pos.x + 7,pos.y + 7,pos.z + 7));
		for(unsigned int i = 0; i < mChunks.size();i++)
		{
			if(lBox.intersect(mChunks[i]->bBox))
			{
				rebuildChunk(i);
				rebuildTransparentChunk(i);
			}
		}
	}
}

void CraftWorld::rebuildNearestChunks(int id,Vector3 pos)
{
	Vector3 test = mChunks[id]->bBox.onBorder(pos);
	Vector3 temp = pos;
	int chunkNumber = -1;

	if(test.x != 0)
	{
		temp.x +=test.x;

		chunkNumber = getChunkId(temp);
		if(chunkNumber != -1)
		{
			rebuildChunk(chunkNumber);
			rebuildTransparentChunk(chunkNumber);
		}


		//now check for other with this... //needed by shadows
		if(test.y != 0)
		{
			temp.y +=test.y;
			chunkNumber = getChunkId(temp);
			if(chunkNumber != -1)
			{
				rebuildChunk(chunkNumber);
				rebuildTransparentChunk(chunkNumber);
			}
		}

		if(test.z != 0)
		{
			temp.z +=test.z;
			chunkNumber = getChunkId(temp);
			if(chunkNumber != -1)
			{
				rebuildChunk(chunkNumber);
				rebuildTransparentChunk(chunkNumber);
			}
		}
	}


	if(test.y != 0)
	{
		temp = pos;
		temp.y +=test.y;

		chunkNumber = getChunkId(temp);
		if(chunkNumber != -1)
		{
			rebuildChunk(chunkNumber);
			rebuildTransparentChunk(chunkNumber);
		}

		if(test.z != 0)
		{
			temp.z +=test.z;
			chunkNumber = getChunkId(temp);
			if(chunkNumber != -1)
			{
				rebuildChunk(chunkNumber);
				rebuildTransparentChunk(chunkNumber);
			}
		}
	}

	if(test.z != 0)
	{
		temp = pos;
		temp.z +=test.z;

		chunkNumber = getChunkId(temp);
		if(chunkNumber != -1)
		{
			rebuildChunk(chunkNumber);
			rebuildTransparentChunk(chunkNumber);
		}

		if(test.x != 0)
		{
			temp.x +=test.x;

			chunkNumber = getChunkId(temp);
			if(chunkNumber != -1)
			{
				rebuildChunk(chunkNumber);
				rebuildTransparentChunk(chunkNumber);
			}
		}
	}

}

void CraftWorld::createWorldChunkss()
{
	createdChunksCount = 0;

	for (int z = 0; z < WORLD_SIZE; z += CHUNK_SIZE)
	{
		for (int y = 0; y < WORLD_SIZE; y += CHUNK_SIZE)
		{
			for (int x = 0; x < WORLD_SIZE; x += CHUNK_SIZE)
			{
				createChunks(x,y,z);
				createTransparentChunks(x,y,z);
			}
		}
	}
}

void CraftWorld::SetAllChunksToUpdate()
{
	for(unsigned int i = 0;i < mChunks.size();i++)
	{
		mChunks[i]->needUpdate = true;
	}

	for(unsigned int i = 0;i < mTransparentChunks.size();i++)
	{
		mTransparentChunks[i]->needUpdate = true;
	}
}

void CraftWorld::drawCubes(int i)
{
	sceGuColor(0xFFFFFFFF);
	sceGuEnable(GU_TEXTURE_2D);
	if(blockTypes[i]->transparent)
		sceGuEnable(GU_BLEND);
	sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF |GU_COLOR_8888| GU_VERTEX_32BITF | GU_TRANSFORM_3D, 36, 0, blockTypes[i]->vertices);
	if(blockTypes[i]->transparent)
		sceGuDisable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);
}

int compareTarnsSort (const void * a, const void * b)
{
	TransparentOrder *ia = (TransparentOrder *)a;
	TransparentOrder *ib = (TransparentOrder *)b;
	return (int)( ia->distance - ib->distance );
}

void CraftWorld::drawWorld(Frustum &camFrustum,bool camUpdate)
{
	drawnTriangles = 0;

	//normal not transparend chunks
	sceGuColor(0xFFFFFFFF);
	sceGuEnable(GU_TEXTURE_2D);
	for(unsigned int i = 0;i < mChunks.size();i++)
	{
		if(mChunks[i]->trienglesCount > 0 || !mChunks[i]->created)
		{
			if(playerZoneBB.intersect(mChunks[i]->bBox))
			{
				if(camFrustum.BoxInFrustum(mChunks[i]->bBox) == Frustum::Intersects)
				{
					//check if wee need to build vertices of this chunk
					if((!mChunks[i]->created || mChunks[i]->needUpdate) && chunksCreatedInFrameCount == 0)
					{
						rebuildChunk(i);
						chunksCreatedInFrameCount++;
					}

					mChunks[i]->drawChunk();
					drawnTriangles += mChunks[i]->trienglesCount;
				}
			}else if(mChunks[i]->created)
			{
				mChunks[i]->reset();
			}
		}
	}


	//transparent chunks
	//transparent sorting
	if(camUpdate)
	{
		transOrderCont = 0;
		int j = 0;

		for(unsigned int i = 0;i < mTransparentChunks.size();i++)
		{
			if(mTransparentChunks[i]->trienglesCount > 0 || !mTransparentChunks[i]->created)
			{
				if(playerZoneBB.intersect(mTransparentChunks[i]->bBox))
				{
					transOrder[j].distance =  Vector3::distanceSq(playerPos,mTransparentChunks[i]->bBox.getCenter());
					transOrder[j].chunk = i;
					j++;
					transOrderCont++;
				}else
				{
					mTransparentChunks[i]->reset();
				}
			}
		}

		//sorting
		qsort(transOrder,transOrderCont,sizeof(TransparentOrder),compareTarnsSort);
	}

	//transparent rendering
	if(transOrderCont > 0)
	{
		for(int i = 0;i < transOrderCont;i++)
		{
			int chunk = transOrder[i].chunk;

			//check if wee need to build vertices of this chunk
			if((!mTransparentChunks[chunk]->created || mTransparentChunks[chunk]->needUpdate) && transparentchunksCreatedInFrameCount == 0)
			{
				rebuildTransparentChunk(chunk);
				transparentchunksCreatedInFrameCount++;
			}
		}
	}

	if(transOrderCont > 0)
	{
		//draw
		sceGuDepthMask(1);
		sceGuColor(0x55FFFFFF);
		sceGuEnable(GU_BLEND);
		//sceGuDisable(GU_CULL_FACE);

		for(int i = transOrderCont - 1;i >=0;i--)
		{
			int chunk = transOrder[i].chunk;
			mTransparentChunks[chunk]->drawChunk();
			drawnTriangles += mChunks[chunk]->trienglesCount;
		}

		//sceGuEnable(GU_CULL_FACE);
		sceGuDisable(GU_BLEND);
		sceGuDisable(GU_TEXTURE_2D);
		sceGuDepthMask(0);
	}
}

void CraftWorld::UpdatePlayerZoneBB(Vector3 playerPosition)
{
	playerPos = playerPosition;
	playerZoneBB = BoundingBox(Vector3(playerPosition.x - playerZoneSize.x,playerPosition.y - playerZoneSize.y,playerPosition.z - playerZoneSize.z),Vector3(playerPosition.x + playerZoneSize.x,playerPosition.y + playerZoneSize.y,playerPosition.z + playerZoneSize.z));
}

Vector3 CraftWorld::GetPlayerPos()
{
	return playerPos;
}

void CraftWorld::UpdateWorldTime(float dt)
{
	worldTime += dt;

	//stuff that goes on tick
	//1 tick = 1 second

	//rebuilding chunks
	updateChunkTimer += dt;
	if(updateChunkTimer > 0.3f)//update time 1/4 second
	{
		chunksCreatedInFrameCount = 0;
		transparentchunksCreatedInFrameCount = 0;
		updateChunkTimer = 0.0f;
	}

	if(worldTime > 1.0f)
	{
		worldTime = 0.0f;
	}

	if(!freezeDayTime)
	{
		//world day/night time
		worldHour += dt;
		if(worldHour > 50.0f)//one game hour = 50 sec
		{
			worldHour = 0.0f;
			worldDayTime += 1.0f;

			if(worldDayTime > 24.0f)
				worldDayTime = 0.0f;

			SetWolrdTime(worldDayTime);
			SetAllChunksToUpdate();
		}
	}
}

bool CraftWorld::isSolid(const int x, const int y, const int z)
{
	return !(GetBlock(x,y,z) == 0 || GetBlock(x,y,z) == 4 || GetBlock(x,y,z) == Torch::getID());
}

int CraftWorld::GetBlockTypesCount()
{
	return blockTypes.size();
}

int CraftWorld::groundHeight(const int x, const int z)
{
	if (x < 0 || z < 0 || x >= WORLD_SIZE || z >= WORLD_SIZE) return -1;

	for (int y = WORLD_SIZE - 1; y >= 0; --y)
	{
		block_t Block = GetBlock(x, y, z);
		if (Block != 0 && Block != 4) return (y);
	}

	return -1;
}

int CraftWorld::getChunkId(Vector3 pos)
{
	for(unsigned int i = 0;i < mChunks.size();i++)
	{
		if(mChunks[i]->bBox.contains(pos))
			return mChunks[i]->id;
	}

	return -1;
}

bool CraftWorld::SolidAtPoint(Vector3 pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	//outside the world so collision
	if (x <= 0 || y <= 0 || z <= 0  || x >= WORLD_SIZE-1 || y >= WORLD_SIZE-1 || z >= WORLD_SIZE-1) return true;

	//don't collide with water and air
	if(GetBlock(x,y,z) == 0 || GetBlock(x,y,z) == 4 || GetBlock(x,y,z) == Torch::getID())
	{
		//
		return false;
	}

	//collision
	return true;
}

int CraftWorld::BlockSoundAtPos(Vector3 pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	//outside the world so collision
	if (x <= 0 || y <= 0 || z <= 0  || x >= WORLD_SIZE-1 || y >= WORLD_SIZE-1 || z >= WORLD_SIZE-1) return -1;

	return blockTypes[GetBlock(x,y,z)]->soundType;
}

bool CraftWorld::SolidAtPointForPlayer(Vector3 pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	//outside the world so collision
	if (x <= 0 || y <= 0 || z <= 0  || x >= WORLD_SIZE-1 || y >= WORLD_SIZE-1 || z >= WORLD_SIZE-1) return true;

	//don't collide with water and air
	if(GetBlock(x,y,z) == 0 || GetBlock(x,y,z) == 4 || GetBlock(x,y,z) == Torch::getID())
	{
		//4 corners
		if(SolidAtPoint(Vector3(pos.x-0.15f,pos.y,pos.z-0.15f))) return true;
		if(SolidAtPoint(Vector3(pos.x+0.15f,pos.y,pos.z+0.15f))) return true;
		if(SolidAtPoint(Vector3(pos.x+0.15f,pos.y,pos.z-0.15f))) return true;
		if(SolidAtPoint(Vector3(pos.x-0.15f,pos.y,pos.z+0.15f))) return true;
		//
		return false;
	}

	//collision
	return true;
}

bool CraftWorld::PlayerInWater(Vector3 pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE-1 || y >= WORLD_SIZE-1 || z >= WORLD_SIZE-1) return false;

	if(GetBlock(x,y,z) == 4)
		return true;

	return false;
}

block_t CraftWorld::BlockAtPoint(Vector3 pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	if (x < 0 || y < 0 || z < 0  || x >= WORLD_SIZE || y >= WORLD_SIZE || z >= WORLD_SIZE) return -1;

	return GetBlock(x,y,z);
}
