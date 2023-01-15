#include "petit2d.h"

#include <map>
#include <cmath>
#include <string>
#include <fstream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

#ifdef _DEBUG
#  include <cstdio>
#  define DEBUG(...) printf(__VA_ARGS__)
#else
#  define DEBUG(...)
#endif

#define MAX_SPRITES_PER_SPRITE_BATCH    16384 // 65536   // yes.
#define MAX_VERTICES_PER_SHAPE_BATCH    4096
#define M_PI_DIV_180                    3.14f / 180.0f

namespace Petit2D
{

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Forward private declarations and basic types
//-----------------------------------------------------------------------------

            GLuint  compileShader       (GLenum type, const char* src);
            void    checkProgram        (GLuint id);
constexpr   GLenum  getTextureFilter    (Petit2D::Texture::Filter filter);
constexpr   GLenum  getTextureWrap      (Petit2D::Texture::Wrap wrap);
constexpr   GLenum  getInternalFormat   (Petit2D::Texture::InternalFormat format);
constexpr   GLenum  getFormat           (Petit2D::Texture::Format format);
constexpr   GLenum  getDataType         (Petit2D::Texture::DataType dataType);
constexpr   GLenum  getDrawType         (Petit2D::Shape::DrawType drawType);

//-----------------------------------------------------------------------------
// [SECTION] Texture
//-----------------------------------------------------------------------------

namespace Texture
{

struct Context
{
    GLuint texture[TextureUnit::UNIT_COUNT] = { 0 };
} g_context;

struct Texture
{
    GLuint  id      = 0;
    int     width   = 0;
    int     height  = 0;
};

Texture* Create()
{
    GLuint id;
    glGenTextures(1, &id);

    if (id == 0)
    {
        return nullptr;
    }

    auto texture = new Texture();
    texture->id = id;

    return texture;
}

void Destroy(Texture* texture)
{
    if (texture != nullptr)
    {
        glDeleteTextures(1, &texture->id);
        delete(texture);
        texture = nullptr;
    }
}

void Init(Texture* texture, const char* filename)
{
    int width;
    int height;
    int channels;
    int desired_channels = 4;

    auto image = stbi_load(filename, &width, &height, &channels, desired_channels);
    if(image == nullptr)
    {
        DEBUG("Error in loading the image %s\n", filename);
        return;
    }

    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    texture->width = width;
    texture->height = height;

    stbi_image_free(image);
}

void Init(Texture* texture, int width, int height, InternalFormat internalFormat, Format format, DataType type, void* pixels)
{
    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, getInternalFormat(internalFormat), width, height, 0, getFormat(format), getDataType(type), pixels);
    texture->width = width;
    texture->height = height;
}

void SetWrap(Texture* texture, Wrap s, Wrap t)
{
    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getTextureWrap(s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getTextureWrap(t));
}

void SetFilter(Texture* texture, Filter min, Filter mag)
{
    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getTextureFilter(min));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getTextureFilter(mag));
}

int GetWidth(const Texture* texture)
{
    return texture->width;
}

int GetHeight(const Texture* texture)
{
    return texture->height;
}

} // namespace Texture

//-----------------------------------------------------------------------------
// [SECTION] FrameBuffer
//-----------------------------------------------------------------------------

