#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <float.h>

typedef struct GfxDevice_T*			GfxDevice;
typedef struct GfxCommandBuffer_T*  GfxCommandBuffer;
typedef struct GfxBuffer_T*			GfxBuffer;
typedef struct GfxTexture_T*		GfxTexture;
typedef struct GfxTechnique_T*		GfxTechnique;
typedef struct GfxRenderSetup_T*    GfxRenderSetup;
typedef struct GfxModel_T*          GfxModel;
typedef uint64_t					GfxSampler;

enum GfxFormat
{
	GFX_FORMAT_UNKNOWN = 0,
    GFX_FORMAT_R8_UNORM,
    GFX_FORMAT_R8_SNORM,
    GFX_FORMAT_R8_UINT,
    GFX_FORMAT_R8_SINT,
    GFX_FORMAT_R8_SRGB,
    GFX_FORMAT_R8G8_UNORM,
    GFX_FORMAT_R8G8_SNORM,
    GFX_FORMAT_R8G8_UINT,
    GFX_FORMAT_R8G8_SINT,
    GFX_FORMAT_R8G8_SRGB,
    GFX_FORMAT_R8G8B8_UNORM,
    GFX_FORMAT_R8G8B8_SNORM,
    GFX_FORMAT_R8G8B8_UINT,
    GFX_FORMAT_R8G8B8_SINT,
    GFX_FORMAT_R8G8B8_SRGB,
    GFX_FORMAT_R8G8B8A8_UNORM,
    GFX_FORMAT_R8G8B8A8_SNORM,
    GFX_FORMAT_R8G8B8A8_UINT,
    GFX_FORMAT_R8G8B8A8_SINT,
    GFX_FORMAT_R8G8B8A8_SRGB,
    GFX_FORMAT_R10G10B10A2_UNORM,
    GFX_FORMAT_R10G10B10A2_SNORM,
    GFX_FORMAT_R10G10B10A2_UINT,
    GFX_FORMAT_R10G10B10A2_SINT,
    GFX_FORMAT_R16_UNORM,
    GFX_FORMAT_R16_SNORM,
    GFX_FORMAT_R16_UINT,
    GFX_FORMAT_R16_SINT,
    GFX_FORMAT_R16_SFLOAT,
    GFX_FORMAT_R16G16_UNORM,
    GFX_FORMAT_R16G16_SNORM,
    GFX_FORMAT_R16G16_UINT,
    GFX_FORMAT_R16G16_SINT,
    GFX_FORMAT_R16G16_SFLOAT,
    GFX_FORMAT_R16G16B16_UNORM,
    GFX_FORMAT_R16G16B16_SNORM,
    GFX_FORMAT_R16G16B16_UINT,
    GFX_FORMAT_R16G16B16_SINT,
    GFX_FORMAT_R16G16B16_SFLOAT,
    GFX_FORMAT_R16G16B16A16_UNORM,
    GFX_FORMAT_R16G16B16A16_SNORM,
    GFX_FORMAT_R16G16B16A16_UINT,
    GFX_FORMAT_R16G16B16A16_SINT,
    GFX_FORMAT_R16G16B16A16_SFLOAT,
    GFX_FORMAT_R32_UINT,
    GFX_FORMAT_R32_SINT,
    GFX_FORMAT_R32_SFLOAT,
    GFX_FORMAT_R32G32_UINT,
    GFX_FORMAT_R32G32_SINT,
    GFX_FORMAT_R32G32_SFLOAT,
    GFX_FORMAT_R32G32B32_UINT,
    GFX_FORMAT_R32G32B32_SINT,
    GFX_FORMAT_R32G32B32_SFLOAT,
    GFX_FORMAT_R32G32B32A32_UINT,
    GFX_FORMAT_R32G32B32A32_SINT,
    GFX_FORMAT_R32G32B32A32_SFLOAT,
    GFX_FORMAT_R11G11B10_UFLOAT,
    GFX_FORMAT_D16_UNORM,
    GFX_FORMAT_D32_SFLOAT,
    GFX_FORMAT_D16_UNORM_S8_UINT,
    GFX_FORMAT_D32_SFLOAT_S8_UINT,
};
enum GfxBufferAccess
{
    GFX_BUFFER_ACCESS_NONE = 0,
    GFX_BUFFER_ACCESS_VERTEX_READ,
    GFX_BUFFER_ACCESS_INDEX_READ,
    GFX_BUFFER_ACCESS_INDIRECT_READ,
    GFX_BUFFER_ACCESS_UNIFORM_READ,
    GFX_BUFFER_ACCESS_SHADER_READ,
    GFX_BUFFER_ACCESS_SHADER_WRITE,
    GFX_BUFFER_ACCESS_COPY_SRC,
    GFX_BUFFER_ACCESS_COPY_DST,
};
enum GfxTextureType
{
    GFX_TEXTURE_TYPE_1D,
    GFX_TEXTURE_TYPE_2D,
    GFX_TEXTURE_TYPE_3D,
    GFX_TEXTURE_TYPE_CUBE,
    GFX_TEXTURE_TYPE_2D_ARRAY,
    GFX_TEXTURE_TYPE_CUBE_ARRAY,
};
enum GfxTextureState
{
	GFX_TEXTURE_STATE_SHADER_READ = 0,
    GFX_TEXTURE_STATE_SHADER_WRITE,
	GFX_TEXTURE_STATE_COLOR_ATTACHMENT,
	GFX_TEXTURE_STATE_DEPTH_ATTACHMENT,
	GFX_TEXTURE_STATE_COPY_SRC,
	GFX_TEXTURE_STATE_COPY_DST,
    GFX_TEXTURE_STATE_PRESENT,
};
enum GfxFilter
{
	GFX_FILTER_NEAREST = 0,
	GFX_FILTER_LINEAR,
};
enum GfxAddressMode
{
	GFX_ADDRESS_MODE_WRAP = 0,
	GFX_ADDRESS_MODE_MIRROR,
	GFX_ADDRESS_MODE_CLAMP,
	GFX_ADDRESS_MODE_BORDER,
};
enum GfxCompareOp
{
	GFX_COMPARE_OP_NEVER = 0,
	GFX_COMPARE_OP_EQUAL,
	GFX_COMPARE_OP_NOT_EQUAL,
	GFX_COMPARE_OP_LESS,
	GFX_COMPARE_OP_LESS_EQUAL,
	GFX_COMPARE_OP_GREATER,
	GFX_COMPARE_OP_GREATER_EQUAL,
	GFX_COMPARE_OP_ALWAYS,
};
enum GfxBorderColor
{
	GFX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = 0,
	GFX_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	GFX_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	GFX_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	GFX_BORDER_COLOR_INT_OPAQUE_BLACK,
	GFX_BORDER_COLOR_INT_OPAQUE_WHITE,
};
enum GfxModelVertexAttribute
{
    GFX_MODEL_VERTEX_ATTRIBUTE_POSITION = 0,         // RGBM16_UNORM
    GFX_MODEL_VERTEX_ATTRIBUTE_TEXCOORD,             // RG16_SFLOAT
    GFX_MODEL_VERTEX_ATTRIBUTE_NORMAL,               // RGBA8_UNORM
    GFX_MODEL_VERTEX_ATTRIBUTE_COUNT
};
enum GfxBufferUsageBits : uint32_t
{
	GFX_BUFFER_USAGE_UNIFORM_BUFFER_BIT				= 1 << 0,
	GFX_BUFFER_USAGE_STORAGE_BUFFER_BIT				= 1 << 1,
	GFX_BUFFER_USAGE_INDEX_BUFFER_BIT				= 1 << 2,
	GFX_BUFFER_USAGE_VERTEX_BUFFER_BIT				= 1 << 3,
	GFX_BUFFER_USAGE_INDIRECT_BUFFER_BIT			= 1 << 4,
};
enum GfxTextureUsageBits : uint32_t
{
	GFX_TEXTURE_USAGE_SAMPLE_BIT					= 1 << 0,
	GFX_TEXTURE_USAGE_STORE_BIT						= 1 << 1,
	GFX_TEXTURE_USAGE_COLOR_ATTACHMENT_BIT			= 1 << 2,
	GFX_TEXTURE_USAGE_DEPTH_ATTACHMENT_BIT	        = 1 << 3,
};

