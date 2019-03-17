#ifndef GFX_INTERNAL_H
#define GFX_INTERNAL_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <Windows.h>
#else
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xutil.h>
#endif

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "../include/Gfx.h"

#include "GfxUtil.h"
#include "GfxConversion.h"

typedef void(*GfxCmdFunction)(VkCommandBuffer, void*);

struct GfxBuffer_T
{
    VkBuffer						    m_Buffer;
    VmaAllocation					    m_Allocation;
    VkDeviceSize					    m_Size;
};

struct GfxTexture_T
{
    VkImage							    m_Image;
    VkImageView						    m_ImageView;
    VmaAllocation					    m_Allocation;
    uint32_t						    m_Width;
    uint32_t						    m_Height;
    uint32_t                            m_Depth;
    VkFormat						    m_Format;
};

struct GfxCommandBuffer_T
{
    VkCommandBuffer                     m_CommandBuffer;
    VkFence                             m_CommandBufferFence;
    VkSemaphore                         m_CommandBufferSemaphore;
    VkSemaphore                         m_PresentSemaphore;

    VkDeviceSize                        m_StagingBufferTail;

    GfxDevice                           m_Device;
    GfxTechnique                        m_Technique;
    GfxRenderSetup                      m_RenderSetup;

    VkDescriptorPool				    m_DescriptorPool;
    Array<VkWriteDescriptorSet>      m_DescriptorWrites;
    Array<VkDescriptorBufferInfo>    m_DescriptorBufferInfo;
    Array<VkDescriptorImageInfo>     m_DescriptorImageInfo;
    bool                                m_IsDescriptorSetDirty;
};

struct GfxDevice_T
{
	VkInstance						    m_Instance;

	VkSurfaceKHR					    m_Surface;

	VkPhysicalDevice				    m_PhysicalDevice;
	VkDevice						    m_Device;

	VkQueue							    m_GraphicsQueue;
	uint32_t						    m_GraphicsQueueIndex;

	VkSwapchainKHR					    m_Swapchain;
	VkExtent2D						    m_SwapchainImageExtent;
	uint32_t						    m_SwapchainImageCount;
	uint32_t						    m_SwapchainImageIndex;
    Array<GfxTexture_T>              m_SwapchainTextures;
	VkSurfaceFormatKHR				    m_SwapchainSurfaceFormat;

	VkCommandPool					    m_CommandPool;

	VmaAllocator					    m_Allocator;

    GfxBuffer_T                         m_StagingBuffer;
	VkDeviceMemory					    m_StagingBufferMemory;
	VkDeviceSize					    m_StagingBufferMemoryOffset;
	uint8_t*						    m_StagingBufferMappedData;
	VkDeviceSize					    m_StagingBufferHead;

    uint32_t						    m_CommandBufferIndexCurr;
    uint32_t						    m_CommandBufferIndexNext;
    Array<GfxCommandBuffer_T>        m_CommandBuffers;

    Array<GfxCmdFunction>            m_CmdFunctions;
    Array<uint8_t>                   m_CmdFunctionUserData;
    Array<uint32_t>                  m_CmdFunctionUserDataOffsets;

    struct TechniqueEntry
    {
        GfxTechnique                    m_Technique;
        String                       m_Filepath;
        uint64_t                        m_Checksum;
    };
    HashTable<TechniqueEntry>        m_TechniqueEntries;

#if defined(_DEBUG) || !defined(NDEBUG)
	VkDebugReportCallbackEXT		    m_DebugCallback;
#endif

    GfxDevice_T()
        : m_TechniqueEntries(1024)
    {
    }
};