namespace FrameBuffer
{

struct Context
{
    GLuint frameBuffer = 0;
} g_context;

struct FrameBuffer
{
    GLuint frameBufferId;
    GLuint renderBufferId;
};

FrameBuffer* Create()
{
    GLuint frameBufferId;
    glGenFramebuffers(1, &frameBufferId);

    if (frameBufferId == 0)
    {
        return nullptr;
    }

    GLuint renderBufferId;
    glGenRenderbuffers(1, &renderBufferId);

    if (renderBufferId == 0)
    {
        glDeleteFramebuffers(1, &frameBufferId);
        return nullptr;
    }

    auto frameBuffer = new FrameBuffer();
    frameBuffer->frameBufferId = frameBufferId;
    frameBuffer->renderBufferId = renderBufferId;

    return frameBuffer;
}

void Destroy(FrameBuffer* frameBuffer)
{
    if (frameBuffer != nullptr)
    {
        glDeleteFramebuffers(1, &frameBuffer->frameBufferId);
        glDeleteRenderbuffers(1, &frameBuffer->renderBufferId);
        delete(frameBuffer);
        frameBuffer = nullptr;
    }
}

void Init(FrameBuffer* frameBuffer, const Texture::Texture* texture)
{
    glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->renderBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture->width, texture->height);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->frameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->renderBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        DEBUG("glCheckFramebufferStatus\n");
    }

    g_context.frameBuffer = frameBuffer->frameBufferId;
}

} // namespace FrameBuffer

//-----------------------------------------------------------------------------
// [SECTION] Sprites
//-----------------------------------------------------------------------------

namespace Sprite
{

const char* VERTEX_SRC = R"text(
    #version 330 core
    precision lowp float;

    layout (location = 0) in vec2 size;
    layout (location = 1) in vec4 coords;
    layout (location = 2) in vec4 color;
    layout (location = 3) in float angle;
    layout (location = 4) in vec2 translation;
    layout (location = 5) in vec2 scale;

    out vec4 inColor;
    out vec2 inTexCoord;

    uniform mat4 projection;

    void main() {
        const ivec2 tlut[4] = ivec2[4] (
            ivec2(2, 1),
            ivec2(0, 1),
            ivec2(2, 3),
            ivec2(0, 3)
        );

        const vec2 plut[4] = vec2[4] (
            vec2(0.5, -0.5),
            vec2(-0.5, -0.5),
            vec2(0.5, 0.5),
            vec2(-0.5, 0.5)
        );
        
        mat3 rotate_mat = mat3 (
            cos(angle), -sin(angle), 0.0,
            sin(angle), cos(angle), 0.0,
            0.0, 0.0, 1.0
        );

        mat3 scale_mat = mat3 (
            scale.x, 0.0, 0.0,
            0.0, scale.y, 0.0,
            0.0, 0.0, 1.0
        );

        mat3 translate_mat = mat3 (
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            translation.x, translation.y, 0.0
        );

        vec3 transformed = translate_mat * rotate_mat * scale_mat * vec3(plut[gl_VertexID] * size, 1.0);
        gl_Position = projection * vec4(transformed, 1.0);
        inTexCoord = vec2(coords[tlut[gl_VertexID].x], coords[tlut[gl_VertexID].y]);
        inColor = color;
    }
)text";

const char* FRAGMENT_SRC = R"text(
    #version 330 core
    precision lowp float;

    in vec4 inColor;
    in vec2 inTexCoord;

    out vec4 fragColor;

    uniform sampler2D tex2D;

    void main() {
        fragColor = inColor * texture(tex2D, inTexCoord);
    }
)text";

struct Context
{
    GLuint  vertexBufferId          = 0;
    GLuint  vertexArrayId           = 0;
    int     spriteCount             = 0;
    int     maxSprite               = 0;
    void*   storage                 = nullptr;

    GLuint  programShaderId         = 0;
    GLint   matrixUniform           = 0;
    GLint   textureUniform          = 0;
    GLint   sizeLocation            = 0;
    GLint   colorLocation           = 0;
    GLint   coordsLocation         = 0;
    GLint   translationLocation     = 0;
    GLint   scaleLocation           = 0;
    GLint   angleLocation           = 0;
} g_context;

struct SpriteInstance
{
    float s                 = 0.0f; // 4
    float t                 = 0.0f; // 8
    float p                 = 0.0f; // 12
    float q                 = 0.0f; // 16
    float rotation          = 0.0f; // 24
    float scale_x           = 0.0f; // 32
    float scale_y           = 0.0f; // 40
    short w                 = 0; // 42
    short h                 = 0; // 44
    short translation_x     = 0; // 46
    short translation_y     = 0; // 48
    unsigned char r         = 0; // 49
    unsigned char g         = 0; // 50
    unsigned char b         = 0; // 51
    unsigned char a         = 0; // 52
};