#ifdef _WIN32
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __attribute__((visibility("default")))
#endif

struct GfxCreateDeviceParams
{
	void*						m_WindowHandle;             // Platform specific
	uint32_t					m_BackBufferWidth;
	uint32_t					m_BackBufferHeight;
	uint32_t					m_DesiredBackBufferCount;
    bool                        m_EnableValidationLayer;
};
LIB_EXPORT GfxDevice			GfxCreateDevice(const GfxCreateDeviceParams& params);
LIB_EXPORT void					GfxDestroyDevice(GfxDevice device);
LIB_EXPORT void					GfxResizeSwapchain(GfxDevice device, uint32_t width, uint32_t height);
LIB_EXPORT void                 GfxWaitForGpu(GfxDevice device);

LIB_EXPORT uint32_t             GfxGetBackBufferCount(GfxDevice device);
LIB_EXPORT uint32_t             GfxGetBackBufferIndex(GfxDevice device);
LIB_EXPORT GfxTexture           GfxGetBackBuffer(GfxDevice device, uint32_t index);

struct GfxAllocation
{
    GfxBuffer                   m_Buffer;
    uint64_t                    m_Offset;
    uint8_t*                    m_Data;
};
LIB_EXPORT GfxAllocation        GfxAllocateUploadBuffer(GfxDevice device, size_t size);

