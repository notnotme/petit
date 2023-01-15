#pragma once

#include <vector>

namespace Petit2D
{

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Forward declarations and basic types
//-----------------------------------------------------------------------------

enum            BlendMode           : int;

//-----------------------------------------------------------------------------
// [SECTION] Texture
//-----------------------------------------------------------------------------

namespace Texture
{
    //-----------------------------------------------------------------------------
    // [SECTION] Texture - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    struct          Texture;

    enum            TextureUnit     : int;
    enum            InternalFormat  : int;
    enum            Format          : int;
    enum            DataType        : int;
    enum            Wrap            : int;
    enum            Filter          : int;

    //-----------------------------------------------------------------------------
    // [SECTION] Texture - End-user API functions
    //-----------------------------------------------------------------------------

    Texture*        Create          ();
    void            Destroy         (Texture* texture);
    void            Init            (Texture* texture, const char* filename);
    void            Init            (Texture* texture, int width, int height, InternalFormat internalFormat, Format format, DataType type, void* pixels);
    void            SetWrap         (Texture* texture, Wrap s, Wrap t);
    void            SetFilter       (Texture* texture, Filter min, Filter mag);
    int             GetWidth        (const Texture* texture);
    int             GetHeight       (const Texture* texture);

} // namespace Texture

//-----------------------------------------------------------------------------
// [SECTION] FrameBuffer
//-----------------------------------------------------------------------------

namespace FrameBuffer
{
    //-----------------------------------------------------------------------------
    // [SECTION] FrameBuffer - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    struct          FrameBuffer;
    
    //-----------------------------------------------------------------------------
    // [SECTION] FrameBuffer - End-user API functions
    //-----------------------------------------------------------------------------

    FrameBuffer*    Create          ();
    void            Destroy         (FrameBuffer* frameBuffer);
    void            Init            (FrameBuffer* frameBuffer, const Texture::Texture* texture);

} // namespace FrameBuffer

//-----------------------------------------------------------------------------
// [SECTION] Sprites
//-----------------------------------------------------------------------------

namespace Sprite
{

    //-----------------------------------------------------------------------------
    // [SECTION] Sprites - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    struct          Sprite;
    
    //-----------------------------------------------------------------------------
    // [SECTION] Sprites - End-user API functions
    //-----------------------------------------------------------------------------

    void            Use             ();
    void            SetTexture      (Texture::TextureUnit unit);
    void            SetMatrix       (const float* value);
    void            Begin           ();
    void            Add             (const Sprite& sprite);
    void            End             ();
    void            Render          ();
    int             GetMaxSprites   ();

} // namespace Sprite

//-----------------------------------------------------------------------------
// [SECTION] Shapes
//-----------------------------------------------------------------------------

namespace Shape
{

    //-----------------------------------------------------------------------------
    // [SECTION] Shapes - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    struct      Vertex;

    enum        DrawType            : int;
    
    //-----------------------------------------------------------------------------
    // [SECTION] Shapes - End-user API functions
    //-----------------------------------------------------------------------------

    void        Use                 ();
    void        SetMatrix           (const float* value);
    void        SetPointSize        (float size);
    float       GetPointSize        ();
    void        SetLineWidth        (float width);
    float       GetLineWidth        ();
    void        Begin               ();
    void        Add                 (const Vertex& vertex);
    void        End                 ();
    void        Render              (const DrawType drawType);
    int         GetMaxVertices      ();

} // namespace Shape

//-----------------------------------------------------------------------------
// [SECTION] Catalog
//-----------------------------------------------------------------------------

namespace Catalog
{

    //-----------------------------------------------------------------------------
    // [SECTION] Catalog - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    struct      Catalog;
    struct      SpriteDef;
    
    //-----------------------------------------------------------------------------
    // [SECTION] Catalog - End-user API functions
    //-----------------------------------------------------------------------------