void Create()
{
    auto vertexShader = compileShader(GL_VERTEX_SHADER, VERTEX_SRC);
    auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SRC);

    g_context.programShaderId = glCreateProgram();
    glAttachShader(g_context.programShaderId, vertexShader);
    glAttachShader(g_context.programShaderId, fragmentShader);
    glLinkProgram(g_context.programShaderId);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    checkProgram(g_context.programShaderId);

    g_context.matrixUniform = glGetUniformLocation(g_context.programShaderId, "projection");
    g_context.textureUniform = glGetUniformLocation(g_context.programShaderId, "tex2D");
    g_context.sizeLocation = glGetAttribLocation(g_context.programShaderId, "size");
    g_context.colorLocation = glGetAttribLocation(g_context.programShaderId, "color");
    g_context.coordsLocation = glGetAttribLocation(g_context.programShaderId, "coords");
    g_context.translationLocation = glGetAttribLocation(g_context.programShaderId, "translation");
    g_context.scaleLocation = glGetAttribLocation(g_context.programShaderId, "scale");
    g_context.angleLocation = glGetAttribLocation(g_context.programShaderId, "angle");

    glGenBuffers(1, &g_context.vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteInstance) * MAX_SPRITES_PER_SPRITE_BATCH, nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &g_context.vertexArrayId);
    glBindVertexArray(g_context.vertexArrayId);

    glVertexAttribPointer(g_context.sizeLocation, 2, GL_SHORT, false, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, w));
    glVertexAttribPointer(g_context.coordsLocation, 4, GL_FLOAT, false, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, s));
    glVertexAttribPointer(g_context.colorLocation, 4, GL_UNSIGNED_BYTE, true, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, r));
    glVertexAttribPointer(g_context.angleLocation, 1, GL_FLOAT, false, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, rotation));
    glVertexAttribPointer(g_context.translationLocation, 2, GL_SHORT, false, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, translation_x));
    glVertexAttribPointer(g_context.scaleLocation, 2, GL_FLOAT, false, sizeof(SpriteInstance), (void*) offsetof(SpriteInstance, scale_x));

    glVertexAttribDivisor(g_context.sizeLocation, 1);
    glVertexAttribDivisor(g_context.coordsLocation, 1);
    glVertexAttribDivisor(g_context.colorLocation, 1);
    glVertexAttribDivisor(g_context.angleLocation, 1);
    glVertexAttribDivisor(g_context.translationLocation, 1);
    glVertexAttribDivisor(g_context.scaleLocation, 1);

    glEnableVertexAttribArray(g_context.sizeLocation);
    glEnableVertexAttribArray(g_context.coordsLocation);
    glEnableVertexAttribArray(g_context.colorLocation);
    glEnableVertexAttribArray(g_context.angleLocation);
    glEnableVertexAttribArray(g_context.translationLocation);
    glEnableVertexAttribArray(g_context.scaleLocation);

    glBindVertexArray(0);
}

void Destroy()
{
    glDeleteProgram(g_context.programShaderId);
    glDeleteBuffers(1, &g_context.vertexBufferId);
    glDeleteVertexArrays(1, &g_context.vertexArrayId);
}