LIB_EXPORT GfxCommandBuffer		GfxBeginFrame(GfxDevice device);
LIB_EXPORT void					GfxEndFrame(GfxDevice device);


struct GfxCreateBufferParams
{
	uint64_t					m_Size						= 0;
	uint32_t					m_Usage						= 0;
	const void*					m_Data						= NULL;
};
LIB_EXPORT GfxBuffer			GfxCreateBuffer(GfxDevice device, const GfxCreateBufferParams& params);
LIB_EXPORT void					GfxDestroyBuffer(GfxDevice device, GfxBuffer buffer);


struct GfxCreateTextureParams
{
    GfxTextureType              m_Type                      = GFX_TEXTURE_TYPE_2D;
	uint32_t					m_Width						= 0;
	uint32_t					m_Height					= 1;
    uint32_t                    m_Depth                     = 1;
	GfxFormat					m_Format					= GFX_FORMAT_UNKNOWN;
	uint32_t					m_Usage						= 0;
	GfxTextureState				m_InitialState				= GFX_TEXTURE_STATE_SHADER_READ;
	const void*					m_Data						= NULL;
	size_t						m_DataSize					= 0;
    bool                        m_GenerateMipmaps           = false;
};
LIB_EXPORT GfxTexture			GfxCreateTexture(GfxDevice device, const GfxCreateTextureParams& params);
LIB_EXPORT GfxTexture           GfxLoadTexture(GfxDevice device, const char* filepath);
LIB_EXPORT void					GfxDestroyTexture(GfxDevice device, GfxTexture texture);


struct GfxCreateSamplerParams
{
	GfxFilter					m_MagFilter					= GFX_FILTER_LINEAR;
	GfxFilter					m_MinFilter					= GFX_FILTER_LINEAR;
	GfxFilter					m_MipFilter					= GFX_FILTER_LINEAR;
	GfxAddressMode				m_AddressModeU				= GFX_ADDRESS_MODE_CLAMP;
	GfxAddressMode				m_AddressModeV				= GFX_ADDRESS_MODE_CLAMP;
	GfxAddressMode				m_AddressModeW				= GFX_ADDRESS_MODE_CLAMP;
	float						m_MipLodBias				= 0.0f;
	bool						m_AnisotropyEnable			= false;
	float						m_MaxAnisotropy				= 1.0f;
	bool						m_CompareEnable				= false;
	GfxCompareOp				m_CompareOp					= GFX_COMPARE_OP_NEVER;
	float						m_MinLod					= -FLT_MAX;
	float						m_MaxLod					= FLT_MAX;
	GfxBorderColor				m_BorderColor				= GFX_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
};
LIB_EXPORT GfxSampler			GfxCreateSampler(GfxDevice device, const GfxCreateSamplerParams& params);
LIB_EXPORT void					GfxDestroySampler(GfxDevice device, GfxSampler sampler);


LIB_EXPORT GfxTechnique			GfxCreateTechnique(GfxDevice device, const void* data, size_t size, GfxTechnique old_tech = NULL);
LIB_EXPORT GfxTechnique         GfxLoadTechnique(GfxDevice device, const char* filepath);
LIB_EXPORT void					GfxDestroyTechnique(GfxDevice device, GfxTechnique tech);
#ifdef _DEBUG
LIB_EXPORT void                 GfxReloadAllTechniques(GfxDevice device);
#endif

struct GfxCreateRenderSetupParams
{
    uint32_t                    m_ColorAttachmentCount      = 0;
    const GfxTexture*           m_ColorAttachments          = NULL;
    GfxTexture                  m_DepthAttachment           = NULL;
};
LIB_EXPORT GfxRenderSetup       GfxCreateRenderSetup(GfxDevice device, GfxTechnique tech, const GfxCreateRenderSetupParams& params);
LIB_EXPORT void                 GfxDestroyRenderSetup(GfxDevice device, GfxRenderSetup setup);


LIB_EXPORT GfxModel             GfxLoadModel(GfxDevice device, const char* filepath, const char* material_dir);
LIB_EXPORT void                 GfxDestroyModel(GfxDevice device, GfxModel model);

