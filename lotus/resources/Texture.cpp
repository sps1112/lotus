#include "lotus/debug.h"
#include "lotus/resources.h"
#include "lotus/rendering.h"
#include "stb_image.h"

namespace Lotus::Resource
{
    Texture::Texture(const std::string& path_, const std::string& type_)
    {
        path = path_;
        type = type_;
    }

    int Texture::import()
    {
        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        unsigned int format;
        if (data)
        {
            if (nrComponents == 1)
            {
                format = RED;
            }
            else if (nrComponents == 3)
            {
                format = RGB;
            }
            else if (nrComponents == 4)
            {
                format = RGBA;
            }
            else
            {
                LOG_ERROR("Cannot load texture. Invalid image color format.");
                stbi_image_free(data);
                throw std::invalid_argument("image color format unknown");
            }
        }
        else
        {
            LOG_ERROR("Texture failed to load at path: {}", path);
            stbi_image_free(data);
            throw std::invalid_argument("image data invalid");
        }

        // TODO: Make API independent. Fire an event?
        Lotus::Rendering::GLRenderer& renderer = Lotus::Rendering::GLRenderer::get();
        unsigned int textureID = renderer.createTexture(data, width, height, format);

        id = textureID;

        AssetManager::registerAsset(path);
        return IMPORT_SUCCESS_CODE;
    }

    Cubemap::Cubemap(const std::vector<std::string>& paths)
    {
        glActiveTexture(GL_TEXTURE12);
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        int width, height, nrChannels;
        unsigned int i = 0;
        for (const std::string& texPath : paths)
        {
            unsigned char *data = stbi_load(texPath.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                unsigned int format = 0;
                if (nrChannels == 1)
                {
                    format = GL_RED;
                }
                else if (nrChannels == 3)
                {
                    format = GL_RGB;
                }
                else if (nrChannels == 4)
                {
                    format = GL_RGBA;
                }

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                LOG_ERROR("Cubemap tex failed to load at path: {}", texPath);
                stbi_image_free(data);
            }
            i++;
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glActiveTexture(GL_TEXTURE0);
    }

    int Cubemap::import() {
        return IMPORT_SUCCESS_CODE;
    }
}