    Catalog*    Create              ();
    void        Init                (Catalog* catalog, const char* filename);
    void        Destroy             (Catalog* catalog);
    void        Set                 (Catalog* catalog, const char* name, Sprite::Sprite& sprite, bool setWidth = true, bool setHeight = true);
    void        Set                 (Catalog* catalog, const char* name, Sprite::Sprite& sprite, int width, int height);
    SpriteDef   Get                 (Catalog* catalog, const char* name);
    void        PopulateFontGlyphs  (std::vector<SpriteDef>& glyphs, const SpriteDef& spriteDef);

} // namespace Catalog

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - End-user API functions
//-----------------------------------------------------------------------------

void            Create              ();
void            Destroy             ();
void            SetClearColor       (float r, float g, float b, float a);
void            Clear               ();
void            SetViewport         (int x, int y, int width, int height);
void            SetBlending         (BlendMode mode);
void            SetFrameBuffer      (const FrameBuffer::FrameBuffer* FrameBuffer);
void            SetTexture          (const Texture::Texture* texture, Texture::TextureUnit unit);

} // namespace Petit2D

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Public declarations and basic types
//-----------------------------------------------------------------------------

enum Petit2D::BlendMode : int
{
    NONE                = 0,
    ALPHA               = 1,
    ADDITIVE            = 2
};

//-----------------------------------------------------------------------------
// [SECTION] Sprites - Public declarations and basic types
//-----------------------------------------------------------------------------

struct Petit2D::Sprite::Sprite
{
    float           x           = 0.0f;
    float           y           = 0.0f;
    float           s           = 0.0f;
    float           t           = 0.0f;
    float           p           = 1.0f;
    float           q           = 1.0f;
    unsigned char   r           = 255;
    unsigned char   g           = 255;
    unsigned char   b           = 255;
    unsigned char   a           = 255;
    float           scale_x     = 1.0f;
    float           scale_y     = 1.0f;
    float           rotation    = 0.0f;
    int             width       = 0.0f;
    int             height      = 0.0f;
};

//-----------------------------------------------------------------------------
// [SECTION] Shapes - Public declarations and basic types
//-----------------------------------------------------------------------------

struct Petit2D::Shape::Vertex
{
    float   x           = 0.0f;
    float   y           = 0.0f;
    unsigned char   r   = 255;
    unsigned char   g   = 255;
    unsigned char   b   = 255;
    unsigned char   a   = 255;
};

enum Petit2D::Shape::DrawType : int
{
    POINTS              = 0,
    LINES               = 1,
    LINE_STRIP          = 2,
    LINE_LOOP           = 3,
    TRIANGLES           = 4,
    TRIANGLES_STRIP     = 5,
    TRIANGLES_FAN       = 6
};

//-----------------------------------------------------------------------------
// [SECTION] Texture - Public declarations and basic types
//-----------------------------------------------------------------------------

enum Petit2D::Texture::TextureUnit : int
{
    UNIT_0              = 0,
    UNIT_1              = 1,
    UNIT_2              = 2,
    UNIT_3              = 3,
    UNIT_COUNT          = 4
};

enum Petit2D::Texture::InternalFormat : int
{
    RGBA8               = 0
};

enum Petit2D::Texture::Format : int
{
    RGBA                = 0
};

enum Petit2D::Texture::DataType : int
{
    UNSIGNED_BYTE       = 0
};

enum Petit2D::Texture::Wrap : int
{
    CLAMP               = 0,
    REPEAT              = 1
};

enum Petit2D::Texture::Filter : int
{
    NEAREST             = 0,
    LINEAR              = 1
};

//-----------------------------------------------------------------------------
// [SECTION] Catalog - Public declarations and basic types
//-----------------------------------------------------------------------------

struct Petit2D::Catalog::SpriteDef
{
    int     width       = 0;
    int     height      = 0;
    float   s           = 0.0f;
    float   t           = 0.0f;
    float   p           = 1.0f;
    float   q           = 1.0f;
};
