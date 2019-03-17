#ifndef GFX_CONVERSION_H
#define GFX_CONVERSION_H

#include <vulkan/vulkan.h>

inline VkFormat ToVkFormat(GfxFormat format)
{
    switch (format)
    {
    case GFX_FORMAT_UNKNOWN:                            return VK_FORMAT_UNDEFINED;
    case GFX_FORMAT_R8_UNORM:                           return VK_FORMAT_R8_UNORM;
    case GFX_FORMAT_R8_SNORM:                           return VK_FORMAT_R8_SNORM;
    case GFX_FORMAT_R8_UINT:                            return VK_FORMAT_R8_UINT;
    case GFX_FORMAT_R8_SINT:                            return VK_FORMAT_R8_SINT;
    case GFX_FORMAT_R8_SRGB:                            return VK_FORMAT_R8_SRGB;
    case GFX_FORMAT_R8G8_UNORM:                         return VK_FORMAT_R8G8_UNORM;
    case GFX_FORMAT_R8G8_SNORM:                         return VK_FORMAT_R8G8_SNORM;
    case GFX_FORMAT_R8G8_UINT:                          return VK_FORMAT_R8G8_UINT;
    case GFX_FORMAT_R8G8_SINT:                          return VK_FORMAT_R8G8_SINT;
    case GFX_FORMAT_R8G8_SRGB:                          return VK_FORMAT_R8G8_SRGB;
    case GFX_FORMAT_R8G8B8_UNORM:                       return VK_FORMAT_R8G8B8_UNORM;
    case GFX_FORMAT_R8G8B8_SNORM:                       return VK_FORMAT_R8G8B8_SNORM;
    case GFX_FORMAT_R8G8B8_UINT:                        return VK_FORMAT_R8G8B8_UINT;
    case GFX_FORMAT_R8G8B8_SINT:                        return VK_FORMAT_R8G8B8_SINT;
    case GFX_FORMAT_R8G8B8_SRGB:                        return VK_FORMAT_R8G8B8_SRGB;
    case GFX_FORMAT_R8G8B8A8_UNORM:                     return VK_FORMAT_R8G8B8A8_UNORM;
    case GFX_FORMAT_R8G8B8A8_SNORM:                     return VK_FORMAT_R8G8B8A8_SNORM;
    case GFX_FORMAT_R8G8B8A8_UINT:                      return VK_FORMAT_R8G8B8A8_UINT;
    case GFX_FORMAT_R8G8B8A8_SINT:                      return VK_FORMAT_R8G8B8A8_SINT;
    case GFX_FORMAT_R8G8B8A8_SRGB:                      return VK_FORMAT_R8G8B8A8_SRGB;
    case GFX_FORMAT_R10G10B10A2_UNORM:                  return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case GFX_FORMAT_R10G10B10A2_SNORM:                  return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
    case GFX_FORMAT_R10G10B10A2_UINT:                   return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case GFX_FORMAT_R10G10B10A2_SINT:                   return VK_FORMAT_A2R10G10B10_SINT_PACK32;
    case GFX_FORMAT_R16_UNORM:                          return VK_FORMAT_R16_UNORM;
    case GFX_FORMAT_R16_SNORM:                          return VK_FORMAT_R16_SNORM;
    case GFX_FORMAT_R16_UINT:                           return VK_FORMAT_R16_UINT;
    case GFX_FORMAT_R16_SINT:                           return VK_FORMAT_R16_SINT;
    case GFX_FORMAT_R16_SFLOAT:                         return VK_FORMAT_R16_SFLOAT;
    case GFX_FORMAT_R16G16_UNORM:                       return VK_FORMAT_R16G16_UNORM;
    case GFX_FORMAT_R16G16_SNORM:                       return VK_FORMAT_R16G16_SNORM;
    case GFX_FORMAT_R16G16_UINT:                        return VK_FORMAT_R16G16_UINT;
    case GFX_FORMAT_R16G16_SINT:                        return VK_FORMAT_R16G16_SINT;
    case GFX_FORMAT_R16G16_SFLOAT:                      return VK_FORMAT_R16G16_SFLOAT;
    case GFX_FORMAT_R16G16B16_UNORM:                    return VK_FORMAT_R16G16B16_UNORM;
    case GFX_FORMAT_R16G16B16_SNORM:                    return VK_FORMAT_R16G16B16_SNORM;
    case GFX_FORMAT_R16G16B16_UINT:                     return VK_FORMAT_R16G16B16_UINT;
    case GFX_FORMAT_R16G16B16_SINT:                     return VK_FORMAT_R16G16B16_SINT;
    case GFX_FORMAT_R16G16B16_SFLOAT:                   return VK_FORMAT_R16G16B16_SFLOAT;
    case GFX_FORMAT_R16G16B16A16_UNORM:                 return VK_FORMAT_R16G16B16A16_UNORM;
    case GFX_FORMAT_R16G16B16A16_SNORM:                 return VK_FORMAT_R16G16B16A16_SNORM;
    case GFX_FORMAT_R16G16B16A16_UINT:                  return VK_FORMAT_R16G16B16A16_UINT;
    case GFX_FORMAT_R16G16B16A16_SINT:                  return VK_FORMAT_R16G16B16A16_SINT;
    case GFX_FORMAT_R16G16B16A16_SFLOAT:                return VK_FORMAT_R16G16B16A16_SFLOAT;
    case GFX_FORMAT_R32_UINT:                           return VK_FORMAT_R32_UINT;
    case GFX_FORMAT_R32_SINT:                           return VK_FORMAT_R32_SINT;
    case GFX_FORMAT_R32_SFLOAT:                         return VK_FORMAT_R32_SFLOAT;
    case GFX_FORMAT_R32G32_UINT:                        return VK_FORMAT_R32G32_UINT;
    case GFX_FORMAT_R32G32_SINT:                        return VK_FORMAT_R32G32_SINT;
    case GFX_FORMAT_R32G32_SFLOAT:                      return VK_FORMAT_R32G32_SFLOAT;
    case GFX_FORMAT_R32G32B32_UINT:                     return VK_FORMAT_R32G32B32_UINT;
    case GFX_FORMAT_R32G32B32_SINT:                     return VK_FORMAT_R32G32B32_SINT;
    case GFX_FORMAT_R32G32B32_SFLOAT:                   return VK_FORMAT_R32G32B32_SFLOAT;
    case GFX_FORMAT_R32G32B32A32_UINT:                  return VK_FORMAT_R32G32B32A32_UINT;
    case GFX_FORMAT_R32G32B32A32_SINT:                  return VK_FORMAT_R32G32B32A32_SINT;
    case GFX_FORMAT_R32G32B32A32_SFLOAT:                return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GFX_FORMAT_R11G11B10_UFLOAT:                   return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case GFX_FORMAT_D16_UNORM:                          return VK_FORMAT_D16_UNORM;
    case GFX_FORMAT_D32_SFLOAT:                         return VK_FORMAT_D32_SFLOAT;
    case GFX_FORMAT_D16_UNORM_S8_UINT:                  return VK_FORMAT_D16_UNORM_S8_UINT;
    case GFX_FORMAT_D32_SFLOAT_S8_UINT:                 return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    return VK_FORMAT_MAX_ENUM;
}
inline VkFormat ToVkFormat(const char* str)
{

         if (strcmp(str, "r8_unorm") == 0)              return VK_FORMAT_R8_UNORM;
    else if (strcmp(str, "r8_snorm") == 0)              return VK_FORMAT_R8_SNORM;
    else if (strcmp(str, "r8_uint") == 0)               return VK_FORMAT_R8_UINT;
    else if (strcmp(str, "r8_sint") == 0)               return VK_FORMAT_R8_SINT;
    else if (strcmp(str, "r8_srgb") == 0)               return VK_FORMAT_R8_SRGB;
    else if (strcmp(str, "r8g8_unorm") == 0)            return VK_FORMAT_R8G8_UNORM;
    else if (strcmp(str, "r8g8_snorm") == 0)            return VK_FORMAT_R8G8_SNORM;
    else if (strcmp(str, "r8g8_uint") == 0)             return VK_FORMAT_R8G8_UINT;
    else if (strcmp(str, "r8g8_sint") == 0)             return VK_FORMAT_R8G8_SINT;
    else if (strcmp(str, "r8g8_srgb") == 0)             return VK_FORMAT_R8G8_SRGB;
    else if (strcmp(str, "r8g8b8_unorm") == 0)          return VK_FORMAT_R8G8B8_UNORM;
    else if (strcmp(str, "r8g8b8_snorm") == 0)          return VK_FORMAT_R8G8B8_SNORM;
    else if (strcmp(str, "r8g8b8_uint") == 0)           return VK_FORMAT_R8G8B8_UINT;
    else if (strcmp(str, "r8g8b8_sint") == 0)           return VK_FORMAT_R8G8B8_SINT;
    else if (strcmp(str, "r8g8b8_srgb") == 0)           return VK_FORMAT_R8G8B8_SRGB;
    else if (strcmp(str, "r8g8b8a8_unorm") == 0)        return VK_FORMAT_R8G8B8A8_UNORM;
    else if (strcmp(str, "r8g8b8a8_snorm") == 0)        return VK_FORMAT_R8G8B8A8_SNORM;
    else if (strcmp(str, "r8g8b8a8_uint") == 0)         return VK_FORMAT_R8G8B8A8_UINT;
    else if (strcmp(str, "r8g8b8a8_sint") == 0)         return VK_FORMAT_R8G8B8A8_SINT;
    else if (strcmp(str, "r8g8b8a8_srgb") == 0)         return VK_FORMAT_R8G8B8A8_SRGB;
    else if (strcmp(str, "r10g10b10a2_unorm") == 0)     return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    else if (strcmp(str, "r10g10b10a2_snorm") == 0)     return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
    else if (strcmp(str, "r10g10b10a2_uint") == 0)      return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    else if (strcmp(str, "r10g10b10a2_sint") == 0)      return VK_FORMAT_A2R10G10B10_SINT_PACK32;
    else if (strcmp(str, "r16_unorm") == 0)             return VK_FORMAT_R16_UNORM;
    else if (strcmp(str, "r16_snorm") == 0)             return VK_FORMAT_R16_SNORM;
    else if (strcmp(str, "r16_uint") == 0)              return VK_FORMAT_R16_UINT;
    else if (strcmp(str, "r16_sint") == 0)              return VK_FORMAT_R16_SINT;
    else if (strcmp(str, "r16_sfloat") == 0)            return VK_FORMAT_R16_SFLOAT;
    else if (strcmp(str, "r16g16_unorm") == 0)          return VK_FORMAT_R16G16_UNORM;
    else if (strcmp(str, "r16g16_snorm") == 0)          return VK_FORMAT_R16G16_SNORM;
    else if (strcmp(str, "r16g16_uint") == 0)           return VK_FORMAT_R16G16_UINT;
    else if (strcmp(str, "r16g16_sint") == 0)           return VK_FORMAT_R16G16_SINT;
    else if (strcmp(str, "r16g16_sfloat") == 0)         return VK_FORMAT_R16G16_SFLOAT;
    else if (strcmp(str, "r16g16b16_unorm") == 0)       return VK_FORMAT_R16G16B16_UNORM;
    else if (strcmp(str, "r16g16b16_snorm") == 0)       return VK_FORMAT_R16G16B16_SNORM;
    else if (strcmp(str, "r16g16b16_uint") == 0)        return VK_FORMAT_R16G16B16_UINT;
    else if (strcmp(str, "r16g16b16_sint") == 0)        return VK_FORMAT_R16G16B16_SINT;
    else if (strcmp(str, "r16g16b16_sfloat") == 0)      return VK_FORMAT_R16G16B16_SFLOAT;
    else if (strcmp(str, "r16g16b16a16_unorm") == 0)    return VK_FORMAT_R16G16B16A16_UNORM;
    else if (strcmp(str, "r16g16b16a16_snorm") == 0)    return VK_FORMAT_R16G16B16A16_SNORM;
    else if (strcmp(str, "r16g16b16a16_uint") == 0)     return VK_FORMAT_R16G16B16A16_UINT;
    else if (strcmp(str, "r16g16b16a16_sint") == 0)     return VK_FORMAT_R16G16B16A16_SINT;
    else if (strcmp(str, "r16g16b16a16_sfloat") == 0)   return VK_FORMAT_R16G16B16A16_SFLOAT;
    else if (strcmp(str, "r32_uint") == 0)              return VK_FORMAT_R32_UINT;
    else if (strcmp(str, "r32_sint") == 0)              return VK_FORMAT_R32_SINT;
    else if (strcmp(str, "r32_sfloat") == 0)            return VK_FORMAT_R32_SFLOAT;
    else if (strcmp(str, "r32g32_uint") == 0)           return VK_FORMAT_R32G32_UINT;
    else if (strcmp(str, "r32g32_sint") == 0)           return VK_FORMAT_R32G32_SINT;
    else if (strcmp(str, "r32g32_sfloat") == 0)         return VK_FORMAT_R32G32_SFLOAT;
    else if (strcmp(str, "r32g32b32_uint") == 0)        return VK_FORMAT_R32G32B32_UINT;
    else if (strcmp(str, "r32g32b32_sint") == 0)        return VK_FORMAT_R32G32B32_SINT;
    else if (strcmp(str, "r32g32b32_sfloat") == 0)      return VK_FORMAT_R32G32B32_SFLOAT;
    else if (strcmp(str, "r32g32b32a32_uint") == 0)     return VK_FORMAT_R32G32B32A32_UINT;
    else if (strcmp(str, "r32g32b32a32_sint") == 0)     return VK_FORMAT_R32G32B32A32_SINT;
    else if (strcmp(str, "r32g32b32a32_sfloat") == 0)   return VK_FORMAT_R32G32B32A32_SFLOAT;
    else if (strcmp(str, "r11g11b10_ufloat") == 0)      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    else if (strcmp(str, "d16_unorm") == 0)             return VK_FORMAT_D16_UNORM;
    else if (strcmp(str, "d32_sfloat") == 0)            return VK_FORMAT_D32_SFLOAT;
    else if (strcmp(str, "d16_unorm_s8_uint") == 0)     return VK_FORMAT_D16_UNORM_S8_UINT;
    else if (strcmp(str, "d32_sfloat_s8_uint") == 0)    return VK_FORMAT_D32_SFLOAT_S8_UINT;
                                                        return VK_FORMAT_UNDEFINED;
}
inline uint32_t ToTexelStride(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
        return 1;
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
        return 2;
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return 3;
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        return 4;
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
        return 6;
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return 8;
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
        return 12;
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 16;
    }
    return 0;
}
inline const char* ToShaderFormatString(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_SFLOAT:
        return "float";
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_SFLOAT:
        return "vec2";
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return "vec3";
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return "vec4";
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R32_UINT:
        return "uint";
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R32G32_UINT:
        return "uvec2";
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R32G32B32_UINT:
        return "uvec3";
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R32G32B32A32_UINT:
        return "uvec4";
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R32_SINT:
        return "int";
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R32G32_SINT:
        return "ivec2";
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R32G32B32_SINT:
        return "ivec3";
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R32G32B32A32_SINT:
        return "ivec4";
    }
    return NULL;
}
inline VkImageAspectFlags ToVkImageAspectMask(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

inline VkImageLayout ToVkImageLayout(GfxTextureState state)
{
	switch (state)
	{
	case GFX_TEXTURE_STATE_SHADER_READ:                 return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case GFX_TEXTURE_STATE_SHADER_WRITE:                return VK_IMAGE_LAYOUT_GENERAL;
	case GFX_TEXTURE_STATE_COLOR_ATTACHMENT:            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case GFX_TEXTURE_STATE_DEPTH_ATTACHMENT:            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case GFX_TEXTURE_STATE_COPY_SRC:                    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case GFX_TEXTURE_STATE_COPY_DST:                    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case GFX_TEXTURE_STATE_PRESENT:                     return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	return VK_IMAGE_LAYOUT_MAX_ENUM;
}
inline VkAccessFlags ToVkAccessMask(GfxBufferAccess access)
{
    switch (access)
    {
        case GFX_BUFFER_ACCESS_NONE:                        return 0;
        case GFX_BUFFER_ACCESS_VERTEX_READ:                 return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        case GFX_BUFFER_ACCESS_INDEX_READ:                  return VK_ACCESS_INDEX_READ_BIT;
        case GFX_BUFFER_ACCESS_INDIRECT_READ:               return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        case GFX_BUFFER_ACCESS_UNIFORM_READ:                return VK_ACCESS_UNIFORM_READ_BIT;
        case GFX_BUFFER_ACCESS_SHADER_READ:                 return VK_ACCESS_SHADER_READ_BIT;
        case GFX_BUFFER_ACCESS_SHADER_WRITE:                return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        case GFX_BUFFER_ACCESS_COPY_SRC:                    return VK_ACCESS_TRANSFER_READ_BIT;
        case GFX_BUFFER_ACCESS_COPY_DST:                    return VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    return 0;
}
inline VkAccessFlags ToVkAccessMask(GfxTextureState state)
{
	switch (state)
	{
    case GFX_TEXTURE_STATE_SHADER_READ:                 return VK_ACCESS_SHADER_READ_BIT;
	case GFX_TEXTURE_STATE_SHADER_WRITE:                return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	case GFX_TEXTURE_STATE_COLOR_ATTACHMENT:            return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case GFX_TEXTURE_STATE_DEPTH_ATTACHMENT:            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case GFX_TEXTURE_STATE_COPY_SRC:                    return VK_ACCESS_TRANSFER_READ_BIT;
	case GFX_TEXTURE_STATE_COPY_DST:                    return VK_ACCESS_TRANSFER_WRITE_BIT;
    case GFX_TEXTURE_STATE_PRESENT:                     return VK_ACCESS_MEMORY_READ_BIT;
	}
	return 0;
}
inline VkPipelineStageFlags ToVkPipelineStageMask(GfxBufferAccess access)
{
    switch (access)
    {
        case GFX_BUFFER_ACCESS_NONE:                        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        case GFX_BUFFER_ACCESS_VERTEX_READ:                 return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        case GFX_BUFFER_ACCESS_INDEX_READ:                  return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        case GFX_BUFFER_ACCESS_INDIRECT_READ:               return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        case GFX_BUFFER_ACCESS_UNIFORM_READ:                return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case GFX_BUFFER_ACCESS_SHADER_READ:                 return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case GFX_BUFFER_ACCESS_SHADER_WRITE:                return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case GFX_BUFFER_ACCESS_COPY_SRC:                    return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case GFX_BUFFER_ACCESS_COPY_DST:                    return VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}
inline VkPipelineStageFlags ToVkPipelineStageMask(GfxTextureState state)
{
    switch (state)
    {
        case GFX_TEXTURE_STATE_SHADER_READ:             return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case GFX_TEXTURE_STATE_SHADER_WRITE:            return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case GFX_TEXTURE_STATE_COLOR_ATTACHMENT:        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case GFX_TEXTURE_STATE_DEPTH_ATTACHMENT:        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        case GFX_TEXTURE_STATE_COPY_SRC:                return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case GFX_TEXTURE_STATE_COPY_DST:                return VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}
inline VkImageType ToVkImageType(GfxTextureType type)
{
    switch (type)
    {
        case GFX_TEXTURE_TYPE_1D:                       return VK_IMAGE_TYPE_1D;
        case GFX_TEXTURE_TYPE_2D:                       return VK_IMAGE_TYPE_2D;
        case GFX_TEXTURE_TYPE_3D:                       return VK_IMAGE_TYPE_3D;
        case GFX_TEXTURE_TYPE_CUBE:                     return VK_IMAGE_TYPE_2D;
        case GFX_TEXTURE_TYPE_2D_ARRAY:                 return VK_IMAGE_TYPE_2D;
        case GFX_TEXTURE_TYPE_CUBE_ARRAY:               return VK_IMAGE_TYPE_2D;
    }
    return VK_IMAGE_TYPE_MAX_ENUM;
}
inline VkImageViewType ToVkImageViewType(GfxTextureType type)
{
    switch (type)
    {
        case GFX_TEXTURE_TYPE_1D:                       return VK_IMAGE_VIEW_TYPE_1D;
        case GFX_TEXTURE_TYPE_2D:                       return VK_IMAGE_VIEW_TYPE_2D;
        case GFX_TEXTURE_TYPE_3D:                       return VK_IMAGE_VIEW_TYPE_3D;
        case GFX_TEXTURE_TYPE_CUBE:                     return VK_IMAGE_VIEW_TYPE_CUBE;
        case GFX_TEXTURE_TYPE_2D_ARRAY:                 return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case GFX_TEXTURE_TYPE_CUBE_ARRAY:               return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    }
    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}
inline VkFilter ToVkFilter(GfxFilter filter)
{
	switch (filter)
	{
	case GFX_FILTER_NEAREST:                            return VK_FILTER_NEAREST;
	case GFX_FILTER_LINEAR:                             return VK_FILTER_LINEAR;
	}
	return VK_FILTER_MAX_ENUM;
}
inline VkSamplerMipmapMode ToVkSamplerMipmapMode(GfxFilter filter)
{
	switch (filter)
	{
	case GFX_FILTER_NEAREST:                            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case GFX_FILTER_LINEAR:                             return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
	return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}
inline VkSamplerAddressMode ToVkSamplerAddressMode(GfxAddressMode mode)
{
	switch (mode)
	{
	case GFX_ADDRESS_MODE_WRAP:                         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case GFX_ADDRESS_MODE_MIRROR:                       return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case GFX_ADDRESS_MODE_CLAMP:                        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case GFX_ADDRESS_MODE_BORDER:                       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
	return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}
inline VkCompareOp ToVkCompareOp(GfxCompareOp op)
{
	switch (op)
	{
	case GFX_COMPARE_OP_NEVER:                          return VK_COMPARE_OP_NEVER;
	case GFX_COMPARE_OP_EQUAL:                          return VK_COMPARE_OP_EQUAL;
	case GFX_COMPARE_OP_NOT_EQUAL:                      return VK_COMPARE_OP_NOT_EQUAL;
	case GFX_COMPARE_OP_LESS:                           return VK_COMPARE_OP_LESS;
	case GFX_COMPARE_OP_LESS_EQUAL:                     return VK_COMPARE_OP_LESS_OR_EQUAL;
	case GFX_COMPARE_OP_GREATER:                        return VK_COMPARE_OP_GREATER;
	case GFX_COMPARE_OP_GREATER_EQUAL:                  return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case GFX_COMPARE_OP_ALWAYS:                         return VK_COMPARE_OP_ALWAYS;
	}
	return VK_COMPARE_OP_MAX_ENUM;
}
inline VkBorderColor ToVkBorderColor(GfxBorderColor color)
{
	switch (color)
	{
	case GFX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:      return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	case GFX_BORDER_COLOR_FLOAT_OPAQUE_BLACK:           return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	case GFX_BORDER_COLOR_FLOAT_OPAQUE_WHITE:           return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	case GFX_BORDER_COLOR_INT_TRANSPARENT_BLACK:        return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
	case GFX_BORDER_COLOR_INT_OPAQUE_BLACK:             return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	case GFX_BORDER_COLOR_INT_OPAQUE_WHITE:             return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
	return VK_BORDER_COLOR_MAX_ENUM;
}

inline VkBufferUsageFlags ToVkBufferUsageMask(uint32_t usage)
{
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	flags |= (usage & GFX_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0 ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
	flags |= (usage & GFX_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0 ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
	flags |= (usage & GFX_BUFFER_USAGE_INDEX_BUFFER_BIT) != 0 ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
	flags |= (usage & GFX_BUFFER_USAGE_VERTEX_BUFFER_BIT) != 0 ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
	flags |= (usage & GFX_BUFFER_USAGE_INDIRECT_BUFFER_BIT) != 0 ? VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT : 0;
	return flags;
}
inline VkImageUsageFlags ToVkImageUsageMask(uint32_t usage)
{
	VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	flags |= (usage & GFX_TEXTURE_USAGE_SAMPLE_BIT) != 0 ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
	flags |= (usage & GFX_TEXTURE_USAGE_STORE_BIT) != 0 ? VK_IMAGE_USAGE_STORAGE_BIT : 0;
	flags |= (usage & GFX_TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) != 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
	flags |= (usage & GFX_TEXTURE_USAGE_DEPTH_ATTACHMENT_BIT) != 0 ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0;
	return flags;
}

inline VkDescriptorType ToVkDescriptorType(const char* str)
{
         if (strcmp(str, "sampler") == 0)               return VK_DESCRIPTOR_TYPE_SAMPLER;
    else if (strcmp(str, "texture1d") == 0)             return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    else if (strcmp(str, "texture2d") == 0)             return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    else if (strcmp(str, "texture3d") == 0)             return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    else if (strcmp(str, "image1d") == 0)               return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    else if (strcmp(str, "image2d") == 0)               return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    else if (strcmp(str, "image3d") == 0)               return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    else if (strcmp(str, "cbuffer") == 0)               return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    else if (strcmp(str, "buffer") == 0)                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                                                        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}
inline const char* ToShaderDescriptorString(const char* str)
{
         if (strcmp(str, "sampler") == 0)               return "uniform sampler";
    else if (strcmp(str, "texture1d") == 0)             return "uniform texture1D";
    else if (strcmp(str, "texture2d") == 0)             return "uniform texture2D";
    else if (strcmp(str, "texture3d") == 0)             return "uniform texture3D";
    else if (strcmp(str, "image1d") == 0)               return "uniform image1D";
    else if (strcmp(str, "image2d") == 0)               return "uniform image2D";
    else if (strcmp(str, "image3d") == 0)               return "uniform image3D";
    else if (strcmp(str, "cbuffer") == 0)               return "uniform";
    else if (strcmp(str, "buffer") == 0)                return "buffer";
                                                        return NULL;
}
inline VkVertexInputRate ToVkVertexInputRate(const char* str)
{
         if (strcmp(str, "vertex") == 0)                return VK_VERTEX_INPUT_RATE_VERTEX;
    else if (strcmp(str, "instance") == 0)              return VK_VERTEX_INPUT_RATE_INSTANCE;
                                                        return VK_VERTEX_INPUT_RATE_MAX_ENUM;
}
inline VkPrimitiveTopology ToVkPrimitiveTopology(const char* str)
{
         if (strcmp(str, "point_list") == 0)            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    else if (strcmp(str, "line_list") == 0)             return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    else if (strcmp(str, "line_strip") == 0)            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    else if (strcmp(str, "triangle_list") == 0)         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    else if (strcmp(str, "triangle_strip") == 0)        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    else if (strcmp(str, "triangle_fan") == 0)          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
                                                        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}
inline VkPolygonMode ToVkPolygonMode(const char* str)
{
         if (strcmp(str, "fill") == 0)                  return VK_POLYGON_MODE_FILL;
    else if (strcmp(str, "line") == 0)                  return VK_POLYGON_MODE_LINE;
    else if (strcmp(str, "point") == 0)                 return VK_POLYGON_MODE_POINT;
                                                        return VK_POLYGON_MODE_MAX_ENUM;
}
inline VkCullModeFlags ToVkCullMode(const char* str)
{
         if (strcmp(str, "none") == 0)                  return VK_CULL_MODE_NONE;
    else if (strcmp(str, "front") == 0)                 return VK_CULL_MODE_FRONT_BIT;
    else if (strcmp(str, "back") == 0)                  return VK_CULL_MODE_BACK_BIT;
                                                        return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}
inline VkFrontFace ToVkFrontFace(const char* str)
{
         if (strcmp(str, "counter_clockwise") == 0)     return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    else if (strcmp(str, "clockwise") == 0)             return VK_FRONT_FACE_CLOCKWISE;
                                                        return VK_FRONT_FACE_MAX_ENUM;
}
inline VkCompareOp ToVkCompareOp(const char* str)
{
         if (strcmp(str, "never") == 0)                 return VK_COMPARE_OP_NEVER;
    else if (strcmp(str, "less") == 0)                  return VK_COMPARE_OP_LESS;
    else if (strcmp(str, "equal") == 0)                 return VK_COMPARE_OP_EQUAL;
    else if (strcmp(str, "less_equal") == 0)            return VK_COMPARE_OP_LESS_OR_EQUAL;
    else if (strcmp(str, "greater") == 0)               return VK_COMPARE_OP_GREATER;
    else if (strcmp(str, "not_equal") == 0)             return VK_COMPARE_OP_NOT_EQUAL;
    else if (strcmp(str, "greater_equal") == 0)         return VK_COMPARE_OP_GREATER_OR_EQUAL;
    else if (strcmp(str, "always") == 0)                return VK_COMPARE_OP_ALWAYS;
                                                        return VK_COMPARE_OP_MAX_ENUM;
}
inline VkStencilOp ToVkStencilOp(const char* str)
{
         if (strcmp(str, "keep") == 0)                  return VK_STENCIL_OP_KEEP;
    else if (strcmp(str, "zero") == 0)                  return VK_STENCIL_OP_ZERO;
    else if (strcmp(str, "replace") == 0)               return VK_STENCIL_OP_REPLACE;
    else if (strcmp(str, "increment_and_clamp") == 0)   return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    else if (strcmp(str, "decrement_and_clamp") == 0)   return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    else if (strcmp(str, "invert") == 0)                return VK_STENCIL_OP_INVERT;
    else if (strcmp(str, "increment_and_wrap") == 0)    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    else if (strcmp(str, "decrement_and_wrap") == 0)    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                                                        return VK_STENCIL_OP_MAX_ENUM;
}
inline VkBlendFactor ToVkBlendFactor(const char* str)
{
         if (strcmp(str, "zero") == 0)                  return VK_BLEND_FACTOR_ZERO;
    else if (strcmp(str, "one") == 0)                   return VK_BLEND_FACTOR_ONE;
    else if (strcmp(str, "src_color") == 0)             return VK_BLEND_FACTOR_SRC_COLOR;
    else if (strcmp(str, "inv_src_color") == 0)         return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    else if (strcmp(str, "dst_color") == 0)             return VK_BLEND_FACTOR_DST_COLOR;
    else if (strcmp(str, "inv_dst_color") == 0)         return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    else if (strcmp(str, "src_alpha") == 0)             return VK_BLEND_FACTOR_SRC_ALPHA;
    else if (strcmp(str, "inv_src_alpha") == 0)         return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    else if (strcmp(str, "dst_alpha") == 0)             return VK_BLEND_FACTOR_DST_ALPHA;
    else if (strcmp(str, "inv_dst_alpha") == 0)         return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                                                        return VK_BLEND_FACTOR_MAX_ENUM;
}
inline VkBlendOp ToVkBlendOp(const char* str)
{
         if (strcmp(str, "add") == 0)                   return VK_BLEND_OP_ADD;
    else if (strcmp(str, "subtract") == 0)              return VK_BLEND_OP_SUBTRACT;
    else if (strcmp(str, "rev_subtract") == 0)          return VK_BLEND_OP_REVERSE_SUBTRACT;
    else if (strcmp(str, "min") == 0)                   return VK_BLEND_OP_MIN;
    else if (strcmp(str, "max") == 0)                   return VK_BLEND_OP_MAX;
                                                        return VK_BLEND_OP_MAX_ENUM;
}
inline VkColorComponentFlags ToVkColorComponentMask(const char* str)
{
    VkColorComponentFlags mask = 0;
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        mask |= str[i] == 'r' ? VK_COLOR_COMPONENT_R_BIT : 0;
        mask |= str[i] == 'g' ? VK_COLOR_COMPONENT_G_BIT : 0;
        mask |= str[i] == 'b' ? VK_COLOR_COMPONENT_B_BIT : 0;
        mask |= str[i] == 'a' ? VK_COLOR_COMPONENT_A_BIT : 0;
    }
    return mask;
}

#endif