LIB_EXPORT uint32_t             GfxGetModelMeshCount(GfxModel model);
LIB_EXPORT uint32_t             GfxGetModelMaterialIndex(GfxModel model, uint32_t mesh_index);
LIB_EXPORT GfxTexture           GfxGetModelDiffuseTexture(GfxModel model, uint32_t material_index);
LIB_EXPORT const float*         GfxGetModelBoundingBoxMin(GfxModel model);
LIB_EXPORT const float*         GfxGetModelBoundingBoxMax(GfxModel model);
LIB_EXPORT float                GfxGetModelQuantizationScale(GfxModel model);

LIB_EXPORT void                 GfxCmdBindModelVertexBuffer(GfxCommandBuffer cmd, GfxModel model, GfxModelVertexAttribute attribute, uint32_t binding);
LIB_EXPORT void                 GfxCmdBindModelIndexBuffer(GfxCommandBuffer cmd, GfxModel model);
LIB_EXPORT void                 GfxCmdDrawModel(GfxCommandBuffer cmd, GfxModel model, uint32_t mesh_index, uint32_t instance_count);


LIB_EXPORT void                 GfxCmdBeginTechnique(GfxCommandBuffer cmd, GfxTechnique tech);
LIB_EXPORT void                 GfxCmdEndTechnique(GfxCommandBuffer cmd);

LIB_EXPORT void                 GfxCmdSetRenderSetup(GfxCommandBuffer cmd, GfxRenderSetup setup);
LIB_EXPORT void                 GfxCmdSetViewport(GfxCommandBuffer cmd, float x, float y, float w, float h, float min_z, float max_z);
LIB_EXPORT void                 GfxCmdSetScissor(GfxCommandBuffer cmd, int32_t x, int32_t y, uint32_t w, uint32_t h);
LIB_EXPORT void                 GfxCmdClearColor(GfxCommandBuffer cmd, uint32_t attachment, const float color[4]);
LIB_EXPORT void                 GfxCmdClearDepth(GfxCommandBuffer cmd, float depth, uint32_t stencil);

LIB_EXPORT void                 GfxCmdBindVertexBuffer(GfxCommandBuffer cmd, uint32_t binding, GfxBuffer buffer, uint64_t offset);
LIB_EXPORT void                 GfxCmdBindIndexBuffer(GfxCommandBuffer cmd, GfxBuffer buffer, uint64_t offset, uint32_t stride);

#define GFX_HASH(str) GfxHash(str, sizeof(str) - 1)
constexpr uint64_t GfxHash(const char* str, size_t len)
{
    // http://www.eecs.harvard.edu/margo/papers/usenix91/paper.ps
    uint64_t hash = 0;
    for (size_t i = 0; i < len; ++i)
        hash = str[i] + (hash << 6) + (hash << 16) - hash;
    return hash;
}

LIB_EXPORT void                 GfxCmdSetBuffer(GfxCommandBuffer cmd, uint64_t hash, GfxBuffer buffer, uint64_t offset, uint64_t size);
LIB_EXPORT void                 GfxCmdSetTexture(GfxCommandBuffer cmd, uint64_t hash, GfxTexture texture, GfxTextureState state);
LIB_EXPORT void                 GfxCmdSetSampler(GfxCommandBuffer cmd, uint64_t hash, GfxSampler sampler);
LIB_EXPORT void*                GfxCmdAllocUploadBuffer(GfxCommandBuffer cmd, uint64_t hash, uint32_t size);

LIB_EXPORT void                 GfxCmdDraw(GfxCommandBuffer cmd, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex);
LIB_EXPORT void                 GfxCmdDrawIndexed(GfxCommandBuffer cmd, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset);
LIB_EXPORT void                 GfxCmdDispatch(GfxCommandBuffer cmd, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

LIB_EXPORT void                 GfxCmdCopyBuffer(GfxCommandBuffer cmd, GfxBuffer dst_buffer, uint64_t dst_offset, GfxBuffer src_buffer, uint64_t src_offset, uint64_t size);
LIB_EXPORT void                 GfxCmdBlitTexture(GfxCommandBuffer cmd, GfxTexture dst_texture, GfxTexture src_texture);

LIB_EXPORT void                 GfxCmdTransitionBuffer(GfxCommandBuffer cmd, GfxBuffer buffer, GfxBufferAccess old_access, GfxBufferAccess new_access, uint64_t offset = 0, uint64_t size = ~0ULL);
LIB_EXPORT void                 GfxCmdTransitionTexture(GfxCommandBuffer cmd, GfxTexture texture, GfxTextureState old_state, GfxTextureState new_state, uint32_t base_mip = 0, uint32_t mip_count = ~0U, uint32_t base_layer = 0, uint32_t layer_count = ~0U);

#endif