void Use()
{
    glUseProgram(g_context.programShaderId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBindVertexArray(g_context.vertexArrayId);
}

void SetTexture(Texture::TextureUnit unit)
{
    glUniform1i(g_context.textureUniform, unit);
}

void SetMatrix(const float* value)
{
    glUniformMatrix4fv(g_context.matrixUniform, 1, GL_FALSE, value);
}

void Begin()
{
    if (g_context.maxSprite < g_context.spriteCount)
    {
        g_context.maxSprite = g_context.spriteCount;
    }
    
    g_context.spriteCount = 0;
    g_context.storage = glMapBufferRange
    (
        GL_ARRAY_BUFFER,
        0,
        sizeof(SpriteInstance) * MAX_SPRITES_PER_SPRITE_BATCH,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
    );
}

void Add(const Sprite& sprite)
{
    if (g_context.storage == nullptr)
    {
        DEBUG("Sprite storage nullptr\n");
        return;
    }

    if (g_context.spriteCount >= MAX_SPRITES_PER_SPRITE_BATCH)
    {
        DEBUG("spriteCount >= MAX_SPRITES_PER_SPRITE_BATCH\n");
        return;
    }

    auto storage = static_cast<SpriteInstance*>(g_context.storage);
    auto& instance = storage[g_context.spriteCount];
    instance.w = sprite.width;
    instance.h = sprite.height;
    instance.s = sprite.s;
    instance.t = sprite.t;
    instance.p = sprite.p;
    instance.q = sprite.q;
    instance.r = sprite.r;
    instance.g = sprite.g ;
    instance.b = sprite.b;
    instance.a = sprite.a;
    instance.translation_x = sprite.x;
    instance.translation_y = sprite.y;
    instance.rotation = sprite.rotation * M_PI_DIV_180;
    instance.scale_x = sprite.scale_x;
    instance.scale_y = sprite.scale_y;

    g_context.spriteCount += 1;
}

void End()
{
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, sizeof(SpriteInstance) * g_context.spriteCount);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Render()
{
    if (g_context.spriteCount > 0)
    {
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g_context.spriteCount);
    }
}

int GetMaxSprites()
{
    return g_context.maxSprite;
}

} // namespace Sprite

//-----------------------------------------------------------------------------
// [SECTION] Shapes
//-----------------------------------------------------------------------------

namespace Shape
{

const char* VERTEX_SRC = R"text(
    #version 330 core
    precision lowp float;

    layout (location = 0) in vec2 position;
    layout (location = 1) in vec4 color;

    out vec4 inColor;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(position, 0.0, 1.0);
        inColor = color;
    }
)text";

const char* FRAGMENT_SRC = R"text(
    #version 330 core
    precision lowp float;

    in vec4 inColor;

    out vec4 fragColor;

    void main() {
        fragColor = inColor;
    }
)text";

struct Context
{
    GLuint  vertexBufferId          = 0;
    GLuint  vertexArrayId           = 0;
    int     verticesCount           = 0;
    int     maxVertices             = 0;
    float   pointSize               = 1.0f;
    float   lineWidth               = 1.0f;
    void*   storage                 = nullptr;

    GLuint  programShaderId         = 0;
    GLint   matrixUniform           = 0;
    GLint   vertexLocation          = 0;
    GLint   colorLocation           = 0;
} g_context;

void Create()
{
    auto vertexShader = compileShader(GL_VERTEX_SHADER, VERTEX_SRC);
    auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SRC);

    g_context.programShaderId = glCreateProgram();
    glAttachShader(g_context.programShaderId, vertexShader);
    glAttachShader(g_context.programShaderId, fragmentShader);
    glLinkProgram(g_context.programShaderId);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    checkProgram(g_context.programShaderId);

    g_context.matrixUniform = glGetUniformLocation(g_context.programShaderId, "projection");
    g_context.vertexLocation = glGetAttribLocation(g_context.programShaderId, "position");
    g_context.colorLocation = glGetAttribLocation(g_context.programShaderId, "color");
 
    glGenBuffers(1, &g_context.vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES_PER_SHAPE_BATCH, nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &g_context.vertexArrayId);
    glBindVertexArray(g_context.vertexArrayId);

    glVertexAttribPointer(g_context.vertexLocation, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, x));
    glVertexAttribPointer(g_context.colorLocation, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*) offsetof(Vertex, r));

    glEnableVertexAttribArray(g_context.vertexLocation);
    glEnableVertexAttribArray(g_context.colorLocation);

    glBindVertexArray(0);
}

