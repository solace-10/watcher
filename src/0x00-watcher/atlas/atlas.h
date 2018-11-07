#pragma once

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <GL/gl.h>

class Atlas;
using AtlasUniquePtr = std::unique_ptr< Atlas >;

class Atlas
{
public:
	Atlas( int windowWidth, int windowHeight );
	~Atlas();

	void Render();
	void GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const;

	void OnWindowSizeChanged( int width, int height );

private:
	void InitialiseTileMaps();
	void LoadTextures();
	GLuint LoadTexture( const std::string& filename );
	GLuint GetTileTexture( int x, int y ) const;

	static const int m_sTileSize = 256;

	int m_NumTilesX;
	int m_NumTilesY;
	int m_TileResolution;
	std::vector< GLuint > m_LowResTextures;
	std::vector< GLuint > m_HighResTextures;

	using TileTextureIdVector = std::vector< GLuint >;
	using TileMaps = std::vector< TileTextureIdVector >;
	TileMaps m_TileMaps;
	int m_MinimumZoomLevel;
	int m_CurrentZoomLevel;
};