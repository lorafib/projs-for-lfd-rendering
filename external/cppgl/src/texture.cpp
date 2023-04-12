#include "texture.h"
#include <vector>
#include <iostream>
#include "image_load_store.h"

CPPGL_NAMESPACE_BEGIN

// ----------------------------------------------------
// helper funcs

inline uint32_t format_to_channels(GLint format) {
    return format == GL_RGBA ? 4 : format == GL_RGB ? 3 : format == GL_RG ? 2 : 1;
}
inline GLint channels_to_format(uint32_t channels) {
    return channels == 4 ? GL_RGBA : channels == 3 ? GL_RGB : channels == 2 ? GL_RG : GL_RED;
}
inline GLint channels_to_float_format(uint32_t channels) {
    return channels == 4 ? GL_RGBA32F : channels == 3 ? GL_RGB32F : channels == 2 ? GL_RG32F : GL_R32F;
}
inline GLint channels_to_ubyte_format(uint32_t channels) {
    return channels == 4 ? GL_RGBA8 : channels == 3 ? GL_RGB8 : channels == 2 ? GL_RG8 : GL_R8;
}

// ----------------------------------------------------
// Texture2D

Texture2DImpl::Texture2DImpl(const std::string& name, const fs::path& path, bool mipmap) : name(name), loaded_from_path(path), id(0) {
    // load image from disk
    auto [data, w_out, h_out, channels, is_hdr] = image_load(path);
    this->w = w_out;
    this->h = h_out;

    if (is_hdr) {
        internal_format = channels_to_float_format(channels);
        type = GL_FLOAT;
    } else {
        internal_format = channels_to_ubyte_format(channels);
        type = GL_UNSIGNED_BYTE;
    }
    format = channels_to_format(channels);

    // init GL texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    //opengl by default needs 4 byte alignment after every row
    //stbi loaded data is not aligned that way -> pixelStore attributes need to be set
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, &data[0]);
    if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2DImpl::Texture2DImpl(const std::string& name, uint32_t w, uint32_t h, GLint internal_format, GLenum format, GLenum type, const void* data, bool mipmap)
    : name(name), id(0), w(w), h(h), internal_format(internal_format), format(format), type(type) {
    // init GL texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL) ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            mipmap ? GL_LINEAR_MIPMAP_LINEAR : (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL) ? GL_NEAREST : GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, data);
    if (mipmap && data != 0) glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2DImpl::~Texture2DImpl() {
    if (glIsTexture(id))
        glDeleteTextures(1, &id);
}

void Texture2DImpl::resize(uint32_t w, uint32_t h) {
    this->w = w;
    this->h = h;
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2DImpl::bind(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture2DImpl::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2DImpl::bind_image(uint32_t unit, GLenum access, GLenum format) const {
    glBindImageTexture(unit, id, 0, GL_FALSE, 0, access, format);
}

void Texture2DImpl::unbind_image(uint32_t unit) const {
    glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
}

void Texture2DImpl::save_ldr(const fs::path& path, bool flip, bool async) const {
    std::vector<uint8_t> pixels(size_t(w) * h * format_to_channels(format));
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, &pixels[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
    image_store_ldr(path, pixels.data(), w, h, format_to_channels(format), flip, async);
}

// ----------------------------------------------------
// Texture3D

Texture3DImpl::Texture3DImpl(const std::string& name, uint32_t w, uint32_t h, uint32_t d, GLint internal_format, GLenum format, GLenum type, const void* data, bool mipmap)
    : name(name), id(0), w(w), h(h), d(d), internal_format(internal_format), format(format), type(type) {
    // init GL texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);
    // default border color is (0, 0, 0, 0)
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, internal_format, w, h, d, 0, format, type, data);
    if (mipmap && data != 0) glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3DImpl::~Texture3DImpl() {
    if (glIsTexture(id))
        glDeleteTextures(1, &id);
}

void Texture3DImpl::resize(uint32_t w, uint32_t h, uint32_t d) {
    this->w = w;
    this->h = h;
    this->d = d;
    glBindTexture(GL_TEXTURE_3D, id);
    glTexImage2D(GL_TEXTURE_3D, 0, internal_format, w, h, 0, format, type, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture3DImpl::bind(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_3D, id);
}

void Texture3DImpl::unbind() const {
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture3DImpl::bind_image(uint32_t unit, GLenum access, GLenum format) const {
    glBindImageTexture(unit, id, 0, GL_FALSE, 0, access, format);
}

void Texture3DImpl::unbind_image(uint32_t unit) const {
    glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
}

CPPGL_NAMESPACE_END