void Destroy()
{
    glDeleteProgram(g_context.programShaderId);
    glDeleteBuffers(1, &g_context.vertexBufferId);
    glDeleteVertexArrays(1, &g_context.vertexArrayId);
}

void Use()
{
    glUseProgram(g_context.programShaderId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBindVertexArray(g_context.vertexArrayId);
}

void SetMatrix(const float* value)
{
    glUniformMatrix4fv(g_context.matrixUniform, 1, GL_FALSE, value);
}

void SetPointSize(float size)
{
    if (size < 0)
    {
        DEBUG("Point size less than 0, adjusting to 0.\n");
        size = 0;

    }

    if (size > 32)
    {
        DEBUG("Point size greater than 32, adjusting to 32.\n");
        size = 32;
    }

    g_context.pointSize = size;
    glPointSize(size);
}

float GetPointSize()
{
    return g_context.pointSize;
}

void SetLineWidth(float width)
{
    if (width < 0)
    {
        DEBUG("Line width less than 0, adjusting to 0.\n");
        width = 0;

    }

    if (width > 32)
    {
        DEBUG("Line width greater than 32, adjusting to 32.\n");
        width = 32;
    }

    g_context.lineWidth = width;
    glLineWidth(width);
}

float GetLineWidth()
{
    return g_context.lineWidth;
}

void Begin()
{
    if (g_context.maxVertices < g_context.verticesCount)
    {
        g_context.maxVertices = g_context.verticesCount;
    }

    g_context.verticesCount = 0;
    g_context.storage = glMapBufferRange
    (
        GL_ARRAY_BUFFER,
        0,
        sizeof(Vertex) * MAX_VERTICES_PER_SHAPE_BATCH,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
    );
}

void Add(const Vertex& vertex)
{
    if (g_context.storage == nullptr)
    {
        DEBUG("Shape storage nullptr\n");
        return;
    }

    if (g_context.verticesCount >= MAX_VERTICES_PER_SHAPE_BATCH)
    {
        DEBUG("verticesCount >= MAX_VERTICES_PER_SHAPE_BATCH\n");
        return;
    }

    auto storage = static_cast<Vertex*>(g_context.storage);
    auto index = g_context.verticesCount;
        
    storage[index].x = vertex.x;
    storage[index].y = vertex.y;
    storage[index].r = vertex.r;
    storage[index].g = vertex.g;
    storage[index].b = vertex.b;
    storage[index].a = vertex.a;

    ++g_context.verticesCount;
}

void End()
{
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * g_context.verticesCount);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Render(const DrawType drawType)
{
    if (g_context.verticesCount > 0)
    {
        glDrawArrays(getDrawType(drawType), 0, g_context.verticesCount);
    }
}

int GetMaxVertices()
{
    return g_context.maxVertices;
}

} // namespace Shape

//-----------------------------------------------------------------------------
// [SECTION] Catalog
//-----------------------------------------------------------------------------