struct GfxTechniqueBlob_T
{
    VkPipelineBindPoint                 m_BindPoint                     = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    uint32_t						    m_ShaderBindingCount            = 0;
    struct
    {
        uint64_t					    m_Hash                          = 0;
        VkDescriptorType			    m_Type                          = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    } m_ShaderBindings[16];
};
struct GfxGraphicsTechniqueBlob_T : public GfxTechniqueBlob_T
{
	uint32_t						    m_ColorAttachmentCount			= 0;
	VkFormat						    m_ColorAttachmentFormats[4];
	VkFormat						    m_DepthAttachmentFormat			= VK_FORMAT_UNDEFINED;
	uint32_t						    m_VertexAttributeCount			= 0;
    struct
    {
        uint32_t					    m_Binding                       = ~0U;
        VkFormat					    m_Format                        = VK_FORMAT_UNDEFINED;
        uint32_t					    m_Offset                        = ~0U;
        VkVertexInputRate			    m_InputRate                     = VK_VERTEX_INPUT_RATE_MAX_ENUM;
    } m_VertexAttributes[8];
	struct
	{
		VkPrimitiveTopology		        m_Topology						= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bool						    m_PrimitiveRestartEnable		= false;
	} m_InputAssembly;
	struct
	{
		bool						    m_DepthClampEnable				= false;
		bool						    m_RasterizerDiscardEnable		= false;
		VkPolygonMode				    m_PolygonMode					= VK_POLYGON_MODE_FILL;
        VkCullModeFlags				    m_CullMode						= VK_CULL_MODE_NONE;
		VkFrontFace				        m_FrontFace						= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		bool						    m_DepthBiasEnable				= false;
		float						    m_DepthBiasConstantFactor		= 0.0f;
		float						    m_DepthBiasClamp				= 0.0f;
		float						    m_DepthBiasSlopeFactor			= 0.0f;
		float						    m_LineWidth						= 1.0f;
	} m_RasterizerState;
	struct
	{
		bool						    m_DepthTestEnable				= false;
		bool						    m_DepthWriteEnable				= false;
        VkCompareOp				        m_DepthCompareOp				= VK_COMPARE_OP_LESS_OR_EQUAL;
		bool						    m_DepthBoundsTestEnable			= false;
		bool						    m_StencilTestEnable				= false;
		VkStencilOpState			    m_Front							= { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 };
		VkStencilOpState			    m_Back							= { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 };
		float						    m_MinDepthBounds				= 0.0f;
		float						    m_MaxDepthBounds				= 0.0f;
	} m_DepthStencilState;
	struct
	{
		bool						    m_BlendEnable					= false;
		VkBlendFactor				    m_SrcColorBlendFactor			= VK_BLEND_FACTOR_ZERO;
		VkBlendFactor				    m_DstColorBlendFactor			= VK_BLEND_FACTOR_ZERO;
		VkBlendOp					    m_ColorBlendOp					= VK_BLEND_OP_ADD;
		VkBlendFactor				    m_SrcAlphaBlendFactor			= VK_BLEND_FACTOR_ZERO;
		VkBlendFactor				    m_DstAlphaBlendFactor			= VK_BLEND_FACTOR_ZERO;
		VkBlendOp					    m_AlphaBlendOp					= VK_BLEND_OP_ADD;
		VkColorComponentFlags		    m_ColorWriteMask				= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	} m_BlendAttachments[ARRAY_COUNT(GfxGraphicsTechniqueBlob_T::m_ColorAttachmentFormats)];
};

struct GfxTechnique_T
{
    VkPipelineBindPoint                 m_BindPoint;
    VkPipeline						    m_Pipeline;
    VkPipelineLayout				    m_PipelineLayout;
    VkDescriptorSetLayout			    m_DescriptorSetLayout;
    VkRenderPass					    m_RenderPass;

    struct ShaderBinding
    {
        uint32_t                        m_Binding;
        VkDescriptorType                m_Type;
    };
    HashTable<ShaderBinding>	        m_ShaderBindings;
    uint32_t                            m_ShaderBindingCount;

#if defined(_DEBUG) || !defined(NDEBUG)
    GfxRenderSetup                      m_RenderSetupHead;
    GfxRenderSetup                      m_RenderSetupTail;
#endif

    GfxTechnique_T()
        : m_ShaderBindings(ARRAY_COUNT(GfxTechniqueBlob_T::m_ShaderBindings))
    {
    }
};

struct GfxRenderSetup_T
{
    VkFramebuffer					    m_Framebuffer;
    VkExtent2D						    m_Extent;

#if defined(_DEBUG) || !defined(NDEBUG)
    Array<VkImageView>               m_ImageViews;
    GfxTechnique                        m_Technique;
    GfxRenderSetup                      m_Next;
    GfxRenderSetup                      m_Prev;
#endif
};

struct GfxModel_T
{
    struct Mesh
    {
        uint32_t                        m_MaterialIndex;
        uint32_t                        m_VertexOffset;
        uint32_t                        m_VertexCount;
        uint32_t                        m_IndexOffset;
        uint32_t                        m_IndexCount;
    };
    Array<Mesh>                      m_Meshes;

    struct Material
    {
        int32_t                         m_DiffuseTextureIndex;
    };
    Array<Material>                  m_Materials;
    Array<GfxTexture>                m_Textures;

    GfxBuffer                           m_VertexBuffer;
    GfxBuffer                           m_IndexBuffer;

    VkDeviceSize                        m_VertexBufferOffsets[GFX_MODEL_VERTEX_ATTRIBUTE_COUNT];

    float                               m_BoundingBoxMin[3];
    float                               m_BoundingBoxMax[3];

    float                               m_QuantizationScale;
};

#endif
