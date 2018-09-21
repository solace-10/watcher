#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include "atlas/atlas.h"
#include "imgui/imgui.h"

Atlas::Atlas(int numTilesX, int numTilesY, int tileResolution) :
m_NumTilesX( numTilesX ),
m_NumTilesY( numTilesY ),
m_TileResolution( tileResolution ),
m_LowResTextures( numTilesX * numTilesY ),
m_HighResTextures( numTilesX * numTilesY )
{
	SDL_assert( m_NumTilesX > 0 );
	SDL_assert( m_NumTilesY > 0 );
	SDL_assert( m_TileResolution >= 256 );
}

Atlas::~Atlas()
{

}

void Atlas::Render()
{
	float zoom = 1.0f;
	float tileSize = m_TileResolution * zoom;
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	for ( int x = 0u; x < m_NumTilesX; ++x )
	{
		for ( int y = 0; y < m_NumTilesY; ++y )
		{
			pDrawList->AddRectFilled(
				ImVec2( tileSize * (float)x, tileSize * (float)y ),
				ImVec2( tileSize * (float)( x + 1u ), tileSize * (float)( y + 1u ) ),
				( ( x + y ) % 2 == 0 ) ? ImColor( 1.0f, 0.0f, 0.0f, 0.5f ) : ImColor( 0.0f, 0.0f, 1.0f, 0.5f ) 
			);
		}
	}
}

// Returns the texture for a given tile coordinate, prefering the high resolution version if loaded.
GLuint Atlas::GetTileTexture( int x, int y ) const
{
	int idx = y * m_NumTilesX + x;
	return m_HighResTextures.at( idx ) == 0 ? m_LowResTextures[ idx ] : m_HighResTextures[ idx ];
}

void Atlas::LoadTextures()
{

}

GLuint Atlas::LoadTexture( const std::string& filename )
{
	SDL_Surface* pSurface = IMG_Load( filename.c_str() );
	SDL_assert( pSurface != nullptr );
	if ( pSurface == nullptr )
	{
		return 0;
	}

	GLuint tex;
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );
	int mode = ( pSurface->format->BytesPerPixel == 4 ) ? GL_RGBA : GL_RGB;

	glTexImage2D( GL_TEXTURE_2D, 0, mode, pSurface->w, pSurface->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pSurface->pixels );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	return tex;
}