namespace Catalog
{

//-----------------------------------------------------------------------------
// [SECTION] Catalog - Forward declarations and basic types
//-----------------------------------------------------------------------------

struct  Catalog
{
    int imageWidth      = 0;
    int imageHeight     = 0;
    int spriteCount     = 0;
    std::map<std::string, SpriteDef> sprites;
};

//-----------------------------------------------------------------------------
// [SECTION] Catalog - End-user API functions
//-----------------------------------------------------------------------------

Catalog* Create()
{
    return new Catalog();
}

void Init(Catalog* catalog, const char* filename)
{
    auto file = std::ifstream(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        DEBUG("Catalog not found %s\n", filename);
        return;
    }

    if (file.tellg() <= 6)
    {
        DEBUG("Not a catalog file %s\n", filename);
        return;
    }

    char check[7] = { 0 };
    file.seekg(std::ios::beg);
    file.read(check, 6);

    if (std::string(check) != "SPRCAT")
    {
        file.close();
        DEBUG("SPRCAT signature not present.\n");
        return;
    }

    file.read(reinterpret_cast<char*>(&catalog->imageWidth), sizeof(catalog->imageWidth));
    file.read(reinterpret_cast<char*>(&catalog->imageHeight), sizeof(catalog->imageHeight));
    file.read(reinterpret_cast<char*>(&catalog->spriteCount), sizeof(catalog->spriteCount));

    catalog->sprites.clear();
    for (int i=0; i<catalog->spriteCount; ++i)
    {
        char name[32] = { 0 };
        file.read(name, 32);

        SpriteDef spriteDef;
        file.read(reinterpret_cast<char*>(&spriteDef), sizeof(SpriteDef));
        
        catalog->sprites.insert(
            std::pair<std::string, SpriteDef>(std::string(name), spriteDef)
        );
    }

    file.close();
}

void Destroy(Catalog* catalog)
{
    if (catalog != nullptr)
    {
        catalog->sprites.clear();
        delete(catalog);
        catalog = nullptr;
    }
}

void Set(Catalog* catalog, const char* name, Sprite::Sprite& sprite, bool setWidth, bool setHeight)
{
    if (catalog->sprites.find(name) == catalog->sprites.end())
    {
        DEBUG("Sprite not found: %s\n", name);
        return;
    }

    const auto& spriteDef = catalog->sprites[name];
    if (setWidth)
    {
        sprite.width = spriteDef.width;
    }
    if (setHeight)
    {
        sprite.height = spriteDef.height;
    }
    sprite.s = spriteDef.s;
    sprite.t = spriteDef.t;
    sprite.p = spriteDef.p;
    sprite.q = spriteDef.q;
}

void Set(Catalog* catalog, const char* name, Sprite::Sprite& sprite, int width, int height)
{
    if (catalog->sprites.find(name) == catalog->sprites.end())
    {
        DEBUG("Sprite not found: %s\n", name);
        return;
    }

    const auto& spriteDef = catalog->sprites[name];
    sprite.width = width;
    sprite.height = height;
    sprite.s = spriteDef.s;
    sprite.t = spriteDef.t;
    sprite.p = spriteDef.p;
    sprite.q = spriteDef.q;
}

SpriteDef Get(Catalog* catalog, const char* name)
{
    if (catalog->sprites.find(name) == catalog->sprites.end())
    {
        DEBUG("Sprite not found: %s\n", name);
        return SpriteDef();
    }

    return catalog->sprites[name];
}

void PopulateFontGlyphs(std::vector<SpriteDef>& glyphs, const SpriteDef& spriteDef)
{
    auto glyphWidth = (spriteDef.p - spriteDef.s) / 16.0f;
    auto glyphHeight = (spriteDef.q - spriteDef.t)  / 16.0f;
    auto fontWidth = spriteDef.width / 16;
    auto fontHeight = spriteDef.height / 16;
    for (auto y=0; y<16; ++y)
    {
        for (auto x=0; x<16; ++x)
        {
            glyphs.emplace_back((SpriteDef) {
                .width = fontWidth,
                .height = fontHeight,
                .s = spriteDef.s + (x * glyphWidth),
                .t = spriteDef.t + (y * glyphHeight),
                .p = spriteDef.s + ((x * glyphWidth) + glyphWidth),
                .q = spriteDef.t + ((y * glyphHeight) + glyphHeight)
            });
        }
    }
}

} // namespace Catalog

//-----------------------------------------------------------------------------
// [SECTION] Petit2D
//-----------------------------------------------------------------------------

void Create()
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_WRITEMASK);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);

    Shape::Create();
    Shape::SetPointSize(1.0f);
    Shape::SetLineWidth(1.0f);
    Sprite::Create();
}

void Destroy()
{
    Shape::Destroy();
    Sprite::Destroy();
}

void SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void SetBlending(BlendMode mode)
{
    switch (mode)
    {
    default: 
    case NONE:      glDisable(GL_BLEND); break;
    case ALPHA:     glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
    case ADDITIVE:  glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
    }
}

void SetFrameBuffer(const FrameBuffer::FrameBuffer* frameBuffer)
{
    if (frameBuffer == nullptr)
    {
        if (FrameBuffer::g_context.frameBuffer != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            FrameBuffer::g_context.frameBuffer = 0;
        }
    }
    else
    {
        if (FrameBuffer::g_context.frameBuffer != frameBuffer->frameBufferId)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->frameBufferId);
            FrameBuffer::g_context.frameBuffer = frameBuffer->frameBufferId;
        }
    }
}

void SetTexture(const Texture::Texture* texture, Texture::TextureUnit unit)
{
    if (Texture::g_context.texture[unit] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        Texture::g_context.texture[unit] = texture->id;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Private declarations and basic types
//-----------------------------------------------------------------------------

GLuint compileShader(GLenum type, const char* src)
{
    GLint success;
    auto id = glCreateShader(type);

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[512] = { 0 };
        glGetShaderInfoLog(id, 512, nullptr, buf);
        DEBUG("Could not compile shader: %s\n", buf);
        return 0;
    }

    return id;
}

void checkProgram(GLuint id)
{
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        char buf[512] = { 0 };
        glGetProgramInfoLog(id, 512, nullptr, buf);
        DEBUG("Could not link program: %s\n", buf);
    }
}

constexpr GLenum getTextureFilter(Petit2D::Texture::Filter filter)
{
    switch (filter)
    {
    default:
    case Petit2D::Texture::Filter::NEAREST:   return GL_NEAREST;
    case Petit2D::Texture::Filter::LINEAR:    return GL_LINEAR;
    }
}

constexpr GLenum getTextureWrap(Petit2D::Texture::Wrap wrap)
{
    switch (wrap)
    {
    case Petit2D::Texture::Wrap::REPEAT:    return GL_REPEAT;
    case Petit2D::Texture::Wrap::CLAMP:     return GL_CLAMP_TO_EDGE;
    default:                                return wrap;
    }
}
constexpr GLenum getInternalFormat(Petit2D::Texture::InternalFormat format)
{
    switch (format)
    {
    case Petit2D::Texture::InternalFormat::RGBA8:   return GL_RGBA8;
    default:                                        return format;
    }
}

constexpr GLenum getFormat(Petit2D::Texture::Format format)
{
    switch (format)
    {
    case Petit2D::Texture::Format::RGBA:    return GL_RGBA;
    default:                                return format;
    }
}

constexpr GLenum getDataType(Petit2D::Texture::DataType dataType)
{
    switch (dataType)
    {
    case Petit2D::Texture::DataType::UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
    default:                                        return dataType;
    }
}

constexpr GLenum getDrawType(Petit2D::Shape::DrawType drawType)
{
    switch (drawType) 
    {
    default:
    case Petit2D::Shape::DrawType::POINTS:
        return GL_POINTS;
    break;

    case Petit2D::Shape::DrawType::LINES:
        return GL_LINES;
    break;

    case Petit2D::Shape::DrawType::LINE_STRIP:
        return GL_LINE_STRIP;
    break;

    case Petit2D::Shape::DrawType::LINE_LOOP:
        return GL_LINE_LOOP;
    break;

    case Petit2D::Shape::DrawType::TRIANGLES:
        return GL_TRIANGLES;
    break;

    case Petit2D::Shape::DrawType::TRIANGLES_STRIP:
        return GL_TRIANGLE_STRIP;
    break;
    
    case Petit2D::Shape::DrawType::TRIANGLES_FAN:
        return GL_TRIANGLE_FAN;
    break;
    }
}

} // namespace Petit2D
